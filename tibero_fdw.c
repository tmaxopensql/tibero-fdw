/*-------------------------------------------------------------------------
 *
 * tibero_fdw.c
 *		  Foreign-data wrapper for remote Tibero servers
 *
 * Portions Copyright (c) 2022-2023, HyperSQL Research & Development Team
 *
 * IDENTIFICATION
 *		  contrib/tibero_fdw/tibero_fdw.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <limits.h>

#include "access/htup_details.h"
#include "access/sysattr.h"
#include "access/table.h"
#include "access/xact.h"                /* IsolationUsesXactSnapshot          */
#include "catalog/pg_class.h"
#include "catalog/pg_opfamily.h"
#include "commands/defrem.h"
#include "commands/explain.h"
#include "commands/vacuum.h"
#include "executor/execAsync.h"
#include "foreign/fdwapi.h"
#include "funcapi.h"
#include "miscadmin.h"
#include "nodes/makefuncs.h"
#include "nodes/nodeFuncs.h"
#include "optimizer/appendinfo.h"
#include "optimizer/clauses.h"
#include "optimizer/cost.h"
#include "optimizer/optimizer.h"
#include "optimizer/pathnode.h"
#include "optimizer/paths.h"
#include "optimizer/planmain.h"
#include "optimizer/prep.h"
#include "optimizer/restrictinfo.h"
#include "optimizer/tlist.h"
#include "parser/parsetree.h"
#include "storage/latch.h"
#include "utils/builtins.h"
#include "utils/float.h"
#include "utils/guc.h"
#include "utils/lsyscache.h"
#include "utils/memutils.h"
#include "utils/rel.h"
#include "utils/sampling.h"
#include "utils/selfuncs.h"
#include "utils/syscache.h"						  /* TYPEOID                            */

#include "tibero_fdw.h"
#include "connection.h"

PG_MODULE_MAGIC;

#define DEFAULT_FDW_STARTUP_COST	100.0
#define DEFAULT_FDW_TUPLE_COST		0.01
#define DEFAULT_FDW_FETCH_SIZE    100
#define TB_MAXLEN_SQLID_WITH_NULL 129
#define TB_FDW_INIT_TUPLE_CNT -1

enum FdwScanPrivateIndex
{
	FdwScanPrivateSelectSql,
	FdwScanPrivateRetrievedAttrs,
	FdwScanPrivateFetchSize,
	FdwScanPrivateUseFbQuery
};

enum FdwPathPrivateIndex
{
	FdwPathPrivateHasFinalSort,
	FdwPathPrivateHasLimit
};

typedef struct TbColumn
{
	SQLCHAR col_name[TB_MAXLEN_SQLID_WITH_NULL];
	SQLSMALLINT col_name_len;
	SQLSMALLINT data_type;
	SQLUINTEGER col_size;
	SQLSMALLINT scale;
	SQLSMALLINT nullable;
	unsigned char	*data;
	SQLLEN  *ind;
} TbColumn;

typedef struct TbTable
{
	TbColumn **column;
} TbTable;

typedef struct TbFdwScanState
{
	Relation rel;
	TupleDesc tupdesc;
	AttInMetadata *attinmeta;

	unsigned char *query;
	List *retrieved_attrs;

	MemoryContext batch_ctx;
	MemoryContext temp_ctx;

	HeapTuple  *tuples;
	uint64 fetch_size;
	int tuple_cnt;
	int cur_tuple_idx;
	bool end_of_fetch;

	TbStatement *tbStmt;
	TbTable *table;

	bool use_fb_query;
} TbFdwScanState;

PG_FUNCTION_INFO_V1(tibero_fdw_handler);

/* {{{ FDW callback routines **************************************************/
static void tiberoGetForeignRelSize(PlannerInfo *root, RelOptInfo *baserel,
									  								Oid foreigntableid);
static void tiberoGetForeignPaths(PlannerInfo *root, RelOptInfo *baserel,
																	Oid foreigntableid);
static ForeignScan *tiberoGetForeignPlan(PlannerInfo *root, RelOptInfo *foreignrel,
																			   Oid foreigntableid, ForeignPath *best_path,
																				 List *tlist, List *scan_clauses,
																				 Plan *outer_plan);
static void tiberoBeginForeignScan(ForeignScanState *node, int eflags);
static TupleTableSlot *tiberoIterateForeignScan(ForeignScanState *node);
static void tiberoReScanForeignScan(ForeignScanState *node);
static void tiberoEndForeignScan(ForeignScanState *node);
static void tiberoGetForeignJoinPaths(PlannerInfo *root, RelOptInfo *joinrel,
																			RelOptInfo *outerrel, RelOptInfo *innerrel,
																			JoinType jointype, JoinPathExtraData *extra);
/************************************************** FDW callback routines }}} */

/* {{{ Helper functions *******************************************************/
static bool foreign_join_ok(PlannerInfo *root, RelOptInfo *joinrel, JoinType jointype,
														RelOptInfo *outerrel, RelOptInfo *innerrel,
														JoinPathExtraData *extra);
/******************************************************* Helper functions }}} */

Datum
tibero_fdw_handler(PG_FUNCTION_ARGS)
{
	FdwRoutine *routine = makeNode(FdwRoutine);

	/* Functions for scanning foreign tables */
	routine->GetForeignRelSize = tiberoGetForeignRelSize;
	routine->GetForeignPaths = tiberoGetForeignPaths;
	routine->GetForeignPlan = tiberoGetForeignPlan;
	routine->BeginForeignScan = tiberoBeginForeignScan;
	routine->IterateForeignScan = tiberoIterateForeignScan;
	routine->ReScanForeignScan = tiberoReScanForeignScan;
	routine->EndForeignScan = tiberoEndForeignScan;

	/* Support functions for join push-down */
	routine->GetForeignJoinPaths = tiberoGetForeignJoinPaths;

	PG_RETURN_POINTER(routine);
}

static void
apply_server_options(TbFdwRelationInfo *fpinfo)
{
	ListCell   *lc;

	foreach(lc, fpinfo->server->options) {
		DefElem    *def = (DefElem *) lfirst(lc);

		if (strcmp(def->defname, "use_remote_estimate") == 0)
			fpinfo->use_remote_estimate = false;
		else if (strcmp(def->defname, "fdw_startup_cost") == 0)
			(void) parse_real(defGetString(def), &fpinfo->fdw_startup_cost, 0,
							  NULL);
		else if (strcmp(def->defname, "fdw_tuple_cost") == 0)
			(void) parse_real(defGetString(def), &fpinfo->fdw_tuple_cost, 0,
							  NULL);
		else if (strcmp(def->defname, "extensions") == 0)
			fpinfo->shippable_extensions = NIL;
		else if (strcmp(def->defname, "fetch_size") == 0)
			(void) parse_int(defGetString(def), &fpinfo->fetch_size, 0, NULL);
		else if (strcmp(def->defname, "use_fb_query") == 0)
			fpinfo->use_fb_query = defGetBoolean(def);
		else if (strcmp(def->defname, "async_capable") == 0) {
			/* TODO */
		}
	}
}

static void
apply_table_options(TbFdwRelationInfo *fpinfo)
{
	ListCell *lc;

	foreach(lc, fpinfo->table->options) {
		DefElem *def = (DefElem *) lfirst(lc);

		if (strcmp(def->defname, "use_remote_estimate") == 0)
			fpinfo->use_remote_estimate = false;
		else if (strcmp(def->defname, "fetch_size") == 0)
			(void) parse_int(defGetString(def), &fpinfo->fetch_size, 0, NULL);
		else if (strcmp(def->defname, "async_capable") == 0) {
			/* TODO */
		}
	}
}

static void
tiberoGetForeignRelSize(PlannerInfo *root, RelOptInfo *baserel,
												Oid foreigntableid)
{
  TbFdwRelationInfo *fpinfo;
	ListCell *lc;

	fpinfo = (TbFdwRelationInfo *) palloc0(sizeof(TbFdwRelationInfo));
	baserel->fdw_private = (void *) fpinfo;

	fpinfo->pushdown_safe = true;

	fpinfo->table = GetForeignTable(foreigntableid);
	fpinfo->server = GetForeignServer(fpinfo->table->serverid);

	fpinfo->use_remote_estimate = false;

	fpinfo->fdw_startup_cost = DEFAULT_FDW_STARTUP_COST;
	fpinfo->fdw_tuple_cost = DEFAULT_FDW_TUPLE_COST;
	fpinfo->shippable_extensions = NIL;
	fpinfo->fetch_size = DEFAULT_FDW_FETCH_SIZE;

	fpinfo->use_fb_query = false;

	apply_server_options(fpinfo);
	apply_table_options(fpinfo);

	if (fpinfo->use_remote_estimate) {
		/* TODO */
	} else {
		fpinfo->user = NULL;
	}

	classify_conditions(root, baserel, baserel->baserestrictinfo,
											&fpinfo->remote_conds, &fpinfo->local_conds);

	fpinfo->attrs_used = NULL;
	pull_varattnos((Node *) baserel->reltarget->exprs, baserel->relid,
								 &fpinfo->attrs_used);
	foreach(lc, fpinfo->local_conds) {
		RestrictInfo *rinfo = lfirst_node(RestrictInfo, lc);
		pull_varattnos((Node *) rinfo->clause, baserel->relid,
									 &fpinfo->attrs_used);
	}

	fpinfo->local_conds_sel = clauselist_selectivity(root, fpinfo->local_conds,
																									 baserel->relid, JOIN_INNER,
																									 NULL);
	cost_qual_eval(&fpinfo->local_conds_cost, fpinfo->local_conds, root);

	if (fpinfo->use_remote_estimate) {
		/* TODO */
	} else {
		if (baserel->tuples < 0) {
			baserel->pages = 10;
			baserel->tuples =
				(10 * BLCKSZ) /
				(baserel->reltarget->width + MAXALIGN(SizeofHeapTupleHeader));
		}

		set_baserel_size_estimates(root, baserel);
	}

	fpinfo->relation_name = psprintf("%u", baserel->relid);
}

static void
tiberoGetForeignPaths(PlannerInfo *root, RelOptInfo *baserel,
											Oid foreigntableid)
{
	TbFdwRelationInfo *fpinfo = (TbFdwRelationInfo *) baserel->fdw_private;
	add_path(baserel,
					 (Path *) create_foreignscan_path(root, baserel, NULL, fpinfo->rows,
																						fpinfo->startup_cost,
																						fpinfo->total_cost, NIL,
																						baserel->lateral_relids, NULL,
																						NIL));

	if (!fpinfo->use_remote_estimate) return;
}

static ForeignScan *
tiberoGetForeignPlan(PlannerInfo *root,
					   RelOptInfo *foreignrel,
					   Oid foreigntableid,
					   ForeignPath *best_path,
					   List *tlist,
					   List *scan_clauses,
					   Plan *outer_plan)
{
	TbFdwRelationInfo *fpinfo = (TbFdwRelationInfo *) foreignrel->fdw_private;
	Index scan_relid;
	List *fdw_private;
	List *remote_exprs = NIL;
	List *local_exprs = NIL;
	List *params_list = NIL;
	List *fdw_scan_tlist = NIL;
	List *fdw_recheck_quals = NIL;
	List *retrieved_attrs;
	StringInfoData sql;
	bool has_final_sort = false;
	bool has_limit = false;
	ListCell *lc;

	if (best_path->fdw_private) {
		has_final_sort = intVal(list_nth(best_path->fdw_private,
																		 FdwPathPrivateHasFinalSort));
		has_limit = intVal(list_nth(best_path->fdw_private,
																FdwPathPrivateHasLimit));
	}

	if (IS_SIMPLE_REL(foreignrel)) {
		scan_relid = foreignrel->relid;

		foreach(lc, scan_clauses) {
			RestrictInfo *rinfo = lfirst_node(RestrictInfo, lc);

			if (rinfo->pseudoconstant)
				continue;

			if (list_member_ptr(fpinfo->remote_conds, rinfo))
				remote_exprs = lappend(remote_exprs, rinfo->clause);
			else if (list_member_ptr(fpinfo->local_conds, rinfo))
				local_exprs = lappend(local_exprs, rinfo->clause);
			else if (is_foreign_expr(root, foreignrel, rinfo->clause))
				remote_exprs = lappend(remote_exprs, rinfo->clause);
			else
				local_exprs = lappend(local_exprs, rinfo->clause);
		}

		fdw_recheck_quals = remote_exprs;
	} else {
		scan_relid = 0;
		Assert(false);
	}

	initStringInfo(&sql);
	deparse_select_stmt_for_rel(&sql, root, foreignrel, fdw_scan_tlist,
															remote_exprs, best_path->path.pathkeys,
															has_final_sort, has_limit, false,
															&retrieved_attrs, &params_list, fpinfo->use_fb_query);

	fdw_private = list_make4(makeString(sql.data),
													 retrieved_attrs,
													 makeInteger(fpinfo->fetch_size),
													 makeInteger(fpinfo->use_fb_query));

	Assert(IS_SIMPLE_REL(foreignrel));

  return make_foreignscan(tlist, local_exprs, scan_relid, params_list,
													fdw_private, fdw_scan_tlist, fdw_recheck_quals,
													outer_plan);
}

static void
tiberoBeginForeignScan(ForeignScanState *node, int eflags)
{
	ForeignScan *fsplan = (ForeignScan *) node->ss.ps.plan;
	EState *estate = node->ss.ps.state;
	TbFdwScanState *fsstate;
	RangeTblEntry *rte = NULL;
	Oid userid;
	ForeignTable *table;
	UserMapping *user;
	int rtindex;
	int i;
	SQLLEN ind = 0;

	if (eflags & EXEC_FLAG_EXPLAIN_ONLY)
		return;

	fsstate = (TbFdwScanState *) palloc0(sizeof(TbFdwScanState));
	node->fdw_state = (void *) fsstate;

	/* Join/Aggregation case */
	Assert(fsplan->scan.scanrelid > 0);
	rtindex = fsplan->scan.scanrelid;
	rte = exec_rt_fetch(rtindex, estate);
	userid = rte->checkAsUser ? rte->checkAsUser : GetUserId();

	table = GetForeignTable(rte->relid);
	user = GetUserMapping(userid, table->serverid);

	fsstate->query = (unsigned char *) strVal(list_nth(fsplan->fdw_private,
																   				  				 FdwScanPrivateSelectSql));
	fsstate->retrieved_attrs = (List *) list_nth(fsplan->fdw_private,
												 											 FdwScanPrivateRetrievedAttrs);
	fsstate->fetch_size = intVal(list_nth(fsplan->fdw_private,
										  								  FdwScanPrivateFetchSize));
	fsstate->use_fb_query = intVal(list_nth(fsplan->fdw_private,
																					FdwScanPrivateUseFbQuery));

	fsstate->tuple_cnt = TB_FDW_INIT_TUPLE_CNT;
	fsstate->cur_tuple_idx = 0;
	fsstate->end_of_fetch = false;

	fsstate->batch_ctx = AllocSetContextCreate(estate->es_query_cxt,
																						 "tibero_fdw tuple data",
																						 ALLOCSET_DEFAULT_SIZES);
	fsstate->temp_ctx = AllocSetContextCreate(estate->es_query_cxt,
																						"tibero_fdw temporary data",
																						ALLOCSET_SMALL_SIZES);

	fsstate->rel = node->ss.ss_currentRelation;
	fsstate->tupdesc = RelationGetDescr(fsstate->rel);

	fsstate->attinmeta = TupleDescGetAttInMetadata(fsstate->tupdesc);

	fsstate->table = (TbTable *) palloc0(sizeof(TbTable));
	fsstate->table->column = (TbColumn **) palloc0(sizeof(TbColumn *) * 
																								 fsstate->tupdesc->natts);
	for (i = 0; i < fsstate->tupdesc->natts; i++) {
		fsstate->table->column[i] = (TbColumn *) palloc0(sizeof(TbColumn));
	}

	fsstate->tbStmt = (TbStatement *) palloc0(sizeof(TbStatement));
	GetTbStatement(user, fsstate->tbStmt, fsstate->use_fb_query);

	TbSQLPrepare(fsstate->tbStmt, (SQLCHAR *)fsstate->query, SQL_NTS);
	TbSQLNumResultCols(fsstate->tbStmt, &fsstate->tbStmt->res_col_cnt);

	for (i = 0; i < fsstate->tbStmt->res_col_cnt; i++) {
		TbColumn *col = fsstate->table->column[i];
		TbSQLDescribeCol(fsstate->tbStmt, (SQLSMALLINT)i + 1, col->col_name,
										 sizeof(col->col_name), &col->col_name_len, &col->data_type,
										 &col->col_size, &col->scale, &col->nullable);

		col->data = palloc0(sizeof(unsigned char) * col->col_size * fsstate->fetch_size);
		col->ind = palloc0(sizeof(SQLLEN) * fsstate->fetch_size);
	}

	if (fsstate->use_fb_query && !IsolationUsesXactSnapshot()) {
		ind = SQL_NTS;
		TbSQLBindParameter(fsstate->tbStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
											 SQL_NUMERIC, 0, 0, fsstate->tbStmt->conn->tsn,
											 sizeof(fsstate->tbStmt->conn->tsn), &ind);
	}

	TbSQLSetStmtAttr(fsstate->tbStmt, SQL_ATTR_ROW_ARRAY_SIZE, 
									 (SQLPOINTER)fsstate->fetch_size, 0);
  TbSQLSetStmtAttr(fsstate->tbStmt, SQL_ATTR_ROWS_FETCHED_PTR, 
									 (SQLPOINTER)&fsstate->tuple_cnt, 0);
	TbSQLExecute(fsstate->tbStmt);

	for (i = 0; i < fsstate->tbStmt->res_col_cnt; i++) {
		TbColumn *col = fsstate->table->column[i];
		TbSQLBindCol(fsstate->tbStmt, i + 1, SQL_C_CHAR, (SQLCHAR *)col->data,
								 col->col_size, col->ind);
	}
}

static Datum
tibero_convert_to_pg(Oid pgtyp, int pgtypmod, TbColumn *column, int tuple_idx)
{
	Datum		value_datum;
	Datum		valueDatum;
	regproc		typeinput;
	HeapTuple	tuple;

	tuple = SearchSysCache1(TYPEOID, ObjectIdGetDatum(pgtyp));
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "cache lookup failed for type%u", pgtyp);

	typeinput = ((Form_pg_type) GETSTRUCT(tuple))->typinput;
	ReleaseSysCache(tuple);

	valueDatum = CStringGetDatum((char *) &column->data[tuple_idx * column->col_size]);
	value_datum = OidFunctionCall3(typeinput, valueDatum, ObjectIdGetDatum(pgtyp),
																 Int32GetDatum(pgtypmod));

	return value_datum;
}

static bool
NeedToFetch(TbFdwScanState *fsstate)
{
	return !fsstate->end_of_fetch &&
				 (fsstate->tuple_cnt == TB_FDW_INIT_TUPLE_CNT ||
				  fsstate->cur_tuple_idx == fsstate->tuple_cnt);
}

static TupleTableSlot *
GetNextTuple(ForeignScanState *node)
{
	TbFdwScanState *fsstate = (TbFdwScanState *) node->fdw_state;
	TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;

	if (fsstate->end_of_fetch) {
		return ExecClearTuple(slot);
	} else {
		Assert(fsstate->cur_tuple_idx < fsstate->tuple_cnt);
		return ExecStoreHeapTuple(fsstate->tuples[fsstate->cur_tuple_idx++], slot, false);
	}
}

static void
MakeTuples(ForeignScanState *node)
{
	TbFdwScanState *fsstate = (TbFdwScanState *) node->fdw_state;
	TupleTableSlot *tupleSlot = node->ss.ss_ScanTupleSlot;
	int attid;
	ListCell *lc;
	Datum *dvalues;
	bool *nulls;
	int natts;
	AttInMetadata *attinmeta = fsstate->attinmeta;
	int i, j;
	MemoryContext oldcontext;

	natts = attinmeta->tupdesc->natts;
	fsstate->tuples = NULL;

	MemoryContextReset(fsstate->batch_ctx);
	oldcontext = MemoryContextSwitchTo(fsstate->batch_ctx);

	fsstate->tuples = (HeapTuple *) palloc0(fsstate->tuple_cnt * sizeof(HeapTuple));
	dvalues = palloc0(natts * sizeof(Datum));
	nulls = palloc(natts * sizeof(bool));
	memset(nulls, true, natts * sizeof(bool));
	ExecClearTuple(tupleSlot);

	for (i = 0; i < fsstate->tuple_cnt; i++) {
		attid = 0;
		foreach(lc, fsstate->retrieved_attrs) {
			int			attnum = lfirst_int(lc) - 1;
			Oid			pgtype = TupleDescAttr(attinmeta->tupdesc, attnum)->atttypid;
			int32		pgtypmod = TupleDescAttr(attinmeta->tupdesc, attnum)->atttypmod;

			nulls[attnum] = false;
			dvalues[attnum] = tibero_convert_to_pg(pgtype, pgtypmod,
																						 fsstate->table->column[attid++], i);
		}
		fsstate->tuples[i] = heap_form_tuple(attinmeta->tupdesc, dvalues, nulls);

		for (j = 0; j < natts; j++) {
			if (dvalues[j] && !TupleDescAttr(attinmeta->tupdesc, j)->attbyval)
				pfree(DatumGetPointer(dvalues[j]));
		}
	}

	MemoryContextSwitchTo(oldcontext);
}

static void
FetchTuples(ForeignScanState *node)
{
	TbFdwScanState *fsstate = (TbFdwScanState *) node->fdw_state;
	TbSQLFetch(fsstate->tbStmt, &fsstate->cur_tuple_idx, &fsstate->end_of_fetch);
	if (!fsstate->end_of_fetch) MakeTuples(node);
}

static TupleTableSlot *
tiberoIterateForeignScan(ForeignScanState *node)
{
	TbFdwScanState *fsstate = (TbFdwScanState *) node->fdw_state;
	if (NeedToFetch(fsstate)) FetchTuples(node);
	return GetNextTuple(node);
}

static void
tiberoReScanForeignScan(ForeignScanState *node)
{
	/* TODO */
}

static void
tiberoEndForeignScan(ForeignScanState *node)
{
	TbFdwScanState *fsstate = (TbFdwScanState *) node->fdw_state;

	/* When called for EXPLAIN */
	if (fsstate == NULL) return;

	TbSQLFreeStmt(fsstate->tbStmt, SQL_DROP);
}

static bool
foreign_join_ok(PlannerInfo *root, RelOptInfo *joinrel, JoinType jointype,
								RelOptInfo *outerrel, RelOptInfo *innerrel,
								JoinPathExtraData *extra)
{
	/* TODO Implementation */
	return false;
}

static void
tiberoGetForeignJoinPaths(PlannerInfo *root,
							RelOptInfo *joinrel,
							RelOptInfo *outerrel,
							RelOptInfo *innerrel,
							JoinType jointype,
							JoinPathExtraData *extra)
{
	if (!foreign_join_ok(root, joinrel, jointype, outerrel, innerrel, extra)) return;
}




