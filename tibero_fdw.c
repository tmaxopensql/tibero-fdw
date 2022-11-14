#include <postgres.h>
#include <fmgr.h>

#include <limits.h>

#include "access/htup_details.h"
#include "access/sysattr.h"
#include "access/table.h"
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

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(tibero_fdw_handler);
PG_FUNCTION_INFO_V1(tibero_fdw_validator);

/*
 * FDW callback routines
 */
static void tiberoGetForeignRelSize(PlannerInfo *root,
                                    RelOptInfo *baserel,
                                    Oid foreigntableid);

static void tiberoGetForeignPaths(PlannerInfo *root,
                                  RelOptInfo *baserel,
                                  Oid foreigntableid);

static ForeignScan *tiberoGetForeignPlan(PlannerInfo *root,
                                         RelOptInfo *foreignrel,
                                         Oid foreigntableid,
                                         ForeignPath *best_path,
                                         List *tlist,
                                         List *scan_clauses,
                                         Plan *outer_plan);

static void tiberoBeginForeignScan(ForeignScanState *node, int eflags);


Datum tibero_fdw_validator(PG_FUNCTION_ARGS)
{

  /*
   validate the generic optios given to a FOREIGNDATA WRAPPER, SERVER,
   USER MAPPING or FOREIGN TABLE that uses in tibero_fdw

   */
}

Datum tibero_fdw_handler(PG_FUNCTION_ARGS)
{

  FdwRoutine *fdwroutine = makeNode(FdwRoutine);

  /*for scan*/
  fdwroutine->GetForeignRelSize = tiberoGetForeignRelSize;
  fdwroutine->GetForeignPaths = tiberoGetForeignPaths;
  fdwroutine->GetForeignPlan = tiberoGetForeignPlan;
  fdwroutine->BeginForeignScan = tiberoBeginForeignScan;

  PG_RETURN_POINTER(fdwroutine);
}

static void tiberoGetForeignRelSize(PlannerInfo *root,
                                    RelOptInfo *baserel,
                                    Oid foreigntableid)
{
}

static void tiberoGetForeignPaths(PlannerInfo *root,
                                  RelOptInfo *baserel,
                                  Oid foreigntableid)
{
}

static ForeignScan *tiberoGetForeignPlan(PlannerInfo *root,
                                         RelOptInfo *foreignrel,
                                         Oid foreigntableid,
                                         ForeignPath *best_path,
                                         List *tlist,
                                         List *scan_clauses,
                                         Plan *outer_plan)
{
}

static void tiberoBeginForeignScan(ForeignScanState *node, int eflags)
{
}
