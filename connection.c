/*-------------------------------------------------------------------------
 *
 * connection.c
 *		  Connection management functions for tibero_fdw
 *
 * Portions Copyright (c) 2022-2023, Tmax OpenSQL Research & Development Team
 *
 * IDENTIFICATION
 *		  contrib/tibero_fdw/connection.c
 *
 *-------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>

#include "postgres.h"
#include "access/xact.h"                /* XactEvent                          */
#include "commands/defrem.h"            /* defGetBoolean                      */
#include "utils/hsearch.h"              /* HTAB                               */
#include "utils/syscache.h"             /* FOREIGNSERVEROID                   */
#include "utils/inval.h"                /* CacheRegisterSyscacheCallback      */
#include "utils/elog.h"                 /* ereport                            */
#include "nodes/execnodes.h"            /* ForeignScanState                   */

#include "tibero_fdw.h"
#include "connection.h"

/* {{{ Global variables *******************************************************/
static HTAB *ConnectionHash = NULL;
static bool xact_got_connection = false;
/******************************************************* Global variables }}} */

void TbFdwReportError(int elevel, int sql_errcode, SQLRETURN rc, 
                      ConnCacheEntry *conn);

static void MakeNewConnection(ConnCacheEntry *conn, UserMapping *user);

static void TbfdwXactCallback(XactEvent event, void *arg);
static void TbfdwSubxactCallback(SubXactEvent event, SubTransactionId mySubid,
                                 SubTransactionId parentSubid, void *arg);
static void TbfdwInvalCallback(Datum arg, int cacheid, uint32 hashvalue);

static void
BeginRemoteXact(TbStatement *tbStmt, bool isSerializable)
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
ConnectTbServer(ConnCacheEntry *conn, const char *host, const char *port, 
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
DisconnectTbServer(ConnCacheEntry *conn)
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
NeedRemoteSnapshot(ConnCacheEntry *conn)
{
  return !IsolationUsesXactSnapshot() && 
         (conn->stmt_ts != GetCurrentStatementStartTimestamp());
}

void
GetTbStatement(UserMapping *user, TbStatement *tbStmt, bool use_fb_query)
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
      DisconnectTbServer(conn);

    if (conn->connected == false)
      MakeNewConnection(conn, user);

    TbSQLAllocHandle(conn, SQL_HANDLE_STMT, conn->hdbc, &tbStmt->hstmt);

    if (conn->begin_remote_xact == false)
      BeginRemoteXact(tbStmt, IsolationUsesXactSnapshot());

    if (use_fb_query && NeedRemoteSnapshot(conn)) {
      SQLUINTEGER len;
      TbSQLExecDirect(tbStmt, (SQLCHAR *)"SELECT current_tsn FROM v$database", 
                      SQL_NTS);
      TbSQLBindCol(tbStmt, 1, SQL_C_CHAR, (SQLCHAR *)conn->tsn, 
                   sizeof(conn->tsn), (long *)&len);
      TbSQLFetch(tbStmt, NULL, NULL);
      TbSQLFreeStmt(tbStmt, SQL_CLOSE);
      conn->stmt_ts = GetCurrentStatementStartTimestamp();
    }

    memcpy (tbStmt->tsn, conn->tsn, sizeof(tbStmt->tsn));
  }
  PG_CATCH();
  {
    MemoryContext ectx = MemoryContextSwitchTo(cctx);
    ErrorData  *errdata = CopyErrorData();

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
    DisconnectTbServer(conn);
    MakeNewConnection(conn, user);
    TbSQLAllocHandle(conn, SQL_HANDLE_STMT, conn->hdbc, &tbStmt->hstmt);
    BeginRemoteXact(tbStmt, IsolationUsesXactSnapshot());
  }
}

static void
MakeNewConnection(ConnCacheEntry *conn, UserMapping *user)
{
  ForeignServer *server = GetForeignServer(user->serverid);
  ListCell   *lc;
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
		DefElem    *def = (DefElem *) lfirst(lc);
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

  ConnectTbServer(conn, host, port, dbname, username, password);
}

static void
TbfdwXactCallback(XactEvent event, void *arg)
{
  HASH_SEQ_STATUS scan;
	ConnCacheEntry *conn;

  if (!xact_got_connection)
    return;

	hash_seq_init(&scan, ConnectionHash);
	while ((conn = (ConnCacheEntry *) hash_seq_search(&scan))) {
    if (conn->begin_remote_xact) {
      switch (event) {
				case XACT_EVENT_PARALLEL_PRE_COMMIT:
				case XACT_EVENT_PRE_COMMIT:
          if (conn->connected) {
            TbSQLEndTran(conn, SQL_COMMIT);
            if (conn->invalidated || !conn->keep_connections) {
              DisconnectTbServer(conn);
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
              DisconnectTbServer(conn);
            }
          } break;
			}
    }
  }
  xact_got_connection = false;
}

static void
TbfdwSubxactCallback(SubXactEvent event, SubTransactionId mySubid,
					   SubTransactionId parentSubid, void *arg)
{
  if (!xact_got_connection) 
    return;

  /* TODO Implementation */
}

static void
TbfdwInvalCallback(Datum arg, int cacheid, uint32 hashvalue)
{	
  HASH_SEQ_STATUS scan;
	ConnCacheEntry *conn;

	Assert(cacheid == FOREIGNSERVEROID || cacheid == USERMAPPINGOID);

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
}

void
TbFdwReportError(int elevel, int sql_errcode, SQLRETURN rc, ConnCacheEntry *conn)
{
	PG_TRY();
	{
    /* NOTE Do not call tbcli wrapper function */
    if (conn->connected) {
      conn->begin_remote_xact = false;
      conn->connected = false;
      SQLDisconnect(conn->hdbc);
      SQLFreeHandle(SQL_HANDLE_DBC, conn->hdbc);
      SQLFreeHandle(SQL_HANDLE_ENV, conn->henv);
    }

    /* TODO Set the specific sql_errcode from the rc using diagnosis */
		ereport(elevel, errcode(sql_errcode), errmsg("tbcli error (%d)", rc));
	}
	PG_FINALLY();
	{
	}
	PG_END_TRY();
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
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, tbStmt->conn);
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
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, tbStmt->conn);
  }
}

void 
TbSQLEndTran(ConnCacheEntry *conn, SQLSMALLINT completion_type)
{
  SQLRETURN rc = SQLEndTran(SQL_HANDLE_DBC, conn->hdbc, completion_type);
  if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
    /* TODO Add processing for SQL_SUCCESS_WITH_INFO */
  } else {
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, conn);
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
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, tbStmt->conn);
  }
}

void
TbSQLExecDirect(TbStatement *tbStmt, SQLCHAR *sql, SQLINTEGER sql_len)
{
  SQLRETURN rc = SQLExecDirect(tbStmt->hstmt, sql, sql_len);
  if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
    /* TODO Add processing for SQL_SUCCESS_WITH_INFO */
  } else {
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, tbStmt->conn);
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
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, conn);
  }
}

void
TbSQLFreeStmt(TbStatement *tbStmt, SQLUSMALLINT option)
{
  /* XXX SQL_DROP을 쓰는 경우가 필요한지 확인하기 */
  SQLRETURN rc = SQLFreeStmt(tbStmt->hstmt, option);
  if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
    /* TODO Add processing for SQL_SUCCESS_WITH_INFO */
  } else {
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, tbStmt->conn);
  }
}

void
TbSQLExecute(TbStatement *tbStmt)
{
  SQLRETURN rc = SQLExecute(tbStmt->hstmt);
  if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
    /* TODO Add processing for SQL_SUCCESS_WITH_INFO */
  } else {
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, tbStmt->conn);
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
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, tbStmt->conn);
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
  } else {
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, tbStmt->conn);
  }  
}

void
TbSQLPrepare(TbStatement *tbStmt, SQLCHAR *sql, SQLINTEGER sql_len)
{
  SQLRETURN rc = SQLPrepare(tbStmt->hstmt, sql, sql_len);
  if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
    /* TODO Add processing for SQL_SUCCESS_WITH_INFO */
  } else {
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, tbStmt->conn);
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
    TbFdwReportError(ERROR, ERRCODE_FDW_UNABLE_TO_ESTABLISH_CONNECTION, rc, conn);
  }
}

void
TbSQLDisconnect(ConnCacheEntry *conn)
{
  SQLRETURN rc = SQLDisconnect(conn->hdbc);
  if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
    /* TODO Add processing for SQL_SUCCESS_WITH_INFO */
  } else {
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, conn);
  }  
}

void
TbSQLFreeHandle(ConnCacheEntry *conn, SQLSMALLINT handle_type, SQLHANDLE handle)
{
  SQLRETURN rc = SQLFreeHandle(handle_type, handle);
  if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
    /* TODO Add processing for SQL_SUCCESS_WITH_INFO */
  } else {
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, conn);
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
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, conn);
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
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, conn);
  }
}

void
TbSQLNumResultCols(TbStatement *tbStmt, SQLSMALLINT *col_cnt)
{
  SQLRETURN rc = SQLNumResultCols(tbStmt->hstmt, col_cnt);
  if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
    /* TODO Add processing for SQL_SUCCESS_WITH_INFO */
  } else {
    TbFdwReportError(ERROR, ERRCODE_FDW_ERROR, rc, tbStmt->conn);
  }
}
