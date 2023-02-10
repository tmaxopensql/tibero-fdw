/*-------------------------------------------------------------------------
 *
 * connection.h
 *		  Connection management functions for tibero_fdw
 *
 * Portions Copyright (c) 2022-2023, HyperSQL Research & Development Team
 *
 * IDENTIFICATION
 *		  contrib/tibero_fdw/connection.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef TIBERO_FDW_CONNECTION_H
#define TIBERO_FDW_CONNECTION_H

#include "sqlcli.h"
#include "sqlcli_types.h"

typedef Oid ConnCacheKey;

typedef struct ConnCacheEntry
{
  ConnCacheKey key;

  SQLHANDLE henv;
  SQLHANDLE hdbc;

  bool connected;

  bool begin_remote_xact;

  bool invalidated;
  bool keep_connections;

  Oid serverid;
  uint32 server_hashvalue;
  uint32 mapping_hashvalue;

  char tsn[32];

	TimestampTz stmt_ts;

} ConnCacheEntry;

typedef struct TbStatement
{
	SQLHANDLE hstmt;
	char tsn[32];
	ConnCacheEntry *conn;
	SQLSMALLINT res_col_cnt;
} TbStatement;

/* {{{ tbcli wrapper **********************************************************/
void TbSQLFetch(TbStatement *tbStmt, int *cur_tuple_idx, bool *end_of_fetch);
void TbSQLBindCol(TbStatement *tbStmt, SQLUSMALLINT col_no, 
									SQLSMALLINT target_type, SQLPOINTER target_value, 
									SQLLEN buffer_len, SQLLEN *str_len_or_ind);
void TbSQLEndTran(ConnCacheEntry *entry, SQLSMALLINT completion_type);
void TbSQLSetStmtAttr(TbStatement *tbStmt, SQLINTEGER attribute, SQLPOINTER value, 
											SQLINTEGER str_len);
void TbSQLExecDirect(TbStatement *tbStmt, SQLCHAR *sql, SQLINTEGER sql_len);
void TbSQLAllocHandle(ConnCacheEntry *entry, SQLSMALLINT handle_type, 
											SQLHANDLE input_handle, SQLHANDLE *output_handle);
void TbSQLFreeStmt(TbStatement *tbStmt, SQLUSMALLINT option);
void TbSQLExecute(TbStatement *tbStmt);
void TbSQLBindParameter(TbStatement *tbStmt, SQLUSMALLINT param_no,
                      	SQLSMALLINT input_output_type, SQLSMALLINT value_type,
                        SQLSMALLINT param_type, SQLULEN col_size,
												SQLSMALLINT decimal_digits, SQLPOINTER param_value,
												SQLLEN buffer_len, SQLLEN *str_len_or_ind);
void TbSQLDescribeCol(TbStatement *tbStmt, SQLUSMALLINT col_no, SQLCHAR *col_name,
											SQLSMALLINT buffer_len, SQLSMALLINT *name_length,
											SQLSMALLINT *data_type, SQLULEN *col_size, 
											SQLSMALLINT *decimal_digits, SQLSMALLINT *nullable);
void TbSQLPrepare(TbStatement *tbStmt, SQLCHAR *sql, SQLINTEGER sql_len);
void TbSQLDriverConnect(ConnCacheEntry *entry, SQLHWND window_handle, 
												SQLCHAR *in_conn_str, SQLSMALLINT str_len1, 
												SQLCHAR *out_conn_str, SQLSMALLINT buffer_len, 
												SQLSMALLINT *str_len2, SQLUSMALLINT driver_completion);
void TbSQLDisconnect(ConnCacheEntry *entry);
void TbSQLFreeHandle(ConnCacheEntry *entry, SQLSMALLINT handle_type, SQLHANDLE handle);
void TbSQLSetConnectAttr(ConnCacheEntry *entry, SQLINTEGER attribute, 
												 SQLPOINTER value, SQLINTEGER str_len);
void TbSQLSetEnvAttr(ConnCacheEntry *entry, SQLINTEGER attribute, SQLPOINTER value,
                		 SQLINTEGER str_len);
void TbSQLNumResultCols(TbStatement *tbStmt, SQLSMALLINT *col_cnt);
/********************************************************** tbcli wrapper }}} */

void GetTbStatement(UserMapping *user, TbStatement *tbStmt, bool use_fb_query);

#endif							/* TIBERO_FDW_CONNECTION_H */
