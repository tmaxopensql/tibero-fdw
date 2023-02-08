/*-------------------------------------------------------------------------
 *
 * deparse.c
 *		  Query deparser for tibero_fdw
 *
 * Portions Copyright (c) 2022-2023, HyperSQL Research & Development Team
 *
 * IDENTIFICATION
 *		  contrib/tibero_fdw/deparse.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/htup_details.h"
#include "access/sysattr.h"
#include "access/table.h"
#include "access/xact.h"                /* IsolationUsesXactSnapshot          */
#include "catalog/pg_aggregate.h"
#include "catalog/pg_collation.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_operator.h"
#include "catalog/pg_opfamily.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_type.h"
#include "commands/defrem.h"
#include "nodes/makefuncs.h"
#include "nodes/nodeFuncs.h"
#include "nodes/plannodes.h"
#include "optimizer/optimizer.h"
#include "optimizer/prep.h"
#include "optimizer/tlist.h"
#include "parser/parsetree.h"
#include "tibero_fdw.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "utils/syscache.h"
#include "utils/typcache.h"
#include "commands/tablecmds.h"

typedef struct deparse_expr_cxt
{
	PlannerInfo *root;
	RelOptInfo *foreignrel;
	RelOptInfo *scanrel;
	StringInfo	buf;
	List	  **params_list;
} deparse_expr_cxt;

#define REL_ALIAS_PREFIX	"r"
#define ADD_REL_QUALIFIER(buf, varno)	\
				appendStringInfo((buf), "%s%d.", REL_ALIAS_PREFIX, (varno))

static void daprse_target_list(StringInfo buf, RangeTblEntry *rte, Index rtindex,
							  							Relation rel, bool is_returning, 
															Bitmapset *attrs_used, bool qualify_col,
							  							List **retrieved_attrs);
static void deparse_column_ref(StringInfo buf, int varno, int varattno,
							 							 RangeTblEntry *rte, bool qualify_col);
static void deparse_replation(StringInfo buf, Relation rel);
static void deparse_expr(Expr *expr, deparse_expr_cxt *context);
static void deparse_select_sql(List *tlist, bool is_subquery, List **retrieved_attrs,
							 							 deparse_expr_cxt *context);
static void append_conditions(List *exprs, deparse_expr_cxt *context);
static void deparse_from_expr_for_rel(StringInfo buf, PlannerInfo *root,
								  								RelOptInfo *foreignrel, bool use_alias,
								  								Index ignore_rel, List **ignore_conds,
								  								List **params_list);
static void deparse_from_expr(List *quals, deparse_expr_cxt *context);

void
classify_conditions(PlannerInfo *root, RelOptInfo *baserel, List *input_conds,
				   				 List **remote_conds, List **local_conds)
{
	ListCell   *lc;

	*remote_conds = NIL;
	*local_conds = NIL;

	foreach(lc, input_conds) {
		RestrictInfo *ri = lfirst_node(RestrictInfo, lc);

		if (is_foreign_expr(root, baserel, ri->clause))
			*remote_conds = lappend(*remote_conds, ri);
		else
			*local_conds = lappend(*local_conds, ri);
	}
}

bool
is_foreign_expr(PlannerInfo *root,
				RelOptInfo *baserel,
				Expr *expr)
{
	/* TODO */
	return false;
}

void
deparse_select_stmt_for_rel(StringInfo buf, PlannerInfo *root, RelOptInfo *rel,
														List *tlist, List *remote_conds, List *pathkeys,
														bool has_final_sort, bool has_limit, bool is_subquery,
														List **retrieved_attrs, List **params_list)
{
	deparse_expr_cxt context;
	List *quals;

	Assert(IS_SIMPLE_REL(rel));

	context.buf = buf;
	context.root = root;
	context.foreignrel = rel;
	context.scanrel = rel;
	context.params_list = params_list;

	deparse_select_sql(tlist, is_subquery, retrieved_attrs, &context);

	quals = remote_conds;
	deparse_from_expr(quals, &context);
}

static void
deparse_select_sql(List *tlist, bool is_subquery, List **retrieved_attrs,
								 deparse_expr_cxt *context)
{
	StringInfo	buf = context->buf;
	RelOptInfo *foreignrel = context->foreignrel;
	PlannerInfo *root = context->root;
	TbFdwRelationInfo *fpinfo = (TbFdwRelationInfo *) foreignrel->fdw_private;

	RangeTblEntry *rte = planner_rt_fetch(foreignrel->relid, root);
	Relation	rel = table_open(rte->relid, NoLock);

	appendStringInfoString(buf, "SELECT ");

	daprse_target_list(buf, rte, foreignrel->relid, rel, false,
						fpinfo->attrs_used, false, retrieved_attrs);

	table_close(rel, NoLock);
}

static void
deparse_from_expr(List *quals, deparse_expr_cxt *context)
{
	StringInfo	buf = context->buf;
	RelOptInfo *scanrel = context->scanrel;

	Assert(!IS_UPPER_REL(context->foreignrel) || IS_JOIN_REL(scanrel) || 
				 IS_SIMPLE_REL(scanrel));

	appendStringInfoString(buf, " FROM ");
	deparse_from_expr_for_rel(buf, context->root, scanrel,
						  (bms_membership(scanrel->relids) == BMS_MULTIPLE),
						  (Index) 0, NULL, context->params_list);

	if (quals != NIL) {
		appendStringInfoString(buf, " WHERE ");
		append_conditions(quals, context);
	}
}

static void
daprse_target_list(StringInfo buf, RangeTblEntry *rte, Index rtindex, Relation rel,
				  				bool is_returning, Bitmapset *attrs_used, bool qualify_col,
				  				List **retrieved_attrs)
{
	TupleDesc	tupdesc = RelationGetDescr(rel);
	bool have_wholerow;
	bool first;
	int i;

	*retrieved_attrs = NIL;

	have_wholerow = bms_is_member(0 - FirstLowInvalidHeapAttributeNumber,
															  attrs_used);

	first = true;
	for (i = 1; i <= tupdesc->natts; i++) {
		Form_pg_attribute attr = TupleDescAttr(tupdesc, i - 1);

		if (attr->attisdropped)
			continue;

		if (have_wholerow ||
				bms_is_member(i - FirstLowInvalidHeapAttributeNumber, attrs_used)) {
			if (!first)
				appendStringInfoString(buf, ", ");
			else if (is_returning)
				appendStringInfoString(buf, " RETURNING ");
			first = false;

			deparse_column_ref(buf, rtindex, i, rte, qualify_col);

			*retrieved_attrs = lappend_int(*retrieved_attrs, i);
		}
	}

	if (bms_is_member(SelfItemPointerAttributeNumber - FirstLowInvalidHeapAttributeNumber,
					  				attrs_used)) {
		if (!first)
			appendStringInfoString(buf, ", ");
		else if (is_returning)
			appendStringInfoString(buf, " RETURNING ");
		first = false;

		if (qualify_col)
			ADD_REL_QUALIFIER(buf, rtindex);
		appendStringInfoString(buf, "ctid");

		*retrieved_attrs = lappend_int(*retrieved_attrs,
									   SelfItemPointerAttributeNumber);
	}

	if (first && !is_returning)
		appendStringInfoString(buf, "NULL");
}

static void
append_conditions(List *exprs, deparse_expr_cxt *context)
{
	ListCell *lc;
	bool is_first = true;
	StringInfo buf = context->buf;

	// nestlevel = set_transmission_modes();

	foreach(lc, exprs)
	{
		Expr	   *expr = (Expr *) lfirst(lc);

		if (IsA(expr, RestrictInfo))
			expr = ((RestrictInfo *) expr)->clause;

		if (!is_first)
			appendStringInfoString(buf, " AND ");

		appendStringInfoChar(buf, '(');
		deparse_expr(expr, context);
		appendStringInfoChar(buf, ')');

		is_first = false;
	}

	// reset_transmission_modes(nestlevel);
}

static void
deparse_from_expr_for_rel(StringInfo buf, PlannerInfo *root, RelOptInfo *foreignrel,
					  bool use_alias, Index ignore_rel, List **ignore_conds,
					  List **params_list)
{
	RangeTblEntry *rte = planner_rt_fetch(foreignrel->relid, root);

	Relation	rel = table_open(rte->relid, NoLock);

	deparse_replation(buf, rel);

	if (use_alias)
		appendStringInfo(buf, " %s%d", REL_ALIAS_PREFIX, foreignrel->relid);

	table_close(rel, NoLock);
}

static void
deparse_column_ref(StringInfo buf, int varno, int varattno, RangeTblEntry *rte,
				 bool qualify_col)
{
	if (varattno == SelfItemPointerAttributeNumber) {
		if (qualify_col)
			ADD_REL_QUALIFIER(buf, varno);
		appendStringInfoString(buf, "ctid");
	} else if (varattno < 0) {
		Oid			fetchval = 0;

		if (varattno == TableOidAttributeNumber)
			fetchval = rte->relid;

		if (qualify_col) {
			appendStringInfoString(buf, "CASE WHEN (");
			ADD_REL_QUALIFIER(buf, varno);
			appendStringInfo(buf, "*)::text IS NOT NULL THEN %u END", fetchval);
		}	else {
			appendStringInfo(buf, "%u", fetchval);
		}
	} else if (varattno == 0) {
		Relation	rel;
		Bitmapset  *attrs_used;
		List	   *retrieved_attrs;

		rel = table_open(rte->relid, NoLock);

		attrs_used = bms_add_member(NULL, 0 - FirstLowInvalidHeapAttributeNumber);

		if (qualify_col) {
			appendStringInfoString(buf, "CASE WHEN (");
			ADD_REL_QUALIFIER(buf, varno);
			appendStringInfoString(buf, "*)::text IS NOT NULL THEN ");
		}

		appendStringInfoString(buf, "ROW(");
		daprse_target_list(buf, rte, varno, rel, false, attrs_used, qualify_col,
						  &retrieved_attrs);
		appendStringInfoChar(buf, ')');

		if (qualify_col)
			appendStringInfoString(buf, " END");

		table_close(rel, NoLock);
		bms_free(attrs_used);
	}
	else
	{
		char	   *colname = NULL;
		List	   *options;
		ListCell   *lc;

		Assert(!IS_SPECIAL_VARNO(varno));

		options = GetForeignColumnOptions(rte->relid, varattno);
		foreach(lc, options)
		{
			DefElem    *def = (DefElem *) lfirst(lc);

			if (strcmp(def->defname, "column_name") == 0)
			{
				colname = defGetString(def);
				break;
			}
		}

		if (colname == NULL)
			colname = get_attname(rte->relid, varattno, false);

		if (qualify_col)
			ADD_REL_QUALIFIER(buf, varno);

		appendStringInfoString(buf, quote_identifier(colname));
	}
}

static void
deparse_replation(StringInfo buf, Relation rel)
{
	ForeignTable *table;
	const char *owner_name = NULL;
	const char *rel_name = NULL;
	ListCell   *lc;
	const char *flashback_clause = IsolationUsesXactSnapshot() ? "" : " as of tsn ?";

	table = GetForeignTable(RelationGetRelid(rel));

	foreach(lc, table->options) {
		DefElem    *def = (DefElem *) lfirst(lc);

		if (strcmp(def->defname, "owner_name") == 0)
			owner_name = defGetString(def);
		else if (strcmp(def->defname, "table_name") == 0)
			rel_name = defGetString(def);
	}

	if (rel_name == NULL)
		rel_name = RelationGetRelationName(rel);

	if (owner_name == NULL) {
		appendStringInfo(buf, "%s%s", quote_identifier(rel_name), flashback_clause);
	} else {
		appendStringInfo(buf, "%s.%s%s", quote_identifier(owner_name), 
										 quote_identifier(rel_name), flashback_clause);
	}
}

static void
deparse_expr(Expr *node, deparse_expr_cxt *context)
{
	Assert(node == NULL);
	return;
}
