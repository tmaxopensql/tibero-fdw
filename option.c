/*------------------------------------------------------------------------------
 *
 * option.c
 *		Tibero FDW option definition and its validator implementation
 *
 * Portions Copyright (c) 2022-2023, Tmax OpenSQL Research & Development Team
 *
 * IDENTIFICATION
 *			contrib/tibero_fdw/option.c
 *
 *------------------------------------------------------------------------------
 */

#include "postgres.h"
#include "tibero_fdw.h"

#include "access/reloptions.h"
#include "catalog/pg_foreign_server.h"
#include "catalog/pg_foreign_table.h"
#include "catalog/pg_user_mapping.h"
#include "commands/defrem.h"
#include "utils/builtins.h"
#include "utils/guc.h"

typedef struct TiberoFdwOption
{
  const char *keyword;
  void (*validate)(DefElem *def);
  bool hidden;
} TiberoFdwOption;

static inline void validate_foreign_server_options(const List *input);
static inline void validate_foreign_table_options(const List *input);
static inline void validate_user_mapping_options(const List *input);

static inline void validate_options(const TiberoFdwOption *registered, const List *input);

static void validate_host_option(DefElem *def);
static void validate_port_option(DefElem *def);
static void validate_dbname_option(DefElem *def);
static void validate_fetch_size_option(DefElem *def);
static void validate_username_option(DefElem *def);
static void validate_password_option(DefElem *def);
static void validate_table_name_option(DefElem *def);
static void validate_sleep_on_sig_option(DefElem *def);
static void validate_use_fb_query_option(DefElem *def);

PG_FUNCTION_INFO_V1(tibero_fdw_validator);

Datum
tibero_fdw_validator(PG_FUNCTION_ARGS)
{
  List *options = untransformRelOptions(PG_GETARG_DATUM(0));
  Oid target_object = PG_GETARG_DATUM(1);

  switch (target_object)
  {
    case ForeignServerRelationId:
      validate_foreign_server_options(options);
      break;
    case ForeignTableRelationId:
      validate_foreign_table_options(options);
      break;
    case UserMappingRelationId:
      validate_user_mapping_options(options);
      break;
    default:
      Assert(false && "tibero_fdw_validator function is called by invalid relation");
  }

  PG_RETURN_VOID();
}

static inline void
validate_foreign_server_options(const List *options)
{
  const TiberoFdwOption foreign_server_options[] = {
    {"host", validate_host_option, false},
    {"port", validate_port_option, false},
    {"dbname", validate_dbname_option, false},
    {"fetch_size", validate_fetch_size_option, false},
    {"sleep_on_sig", validate_sleep_on_sig_option, true},
    {"use_fb_query", validate_use_fb_query_option, true},
    {NULL, NULL}
  };
  validate_options(foreign_server_options, options);
}

static inline void
validate_foreign_table_options(const List *options)
{
  const TiberoFdwOption foreign_table_options[] = {
    {"table_name", validate_table_name_option, false},
    {"fetch_size", validate_fetch_size_option, false},
    {"use_fb_query", validate_use_fb_query_option, true},
    {NULL, NULL}
  };
  validate_options(foreign_table_options, options);
}

static inline void
validate_user_mapping_options(const List *options)
{
  const TiberoFdwOption foreign_server_options[] = {
    {"username", validate_username_option, false},
    {"password", validate_password_option, false},
    {NULL, NULL}
  };
  validate_options(foreign_server_options, options);
}

static inline void
validate_options(const TiberoFdwOption *registered, const List *input)
{
  ListCell *cell;

  foreach(cell, input)
  {
    DefElem *def = (DefElem *)lfirst(cell);
    TiberoFdwOption const *iter;
    bool found = false;

    for (iter = registered; iter->keyword; iter++)
    {
      if (strcmp(def->defname, iter->keyword) == 0)
      {
        /* validate function throws error if invalid */
        iter->validate(def);
        found = true;
      }
    }

    /* if reached here, indicates input option doesn't exist */
    if (!found)
    {
      StringInfoData error_hint_buf;
      initStringInfo(&error_hint_buf);

      for (iter = registered; iter->keyword; iter++)
      {
        if (!iter->hidden)
        {
          const char *delimiter = (error_hint_buf.len > 0) ? ", " : "";
          appendStringInfo(&error_hint_buf, "%s%s", delimiter, iter->keyword);
        }
      }

      ereport(ERROR,
              (errcode(ERRCODE_FDW_INVALID_OPTION_NAME),
               errmsg("invalid option \"%s\"", def->defname),
               errhint("Valid options in this context are: %s", error_hint_buf.data)));
    }
  }
}

static void
validate_host_option(DefElem *def)
{
  (void) defGetString(def);
}

static void
validate_port_option(DefElem *def)
{
  (void) defGetString(def);
}

static void
validate_dbname_option(DefElem *def)
{
  (void) defGetString(def);
}

static void
validate_fetch_size_option(DefElem *def)
{
  char *value;
  int int_val;

  value = defGetString(def);

  if (!parse_int(value, &int_val, 0, NULL))
  {
    ereport(ERROR,
            (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
             errmsg("invalid value for integer option \"%s\": %s", def->defname, value)));
  }

  if (int_val <= 0)
  {
    ereport(ERROR,
            (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
             errmsg("\"%s\" must be an integer value greater than zero", def->defname)));
  }

  /* TODO: maximum value for fetch_size? */
  if (int_val >= INT32_MAX)
  {
    ereport(ERROR,
            (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
             errmsg("\"%s\" exceeded the maximum value", def->defname)));
  }
}

static void
validate_username_option(DefElem *def)
{
  (void) defGetString(def);
}

static void
validate_password_option(DefElem *def)
{
  (void) defGetString(def);
}

static void
validate_table_name_option(DefElem *def)
{
  (void) defGetString(def);
}

static void
validate_sleep_on_sig_option(DefElem *def)
{
  (void) defGetBoolean(def);
}

static void
validate_use_fb_query_option(DefElem *def)
{
  (void) defGetBoolean(def);
}