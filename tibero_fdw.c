/*--------------------------------------------------------------------------------------------------
 *
 * tibero_fdw.c
 *			Foreign-data wrapper for remote Tibero servers
 *
 * Portions Copyright (c) 2022-2023, Tmax OpenSQL Research & Development Team
 *
 * IDENTIFICATION
 *			contrib/tibero_fdw/tibero_fdw.c
 *
 *--------------------------------------------------------------------------------------------------
 */
#include "postgres.h"

#include <limits.h>

#include "access/htup_details.h"
#include "access/sysattr.h"
#include "access/table.h"
#include "access/xact.h"													/* IsolationUsesXactSnapshot										*/
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
#include "utils/syscache.h"												/* TYPEOID																			*/

#include "tibero_fdw.h"
#include "connection.h"

/* {{{ global variables ***************************************************************************/
extern bool is_signal_handlers_registered;
/*************************************************************************** global variables }}} */

PG_MODULE_MAGIC;

#define DEFAULT_FDW_STARTUP_COST	100.0
#define DEFAULT_FDW_TUPLE_COST		0.01
#define DEFAULT_FDW_FETCH_SIZE		100
#define TB_MAXLEN_SQLID_WITH_NULL 129
#define TB_FDW_INIT_TUPLE_CNT			-1

enum FdwScanPrivateIndex
{
	FdwScanPrivateSelectSql,
	FdwScanPrivateRetrievedAttrs,
	FdwScanPrivateFetchSize,
	FdwScanPrivateUseFbQuery,
	FdwScanPrivateRelations
};

enum FdwPathPrivateIndex
{
	FdwPathPrivateHasFinalSort,
	FdwPathPrivateHasLimit
};

enum FdwModifyPrivateIndex
{
	TbFdwModifyQuery,
	TbFdwModifyTargetAttrs
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
	SQLLEN	*ind;
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

	HeapTuple	*tuples;
	uint64 fetch_size;
	int tuple_cnt;
	int cur_tuple_idx;
	bool end_of_fetch;

	TbStatement *tbStmt;
	TbTable *table;

	bool use_fb_query;
} TbFdwScanState;

typedef struct TbFdwExecState
{
	char *query;
	List *target_attrs;

	int p_nums;
	FmgrInfo *p_flinfo;

	MemoryContext temp_ctx;
	TbStatement *tbStmt;
} TbFdwExecState;

PG_FUNCTION_INFO_V1(tibero_fdw_handler);

/* {{{ FDW callback routines **********************************************************************/
static void tiberoGetForeignRelSize(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid);
static void tiberoGetForeignPaths(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid);
static ForeignScan *tiberoGetForeignPlan(PlannerInfo *root, RelOptInfo *foreignrel,
																				 Oid foreigntableid, ForeignPath *best_path, List *tlist,
																				 List *scan_clauses, Plan *outer_plan);
static void tiberoBeginForeignScan(ForeignScanState *node, int eflags);
static TupleTableSlot *tiberoIterateForeignScan(ForeignScanState *node);
static void tiberoReScanForeignScan(ForeignScanState *node);
static void tiberoEndForeignScan(ForeignScanState *node);
static void tiberoGetForeignJoinPaths(PlannerInfo *root, RelOptInfo *joinrel, RelOptInfo *outerrel,
																			RelOptInfo *innerrel, JoinType jointype,
																			JoinPathExtraData *extra);
static void tiberoExplainForeignScan(ForeignScanState *node, ExplainState *ex);
static int tiberoIsForeignRelUpdatable(Relation rel);
static List *tiberoPlanForeignModify(PlannerInfo *root, ModifyTable *plan, Index resultRelation,
																		 int subplan_index);
static void tiberoBeginForeignModify(ModifyTableState *mtstate, ResultRelInfo *resultRelInfo,
																		 List *fdw_private, int subplan_index, int eflags);
static TupleTableSlot *tiberoExecForeignInsert(EState *estate, ResultRelInfo *resultRelInfo,
																							 TupleTableSlot *slot, TupleTableSlot *planSlot);
static void tiberoEndForeignModify(EState *estate, ResultRelInfo *resultRelInfo);
/********************************************************************** FDW callback routines }}} */

/* {{{ Helper functions ***************************************************************************/
static bool foreign_join_ok(PlannerInfo *root, RelOptInfo *joinrel, JoinType jointype,
														RelOptInfo *outerrel, RelOptInfo *innerrel, JoinPathExtraData *extra);
static inline bool foreign_scan_has_upper_rels(List *fdw_private);
static inline StringInfo get_foreign_scan_upper_rel_names(ForeignScan *plan, ExplainState *es);
/*************************************************************************** Helper functions }}} */

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

	/* Support functions for EXPLAIN */
	routine->ExplainForeignScan = tiberoExplainForeignScan;

	/* Support functions for updating foreign tables */
	routine->IsForeignRelUpdatable = tiberoIsForeignRelUpdatable;
	routine->PlanForeignModify = tiberoPlanForeignModify;
	routine->BeginForeignModify = tiberoBeginForeignModify;
	routine->ExecForeignInsert = tiberoExecForeignInsert;
	routine->EndForeignModify = tiberoEndForeignModify;

	PG_RETURN_POINTER(routine);
}

static void
apply_server_options(TbFdwRelationInfo *fpinfo)
{
	ListCell *lc;

	foreach(lc, fpinfo->server->options) {
		DefElem *def = (DefElem *) lfirst(lc);

		if (strcmp(def->defname, "use_remote_estimate") == 0)
			fpinfo->use_remote_estimate = false;
		else if (strcmp(def->defname, "fdw_startup_cost") == 0)
			(void) parse_real(defGetString(def), &fpinfo->fdw_startup_cost, 0, NULL);
		else if (strcmp(def->defname, "fdw_tuple_cost") == 0)
			(void) parse_real(defGetString(def), &fpinfo->fdw_tuple_cost, 0, NULL);
		else if (strcmp(def->defname, "fetch_size") == 0)
			(void) parse_int(defGetString(def), &fpinfo->fetch_size, 0, NULL);
		else if (strcmp(def->defname, "use_fb_query") == 0)
			fpinfo->use_fb_query = defGetBoolean(def);
		else if (strcmp(def->defname, "use_sleep_on_sig") == 0)
			fpinfo->use_sleep_on_sig = defGetBoolean(def);
		else if (strcmp(def->defname, "updatable") == 0)
			fpinfo->updatable = defGetBoolean(def);
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
		else if (strcmp(def->defname, "updatable") == 0)
			fpinfo->updatable = defGetBoolean(def);
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

	set_sleep_on_sig_on();

	fpinfo = (TbFdwRelationInfo *) palloc0(sizeof(TbFdwRelationInfo));
	baserel->fdw_private = (void *) fpinfo;

	fpinfo->pushdown_safe = true;

	fpinfo->table = GetForeignTable(foreigntableid);
	fpinfo->server = GetForeignServer(fpinfo->table->serverid);

	fpinfo->use_remote_estimate = false;

	fpinfo->fdw_startup_cost = DEFAULT_FDW_STARTUP_COST;
	fpinfo->fdw_tuple_cost = DEFAULT_FDW_TUPLE_COST;
	fpinfo->fetch_size = DEFAULT_FDW_FETCH_SIZE;

	fpinfo->use_fb_query = false;
	fpinfo->use_sleep_on_sig = false;
	fpinfo->updatable = false;

	apply_server_options(fpinfo);
	apply_table_options(fpinfo);

	if (fpinfo->use_sleep_on_sig && !is_signal_handlers_registered) {
		register_signal_handlers();
	}

	if (fpinfo->use_remote_estimate) {
		/* TODO */
	} else {
		fpinfo->user = NULL;
	}

	classify_conditions(root, baserel, baserel->baserestrictinfo, &fpinfo->remote_conds,
											&fpinfo->local_conds);

	fpinfo->attrs_used = NULL;
	pull_varattnos((Node *) baserel->reltarget->exprs, baserel->relid, &fpinfo->attrs_used);
	foreach(lc, fpinfo->local_conds) {
		RestrictInfo *rinfo = lfirst_node(RestrictInfo, lc);
		pull_varattnos((Node *) rinfo->clause, baserel->relid, &fpinfo->attrs_used);
	}

	fpinfo->local_conds_sel = clauselist_selectivity(root, fpinfo->local_conds, baserel->relid,
																									 JOIN_INNER, NULL);
	cost_qual_eval(&fpinfo->local_conds_cost, fpinfo->local_conds, root);

	if (fpinfo->use_remote_estimate) {
		/* TODO */
	} else {
		if (baserel->tuples < 0) {
			baserel->pages = 10;
			baserel->tuples = (10 * BLCKSZ) /
												(baserel->reltarget->width + MAXALIGN(SizeofHeapTupleHeader));
		}
		set_baserel_size_estimates(root, baserel);
	}

	fpinfo->relation_name = psprintf("%u", baserel->relid);

	set_sleep_on_sig_off();
}

static void
tiberoGetForeignPaths(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid)
{
	TbFdwRelationInfo *fpinfo = (TbFdwRelationInfo *) baserel->fdw_private;

	set_sleep_on_sig_on();

	add_path(baserel,
					 (Path *) create_foreignscan_path(root, baserel, NULL, fpinfo->rows, fpinfo->startup_cost,
																						fpinfo->total_cost, NIL, baserel->lateral_relids, NULL,
																						NIL));

	if (fpinfo->use_remote_estimate) {
		/* TODO */
	}

	set_sleep_on_sig_off();
}

static ForeignScan *
tiberoGetForeignPlan(PlannerInfo *root, RelOptInfo *foreignrel, Oid foreigntableid,
										 ForeignPath *best_path, List *tlist, List *scan_clauses, Plan *outer_plan)
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
	ForeignScan *result_foreign_scan = NULL;

	set_sleep_on_sig_on();

	if (best_path->fdw_private) {
		has_final_sort = intVal(list_nth(best_path->fdw_private, FdwPathPrivateHasFinalSort));
		has_limit = intVal(list_nth(best_path->fdw_private, FdwPathPrivateHasLimit));
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
			else if (expr_inspect_shippability(root, foreignrel, rinfo->clause))
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
	deparse_select_stmt_for_rel(&sql, root, foreignrel, fdw_scan_tlist, remote_exprs,
															best_path->path.pathkeys, has_final_sort, has_limit, false,
															&retrieved_attrs, &params_list, fpinfo->use_fb_query);

	fdw_private = list_make4(makeString(sql.data), retrieved_attrs, makeInteger(fpinfo->fetch_size),
													 makeInteger(fpinfo->use_fb_query));

	Assert(IS_SIMPLE_REL(foreignrel));

	result_foreign_scan = make_foreignscan(tlist, local_exprs, scan_relid, params_list, fdw_private,
																				 fdw_scan_tlist, fdw_recheck_quals, outer_plan);

	set_sleep_on_sig_off();

	return result_foreign_scan;
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

	if (eflags & EXEC_FLAG_EXPLAIN_ONLY)
		return;

	set_sleep_on_sig_on();

	fsstate = (TbFdwScanState *) palloc0(sizeof(TbFdwScanState));
	node->fdw_state = (void *) fsstate;

	/* Join/Aggregation case */
	Assert(fsplan->scan.scanrelid > 0);
	rtindex = fsplan->scan.scanrelid;
	rte = exec_rt_fetch(rtindex, estate);
	userid = rte->checkAsUser ? rte->checkAsUser : GetUserId();

	table = GetForeignTable(rte->relid);
	user = GetUserMapping(userid, table->serverid);

	fsstate->query = (unsigned char *) strVal(list_nth(fsplan->fdw_private, FdwScanPrivateSelectSql));
	fsstate->retrieved_attrs = (List *) list_nth(fsplan->fdw_private, FdwScanPrivateRetrievedAttrs);
	fsstate->fetch_size = intVal(list_nth(fsplan->fdw_private, FdwScanPrivateFetchSize));
	fsstate->use_fb_query = intVal(list_nth(fsplan->fdw_private, FdwScanPrivateUseFbQuery));

	fsstate->tuple_cnt = TB_FDW_INIT_TUPLE_CNT;
	fsstate->cur_tuple_idx = 0;
	fsstate->end_of_fetch = false;

	fsstate->batch_ctx = AllocSetContextCreate(estate->es_query_cxt, "tibero_fdw tuple data",
																						 ALLOCSET_DEFAULT_SIZES);
	fsstate->temp_ctx = AllocSetContextCreate(estate->es_query_cxt, "tibero_fdw temporary data",
																						ALLOCSET_SMALL_SIZES);

	fsstate->rel = node->ss.ss_currentRelation;
	fsstate->tupdesc = RelationGetDescr(fsstate->rel);

	fsstate->attinmeta = TupleDescGetAttInMetadata(fsstate->tupdesc);

	fsstate->table = (TbTable *) palloc0(sizeof(TbTable));
	fsstate->table->column = (TbColumn **) palloc0(sizeof(TbColumn *) * fsstate->tupdesc->natts);
	for (i = 0; i < fsstate->tupdesc->natts; i++) {
		fsstate->table->column[i] = (TbColumn *) palloc0(sizeof(TbColumn));
	}

	fsstate->tbStmt = (TbStatement *) palloc0(sizeof(TbStatement));
	get_tb_statement(user, fsstate->tbStmt, fsstate->use_fb_query);

	TbSQLPrepare(fsstate->tbStmt, (SQLCHAR *)fsstate->query, SQL_NTS);
	TbSQLNumResultCols(fsstate->tbStmt, &fsstate->tbStmt->res_col_cnt);

	for (i = 0; i < fsstate->tbStmt->res_col_cnt; i++) {
		TbColumn *col = fsstate->table->column[i];
		TbSQLDescribeCol(fsstate->tbStmt, (SQLSMALLINT)i + 1, col->col_name, sizeof(col->col_name),
										 &col->col_name_len, &col->data_type, &col->col_size, &col->scale,
										 &col->nullable);

		col->data = palloc0(sizeof(unsigned char) * col->col_size * fsstate->fetch_size);
		col->ind = palloc0(sizeof(SQLLEN) * fsstate->fetch_size);
	}

	if (fsstate->use_fb_query && !IsolationUsesXactSnapshot()) {
		TbSQLBindParameter(fsstate->tbStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, NUMERICOID, 0, 0,
											 fsstate->tbStmt->conn->tsn, strlen(fsstate->tbStmt->conn->tsn), NULL);
	}

	TbSQLSetStmtAttr(fsstate->tbStmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)fsstate->fetch_size, 0);
	TbSQLSetStmtAttr(fsstate->tbStmt, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER)&fsstate->tuple_cnt, 0);

	for (i = 0; i < fsstate->tbStmt->res_col_cnt; i++) {
		TbColumn *col = fsstate->table->column[i];
		TbSQLBindCol(fsstate->tbStmt, i + 1, SQL_C_CHAR, (SQLCHAR *)col->data, col->col_size, col->ind);
	}

	fsstate->tbStmt->query_executed = false;

	set_sleep_on_sig_off();
}

static Datum
tibero_convert_to_pg(Oid pgtyp, int pgtypmod, TbColumn *column, int tuple_idx)
{
	Datum value_datum;
	Datum valueDatum;
	regproc typeinput;
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
need_fetch_tuples(TbFdwScanState *fsstate)
{
	return !fsstate->end_of_fetch &&
				 (fsstate->tuple_cnt == TB_FDW_INIT_TUPLE_CNT ||
					fsstate->cur_tuple_idx == fsstate->tuple_cnt);
}

static TupleTableSlot *
get_next_tuple(ForeignScanState *node)
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
make_tuples(ForeignScanState *node)
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
			int attnum = lfirst_int(lc) - 1;
			Oid pgtype = TupleDescAttr(attinmeta->tupdesc, attnum)->atttypid;
			int32 pgtypmod = TupleDescAttr(attinmeta->tupdesc, attnum)->atttypmod;

			if (fsstate->table->column[attid]->ind[i] == SQL_NULL_DATA) {
				nulls[attnum] = true;
				dvalues[attnum] = PointerGetDatum(NULL);
			} else {
				nulls[attnum] = false;
				dvalues[attnum] = tibero_convert_to_pg(pgtype, pgtypmod, fsstate->table->column[attid], i);
			}
			attid++;
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
fetch_tuples(ForeignScanState *node)
{
	TbFdwScanState *fsstate = (TbFdwScanState *) node->fdw_state;
	TbSQLFetch(fsstate->tbStmt, &fsstate->cur_tuple_idx, &fsstate->end_of_fetch);
	if (!fsstate->end_of_fetch) make_tuples(node);
}

static TupleTableSlot *
tiberoIterateForeignScan(ForeignScanState *node)
{
	TbFdwScanState *fsstate = (TbFdwScanState *) node->fdw_state;
	TupleTableSlot *result_tts = NULL;

	set_sleep_on_sig_on();

	if (!fsstate->tbStmt->query_executed) {
		TbSQLExecute(fsstate->tbStmt);
	}

	if (need_fetch_tuples(fsstate))
		fetch_tuples(node);
	result_tts = get_next_tuple(node);

	set_sleep_on_sig_off();

	return result_tts;
}

static void
tiberoReScanForeignScan(ForeignScanState *node)
{
	TbFdwScanState *fsstate = (TbFdwScanState *) node->fdw_state;

	set_sleep_on_sig_on();

	fsstate->end_of_fetch = false;
	fsstate->cur_tuple_idx = 0;
	fsstate->tbStmt->query_executed = false;

	set_sleep_on_sig_off();
}

static void
tiberoEndForeignScan(ForeignScanState *node)
{
	TbFdwScanState *fsstate = (TbFdwScanState *) node->fdw_state;

	/* When called for EXPLAIN */
	if (fsstate == NULL) return;

	set_sleep_on_sig_on();

	TbSQLFreeStmt(fsstate->tbStmt, SQL_DROP);

	set_sleep_on_sig_off();
}

static bool
foreign_join_ok(PlannerInfo *root, RelOptInfo *joinrel, JoinType jointype, RelOptInfo *outerrel,
								RelOptInfo *innerrel, JoinPathExtraData *extra)
{
	/* TODO Implementation */
	return false;
}

static void
tiberoGetForeignJoinPaths(PlannerInfo *root, RelOptInfo *joinrel, RelOptInfo *outerrel,
													RelOptInfo *innerrel, JoinType jointype, JoinPathExtraData *extra)
{
	set_sleep_on_sig_on();

	if (foreign_join_ok(root, joinrel, jointype, outerrel, innerrel, extra)) {
		/* TODO */
	}

	set_sleep_on_sig_off();
}

static void
tiberoExplainForeignScan(ForeignScanState *node, ExplainState *es)
{
	ForeignScan *plan = castNode(ForeignScan, node->ss.ps.plan);
	List *fdw_private = plan->fdw_private;

	/*
	* TODO: This is a direct copy from postgres_fdw. Might need to check.
	*
	* Identify foreign scans that are really joins or upper relations.  The
	* input looks something like "(1) LEFT JOIN (2)", and we must replace the
	* digit string(s), which are RT indexes, with the correct relation names.
	* We do that here, not when the plan is created, because we can't know
	* what aliases ruleutils.c will assign at plan creation time.
	*/
	if (foreign_scan_has_upper_rels(plan->fdw_private))
	{
		StringInfo rel_names = get_foreign_scan_upper_rel_names(plan, es);
		ExplainPropertyText("Relations", rel_names->data, es);
	}

	/* Add remote query when VERBOSE option is specified */
	if (es->verbose)
	{
		char *sql = strVal(list_nth(fdw_private, FdwScanPrivateSelectSql));
		ExplainPropertyText("Remote SQL", sql, es);
	}
}

static inline bool
foreign_scan_has_upper_rels(List *fdw_private)
{
	return list_length(fdw_private) > FdwScanPrivateRelations;
}


static inline StringInfo
get_foreign_scan_upper_rel_names(ForeignScan *plan, ExplainState *es)
{
	List *fdw_private = plan->fdw_private;
	StringInfo relations;
	char *rawrelations;
	char *ptr;
	int minrti;
	int rtoffset;

	rawrelations = strVal(list_nth(fdw_private, FdwScanPrivateRelations));

	/*
	 * A difficulty with using a string representation of RT indexes is
	 * that setrefs.c won't update the string when flattening the
	 * rangetable.  To find out what rtoffset was applied, identify the
	 * minimum RT index appearing in the string and compare it to the
	 * minimum member of plan->fs_relids.  (We expect all the relids in
	 * the join will have been offset by the same amount; the Asserts
	 * below should catch it if that ever changes.)
	 */
	minrti = INT_MAX;
	ptr = rawrelations;
	while (*ptr)
	{
		if (isdigit((unsigned char) *ptr))
		{
			int			rti = strtol(ptr, &ptr, 10);

			if (rti < minrti)
				minrti = rti;
		}
		else
			ptr++;
	}
	rtoffset = bms_next_member(plan->fs_relids, -1) - minrti;

	/* Now we can translate the string */
	relations = makeStringInfo();
	ptr = rawrelations;
	while (*ptr)
	{
		if (isdigit((unsigned char) *ptr))
		{
			int rti = strtol(ptr, &ptr, 10);
			RangeTblEntry *rte;
			char *relname;
			char *refname;

			rti += rtoffset;
			Assert(bms_is_member(rti, plan->fs_relids));
			rte = rt_fetch(rti, es->rtable);
			Assert(rte->rtekind == RTE_RELATION);

			/* This logic should agree with explain.c's ExplainTargetRel */
			relname = get_rel_name(rte->relid);
			if (es->verbose)
			{
				char *namespace;

				namespace = get_namespace_name(get_rel_namespace(rte->relid));
				appendStringInfo(relations, "%s.%s",
									quote_identifier(namespace),
									quote_identifier(relname));
			}
			else
				appendStringInfoString(relations,
											quote_identifier(relname));
			refname = (char *) list_nth(es->rtable_names, rti - 1);
			if (refname == NULL)
				refname = rte->eref->aliasname;
			if (strcmp(refname, relname) != 0)
				appendStringInfo(relations, " %s",
									quote_identifier(refname));
		}
		else
			appendStringInfoChar(relations, *ptr++);
	}

	return relations;
}

static int
tiberoIsForeignRelUpdatable(Relation rel)
{
	/* TODO change the default value of updatable to true after complete implementation of DML */
	bool updatable = false;
	ForeignTable *table;
	ForeignServer *server;
	ListCell *lc;

	table = GetForeignTable(RelationGetRelid(rel));
	server = GetForeignServer(table->serverid);

	foreach(lc, server->options) {
		DefElem *def = (DefElem *) lfirst(lc);
		if (strcmp(def->defname, "updatable") == 0)
			updatable = defGetBoolean(def);
	}

	foreach(lc, table->options) {
		DefElem *def = (DefElem *) lfirst(lc);
		if (strcmp(def->defname, "updatable") == 0)
			updatable = defGetBoolean(def);
	}

	return updatable ? (1 << CMD_INSERT) : 0;
}

static List *
tiberoPlanForeignModify(PlannerInfo *root, ModifyTable *plan, Index resultRelation,
												int subplan_index)
{
	CmdType operation = plan->operation;
	RangeTblEntry *rte = planner_rt_fetch(resultRelation, root);
	Relation rel;
	List *targetAttrs = NIL;
	StringInfoData sql;

	TupleDesc tupdesc;
	int attnum;

	if (operation != CMD_INSERT)
		return NULL;

	set_sleep_on_sig_on();

	initStringInfo(&sql);

	rel = table_open(rte->relid, NoLock);

	tupdesc = RelationGetDescr(rel);
	for (attnum = 1; attnum <= tupdesc->natts; attnum++) {
		Form_pg_attribute attr = TupleDescAttr(tupdesc, attnum - 1);

		if (!attr->attisdropped)
			targetAttrs = lappend_int(targetAttrs, attnum);
	}

	if (operation == CMD_INSERT) {
		deparse_insert_sql(&sql, root, resultRelation, rel, targetAttrs);
	}

	table_close(rel, NoLock);

	set_sleep_on_sig_off();

	return list_make2(makeString(sql.data), targetAttrs);
}

static void
tiberoBeginForeignModify(ModifyTableState *mtstate, ResultRelInfo *resultRelInfo,
												 List *fdw_private, int subplan_index, int eflags)
{
	TbFdwExecState *fmstate;
	EState *estate = mtstate->ps.state;
	Relation rel = resultRelInfo->ri_RelationDesc;
	AttrNumber n_params;
	Oid typefnoid = InvalidOid;
	bool isvarlena = false;
	ListCell *lc;
	Oid foreignTableId = InvalidOid;
	RangeTblEntry *rte;
	Oid userid;
	ForeignServer *server;
	UserMapping *user;
	ForeignTable *table;

	if (eflags & EXEC_FLAG_EXPLAIN_ONLY)
		return;

	set_sleep_on_sig_on();

	rte = rt_fetch(resultRelInfo->ri_RangeTableIndex, estate->es_range_table);
	userid = rte->checkAsUser ? rte->checkAsUser : GetUserId();

	foreignTableId = RelationGetRelid(rel);

	table = GetForeignTable(foreignTableId);
	server = GetForeignServer(table->serverid);
	user = GetUserMapping(userid, server->serverid);

	fmstate = (TbFdwExecState *) palloc0(sizeof(TbFdwExecState));
	fmstate->query = strVal(list_nth(fdw_private, TbFdwModifyQuery));
	fmstate->target_attrs = (List *) list_nth(fdw_private, TbFdwModifyTargetAttrs);
	n_params = list_length(fmstate->target_attrs) + 1;
	fmstate->p_flinfo = (FmgrInfo *) palloc0(sizeof(FmgrInfo) * n_params);

	fmstate->temp_ctx = AllocSetContextCreate(estate->es_query_cxt, "tibero_fdw temporary data",
																						ALLOCSET_SMALL_SIZES);

	foreach(lc, fmstate->target_attrs)
	{
		int attnum = lfirst_int(lc);
		Form_pg_attribute attr = TupleDescAttr(RelationGetDescr(rel), attnum - 1);

		Assert(!attr->attisdropped);

		getTypeOutputInfo(attr->atttypid, &typefnoid, &isvarlena);
		fmgr_info(typefnoid, &fmstate->p_flinfo[fmstate->p_nums]);
		fmstate->p_nums++;
	}

	if (fmstate->p_nums > n_params) {
		Assert(false);
	}

	fmstate->tbStmt = (TbStatement *) palloc0(sizeof(TbStatement));
	get_tb_statement(user, fmstate->tbStmt, false);

	fmstate->tbStmt->query_executed = false;

	TbSQLPrepare(fmstate->tbStmt, (SQLCHAR *)fmstate->query, SQL_NTS);

	resultRelInfo->ri_FdwState = fmstate;

	set_sleep_on_sig_off();
}

static TupleTableSlot *
tiberoExecForeignInsert(EState *estate, ResultRelInfo *resultRelInfo, TupleTableSlot *slot,
												TupleTableSlot *planSlot)
{
	TbFdwExecState *fmstate;
	ListCell *lc;
	int n_params;
	MemoryContext old_context;
	bool *isnull;
	char **p_values;
	SQLLEN *inds;
	int pindex = 0;
	int findex = 0;

	set_sleep_on_sig_on();

	fmstate = (TbFdwExecState *) resultRelInfo->ri_FdwState;
	n_params = list_length(fmstate->target_attrs);

	old_context = MemoryContextSwitchTo(fmstate->temp_ctx);

	p_values = (char **) palloc(sizeof(char *) * n_params);
	isnull = (bool *) palloc0(sizeof(bool) * n_params);
	inds = (SQLLEN *) palloc(sizeof(SQLLEN) * n_params);

	foreach(lc, fmstate->target_attrs) {
		int attnum = lfirst_int(lc) - 1;
		Form_pg_attribute attr = TupleDescAttr(slot->tts_tupleDescriptor, attnum);
		Oid pg_param_type = attr->atttypid;
		Datum value;

		if (attr->attgenerated)
			continue;

		value = slot_getattr(slot, attnum + 1, &isnull[attnum]);
		if (isnull[attnum]) {
			p_values[pindex] = NULL;
			inds[pindex] = SQL_NULL_DATA;
		} else {
			p_values[pindex] = OutputFunctionCall(&fmstate->p_flinfo[findex], value);
			inds[pindex] = SQL_NTS;
		}

		TbSQLBindParameter(fmstate->tbStmt, attnum + 1, SQL_PARAM_INPUT, SQL_C_CHAR,
											 pg_param_type, 0, 0,
											 p_values[pindex], p_values[pindex] ? strlen(p_values[pindex]) : 0,
											 &inds[pindex]);
		pindex++;
		findex++;
	}

	if (!fmstate->tbStmt->query_executed) {
		TbSQLExecute(fmstate->tbStmt);
	}

	MemoryContextSwitchTo(old_context);
	MemoryContextReset(fmstate->temp_ctx);

	set_sleep_on_sig_off();

	return slot;
}

static void
tiberoEndForeignModify(EState *estate, ResultRelInfo *resultRelInfo)
{
	TbFdwExecState *fmstate = (TbFdwExecState *) resultRelInfo->ri_FdwState;

	/* When called for EXPLAIN */
	if (fmstate == NULL) return;

	set_sleep_on_sig_on();

	TbSQLFreeStmt(fmstate->tbStmt, SQL_DROP);

	set_sleep_on_sig_off();
}
