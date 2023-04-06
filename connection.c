/*------------------------------------------------------------------------------
 *
 * connection.c
 *			Connection management functions for tibero_fdw
 *
 * Portions Copyright (c) 2022-2023, Tmax OpenSQL Research & Development Team
 *
 * IDENTIFICATION
 *			contrib/tibero_fdw/connection.c
 *
 *------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>

#include "postgres.h"
#include "access/xact.h"                /* XactEvent													*/
#include "commands/defrem.h"            /* defGetBoolean											*/
#include "utils/hsearch.h"							/* HTAB                               */
#include "utils/syscache.h"             /* FOREIGNSERVEROID                   */
#include "utils/inval.h"                /* CacheRegisterSyscacheCallback			*/
#include "utils/elog.h"                 /* ereport														*/
#include "nodes/execnodes.h"            /* ForeignScanState                   */

#include "tibero_fdw.h"
#include "connection.h"

/* {{{ Tibero datatype length *************************************************/
#define NUMBER_MAX_STRING 256
#define TB_CHAR_SIZE_MAX 2000
#define TB_VARCHAR2_SIZE_MAX (UINT16_MAX - 3)
#define TB_RAW_SIZE_MAX 2000
#define NLS_DATETIME_FORMAT_SIZE 80
#define TB_TIME_MAX_STR_SIZE 32
#define TB_ITV_YTM_MAX_STR_SIZE 15 /* "+999999999-11" */
#define TB_ITV_DTS_MAX_STR_SIZE 31 /* "+999999999 11:59:59.999999999" */
#define EROWID_SGMT_LEN 6
#define EROWID_FILE_LEN 3
#define EROWID_BLOCK_LEN 6
#define EROWID_ROW_LEN 3
#define EROWID_SIZE (EROWID_SGMT_LEN + EROWID_FILE_LEN + EROWID_BLOCK_LEN + EROWID_ROW_LEN)
/************************************************* Tibero datatype length }}} */

/* {{{ Global variables *******************************************************/
static HTAB *ConnectionHash = NULL;
static bool xact_got_connection = false;
/******************************************************* Global variables }}} */

#define TbFdwReportError(elevel, sql_errcode, msg, conn)											 \
do {																																					 \
	PG_TRY();																																		 \
	{																																						 \
		if (conn->connected) {																										 \
			conn->begin_remote_xact = false;																				 \
			conn->connected = false;																								 \
			/* Don't invoke tbcli wrapper function */																 \
			SQLDisconnect(conn->hdbc);																							 \
			SQLFreeHandle(SQL_HANDLE_DBC, conn->hdbc);															 \
			SQLFreeHandle(SQL_HANDLE_ENV, conn->henv);															 \
		}																																					 \
		ereport(elevel, errcode(sql_errcode), errmsg(msg, ""));										 \
	}																																						 \
	PG_FINALLY();																																 \
	{																																						 \
	}																																						 \
	PG_END_TRY();																																 \
} while (0)

static void TbfdwXactCallback(XactEvent event, void *arg);
static void TbfdwSubxactCallback(SubXactEvent event, SubTransactionId mySubid,
																 SubTransactionId parentSubid, void *arg);
static void TbfdwInvalCallback(Datum arg, int cacheid, uint32 hashvalue);

static void make_tb_connection(ConnCacheEntry *conn, UserMapping *user);
SQLUINTEGER get_tb_type_max_str_size(int type, SQLUINTEGER col_size,
																		 ConnCacheEntry *conn);

static void
begin_remote_xact(TbStatement *tbStmt, bool isSerializable)
{
	TbSQLSetConnectAttr(tbStmt->conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, 0);

	if (isSerializable) {
		TbSQLExecDirect(tbStmt, (SQLCHAR *)"SET TRANSACTION ISOLATION LEVEL SERIALIZABLE", SQL_NTS);
	} else {
		TbSQLExecDirect(tbStmt, (SQLCHAR *)"SET TRANSACTION ISOLATION LEVEL READ COMMITTED", SQL_NTS);
	}

	tbStmt->conn->begin_remote_xact = true;
}

static void
connect_tb_server(ConnCacheEntry *conn, const char *host, const char *port,
								const char *dbname, const char *username, const char *password)
{
	char conn_str[512] = {0,};

	Assert(host != NULL && port != NULL && dbname != NULL && username != NULL &&
				 password != NULL);

	snprintf(conn_str, sizeof(conn_str), "SERVER=%s;PORT=%s;DB=%s;UID=%s;PWD=%s",
					 host, port, dbname, username, password);

	TbSQLAllocHandle(conn, SQL_HANDLE_ENV, SQL_NULL_HANDLE, &conn->henv);
	TbSQLSetEnvAttr(conn, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

	TbSQLAllocHandle(conn, SQL_HANDLE_DBC, conn->henv, &conn->hdbc);
	TbSQLDriverConnect(conn, 0, (SQLCHAR *)conn_str, SQL_NTS, NULL, 0, NULL,
										 SQL_DRIVER_COMPLETE);

	conn->connected = true;
}

static void
disconnect_tb_server(ConnCacheEntry *conn)
{
	if (conn->connected == true) {
		TbSQLDisconnect(conn);
		TbSQLFreeHandle(conn, SQL_HANDLE_DBC, conn->hdbc);
		TbSQLFreeHandle(conn, SQL_HANDLE_ENV, conn->henv);

		conn->begin_remote_xact = false;
		conn->connected = false;
	}
}

static bool
need_remote_snapshot(ConnCacheEntry *conn)
{
	return !IsolationUsesXactSnapshot() &&
				 (conn->stmt_ts != GetCurrentStatementStartTimestamp());
}

void
get_tb_statement(UserMapping *user, TbStatement *tbStmt, bool use_fb_query)
{
	bool found = false;
	bool retry = false;
	ConnCacheEntry *conn;
	ConnCacheKey key;
	MemoryContext cctx = CurrentMemoryContext;

	if (ConnectionHash == NULL) {
		HASHCTL ctl;
		ctl.keysize = sizeof(ConnCacheKey);
		ctl.entrysize = sizeof(ConnCacheEntry);
		ConnectionHash = hash_create("tibero_fdw connections", 8, &ctl, HASH_ELEM|HASH_BLOBS);

		RegisterXactCallback(TbfdwXactCallback, NULL);
		RegisterSubXactCallback(TbfdwSubxactCallback, NULL);
		CacheRegisterSyscacheCallback(FOREIGNSERVEROID, TbfdwInvalCallback, (Datum) 0);
		CacheRegisterSyscacheCallback(USERMAPPINGOID, TbfdwInvalCallback, (Datum) 0);
	}

	xact_got_connection = true;

	key = user->umid;
	conn = hash_search(ConnectionHash, &key, HASH_ENTER, &found);
	if (!found) {
		conn->connected = false;
	}
	tbStmt->conn = conn;

	PG_TRY();
	{
		if (conn->connected && conn->invalidated)
			disconnect_tb_server(conn);

		if (conn->connected == false)
			make_tb_connection(conn, user);

		TbSQLAllocHandle(conn, SQL_HANDLE_STMT, conn->hdbc, &tbStmt->hstmt);

		if (conn->begin_remote_xact == false)
			begin_remote_xact(tbStmt, IsolationUsesXactSnapshot());

		if (use_fb_query && need_remote_snapshot(conn)) {
			SQLUINTEGER len;
			TbSQLExecDirect(tbStmt, (SQLCHAR *)"SELECT current_tsn FROM v$database",
											SQL_NTS);
			TbSQLBindCol(tbStmt, 1, SQL_C_CHAR, (SQLCHAR *)conn->tsn,
									 sizeof(conn->tsn), (long *)&len);
			TbSQLFetch(tbStmt, NULL, NULL);
			TbSQLFreeStmt(tbStmt, SQL_CLOSE);
			conn->stmt_ts = GetCurrentStatementStartTimestamp();
		}

		memcpy(tbStmt->tsn, conn->tsn, sizeof(tbStmt->tsn));
	}
	PG_CATCH();
	{
		MemoryContext ectx = MemoryContextSwitchTo(cctx);
		ErrorData	*errdata = CopyErrorData();

		if (errdata->sqlerrcode != ERRCODE_FDW_ERROR) {
			MemoryContextSwitchTo(ectx);
			PG_RE_THROW();
		}

		FlushErrorState();
		FreeErrorData(errdata);
		errdata = NULL;

		retry = true;
	}
	PG_END_TRY();

	if (retry) {
		disconnect_tb_server(conn);
		make_tb_connection(conn, user);
		TbSQLAllocHandle(conn, SQL_HANDLE_STMT, conn->hdbc, &tbStmt->hstmt);
		begin_remote_xact(tbStmt, IsolationUsesXactSnapshot());
	}
}

static void
make_tb_connection(ConnCacheEntry *conn, UserMapping *user)
{
	ForeignServer *server = GetForeignServer(user->serverid);
	ListCell	 *lc;
	const char *host = NULL;
	const char *port = NULL;
	const char *dbname = NULL;
	const char *username = NULL;
	const char *password = NULL;

	Assert(conn->connected == false);

	conn->serverid = server->serverid;
	conn->server_hashvalue =
		GetSysCacheHashValue1(FOREIGNSERVEROID, ObjectIdGetDatum(server->serverid));
	conn->mapping_hashvalue =
		GetSysCacheHashValue1(USERMAPPINGOID, ObjectIdGetDatum(user->umid));
	conn->invalidated = false;
	conn->begin_remote_xact = false;
	conn->keep_connections = true;
	conn->stmt_ts = 0;

	foreach(lc, server->options) {
		DefElem *def = (DefElem *) lfirst(lc);
		if (strcmp(def->defname, "host") == 0) {
			host = defGetString(def);
		} else if (strcmp(def->defname, "port") == 0) {
			port = defGetString(def);
		} else if (strcmp(def->defname, "dbname") == 0) {
			dbname = defGetString(def);
		} else if (strcmp(def->defname, "keep_connections") == 0) {
			conn->keep_connections = defGetBoolean(def);
		}
	}

	foreach(lc, user->options) {
		DefElem *def = (DefElem *) lfirst(lc);
		if (strcmp(def->defname, "username") == 0) {
			username = defGetString(def);
		} else if (strcmp(def->defname, "password") == 0) {
			password = defGetString(def);
		}
	}

	connect_tb_server(conn, host, port, dbname, username, password);
}

static void
TbfdwXactCallback(XactEvent event, void *arg)
{
	HASH_SEQ_STATUS scan;
	ConnCacheEntry *conn;

	if (!xact_got_connection)
		return;

	set_sleep_on_sig_on();

	hash_seq_init(&scan, ConnectionHash);
	while ((conn = (ConnCacheEntry *) hash_seq_search(&scan))) {
		if (conn->begin_remote_xact) {
			switch (event) {
				case XACT_EVENT_PARALLEL_PRE_COMMIT:
				case XACT_EVENT_PRE_COMMIT:
					if (conn->connected) {
						TbSQLEndTran(conn, SQL_COMMIT);
						if (conn->invalidated || !conn->keep_connections) {
							disconnect_tb_server(conn);
						}
					} break;
				case XACT_EVENT_PRE_PREPARE:
					break;
				case XACT_EVENT_PARALLEL_COMMIT:
				case XACT_EVENT_COMMIT:
				case XACT_EVENT_PREPARE:
					break;
				case XACT_EVENT_PARALLEL_ABORT:
				case XACT_EVENT_ABORT:
					if (in_error_recursion_trouble()) {
						conn->connected = false;
						break;
					}
					if (conn->connected) {
						TbSQLEndTran(conn, SQL_ROLLBACK);
						if (conn->invalidated || !conn->keep_connections) {
							disconnect_tb_server(conn);
						}
					} break;
			}
		}
	}
	xact_got_connection = false;

	set_sleep_on_sig_off();
}

static void
TbfdwSubxactCallback(SubXactEvent event, SubTransactionId mySubid,
										 SubTransactionId parentSubid, void *arg)
{
	if (!xact_got_connection)
		return;

	set_sleep_on_sig_on();

	/* TODO */

	set_sleep_on_sig_off();
}

static void
TbfdwInvalCallback(Datum arg, int cacheid, uint32 hashvalue)
{
	HASH_SEQ_STATUS scan;
	ConnCacheEntry *conn;

	Assert(cacheid == FOREIGNSERVEROID || cacheid == USERMAPPINGOID);

	set_sleep_on_sig_on();

	hash_seq_init(&scan, ConnectionHash);
	while ((conn = (ConnCacheEntry *) hash_seq_search(&scan))) {

		if (conn->connected == false)
			continue;

		if (hashvalue == 0 ||
				(cacheid == FOREIGNSERVEROID && conn->server_hashvalue == hashvalue) ||
				(cacheid == USERMAPPINGOID && conn->mapping_hashvalue == hashvalue)) {
				conn->invalidated = true;
		}
	}

	set_sleep_on_sig_off();
}

void
TbSQLFetch(TbStatement *tbStmt, int *cur_tuple_idx, bool *end_of_fetch)
{
	SQLRETURN rc = SQLFetch(tbStmt->hstmt);
	if (rc == SQL_NO_DATA) {
		if (end_of_fetch != NULL) *end_of_fetch = true;
	} else if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), tbStmt->conn);
	}

	if (cur_tuple_idx != NULL) *cur_tuple_idx = 0;

	/* TODO Return a boolean value for the loop statement */
}

void
TbSQLBindCol(TbStatement *tbStmt, SQLUSMALLINT col_no, SQLSMALLINT target_type,
						 SQLPOINTER target_value, SQLLEN buffer_len,
						 SQLLEN *str_len_or_ind)
{
	SQLRETURN rc = SQLBindCol(tbStmt->hstmt, col_no, target_type, target_value,
														buffer_len, str_len_or_ind);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), tbStmt->conn);
	}
}

void
TbSQLEndTran(ConnCacheEntry *conn, SQLSMALLINT completion_type)
{
	SQLRETURN rc = SQLEndTran(SQL_HANDLE_DBC, conn->hdbc, completion_type);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), conn);
	}
	conn->begin_remote_xact = false;
}

void
TbSQLSetStmtAttr(TbStatement *tbStmt, SQLINTEGER attribute, SQLPOINTER value,
								 SQLINTEGER str_len)
{
	SQLRETURN rc = SQLSetStmtAttr(tbStmt->hstmt, attribute, value, str_len);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), tbStmt->conn);
	}
}

void
TbSQLExecDirect(TbStatement *tbStmt, SQLCHAR *sql, SQLINTEGER sql_len)
{
	SQLRETURN rc = SQLExecDirect(tbStmt->hstmt, sql, sql_len);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), tbStmt->conn);
	}
}

void
TbSQLAllocHandle(ConnCacheEntry *conn, SQLSMALLINT handle_type,
								 SQLHANDLE input_handle, SQLHANDLE *output_handle)
{
	SQLRETURN rc = SQLAllocHandle(handle_type, input_handle, output_handle);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), conn);
	}
}

void
TbSQLFreeStmt(TbStatement *tbStmt, SQLUSMALLINT option)
{
	SQLRETURN rc = SQLFreeStmt(tbStmt->hstmt, option);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), tbStmt->conn);
	}
}

void
TbSQLExecute(TbStatement *tbStmt)
{
	SQLRETURN rc = SQLExecute(tbStmt->hstmt);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), tbStmt->conn);
	}
}

void
TbSQLBindParameter(TbStatement *tbStmt, SQLUSMALLINT param_no,
									 SQLSMALLINT input_output_type, SQLSMALLINT value_type,
									 SQLSMALLINT param_type, SQLULEN col_size,
									 SQLSMALLINT decimal_digits, SQLPOINTER param_value,
									 SQLLEN buffer_len, SQLLEN *str_len_or_ind)
{
	SQLRETURN rc = SQLBindParameter(tbStmt->hstmt, param_no, input_output_type,
																	value_type, param_type, col_size,
																	decimal_digits, param_value, buffer_len,
																	str_len_or_ind);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), tbStmt->conn);
	}
}

void
TbSQLDescribeCol(TbStatement *tbStmt, SQLUSMALLINT col_no, SQLCHAR *col_name,
								 SQLSMALLINT buffer_len, SQLSMALLINT *name_length,
								 SQLSMALLINT *data_type, SQLULEN *col_size,
								 SQLSMALLINT *decimal_digits, SQLSMALLINT *nullable)
{
	SQLRETURN rc = SQLDescribeCol(tbStmt->hstmt, col_no, col_name, buffer_len,
																name_length, data_type, col_size,
																decimal_digits, nullable);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
		*col_size = get_tb_type_max_str_size(*data_type, *col_size, tbStmt->conn);
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), tbStmt->conn);
	}
}

void
TbSQLPrepare(TbStatement *tbStmt, SQLCHAR *sql, SQLINTEGER sql_len)
{
	SQLRETURN rc = SQLPrepare(tbStmt->hstmt, sql, sql_len);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), tbStmt->conn);
	}
}

void
TbSQLDriverConnect(ConnCacheEntry *conn, SQLHWND window_handle, SQLCHAR *in_conn_str,
									 SQLSMALLINT str_len1, SQLCHAR *out_conn_str,
									 SQLSMALLINT buffer_len, SQLSMALLINT *str_len2,
									 SQLUSMALLINT driver_completion)
{
	SQLRETURN rc = SQLDriverConnect(conn->hdbc, window_handle, in_conn_str, str_len1,
																	out_conn_str, buffer_len, str_len2, driver_completion);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_UNABLE_TO_ESTABLISH_CONNECTION,
										 psprintf("return code (%d)", rc), conn);
	}
}

void
TbSQLDisconnect(ConnCacheEntry *conn)
{
	SQLRETURN rc = SQLDisconnect(conn->hdbc);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), conn);
	}
}

void
TbSQLFreeHandle(ConnCacheEntry *conn, SQLSMALLINT handle_type, SQLHANDLE handle)
{
	SQLRETURN rc = SQLFreeHandle(handle_type, handle);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), conn);
	}
}

void
TbSQLSetConnectAttr(ConnCacheEntry *conn, SQLINTEGER attribute,
									SQLPOINTER value, SQLINTEGER str_len)
{
	SQLRETURN rc = SQLSetConnectAttr(conn->hdbc, attribute, value, str_len);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), conn);
	}
}

void
TbSQLSetEnvAttr(ConnCacheEntry *conn, SQLINTEGER attribute, SQLPOINTER value,
								SQLINTEGER str_len)
{
	SQLRETURN rc = SQLSetEnvAttr(conn->henv, attribute, value, str_len);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), conn);
	}
}

void
TbSQLNumResultCols(TbStatement *tbStmt, SQLSMALLINT *col_cnt)
{
	SQLRETURN rc = SQLNumResultCols(tbStmt->hstmt, col_cnt);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* TODO Add processing for SQL_SUCCESS_WITH_INFO */
	} else {
		TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
										 psprintf("return code (%d)", rc), tbStmt->conn);
	}
}

SQLUINTEGER
get_tb_type_max_str_size(int type, SQLUINTEGER col_size, ConnCacheEntry *conn)
{
	SQLUINTEGER res = 0;

	switch (type) {
		case SQL_NUMERIC: /* TB_TYPE_NUMBER */
		case SQL_DECIMAL:
		case SQL_FLOAT:
		case SQL_REAL:
		case SQL_DOUBLE:
		case SQL_INTEGER:
		case SQL_SMALLINT:
		case SQL_BIGINT:
		case SQL_TINYINT:
		case SQL_BIT:
		case SQL_BOOLEAN:
		case SQL_BFLOAT: /* TB_TYPE_BIN_FLOAT */
    case SQL_BDOUBLE: /* TB_TYPE_BIN_DOUBLE */
			res = NUMBER_MAX_STRING;
			break;

		case SQL_CHAR: /* TB_TYPE_CHAR */
		case SQL_WCHAR: /* TB_TYPE_NCHAR */
			res = col_size > 0 ? col_size : TB_CHAR_SIZE_MAX;
			break;

		case SQL_VARCHAR: /* TB_TYPE_VARCHAR */
		case SQL_WVARCHAR: /* TB_TYPE_NVARCHAR */
			res = col_size > 0 ? col_size : TB_VARCHAR2_SIZE_MAX;
			break;

		case SQL_VARBINARY: /* TB_TYPE_RAW */
			res = (col_size > 0 ? col_size : TB_RAW_SIZE_MAX) * 2;
			break;

		case SQL_TYPE_DATE:
		case SQL_DATE: /* TB_TYPE_DATE */
			res = NLS_DATETIME_FORMAT_SIZE;
			break;

		case SQL_TYPE_TIME:
		case SQL_TIME: /* TB_TYPE_TIME */
			res = TB_TIME_MAX_STR_SIZE;
			break;

		case SQL_TYPE_TIMESTAMP:
		case SQL_TIMESTAMP: /* TB_TYPE_TIMESTAMP,
													 TB_TYPE_TS_TZ,
													 TB_TYPE_TS_LTZ */
			res = NLS_DATETIME_FORMAT_SIZE;
			break;

		case SQL_INTERVAL_YEAR_TO_MONTH: /* TB_TYPE_ITV_YTM */
			res = TB_ITV_YTM_MAX_STR_SIZE;
			break;

		case SQL_INTERVAL_DAY_TO_SECOND: /* TB_TYPE_ITV_DTS */
			res = TB_ITV_DTS_MAX_STR_SIZE;
			break;

		case SQL_BINARY: /* TB_TYPE_ROWID */
			res = EROWID_SIZE;
			break;

		case SQL_CSR: /* TB_TYPE_CSR */
			res = NUMBER_MAX_STRING;
			break;

		default:
			TbFdwReportError(ERROR, ERRCODE_FDW_ERROR,
											 psprintf("tbcli datatype error (%d)", type), conn);
			break;
	}

	/* + 1 for null termination */
	return res + 1;
}
