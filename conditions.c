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
#include "catalog/pg_collation.h"
#include "catalog/pg_operator.h"
#include "optimizer/optimizer.h"

typedef enum
{
	TB_FDW_EXPR_COLLATION_SAFE_TO_SHIP,
	TB_FDW_EXPR_COLLATION_NEED_INSPECTION,
	/* Up to here is safe to ship*/
	TB_FDW_EXPR_COLLATION_UNSAFE_TO_SHIP
} TbFDWCollationState;

typedef struct InspectionContext {
	PlannerInfo *root; /* TODO: Not sure where PlannerInfo is used internally */
	RelOptInfo *foreignrel;
	Relids relids;
	Oid collation;
	TbFDWCollationState collation_state;
	bool shippable;
} InspectionContext;

static inline void initialize_inspection_context(InspectionContext *, PlannerInfo *, RelOptInfo *);
static inline void start_inspection(Node *, InspectionContext *);

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
	Node *expr;
	Node *args;
} FuncExprInfo;

static inline void inspect_for_T_FuncExpr_internal(InspectionContext *, FuncExprInfo *);

static inline bool check_func_expr_compatible_with_tibero(FuncExprInfo *func_oid);
static inline bool check_aggr_expr_compatible_with_tibero(Oid aggr_oid);

static inline void compare_collation_with_current_state(InspectionContext *, Oid);
static inline TbFDWCollationState deduce_collation_state_from_collation(InspectionContext *, Oid);
static inline bool check_for_need_to_override_collation(TbFDWCollationState, TbFDWCollationState);
static inline bool check_for_collation_conflict(InspectionContext *, TbFDWCollationState, Oid);

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

	initialize_inspection_context(&context, root, baserel);
	start_inspection(expr_root, &context);

	return context.shippable;
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
	context->collation = InvalidOid;
	context->collation_state = TB_FDW_EXPR_COLLATION_SAFE_TO_SHIP;
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

	if (!context->shippable)
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
			/* FIXME: develop deparsing logic and turn on the flag */
			context->shippable = false;
			break;
		case T_FuncExpr:
			INSPECT_EXPR(T_FuncExpr, expr, context);
			/* FIXME: develop deparsing logic and turn on the flag */
			context->shippable = false;
			break;
		case T_OpExpr:
			INSPECT_EXPR(T_OpExpr, expr, context);
			break;
		case T_DistinctExpr:
			INSPECT_EXPR(T_DistinctExpr, expr, context);
			/* FIXME: develop deparsing logic and turn on the flag */
			context->shippable = false;
			break;
		case T_BoolExpr:
			INSPECT_EXPR(T_BoolExpr, expr, context);
			/* FIXME: develop deparsing logic and turn on the flag */
			context->shippable = false;
			break;
		case T_NullTest:
			INSPECT_EXPR(T_NullTest, expr, context);
			/* FIXME: develop deparsing logic and turn on the flag */
			context->shippable = false;
			break;
		case T_ArrayExpr:
			INSPECT_EXPR(T_ArrayExpr, expr, context);
			/* FIXME: develop deparsing logic and turn on the flag */
			context->shippable = false;
			break;
		case T_List:
			INSPECT_EXPR(T_List, expr, context);
			break;
		case T_Aggref:
			INSPECT_EXPR(T_Aggref, expr, context);
			/* FIXME: develop deparsing logic and turn on the flag */
			context->shippable = false;
			break;
		default:
			/* others are not shippable */
			context->shippable = false;
	}
}

static inline void
inspect_for_T_Var(Node *expr, InspectionContext *context)
{
	Var *var = (Var *)expr;

	if (bms_is_member(var->varno, context->relids) && var->varlevelsup == 0)
	{
		/* System attributes are not shippable except for ROWID (CTID) */
		if (var->varattno < 0 && var->varattno != SelfItemPointerAttributeNumber)
		{
			context->shippable = false;
		}

		/* Whole row reference is not shippable */
		if (var->varattno == 0)
		{
			context->shippable = false;
		}
	}

	compare_collation_with_current_state(context, var->varcollid);
}

static inline void
inspect_for_T_Const(Node *expr, InspectionContext *context)
{
	Const *constant = (Const *)expr;
	compare_collation_with_current_state(context, constant->constcollid);
}

static inline void
inspect_for_T_Param(Node *expr, InspectionContext *context)
{
	Param *param = (Param *)expr;

	if (param->paramkind == PARAM_MULTIEXPR)
	{
		context->shippable = false;
	}

	compare_collation_with_current_state(context, param->paramcollid);
}

static inline void
inspect_for_T_FuncExpr(Node *expr, InspectionContext *context)
{
	FuncExpr *func = (FuncExpr *)expr;
	FuncExprInfo func_info = {
		func->funcid,
		func->inputcollid,
		func->funccollid,
		expr,
		(Node *)func->args
	};

	inspect_for_T_FuncExpr_internal(context, &func_info);
}

static inline void
inspect_for_T_FuncExpr_internal(InspectionContext *context, FuncExprInfo *func_info)
{
	if (!check_func_expr_compatible_with_tibero(func_info))
	{
		context->shippable = false;
		return;
	}

	if (contain_mutable_functions(func_info->expr))
	{
		context->shippable = false;
		return;
	}

	/* recursively inspect function arguments */
	start_inspection((Node *)func_info->args, context);
	compare_collation_with_current_state(context, func_info->input_collid);
	compare_collation_with_current_state(context, func_info->func_collid);
}

/* TODO: Cannot find the following OID in pg_operator macros */
#define TB_FDW_OID_MODULO_OP 530

static inline bool
check_func_expr_compatible_with_tibero(FuncExprInfo *func_info)
{
	bool compatible = true;

	if (!check_oid_builtin(func_info->func_oid))
	{
		return false;
	}

	/* TODO: Which PG builtin functions are compatible with Tibero? */
	switch (func_info->func_oid)
	{
		/* PostgreSQL's regexp operator is '~' whereas Tibero uses REGEXP function */
		case OID_BPCHAR_REGEXEQ_OP:
		/* PostgreSQL's modulo operator is '%' whereas Tibero uses MOD function */
		case TB_FDW_OID_MODULO_OP:
			compatible = false;
			break;
		default:
			/* compatible */
			break;
	}
	return compatible;
}

static inline void
inspect_for_T_OpExpr(Node *expr, InspectionContext *context)
{
	OpExpr *operator = (OpExpr *)expr;
	FuncExprInfo func_info = {
		operator->opno,
		operator->inputcollid,
		operator->opcollid,
		expr,
		(Node *)operator->args
	};

	inspect_for_T_FuncExpr_internal(context, &func_info);
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
	compare_collation_with_current_state(context, InvalidOid);
}

static inline void
inspect_for_T_NullTest(Node *expr, InspectionContext *context)
{
	NullTest *null_test = (NullTest *)expr;

	/* Recursively inspect subexpressions */
	start_inspection((Node *)null_test->arg, context);

	/* Output is always boolean and so noncollatable */
	compare_collation_with_current_state(context, InvalidOid);
}

static inline void
inspect_for_T_ArrayExpr(Node *expr, InspectionContext *context)
{
	ArrayExpr *array_expr = (ArrayExpr *)expr;

	/* Recursively inspect subexpressions */
	start_inspection((Node *)array_expr->elements, context);

	compare_collation_with_current_state(context, array_expr->array_collid);
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

	if (contain_mutable_functions(expr))
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
	if (!check_oid_builtin(aggr_oid))
	{
		return false;
	}

	/* TODO: Which PG builtin aggregates are compatible with Tibero? */
	return false;
}

static inline void
compare_collation_with_current_state(InspectionContext *context, Oid collation)
{
	TbFDWCollationState collation_state = deduce_collation_state_from_collation(context, collation);

	if (check_for_need_to_override_collation(collation_state, context->collation_state))
	{
		context->collation = collation;
		context->collation_state = collation_state;
	}
	else if (check_for_collation_conflict(context, collation_state, collation))
	{
		if (context->collation == DEFAULT_COLLATION_OID)
		{
			context->collation = collation;
		}
		else if (collation != DEFAULT_COLLATION_OID)
		{
			context->collation_state = TB_FDW_EXPR_COLLATION_UNSAFE_TO_SHIP;
		}
	}

	context->shippable &= (context->collation_state != TB_FDW_EXPR_COLLATION_UNSAFE_TO_SHIP);
}

static inline TbFDWCollationState
deduce_collation_state_from_collation(InspectionContext *context, Oid collation)
{
	TbFDWCollationState result_state = TB_FDW_EXPR_COLLATION_SAFE_TO_SHIP;
	bool compare_collations = (context->collation_state == TB_FDW_EXPR_COLLATION_NEED_INSPECTION);

	if (!OidIsValid(collation))
	{
		result_state = TB_FDW_EXPR_COLLATION_SAFE_TO_SHIP;
	}
	else if (compare_collations && collation == context->collation)
	{
		result_state = TB_FDW_EXPR_COLLATION_NEED_INSPECTION;
	}
	else if (collation == DEFAULT_COLLATION_OID)
	{
		result_state = TB_FDW_EXPR_COLLATION_SAFE_TO_SHIP;
	}
	else
	{
		result_state = TB_FDW_EXPR_COLLATION_UNSAFE_TO_SHIP;
	}

	return result_state;
}

static inline bool
check_for_need_to_override_collation(TbFDWCollationState state, TbFDWCollationState cur_state)
{
	return state > cur_state;
}

static inline bool
check_for_collation_conflict(InspectionContext *context, TbFDWCollationState state, Oid collation)
{
	return (state == context->collation_state)
				&& (state == TB_FDW_EXPR_COLLATION_NEED_INSPECTION)
				&& (collation != context->collation);
}
