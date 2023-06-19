/*--------------------------------------------------------------------------------------------------
 *
 * tibero_fdw.h
 *			Foreign-data wrapper for remote Tibero servers
 *
 * Portions Copyright (c) 2022-2023, Tmax OpenSQL Research & Development Team
 *
 * IDENTIFICATION
 *			contrib/tibero_fdw/tibero_fdw.h
 *
 *--------------------------------------------------------------------------------------------------
 */
#ifndef TIBERO_FDW_H
#define TIBERO_FDW_H

#include "foreign/foreign.h"											/* ForeignServer, ForeignTable									*/
#include "lib/stringinfo.h"						  					/* StringInfo                         					*/
#include "nodes/pathnodes.h"											/* PlannerInfo, RelOptInfo											*/
#include "utils/rel.h"														/* Relation																			*/

#include "sqlcli.h"
#include "sqlcli_types.h"

typedef struct TbFdwRelationInfo
{
	bool pushdown_safe;

	List *remote_conds;
	List *local_conds;

	List *final_remote_exprs;

	Bitmapset *attrs_used;

	QualCost local_conds_cost;
	Selectivity local_conds_sel;

	double rows;
	int width;

	Cost startup_cost;
	Cost total_cost;

	bool use_remote_estimate;
	Cost fdw_startup_cost;
	Cost fdw_tuple_cost;

	ForeignTable *table;
	ForeignServer *server;
	UserMapping *user;

	int fetch_size;

	char *relation_name;

	/* Join information */
	RelOptInfo *outerrel;
	RelOptInfo *innerrel;
	JoinType	jointype;
	List	   *joinclauses;
	UpperRelationKind stage;

	bool use_fb_query;
	bool use_sleep_on_sig;
	bool updatable;
} TbFdwRelationInfo;

/* in conditions.c */
extern void classify_conditions(PlannerInfo *root, RelOptInfo *baserel, List *input_conds,
																List **remote_conds, List **local_conds);
extern bool expr_inspect_shippability(PlannerInfo *root, RelOptInfo *baserel, Expr *expr);

/* in deparse.c */
extern void deparse_select_stmt_for_rel(StringInfo buf, PlannerInfo *root, RelOptInfo *rel,
																				List *tlist, List *remote_conds, List *pathkeys,
																				bool has_final_sort, bool has_limit, bool is_subquery,
																				List **retrieved_attrs, List **params_list,
																				bool use_fb_query);
extern void deparse_insert_sql(StringInfo buf, PlannerInfo *root, Index rtindex, Relation rel,
															 List *targetAttrs);

/* in utils.c */
extern void register_signal_handlers(void);
extern void set_sleep_on_sig_on(void);
extern void set_sleep_on_sig_off(void);
extern bool check_oid_builtin(Oid object_id);

#endif							/* TIBERO_FDW_H */
