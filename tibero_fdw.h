/*-------------------------------------------------------------------------
 *
 * tibero_fdw.h
 *		  Foreign-data wrapper for remote Tibero servers
 *
 * Portions Copyright (c) 2022-2023, HyperSQL Research & Development Team
 *
 * IDENTIFICATION
 *		  contrib/tibero_fdw/tibero_fdw.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef TIBERO_FDW_H
#define TIBERO_FDW_H

#include "foreign/foreign.h"            /* ForeignServer, ForeignTable        */
#include "lib/stringinfo.h"             /* StringInfo                         */
#include "nodes/pathnodes.h"            /* PlannerInfo, RelOptInfo            */

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
  List *shippable_extensions;

  ForeignTable *table;
  ForeignServer *server;
  UserMapping *user;

  int fetch_size;

  char *relation_name;
} TbFdwRelationInfo;

/* in deparse.c */
extern void classify_conditions(PlannerInfo *root, RelOptInfo *baserel,
                               List *input_conds, List **remote_conds,
							                 List **local_conds);
extern bool is_foreign_expr(PlannerInfo *root, RelOptInfo *baserel, Expr *expr);
extern void deparse_select_stmt_for_rel(StringInfo buf, PlannerInfo *root, 
                                        RelOptInfo *rel, List *tlist, List *remote_conds, 
                                        List *pathkeys, bool has_final_sort, 
                                        bool has_limit, bool is_subquery,
                                        List **retrieved_attrs, List **params_list);

/* in tibero_fdw.c */
extern int	set_transmission_modes(void);
extern void reset_transmission_modes(int nestlevel);

#endif							/* TIBERO_FDW_H */
