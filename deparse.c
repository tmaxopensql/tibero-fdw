/*--------------------------------------------------------------------------------------------------
 *
 * deparse.c
 *			Query deparser for tibero_fdw
 *
 * Portions Copyright (c) 2022-2023, Tmax OpenSQL Research & Development Team
 *
 * IDENTIFICATION
 *			contrib/tibero_fdw/deparse.c
 *
 *--------------------------------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/htup_details.h"
#include "access/sysattr.h"
#include "access/table.h"
#include "access/xact.h"													/* IsolationUsesXactSnapshot										*/
#include "catalog/pg_aggregate.h"
#include "catalog/pg_collation.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_operator.h"
#include "catalog/pg_opfamily.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_type.h"
#include "commands/defrem.h"
#include "commands/tablecmds.h"
#include "datatype/timestamp.h"
#include "nodes/makefuncs.h"
#include "nodes/nodeFuncs.h"
#include "nodes/plannodes.h"
#include "optimizer/optimizer.h"
#include "optimizer/prep.h"
#include "optimizer/tlist.h"
#include "parser/parsetree.h"
#include "tibero_fdw.h"
#include "utils/builtins.h"
#include "utils/date.h"
#include "utils/datetime.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "utils/syscache.h"
#include "utils/typcache.h"

typedef struct RemoteSQLInfo
{
	int pdepth; /* Parenthesis depth. Must be 0 at the end of creating remote SQL. */
	StringInfo buf; /* String buffer for remote SQL */
} RemoteSQLInfo;

typedef struct SubqueryVarInfo
{
	int rel_no;
	int column_no;
} SubqueryVarInfo;

typedef struct DeparseContext
{
	PlannerInfo *root;
	RelOptInfo *foreignrel;
	RelOptInfo *scanrel;
	RemoteSQLInfo	remote_sql;
	List		**params_list;
	bool use_fb_query;
} DeparseContext;


#define REL_ALIAS_PREFIX	"r"
#define ADD_REL_QUALIFIER(buf, varno)	appendStringInfo((buf), "%s%d.", REL_ALIAS_PREFIX, (varno))
#define SUBQUERY_REL_ALIAS_PREFIX	"s"
#define SUBQUERY_COL_ALIAS_PREFIX	"c"
#define SUBQUERY_REL_NOT_FOUND_ID -1

/* Functions to construct SELECT clause */
static inline void deparse_select_sql(List *tlist, bool is_subquery, List **retrieved_attrs,
																			DeparseContext *context);
static inline void deparse_target_list(StringInfo buf, RangeTblEntry *rte, Index rtindex, Relation rel,
																			 bool is_returning, Bitmapset *attrs_used, bool qualify_col,
																			 List **retrieved_attrs);
static inline void deparse_column_ref(StringInfo buf, int varno, int varattno, RangeTblEntry *rte,
																			bool qualify_col);

/* Functions to construct FROM clause */
static inline void deparse_from_expr(List *quals, DeparseContext *context);
static inline void deparse_from_expr_for_rel(StringInfo buf, PlannerInfo *root, RelOptInfo *foreignrel,
																						 bool use_alias, Index ignore_rel, List **ignore_conds,
																						 List **params_list, bool use_fb_query);
static inline void deparse_relation(StringInfo buf, Relation rel, bool use_fb_query);

/* Functions to construct WHERE, HAVING, RETURNING clause */
static inline void deparse_where_expr(List *quals, DeparseContext *context);

/* Common functions */
static void append_conditions(List *exprs, DeparseContext *context);
static void deparse_expr(Node *expr, DeparseContext *context);

/* Deparse expr functions for each expr type */
static inline void deparse_expr_for_T_Var(Node *, DeparseContext *);
static inline void deparse_expr_for_T_Const(Node *, DeparseContext *);
static inline void deparse_expr_for_T_Param(Node *, DeparseContext *);
static inline void deparse_expr_for_T_FuncExpr(Node *, DeparseContext *);
static inline void deparse_expr_for_T_OpExpr(Node *, DeparseContext *);
static inline void deparse_expr_for_T_DistinctExpr(Node *, DeparseContext *);
static inline void deparse_expr_for_T_BoolExpr(Node *, DeparseContext *);
static inline void deparse_expr_for_T_NullTest(Node *, DeparseContext *);
static inline void deparse_expr_for_T_ArrayExpr(Node *, DeparseContext *);
static inline void deparse_expr_for_T_Aggref(Node *, DeparseContext *);

/* Helper functions */
static inline SubqueryVarInfo get_subquery_info_from_var(Var *, RelOptInfo *);
static inline bool check_var_is_subquery(SubqueryVarInfo);

static inline void deparse_operator_name(StringInfo, Form_pg_operator);
static inline bool check_tibero_handles_pg_data_type(Oid);
static inline void deparse_datum(StringInfo, Datum, Oid data_type);
static inline void deparse_numeric_datum(StringInfo, Datum, regproc typoutput);
static inline void deparse_date_datum(StringInfo, Datum);
static inline void deparse_ts_datum(StringInfo, Datum);
static inline void deparse_ts_tz_datum(StringInfo, Datum);
static inline void deparse_interval_datum(StringInfo, Datum);
static inline void deparse_string_datum(StringInfo, Datum, regproc typoutput);
static inline void deparse_generic_datum(StringInfo, Datum);
static inline void deparse_string_literal(StringInfo, const char *);

static inline void remote_sql_open_parenthesis(RemoteSQLInfo *);
static inline void remote_sql_close_parenthesis(RemoteSQLInfo *);
static inline void remote_sql_check_sanity(RemoteSQLInfo *);
static inline StringInfo remote_sql_get_buffer(RemoteSQLInfo *);


#define DEPARSE_EXPR(type, expr, context) 																												 \
	do {																																														 \
		if (expr != NULL)																																							 \
		{																																															 \
			deparse_expr_for_##type(expr, context);																										 			 \
		}																																															 \
	} while(0)

void
deparse_select_stmt_for_rel(StringInfo buf, PlannerInfo *root, RelOptInfo *rel, List *tlist,
														List *remote_conds, List *pathkeys, bool has_final_sort, bool has_limit,
														bool is_subquery, List **retrieved_attrs, List **params_list,
														bool use_fb_query)
{
	DeparseContext context;
	List *quals;

	Assert(IS_SIMPLE_REL(rel));

	context.root = root;
	context.foreignrel = rel;
	context.scanrel = rel;
	context.remote_sql.buf = buf;
	context.remote_sql.pdepth = 0;
	context.params_list = params_list;
	context.use_fb_query = use_fb_query;

	deparse_select_sql(tlist, is_subquery, retrieved_attrs, &context);

	quals = remote_conds;
	deparse_from_expr(quals, &context);

	remote_sql_check_sanity(&context.remote_sql);
}

static inline void
deparse_select_sql(List *tlist, bool is_subquery, List **retrieved_attrs, DeparseContext *context)
{
	StringInfo buf = remote_sql_get_buffer(&context->remote_sql);
	RelOptInfo *foreignrel = context->foreignrel;
	PlannerInfo *root = context->root;
	TbFdwRelationInfo *fpinfo = (TbFdwRelationInfo *) foreignrel->fdw_private;

	RangeTblEntry *rte = planner_rt_fetch(foreignrel->relid, root);
	Relation rel = table_open(rte->relid, NoLock);

	appendStringInfoString(buf, "SELECT ");

	deparse_target_list(buf, rte, foreignrel->relid, rel, false, fpinfo->attrs_used, false,
										 retrieved_attrs);

	table_close(rel, NoLock);
}

static inline void
deparse_target_list(StringInfo buf, RangeTblEntry *rte, Index rtindex, Relation rel,
									 bool is_returning, Bitmapset *attrs_used, bool qualify_col,
									 List **retrieved_attrs)
{
	TupleDesc	tupdesc = RelationGetDescr(rel);
	bool have_wholerow;
	bool first;
	int i;

	*retrieved_attrs = NIL;

	have_wholerow = bms_is_member(0 - FirstLowInvalidHeapAttributeNumber, attrs_used);

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

		*retrieved_attrs = lappend_int(*retrieved_attrs, SelfItemPointerAttributeNumber);
	}

	if (first && !is_returning)
		appendStringInfoString(buf, "NULL");
}

static inline void
deparse_column_ref(StringInfo buf, int varno, int varattno, RangeTblEntry *rte, bool qualify_col)
{
	/* Attribute is either a ROWID or a valid column reference*/
	Assert(varattno == SelfItemPointerAttributeNumber || varattno > InvalidAttrNumber);

	if (varattno == SelfItemPointerAttributeNumber) {
		if (qualify_col)
		{
			ADD_REL_QUALIFIER(buf, varno);
		}
		appendStringInfoString(buf, "ROWID");
	}
	else {
		char *colname = NULL;
		List *options;
		ListCell *lc;

		Assert(!IS_SPECIAL_VARNO(varno));

		options = GetForeignColumnOptions(rte->relid, varattno);
		foreach(lc, options) {
			DefElem *def = (DefElem *) lfirst(lc);

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

static inline void
deparse_from_expr(List *quals, DeparseContext *context)
{
	StringInfo buf = remote_sql_get_buffer(&context->remote_sql);
	RelOptInfo *scanrel = context->scanrel;

	Assert(!IS_UPPER_REL(context->foreignrel) || IS_JOIN_REL(scanrel) || IS_SIMPLE_REL(scanrel));

	appendStringInfoString(buf, " FROM ");
	deparse_from_expr_for_rel(buf, context->root, scanrel,
														(bms_membership(scanrel->relids) == BMS_MULTIPLE),
														(Index) 0, NULL, context->params_list, context->use_fb_query);

	if (quals != NIL) {
		deparse_where_expr(quals, context);
	}
}

static inline void
deparse_from_expr_for_rel(StringInfo buf, PlannerInfo *root, RelOptInfo *foreignrel, bool use_alias,
													Index ignore_rel, List **ignore_conds, List **params_list,
													bool use_fb_query)
{
	RangeTblEntry *rte = planner_rt_fetch(foreignrel->relid, root);

	Relation rel = table_open(rte->relid, NoLock);

	deparse_relation(buf, rel, use_fb_query);

	if (use_alias)
		appendStringInfo(buf, " %s%d", REL_ALIAS_PREFIX, foreignrel->relid);

	table_close(rel, NoLock);
}

static inline void
deparse_relation(StringInfo buf, Relation rel, bool use_fb_query)
{
	ForeignTable *table;
	const char *owner_name = NULL;
	const char *rel_name = NULL;
	ListCell *lc;
	const char *flashback_clause = (use_fb_query && !IsolationUsesXactSnapshot()) ?
																 " as of tsn ?" : "";

	table = GetForeignTable(RelationGetRelid(rel));

	foreach(lc, table->options) {
		DefElem *def = (DefElem *) lfirst(lc);

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
		appendStringInfo(buf, "%s.%s%s", quote_identifier(owner_name), quote_identifier(rel_name),
										 flashback_clause);
	}
}

static inline void
deparse_where_expr(List *quals, DeparseContext *context)
{
	StringInfo buf = remote_sql_get_buffer(&context->remote_sql);
	appendStringInfoString(buf, " WHERE ");
	append_conditions(quals, context);
}

static inline void
append_conditions(List *exprs, DeparseContext *context)
{
	ListCell *lc;
	bool is_first = true;
	RemoteSQLInfo *remote_sql = &context->remote_sql;
	StringInfo buf = remote_sql_get_buffer(remote_sql);

	foreach(lc, exprs)
	{
		Expr *expr = (Expr *) lfirst(lc);

		if (IsA(expr, RestrictInfo))
			expr = ((RestrictInfo *) expr)->clause;

		if (!is_first)
			appendStringInfoString(buf, " AND ");

		remote_sql_open_parenthesis(remote_sql);
		deparse_expr((Node *)expr, context);
		remote_sql_close_parenthesis(remote_sql);

		is_first = false;
	}
}

static inline void
deparse_expr(Node *expr, DeparseContext *context)
{
	if (expr == NULL)
		return;

	switch (expr->type)
	{
		case T_Var:
			DEPARSE_EXPR(T_Var, expr, context);
			break;
		case T_Const:
			DEPARSE_EXPR(T_Const, expr, context);
			break;
		case T_Param:
			DEPARSE_EXPR(T_Param, expr, context);
			break;
		case T_FuncExpr:
			DEPARSE_EXPR(T_FuncExpr, expr, context);
			break;
		case T_OpExpr:
			DEPARSE_EXPR(T_OpExpr, expr, context);
			break;
		case T_DistinctExpr:
			DEPARSE_EXPR(T_DistinctExpr, expr, context);
			break;
		case T_BoolExpr:
			DEPARSE_EXPR(T_BoolExpr, expr, context);
			break;
		case T_NullTest:
			DEPARSE_EXPR(T_NullTest, expr, context);
			break;
		case T_ArrayExpr:
			DEPARSE_EXPR(T_ArrayExpr, expr, context);
			break;
		case T_Aggref:
			DEPARSE_EXPR(T_Aggref, expr, context);
			break;
		default:
			ereport(ERROR, (errcode(ERRCODE_FDW_ERROR),
							errmsg("unsupported expression type for deparse: %d", (int) expr->type)));
			break;
	}
}

static inline void
deparse_expr_for_T_Var(Node *expr, DeparseContext *context)
{
	Var *var = (Var *)expr;
	Relids relids = context->scanrel->relids;
	StringInfo buf = remote_sql_get_buffer(&context->remote_sql);

	/* Qualify columns when multiple relations are involved. */
	bool qualify_col = (bms_membership(relids) == BMS_MULTIPLE);
	SubqueryVarInfo subquery_var_info = get_subquery_info_from_var(var, context->foreignrel);

	if (check_var_is_subquery(subquery_var_info))
	{
		appendStringInfo(buf, "%s%d.%s%d",
						 SUBQUERY_REL_ALIAS_PREFIX, subquery_var_info.rel_no,
						 SUBQUERY_COL_ALIAS_PREFIX, subquery_var_info.column_no);
		return;
	}

	/* Variable is the column reference of the foreign table */
	if (bms_is_member(var->varno, relids) && var->varlevelsup == 0)
	{
		deparse_column_ref(buf, var->varno, var->varattno,
											 planner_rt_fetch(var->varno, context->root),
											 qualify_col);
	}
	else
	{
		/**
		 * TODO: Don't know in which situation this condition happens
		 * Refer to deparseVar in postgres_fdw.c
		 **/
		Assert(false && "Not implemented");
	}
}

static inline SubqueryVarInfo
get_subquery_info_from_var(Var *var, RelOptInfo *foreignrel)
{
	SubqueryVarInfo subquery_var_info;

	/* If var is not a subquery, then rel_no is set as SUBQUERY_REL_NOT_FOUND_ID */
	subquery_var_info.rel_no = SUBQUERY_REL_NOT_FOUND_ID;

	/* Should only be called in these cases. */
	Assert(IS_SIMPLE_REL(foreignrel) || IS_JOIN_REL(foreignrel));

	/* There isn't any lower subquery if the given relation is not a join relation */
	if (!IS_JOIN_REL(foreignrel))
	{
		return subquery_var_info;
	}
	else
	{
		/* TODO: check and get subquery info from variable. refer to is_subquery_var in postgres_fdw */
		Assert(false && "Not implemented");
	}

	return subquery_var_info;
}

static inline bool
check_var_is_subquery(SubqueryVarInfo subquery_var_info)
{
	return subquery_var_info.rel_no != SUBQUERY_REL_NOT_FOUND_ID;
}

static inline void
deparse_expr_for_T_Const(Node *expr, DeparseContext *context)
{
	Const *constant = (Const *) expr;
	StringInfo	buf = remote_sql_get_buffer(&context->remote_sql);

	Assert(check_tibero_handles_pg_data_type(constant->consttype));

	if (constant->constisnull)
	{
		appendStringInfoString(buf, "NULL");
	}
	else
	{
		deparse_datum(buf, constant->constvalue, constant->consttype);
	}
}

static inline bool
check_tibero_handles_pg_data_type(Oid typeid)
{
	bool can_handle = false;

	switch (typeid)
	{
		case INT2OID:
		case INT4OID:
		case INT8OID:
		case OIDOID:
		case FLOAT4OID:
		case FLOAT8OID:
		case NUMERICOID:
		case DATEOID:
		case TIMESTAMPOID:
		case TIMESTAMPTZOID:
		case INTERVALOID:
		case UUIDOID:
		case TEXTOID:
		case CHAROID:
		case BPCHAROID:
		case VARCHAROID:
		case NAMEOID:
			can_handle = true;
			break;
		default:
			break;
	}

	return can_handle;
}

static inline void
deparse_datum(StringInfo buf, Datum datum, Oid data_type)
{
	HeapTuple catalog_lookup_res;
	regproc typoutput;

	catalog_lookup_res = SearchSysCache1(TYPEOID, ObjectIdGetDatum(data_type));
	if (!HeapTupleIsValid(catalog_lookup_res))
	{
		ereport(ERROR, (errcode(ERRCODE_FDW_INVALID_DATA_TYPE),
						errmsg("cache lookup failed for type: %u", data_type)));
	}
	typoutput = ((Form_pg_type)GETSTRUCT(catalog_lookup_res))->typoutput;
	ReleaseSysCache(catalog_lookup_res);

	switch (data_type)
	{
		case INT2OID:
		case INT4OID:
		case INT8OID:
		case OIDOID:
		case FLOAT4OID:
		case FLOAT8OID:
		case NUMERICOID:
			deparse_numeric_datum(buf, datum, typoutput);
			break;
		case DATEOID:
			deparse_date_datum(buf, datum);
			break;
		case TIMESTAMPOID:
			deparse_ts_datum(buf, datum);
			break;
		case TIMESTAMPTZOID:
			deparse_ts_tz_datum(buf, datum);
			break;
		case INTERVALOID:
			deparse_interval_datum(buf, datum);
			break;
		case UUIDOID:
		case TEXTOID:
		case CHAROID:
		case BPCHAROID:
		case VARCHAROID:
		case NAMEOID:
			deparse_string_datum(buf, datum, typoutput);
			break;
		default:
			deparse_generic_datum(buf, datum);
			break;
	}
}

static inline void
deparse_numeric_datum(StringInfo buf, Datum datum, regproc typoutput)
{
	char *string;
	string = DatumGetCString(OidFunctionCall1(typoutput, datum));
	appendStringInfo(buf, "%s", string);
}

static inline void
deparse_date_datum(StringInfo buf, Datum datum)
{
	struct pg_tm datetime_tm;
	DateADT date_adt = DatumGetDateADT(datum);
	StringInfoData date_val;

	if (DATE_NOT_FINITE(date_adt))
	{
		ereport(ERROR, (errcode(ERRCODE_FDW_INVALID_ATTRIBUTE_VALUE),
						errmsg("infinite date value cannot be stored in Tibero")));
	}

	(void)j2date(date_adt + POSTGRES_EPOCH_JDATE,
								&(datetime_tm.tm_year),
								&(datetime_tm.tm_mon),
								&(datetime_tm.tm_mday));

	initStringInfo(&date_val);
	appendStringInfo(&date_val, "%04d-%02d-%02d",
									 datetime_tm.tm_year > 0 ? datetime_tm.tm_year : -datetime_tm.tm_year + 1,
									 datetime_tm.tm_mon,
									 datetime_tm.tm_mday);

	appendStringInfo(buf, "(TO_DATE('%s', 'SYYYY-MM-DD'))", date_val.data);
}

static inline void
deparse_ts_datum(StringInfo buf, Datum datum)
{
	struct pg_tm datetime_tm;
	fsec_t datetime_fsec;
	Timestamp timestamp = DatumGetTimestamp(datum);
	StringInfoData ts_val;

	if (TIMESTAMP_NOT_FINITE(timestamp))
	{
		ereport(ERROR, (errcode(ERRCODE_FDW_INVALID_ATTRIBUTE_VALUE),
						errmsg("infinite date value cannot be stored in Tibero")));
	}

	(void)timestamp2tm(timestamp,
										 NULL,
										 &datetime_tm,
										 &datetime_fsec,
										 NULL,
										 NULL);

	initStringInfo(&ts_val);
	appendStringInfo(&ts_val,
									 "%04d-%02d-%02d %02d:%02d:%02d.%06d",
									 datetime_tm.tm_year > 0 ? datetime_tm.tm_year : -datetime_tm.tm_year + 1,
									 datetime_tm.tm_mon, datetime_tm.tm_mday, datetime_tm.tm_hour,
									 datetime_tm.tm_min, datetime_tm.tm_sec, (int32)datetime_fsec);

	appendStringInfo(buf, "(TO_TIMESTAMP('%s', 'SYYYY-MM-DD HH24:MI:SS.FF'))", ts_val.data);
}

static inline void
deparse_ts_tz_datum(StringInfo buf, Datum datum)
{
	struct pg_tm datetime_tm;
	fsec_t datetime_fsec;
	int32 tzoffset = 0;
	TimestampTz timestamp_tz = DatumGetTimestampTz(datum);
	StringInfoData ts_val;

	if (TIMESTAMP_NOT_FINITE(timestamp_tz))
	{
		ereport(ERROR, (errcode(ERRCODE_FDW_INVALID_ATTRIBUTE_VALUE),
						errmsg("infinite date value cannot be stored in Tibero")));
	}

	(void)timestamp2tm(timestamp_tz,
										 &tzoffset,
										 &datetime_tm,
										 &datetime_fsec,
										 NULL,
										 NULL);

	initStringInfo(&ts_val);
	appendStringInfo(&ts_val,
									 "%04d-%02d-%02d %02d:%02d:%02d.%06d%+03d:%02d",
									 datetime_tm.tm_year > 0 ? datetime_tm.tm_year : -datetime_tm.tm_year + 1,
									 datetime_tm.tm_mon, datetime_tm.tm_mday, datetime_tm.tm_hour,
									 datetime_tm.tm_min, datetime_tm.tm_sec, (int32)datetime_fsec,
									 -tzoffset / 3600, ((tzoffset > 0) ? tzoffset % 3600 : -tzoffset % 3600) / 60);

	appendStringInfo(buf, "TO_TIMESTAMP_TZ('%s', 'SYYYY-MM-DD HH24:MI:SS.FF TZH:TZM')", ts_val.data);
}

static inline void
deparse_interval_datum(StringInfo buf, Datum datum)
{
	/* TODO: Not Implemented */
}

static inline void
deparse_string_datum(StringInfo buf, Datum datum, regproc typoutput)
{
	char *str = DatumGetCString(OidFunctionCall1(typoutput, datum));
	deparse_string_literal(buf, str);
}

static inline void
deparse_generic_datum(StringInfo buf, Datum datum)
{
	/* Just deparse it as string literal and hope for the best */
	char *str = DatumGetCString(datum);
	deparse_string_literal(buf, str);
}

static inline void
deparse_string_literal(StringInfo buf, const char *str)
{
	const char *ptr;

	if (str[0] == '\0')
	{
		appendStringInfoString(buf, "NULL");
		return;
	}

	/* Quote strings */
	appendStringInfo(buf, "'");
	for (ptr = str; *ptr; ++ptr)
	{
		if (*ptr == '\'')
		{
			appendStringInfo(buf, "'");
		}
		appendStringInfo(buf, "%c", *ptr);
	}
	appendStringInfo(buf, "'");
}

static inline void
deparse_expr_for_T_Param(Node *expr, DeparseContext *context)
{

}

static inline void
deparse_expr_for_T_FuncExpr(Node *expr, DeparseContext *context)
{

}

static inline void
deparse_expr_for_T_OpExpr(Node *expr, DeparseContext *context)
{
	OpExpr *op_expr = (OpExpr *)expr;
	RemoteSQLInfo *remote_sql = &context->remote_sql;
	StringInfo buf = remote_sql_get_buffer(remote_sql);
	HeapTuple catalog_lookup_res;
	Form_pg_operator op_info;
	Node *lhs_expr;
	Node *rhs_expr;

	catalog_lookup_res = SearchSysCache1(OPEROID, ObjectIdGetDatum(op_expr->opno));

	if (!HeapTupleIsValid(catalog_lookup_res))
	{
		ereport(ERROR, (errcode(ERRCODE_FDW_ERROR),
						errmsg("cache lookup failed for operator: %u", op_expr->opno)));
	}

	/* Sanity Check: Unary operator must have one arg and binary operator must have two args */
	op_info = (Form_pg_operator) GETSTRUCT(catalog_lookup_res);
	Assert((op_info->oprkind == 'l' && list_length(op_expr->args) == 1) ||
				 (op_info->oprkind == 'b' && list_length(op_expr->args) == 2));

	/* begin expression parenthesis */
	remote_sql_open_parenthesis(remote_sql);

	/* if it's a binary operator, there is a left hand side */
	lhs_expr = op_info->oprkind == 'b' ? linitial(op_expr->args) : NULL;
	rhs_expr = llast(op_expr->args);

	/* deparse left hand side expr */
	if (lhs_expr != NULL)
	{
		deparse_expr(lhs_expr, context);
		appendStringInfoSpaces(buf, 1);
	}

	/* deparse operator name */
	deparse_operator_name(buf, op_info);
	appendStringInfoSpaces(buf, 1);

	/* deparse right hand side expr */
	deparse_expr(rhs_expr, context);

	/* end expression parenthesis */
	remote_sql_close_parenthesis(remote_sql);

	ReleaseSysCache(catalog_lookup_res);
}

static inline void
deparse_operator_name(StringInfo buf, Form_pg_operator op_info)
{
	char *operator_name;

	/* Only built-in operators are allowed */
	Assert(op_info->oprnamespace == PG_CATALOG_NAMESPACE);

	/* Process exceptions for operator names */
	switch (op_info->oid)
	{
		case OID_NAME_LIKE_OP:
		case OID_TEXT_LIKE_OP:
		case OID_BPCHAR_LIKE_OP:
			operator_name = "LIKE";
			break;
		default:
			operator_name = NameStr(op_info->oprname);
			break;
	}

	appendStringInfoString(buf, operator_name);
}

static inline void
deparse_expr_for_T_DistinctExpr(Node *expr, DeparseContext *context)
{

}

static inline void
deparse_expr_for_T_BoolExpr(Node *expr, DeparseContext *context)
{

}
static inline void
deparse_expr_for_T_NullTest(Node *expr, DeparseContext *context)
{

}

static inline void
deparse_expr_for_T_ArrayExpr(Node *expr, DeparseContext *context)
{

}

static inline void
deparse_expr_for_T_Aggref(Node *expr, DeparseContext *context)
{

}

static inline void
remote_sql_open_parenthesis(RemoteSQLInfo *remote_sql)
{
	appendStringInfoChar(remote_sql->buf, '(');
	remote_sql->pdepth++;
}

static inline void
remote_sql_close_parenthesis(RemoteSQLInfo *remote_sql)
{
	appendStringInfoChar(remote_sql->buf, ')');
	remote_sql->pdepth--;

	Assert(remote_sql->pdepth >= 0);
}

static inline void
remote_sql_check_sanity(RemoteSQLInfo *remote_sql)
{
	if (remote_sql->pdepth != 0)
	{
		ereport(ERROR, (errcode(ERRCODE_FDW_ERROR),
						errmsg("remote SQL has unclosed parenthesis:\n %s", remote_sql->buf->data)));
	}
}

static inline StringInfo
remote_sql_get_buffer(RemoteSQLInfo *remote_sql)
{
	return remote_sql->buf;
}

void
deparse_insert_sql(StringInfo buf, PlannerInfo *root, Index rtindex, Relation rel,
									 List *targetAttrs)
{
	ListCell *lc;
	RangeTblEntry *rte;

	rte = planner_rt_fetch(rtindex, root);

	appendStringInfo(buf, "INSERT INTO ");
	deparse_relation(buf, rel, false);

	if (targetAttrs) {
		bool first;

		appendStringInfoChar(buf, '(');

		first = true;
		foreach(lc, targetAttrs) {
			int attnum = lfirst_int(lc);
			if (!first)
				appendStringInfoString(buf, ", ");
			first = false;
			deparse_column_ref(buf, rtindex, attnum, rte, false);
		}

		appendStringInfoString(buf, ") VALUES (");

		first = true;
		foreach(lc, targetAttrs) {
			if (!first)
				appendStringInfoString(buf, ", ");
			first = false;

			appendStringInfo(buf, "?");
		}
		appendStringInfoChar(buf, ')');
	} else {
		Assert(false);
	}
}
