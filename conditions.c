/*--------------------------------------------------------------------------------------------------
 *
 * conditions.c
 *		Classifying which conditions to send to remote Tibero
 *
 * Portions Copyright (c) 2022-2023, Tmax OpenSQL Research & Development Team
 *
 * IDENTIFICATION
 *			contrib/tibero_fdw/conditions.c
 *
 *--------------------------------------------------------------------------------------------------
 */

#include "postgres.h"
#include "tibero_fdw.h"
#include "access/sysattr.h"
#include "access/transam.h"
#include "catalog/pg_collation.h"

typedef struct InspectionContext {
	PlannerInfo *root; /* TODO: Not sure where PlannerInfo is used internally */
	RelOptInfo *foreignrel;
	Relids relids;
	List *collation_list;
	bool shippable;
} InspectionContext;

typedef enum
{
	TB_FDW_EXPR_COLLATION_SAFE_TO_SHIP,
	TB_FDW_EXPR_COLLATION_NEED_INSPECTION,
	/* Up to here is safe to ship*/
	TB_FDW_EXPR_COLLATION_UNSAFE_TO_SHIP
} TbFDWCollationState;

static inline void initialize_inspection_context(InspectionContext *, PlannerInfo *, RelOptInfo *);
static inline void start_inspection(Node *, InspectionContext *);
static inline bool analyze_inspection_result(InspectionContext *);
static inline void clear_inspection_context(InspectionContext *);

static inline void inspect_for_T_Var(Node *, InspectionContext *);
static inline void inspect_for_T_Const(Node *, InspectionContext *);
static inline void inspect_for_T_Param(Node *, InspectionContext *);
static inline void inspect_for_T_FuncExpr(Node *, InspectionContext *);
static inline void inspect_for_T_OpExpr(Node *, InspectionContext *);
static inline void inspect_for_T_DistinctExpr(Node *, InspectionContext *);
static inline void inspect_for_T_BoolExpr(Node *, InspectionContext *);
static inline void inspect_for_T_NullTest(Node *, InspectionContext *);
static inline void inspect_for_T_ArrayExpr(Node *, InspectionContext *);
static inline void inspect_for_T_List(Node *, InspectionContext *);
static inline void inspect_for_T_Aggref(Node *, InspectionContext *);

/*
 * TODO
 * The following expression types are inspected in Postgres FDW
 * but it's not certain whether these should be shippable in Tibero FDW as well.
 * Need to check if we can ship these expressions to Tibero
 */
#if 0
static inline void inspect_for_T_SubscriptingRef(Node *, InspectionContext *);
static inline void inspect_for_T_ScalarArrayOpExpr(Node *, InspectionContext *);
static inline void inspect_for_T_RelabelType(Node *, InspectionContext *);
#endif

/* Other helper functions */
typedef struct FuncExprInfo {
	Oid func_oid;
	Oid input_collid;
	Oid func_collid;
	Node *args;
} FuncExprInfo;

static inline void inspect_for_T_FuncExpr_Internal(InspectionContext *, FuncExprInfo *);
static inline bool check_func_expr_compatible_with_tibero(Oid func_oid);
static inline bool check_aggr_expr_compatible_with_tibero(Oid aggr_oid);
static inline bool check_expr_builtin(Oid object_id);

void
classify_conditions(PlannerInfo *root, RelOptInfo *baserel, List *input_conds, List **remote_conds,
										List **local_conds)
{
	ListCell *lc;

	*remote_conds = NIL;
	*local_conds = NIL;

	foreach(lc, input_conds) {
		RestrictInfo *ri = lfirst_node(RestrictInfo, lc);

		if (expr_inspect_shippability(root, baserel, ri->clause))
			*remote_conds = lappend(*remote_conds, ri);
		else
			*local_conds = lappend(*local_conds, ri);
	}
}

bool
expr_inspect_shippability(PlannerInfo *root, RelOptInfo *baserel, Expr *expr)
{
	Node *expr_root = (Node *)expr;
	InspectionContext context;
	bool shippable;

	initialize_inspection_context(&context, root, baserel);
	start_inspection(expr_root, &context);

	shippable = analyze_inspection_result(&context);

	clear_inspection_context(&context);

	return shippable;
}

static inline void
initialize_inspection_context(InspectionContext *context, PlannerInfo *root, RelOptInfo *baserel)
{
	TbFdwRelationInfo *fpinfo = (TbFdwRelationInfo *)(baserel->fdw_private);

	context->root = root;
	context->foreignrel = baserel;
	/*
	 * For an upper relation (post-scan/join relations like aggregation or sort),
	 * its own relids aren't set to anything meaningful at this stage by the planner core,
	 * so use the outer relation's relids (driving table for join) instead in this case.
	 */
	context->relids = IS_UPPER_REL(baserel) ? fpinfo->outerrel->relids : baserel->relids;
	context->collation_list = NIL;
	context->shippable = true;
}

#define INSPECT_EXPR(type, expr, context) 																												 \
	do {																																														 \
		if (expr != NULL)																																							 \
		{																																															 \
			inspect_for_##type(expr, context);																										 			 \
		}																																															 \
	} while(0)

static inline void
start_inspection(Node *expr, InspectionContext *context)
{
	if (expr == NULL)
		return;

	switch (expr->type)
	{
		case T_Var:
			INSPECT_EXPR(T_Var, expr, context);
			break;
		case T_Const:
			INSPECT_EXPR(T_Const, expr, context);
			break;
		case T_Param:
			INSPECT_EXPR(T_Param, expr, context);
			break;
		case T_FuncExpr:
			INSPECT_EXPR(T_FuncExpr, expr, context);
			break;
		case T_OpExpr:
			INSPECT_EXPR(T_OpExpr, expr, context);
			break;
		case T_DistinctExpr:
			INSPECT_EXPR(T_DistinctExpr, expr, context);
			break;
		case T_BoolExpr:
			INSPECT_EXPR(T_BoolExpr, expr, context);
			break;
		case T_NullTest:
			INSPECT_EXPR(T_NullTest, expr, context);
			break;
		case T_ArrayExpr:
			INSPECT_EXPR(T_ArrayExpr, expr, context);
			break;
		case T_List:
			INSPECT_EXPR(T_List, expr, context);
			break;
		case T_Aggref:
			INSPECT_EXPR(T_Aggref, expr, context);
			break;
		default:
			/* others are not shippable */
			context->shippable = false;
	}
}

static inline bool
analyze_inspection_result(InspectionContext *context)
{
	ListCell *cell;
	TbFDWCollationState state = TB_FDW_EXPR_COLLATION_SAFE_TO_SHIP;
	Oid collation = InvalidOid;

	/* shortcut if shippability was determined in inspection stage*/
	if (!context->shippable)
	{
		return false;
	}

	foreach(cell, context->collation_list)
	{
		Oid cur_collation = lfirst_oid(cell);
		TbFDWCollationState cur_state = TB_FDW_EXPR_COLLATION_SAFE_TO_SHIP;

		if (!OidIsValid(collation))
		{
			cur_state = TB_FDW_EXPR_COLLATION_SAFE_TO_SHIP;
		}
		else if (state == TB_FDW_EXPR_COLLATION_NEED_INSPECTION && cur_collation == collation)
		{
			cur_state = TB_FDW_EXPR_COLLATION_NEED_INSPECTION;
		}
		else if (cur_collation == DEFAULT_COLLATION_OID)
		{
			cur_state = TB_FDW_EXPR_COLLATION_SAFE_TO_SHIP;
		}
		else
		{
			cur_state = TB_FDW_EXPR_COLLATION_UNSAFE_TO_SHIP;
		}

		if (cur_state > state)
		{
			collation = cur_collation;
			state = cur_state;
		}
		else if (cur_state == state && cur_state == TB_FDW_EXPR_COLLATION_NEED_INSPECTION)
		{
			if (cur_collation != collation)
			{
				if (collation == DEFAULT_COLLATION_OID)
				{
					collation = cur_collation;
				}
				else if (cur_collation != DEFAULT_COLLATION_OID)
				{
					state = TB_FDW_EXPR_COLLATION_UNSAFE_TO_SHIP;
				}
			}
		}
	}

	return state != TB_FDW_EXPR_COLLATION_UNSAFE_TO_SHIP;
}

static inline void
clear_inspection_context(InspectionContext *context)
{
	list_free(context->collation_list);
	context->collation_list = NIL;
}

static inline void
inspect_for_T_Var(Node *expr, InspectionContext *context)
{
	Var *var = (Var *)expr;

	if (bms_is_member(var->varno, context->relids) && var->varlevelsup == 0)
	{
		if (var->varattno < 0 && var->varattno != SelfItemPointerAttributeNumber)
		{
			context->shippable = false;
		}
	}
	context->collation_list = lappend_oid(context->collation_list, var->varcollid);
}

static inline void
inspect_for_T_Const(Node *expr, InspectionContext *context)
{
	Const *constant = (Const *)expr;
	context->collation_list = lappend_oid(context->collation_list, constant->constcollid);
}

static inline void
inspect_for_T_Param(Node *expr, InspectionContext *context)
{
	Param *param = (Param *)expr;

	if (param->paramkind == PARAM_MULTIEXPR)
	{
		context->shippable = false;
	}

	context->collation_list = lappend_oid(context->collation_list, param->paramcollid);
}

static inline void
inspect_for_T_FuncExpr(Node *expr, InspectionContext *context)
{
	FuncExpr *func = (FuncExpr *)expr;
	FuncExprInfo func_info = {
		func->funcid,
		func->inputcollid,
		func->funccollid,
		(Node *)func->args
	};

	inspect_for_T_FuncExpr_Internal(context, &func_info);
}

static inline void
inspect_for_T_FuncExpr_Internal(InspectionContext *context, FuncExprInfo *func_info)
{
	if (!check_func_expr_compatible_with_tibero(func_info->func_oid))
	{
		context->shippable = false;
		return;
	}

	/* recursively inspect function arguments */
	start_inspection((Node *)func_info->args, context);
	context->collation_list = lappend_oid(context->collation_list, func_info->input_collid);
	context->collation_list = lappend_oid(context->collation_list, func_info->func_collid);
}

static inline bool
check_func_expr_compatible_with_tibero(Oid func_oid)
{
	if (!check_expr_builtin(func_oid))
	{
		return false;
	}

	/* TODO: Which PG builtin functions are compatible with Tibero? */
	return true;
}

static inline bool
check_expr_builtin(Oid object_id)
{
	return (object_id < FirstGenbkiObjectId);
}

static inline void
inspect_for_T_OpExpr(Node *expr, InspectionContext *context)
{
	OpExpr *operator = (OpExpr *)expr;
	FuncExprInfo func_info = {
		operator->opno,
		operator->inputcollid,
		operator->opcollid,
		(Node *)operator->args
	};

	inspect_for_T_FuncExpr_Internal(context, &func_info);
}

static inline void
inspect_for_T_DistinctExpr(Node *expr, InspectionContext *context)
{
	inspect_for_T_OpExpr(expr, context);
}

static inline void
inspect_for_T_BoolExpr(Node *expr, InspectionContext *context)
{
	BoolExpr *bool_expr = (BoolExpr *)expr;

	/* Recursively inspect subexpressions */
	start_inspection((Node *)bool_expr->args, context);

	/* Output is alwasy boolean and so noncollatable */
	context->collation_list = lappend_oid(context->collation_list, InvalidOid);
}

static inline void
inspect_for_T_NullTest(Node *expr, InspectionContext *context)
{
	NullTest *null_test = (NullTest *)expr;

	/* Recursively inspect subexpressions */
	start_inspection((Node *)null_test->arg, context);

	/* Output is always boolean and so noncollatable */
	context->collation_list = lappend_oid(context->collation_list, InvalidOid);
}

static inline void
inspect_for_T_ArrayExpr(Node *expr, InspectionContext *context)
{
	ArrayExpr *array_expr = (ArrayExpr *)expr;

	/* Recursively inspect subexpressions */
	start_inspection((Node *)array_expr->elements, context);

	context->collation_list = lappend_oid(context->collation_list, array_expr->array_collid);
}

static inline void
inspect_for_T_List(Node *expr, InspectionContext *context)
{
	List *list = (List *)expr;
	ListCell *cell;

	/* Recursively inspect subexpressions */
	foreach(cell, list)
	{
		start_inspection((Node *)lfirst(cell), context);
	}

	/* No need to append List expression's collation */
}

static inline void
inspect_for_T_Aggref(Node *expr, InspectionContext *context)
{
	Aggref *aggr = (Aggref *)expr;

	if (!IS_UPPER_REL(context->foreignrel))
	{
		context->shippable = false;
		return;
	}

	/* Only non-split aggregates are shippable */
	if (aggr->aggsplit != AGGSPLIT_SIMPLE)
	{
		context->shippable = false;
		return;
	}

	if (check_aggr_expr_compatible_with_tibero(aggr->aggfnoid))
	{
		context->shippable = false;
		return;
	}

	/* TODO: Recursing into input args */
	context->shippable = false;
}

static inline bool
check_aggr_expr_compatible_with_tibero(Oid aggr_oid)
{
	if (!check_expr_builtin(aggr_oid))
	{
		return false;
	}

	/* TODO: Which PG builtin aggregates are compatible with Tibero? */
	return false;
}
