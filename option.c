/*--------------------------------------------------------------------------------------------------
 *
 * option.c
 *		Tibero FDW option definition and its validator implementation
 *
 * Portions Copyright (c) 2022-2023, Tmax OpenSQL Research & Development Team
 *
 * IDENTIFICATION
 *			contrib/tibero_fdw/option.c
 *
 *--------------------------------------------------------------------------------------------------
 */

#include "postgres.h"
#include "tibero_fdw.h"

#include "access/reloptions.h"
#include "catalog/pg_foreign_server.h"
#include "catalog/pg_foreign_table.h"
#include "catalog/pg_user_mapping.h"
#include "commands/defrem.h"
#include "nodes/value.h"
#include "utils/builtins.h"
#include "utils/guc.h"

typedef struct TbFdwOption
{
	const char *keyword;
	void (*validate)(DefElem *def);
	bool hidden;
	bool required;
	bool configured;
} TbFdwOption;

#define TB_FDW_OPTION(kw, hidden, required) {#kw, validate_##kw##_option, hidden, required, false}
#define TB_FDW_OPTION_ARRAY_END {NULL, NULL, false, false, false}
/* TODO: maximum value for fetch_size? */
#define TB_FDW_MAX_FETCH_SIZE INT32_MAX

static inline void validate_foreign_server_options(const List *input);
static inline void validate_foreign_table_options(const List *input);
static inline void validate_user_mapping_options(const List *input);

static inline void validate_options(TbFdwOption *registered, const List *input);
static inline bool search_for_option_in_registered(TbFdwOption *registered, DefElem *def);
static inline void ereport_invalid_option(TbFdwOption *registered, const char *option);
static inline void check_sufficient_input_options(TbFdwOption *registered);

static void validate_host_option(DefElem *def);
static void validate_port_option(DefElem *def);
static void validate_dbname_option(DefElem *def);
static void validate_fetch_size_option(DefElem *def);
static void validate_username_option(DefElem *def);
static void validate_password_option(DefElem *def);
static void validate_owner_name_option(DefElem *def);
static void validate_table_name_option(DefElem *def);
static void validate_use_sleep_on_sig_option(DefElem *def);
static void validate_use_fb_query_option(DefElem *def);
static void validate_keep_connections_option(DefElem *def);
static void validate_password_required_option(DefElem *def);
static void validate_updatable_option(DefElem *def);

static inline char * get_str_value_with_null_check(DefElem *def);
static inline bool get_bool_value_with_null_check(DefElem *def);

PG_FUNCTION_INFO_V1(tibero_fdw_validator);

/*
	tibero_fdw_validator is called when there is a change in options with Tibero FDW objects. The
	first argument is the list of options configured to the object, and the second argument is the
	identification of the target object (e.g., server, user mapping, foreign table).

	Note that the first argument is not the list of options included in the DDL statement. It is the
	list of options that are configured in the result of the DDL statement. For example,

		CREATE SERVER tibero_server FOREIGN DATA WRAPPER tibero_fdw
		OPTIONS (option1 'value1', option2 'value2');

	the statement above passes (option1, value1), (option2, value2) as argument. On the other hand,

		ALTER SERVER tibero_server OPTIONS (DROP option1);

	the statement above passes (option2, value2) as arguement, not (option1, value2). This remark
	is important in the implementation of checking for required options. Since the passed argument
	is the result options, we only need to check whether the input options configure the required
	options or not.
*/

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
	TbFdwOption foreign_server_options[] = {
		TB_FDW_OPTION(host, false, true),
		TB_FDW_OPTION(port, false, true),
		TB_FDW_OPTION(dbname, false, true),
		TB_FDW_OPTION(fetch_size, false, false),
		TB_FDW_OPTION(use_sleep_on_sig, true, false),
		TB_FDW_OPTION(use_fb_query, true, false),
		TB_FDW_OPTION(keep_connections, true, false),
		TB_FDW_OPTION(updatable, true, false),
		TB_FDW_OPTION_ARRAY_END
	};
	validate_options(foreign_server_options, options);
}

static inline void
validate_foreign_table_options(const List *options)
{
	TbFdwOption foreign_table_options[] = {
		TB_FDW_OPTION(owner_name, false, false),
		TB_FDW_OPTION(table_name, false, true),
		TB_FDW_OPTION(fetch_size, false, false),
		TB_FDW_OPTION(use_fb_query, true, false),
		TB_FDW_OPTION(updatable, true, false),
		TB_FDW_OPTION_ARRAY_END
	};
	validate_options(foreign_table_options, options);
}

static inline void
validate_user_mapping_options(const List *options)
{
	TbFdwOption foreign_server_options[] = {
		TB_FDW_OPTION(username, false, true),
		TB_FDW_OPTION(password, false, true),
		TB_FDW_OPTION(password_required, true, false),
		TB_FDW_OPTION_ARRAY_END
	};
	validate_options(foreign_server_options, options);
}

static inline void
validate_options(TbFdwOption *registered, const List *input)
{
	ListCell *cell;
	foreach(cell, input)
	{
		DefElem *def = (DefElem *)lfirst(cell);
		bool is_valid_option = search_for_option_in_registered(registered, def);

		if (!is_valid_option)
		{
			ereport_invalid_option(registered, def->defname);
		}
	}

	check_sufficient_input_options(registered);
}

static inline bool
search_for_option_in_registered(TbFdwOption *registered, DefElem *def)
{
	TbFdwOption *iter;
	bool found = false;

	for (iter = registered; iter->keyword; iter++) {
		if (strcmp(def->defname, iter->keyword) == 0) {
			/* validate function throws error if invalid */
			iter->validate(def);
			iter->configured = true;
			found = true;
			break;
		}
	}

	return found;
}

static inline void
ereport_invalid_option(TbFdwOption *registered, const char *option_name)
{
	TbFdwOption *iter;
	StringInfoData error_hint_buf;

	initStringInfo(&error_hint_buf);

	for (iter = registered; iter->keyword; iter++) {
		if (!iter->hidden) {
			const char *delimiter = (error_hint_buf.len > 0) ? ", " : "";
			appendStringInfo(&error_hint_buf, "%s%s", delimiter, iter->keyword);
		}
	}

	ereport(ERROR,
		(errcode(ERRCODE_FDW_INVALID_OPTION_NAME),
		errmsg("invalid option \"%s\"", option_name),
		errhint("Valid options in this context are: %s", error_hint_buf.data)));
}

static inline void
check_sufficient_input_options(TbFdwOption *registered)
{
	TbFdwOption *iter;
	StringInfoData error_hint_buf;
	bool insufficient = false;
	initStringInfo(&error_hint_buf);

	for (iter = registered; iter->keyword; iter++) {
		if (iter->required && !iter->configured) {
			const char *delimiter = (error_hint_buf.len > 0) ? ", " : "";
			appendStringInfo(&error_hint_buf, "%s%s", delimiter, iter->keyword);
			insufficient = true;
		}
	}

	if (insufficient) {
			ereport(ERROR,
				(errcode(ERRCODE_FDW_ERROR),
				errmsg("insufficient options"),
				errhint("The following required options are missing: %s",error_hint_buf.data)));
	}
}

static void
validate_host_option(DefElem *def)
{
	(void) get_str_value_with_null_check(def);
}

static void
validate_port_option(DefElem *def)
{
	(void) get_str_value_with_null_check(def);
}

static void
validate_dbname_option(DefElem *def)
{
	(void) get_str_value_with_null_check(def);
}

static void
validate_fetch_size_option(DefElem *def)
{
	char *value;
	int int_val;

	value = get_str_value_with_null_check(def);

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

	if (int_val >= TB_FDW_MAX_FETCH_SIZE)
	{
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("\"%s\" exceeded the maximum value", def->defname)));
	}
}

static void
validate_username_option(DefElem *def)
{
	(void) get_str_value_with_null_check(def);
}

static void
validate_password_option(DefElem *def)
{
	(void) get_str_value_with_null_check(def);
}

static void
validate_password_required_option(DefElem *def)
{
	(void) get_bool_value_with_null_check(def);
}

static void
validate_updatable_option(DefElem *def)
{
	(void) get_bool_value_with_null_check(def);
}

static void
validate_owner_name_option(DefElem *def)
{
	(void) get_str_value_with_null_check(def);
}

static void
validate_table_name_option(DefElem *def)
{
	(void) get_str_value_with_null_check(def);
}

static void
validate_use_sleep_on_sig_option(DefElem *def)
{
	(void) get_bool_value_with_null_check(def);
}

static void
validate_use_fb_query_option(DefElem *def)
{
	(void) get_bool_value_with_null_check(def);
}

static void
validate_keep_connections_option(DefElem *def)
{
	(void) get_bool_value_with_null_check(def);
}

static inline char *
get_str_value_with_null_check(DefElem *def)
{
	char *value = defGetString(def);

	if (strcmp(value, "") == 0)
	{
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("\"%s\" requires non-empty value", def->defname)));
	}

	return value;
}

static inline bool
get_bool_value_with_null_check(DefElem *def)
{
	char *str_value = strVal(def->arg);

	if (strcmp(str_value, "") == 0)
	{
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("\"%s\" requires non-empty value", def->defname)));
	}

	return defGetBoolean(def);
}
