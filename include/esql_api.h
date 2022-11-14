/**
 * @file    esql_api.h
 * @brief   Header for external api functions for convenience of AP developers
 */

#ifndef _ESQL_API_H
#define _ESQL_API_H

#include "sqlcli_types.h"

#include "sqlca.h"

#ifndef ESQL_NULL_STR
#define ESQL_NULL_STR "\0"
#endif

/*
  * Function Name
      ESQLGetHandle()

  * Purpose
      Get current handles using Handle Type

  * Syntax
      SQLHANDLE tbesql_get_handle (SQLUSMALLINT handle_type, char *db_name,
                    sql_context *sql_ctx, unsigned int prefetch, int stmt_cache)

  * Parameter
      handle_type (IN)     - Type of the handle that AP attempts to get
      db_name     (IN)     - Name of DB(used when finding handle with db_name)
      sql_ctx     (IN)     - Address of sql_context(runtime context)
      prefetch    (IN)     - Number of rows pre-fetched at cursor OPEN time
      stmt_cache  (IN)     - Size of the statement cache
      sqlca       (IN/OUT) - SQL Communication Area for Errors

  * Comments
      This Function provides application developer to obtain currently used
      SQL handle for CLI. By using this function, application developers can
      use ODBC APIs.
      Parameter 'handle_type' can be one of following 2.
       [ SQL_HANDLE_DBC / SQL_HANDLE_STMT ]
            *  SQL_HANDLE_ENV/SQL_HANDLE_DESC not supported
      Parameter 'db_name' can be NULL String(ESQL_NULL_STR == "\0")
      Parameter 'sql_ctx'  can be NULL if there's no sql_context.
      Parameter 'prefetch' can be 1, if precompile option PREFETCH is 1(default)
      Parameter 'stmt_cache' can be 0, if precompile option STMT_CACHE is 0(default)

      If application developer give above parameters as attributes, the function
      will return Connection or Statement handle in SQLHANDLE type.
 */

extern SQLHANDLE ESQLGetHandle(SQLUSMALLINT handle_type, char *db_name,
                               void **sql_ctx, unsigned int prefetch,
                               int stmt_cache, struct sqlca *_sqlca);


#endif
