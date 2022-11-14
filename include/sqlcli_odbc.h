/**
 * @file    sqlcli_odbc.h
 * @brief   The specific include for ODBC(or CLI) functions
 *
 * @author  eacho, wrpark
 * @version \$Id$
 */

#ifndef _SQLCLI_TB_H
#define _SQLCLI_TB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "extproc.h"
#include "sqlcli_types.h"

/* Maximum Value */
#define SQL_MAXIMUM_MESSAGE_LENGTH      (512)   /* ODBC */
#define SQL_MAX_ROWSET_SIZE             (500)   /* Tibero Defined */

/* for Temporary OCI compatibility */
#define SQL_CONTINUE                    (-999)

/* handle type identifiers */
#define SQL_HANDLE_NULL                 (0)
#define SQL_HANDLE_DPCTX                (6)
#define SQL_HANDLE_SUBS                 (7)
#define SQL_HANDLE_DPCTX_LOAD           (8)

#define SQL_ATTR_SAVEPOINT_NAME         (10027)
#define SQL_ATTR_SAVEPOINT_NUMBER       (10028)

#define SQL_ATTR_PREFETCH_SIZE          (10030)

/* Statement caching attribute */
#define SQL_ATTR_STMT_CACHE             (10031)
#define SQL_ATTR_STMT_CACHE_SIZE        (10032)

/* callback function for connection failover */
#define SQL_ATTR_CONN_FOCBK             (10033)

/* connection pooling attribute */
#define SQL_ATTR_CP_MIN_SIZE            (10034)
#define SQL_ATTR_CP_MAX_SIZE            (10035)
#define SQL_ATTR_CP_INCR_SIZE           (10036)
#define SQL_ATTR_CP_TIMEOUT             (10037)
#define SQL_ATTR_CP_NOWAIT              (10038)
#define SQL_ATTR_CP_TOT_COMMS           (10039)
#define SQL_ATTR_CP_NUM_COMMS           (10040)

/* separate cursor */
#define SQL_ATTR_SEPARATE_CURSOR        (10041)

#define SQL_ATTR_PARAM_ERROR_PTR        (10042)
#define SQL_DESC_ARRAY_ERROR_PTR        (10043)

/* implicit result set */
#define SQL_ATTR_RETURN_RESULT        (10044)

/* direct path loading attribute */
#define SQL_DPCTX_ATTR_TBL_NAME         (0)
#define SQL_DPCTX_ATTR_SCHEMA_NAME      (1)
#define SQL_DPCTX_ATTR_INPUT            (2)
#define SQL_DPCTX_ATTR_DATEFORMAT       (3)
#define SQL_DPCTX_ATTR_PARALLEL         (4)
#define SQL_DPCTX_ATTR_NOLOG            (5)
#define SQL_DPCTX_ATTR_OBJ_CONSTR       (6)
#define SQL_DPCTX_ATTR_NUM_COLS         (7)
#define SQL_DPCTX_ATTR_LIST_COLUMNS     (8)
#define SQL_DPCTX_ATTR_PTYPE            (9)
#define SQL_DPCTX_ATTR_COL_NAME         (10)
#define SQL_DPCTX_ATTR_DATA_TYPE        (11)
#define SQL_DPCTX_ATTR_DATA_SIZE        (12)
#define SQL_DPCTX_ATTR_PRECISION        (13)
#define SQL_DPCTX_ATTR_SCALE            (14)
#define SQL_DPCTX_ATTR_CHARSET_ID       (15)
#define SQL_DPCTX_ATTR_OID              (16)
#define SQL_DPCTX_ATTR_FN_CTX           (17)
#define SQL_DPCTX_ATTR_DESC             (18)
#define SQL_DPCTX_ATTR_ROW_COUNT        (19)
#define SQL_DPCTX_ATTR_STREAM_SIZE      (20)
#define SQL_DPCTX_ATTR_USED_STREAM_SIZE (21)
#define SQL_DPCTX_ATTR_MAX_DPL_BUFCNT   (22)
#define SQL_DPCTX_ATTR_CUR_FILE_POS     (23)
#define SQL_DPCTX_ATTR_FAIL_FILEOFFSET  (24)
#define SQL_DPCTX_ATTR_FAIL_ROWINDEX    (25)
#define SQL_DPCTX_ATTR_SUCC_ROWINDEX    (26)
#define SQL_DPCTX_ATTR_LOAD_BUFCNT      (27)
#define SQL_DPCTX_ATTR_ROLLBACK         (28)
#define SQL_DPCTX_ATTR_PARTITION_NAME   (29)

/* direct path loading attribute */
#define SQL_HLDP_ATTR_TBL_NAME         (0)
#define SQL_HLDP_ATTR_SCHEMA_NAME      (1)
#define SQL_HLDP_ATTR_INPUT            (2)
#define SQL_HLDP_ATTR_DATEFORMAT       (3)
#define SQL_HLDP_ATTR_PARALLEL         (4)
#define SQL_HLDP_ATTR_NOLOG            (5)
#define SQL_HLDP_ATTR_OBJ_CONSTR       (6)
#define SQL_HLDP_ATTR_NUM_COLS         (7)
#define SQL_HLDP_ATTR_LIST_COLUMNS     (8)
#define SQL_HLDP_ATTR_PTYPE            (9)
#define SQL_HLDP_ATTR_COL_NAME         (10)
#define SQL_HLDP_ATTR_DATA_TYPE        (11)
#define SQL_HLDP_ATTR_DATA_SIZE        (12)
#define SQL_HLDP_ATTR_PRECISION        (13)
#define SQL_HLDP_ATTR_SCALE            (14)
#define SQL_HLDP_ATTR_CHARSET_ID       (15)
#define SQL_HLDP_ATTR_OID              (16)
#define SQL_HLDP_ATTR_FN_CTX           (17)
#define SQL_HLDP_ATTR_DESC             (18)
#define SQL_HLDP_ATTR_ROW_COUNT        (19)
#define SQL_HLDP_ATTR_STREAM_SIZE      (20)
#define SQL_HLDP_ATTR_USED_STREAM_SIZE (21)
#define SQL_HLDP_ATTR_MAX_DPL_BUFCNT   (22)
#define SQL_HLDP_ATTR_CUR_FILE_POS     (23)
#define SQL_HLDP_ATTR_FAIL_FILEOFFSET  (24)
#define SQL_HLDP_ATTR_FAIL_ROWINDEX    (25)
#define SQL_HLDP_ATTR_SUCC_ROWINDEX    (26)
#define SQL_HLDP_ATTR_LOAD_BUFCNT      (27)
#define SQL_HLDP_ATTR_ROLLBACK         (28)
#define SQL_HLDP_ATTR_PARTITION_NAME   (29)

/* subscription attributes */
#define SQL_SUBS_ATTR_NAME              (1)
#define SQL_SUBS_ATTR_CALLBACK          (2)
#define SQL_SUBS_ATTR_PORT              (3)
#define SQL_SUBS_ATTR_IPADDR            (4)
#define SQL_SUBS_ATTR_NAMESPACE         (5)

#define TBI_ATTR_POSITION               (1001)

/* Statement attributes */
#define SQL_ATTR_CURSOR_HOLDABLE        (-3)
#define SQL_ATTR_APD_HANDLE             (20011)     /* temp value */
#define SQL_ATTR_ARD_HANDLE             (20012)     /* temp value */

/* SQL_ATTR_PARAM_BIND_TYPE  values */
#define SQL_PARAM_BIND_BY_ROW           (2)

/* SQL_ATTR_CURSOR_HOLDABLE     values */
#define SQL_NONHOLDABLE                 (0)
#define SQL_HOLDABLE                    (1)

/* SQL_ATTR_ROW_BIND_TYPE       values */
#define SQL_BIND_BY_ROW                 (1)

/* identifiers of fields in the SQL descriptor */
#define SQL_DESC_ARRAY_STATUS_POINTER           (21)
#define SQL_DESC_ROWS_PROCESSED_POINTER         (34)
#define SQL_DESC_COLLATION_CATALOG              (1015)
#define SQL_DESC_COLLATION_SCHEMA               (1016)
#define SQL_DESC_COLLATION_NAME                 (1017)
#define SQL_DESC_CHARACTER_SET_CATALOG          (1018)
#define SQL_DESC_CHARACTER_SET_SCHEMA           (1019)
#define SQL_DESC_CHARACTER_SET_NAME             (1020)
#define SQL_DESC_PARAMETER_MODE                 (1021)
#define SQL_DESC_PARAMETER_ORDINAL_POSITION     (1022)
#define SQL_DESC_PARAMETER_SPECIFIC_CATALOG     (1023)
#define SQL_DESC_PARAMETER_SPECIFIC_SCHEMA      (1024)
#define SQL_DESC_PARAMETER_SPECIFIC_NAME        (1025)
#define SQL_DESC_UDT_CATALOG                    (1026)
#define SQL_DESC_UDT_SCHEMA                     (1027)
#define SQL_DESC_UDT_NAME                       (1028)
#define SQL_DESC_KEY_TYPE                       (1029)
#define SQL_DESC_KEY_MEMBER                     (1030)
#define SQL_DESC_DYNAMIC_FUNCTION               (1031)
#define SQL_DESC_DYNAMIC_FUNCTION_CODE          (1032)
#define SQL_DESC_SCOPE_CATALOG                  (1033)
#define SQL_DESC_SCOPE_SCEHMA                   (1034)
#define SQL_DESC_SCOPE_NAME                     (1035)
#define SQL_DESC_SPECIFIC_TYPE_CATALOG          (1036)
#define SQL_DESC_SPECIFIC_TYPE_SCHEMA           (1037)
#define SQL_DESC_SPECIFIC_TYPE_NAME             (1038)
#define SQL_DESC_CURRENT_TRANSFORM_GROUP        (1039)
#define SQL_DESC_CARDINALITY                    (1040)
#define SQL_DESC_DEGREE                         (1041)
#define SQL_DESC_LEVEL                          (1042)
#define SQL_DESC_RETURNED_CARDINALITY_POINTER   (1043)
#define SQL_DESC_TOP_LEVEL_COUNT                (1044)
#define SQL_DESC_DT_FMT                         (1045)
#define SQL_DESC_SVR_TYPE                       (1046)

/* identifiers of fields in the diagnostics area */
#define SQL_DIAG_MORE                           (13)
#define SQL_DIAG_TRANSACTION_COMMITED           (34)
#define SQL_DIAG_TRANSACTION_ROLLED_BACK        (35)
#define SQL_DIAG_TRANSACTION_ACTIVE             (36)

#define SQL_DIAG_CATALOG_NAME                   (18)
#define SQL_DIAG_COLUMN_NAME                    (21)
#define SQL_DIAG_CONDITION_NAME                 (25)
#define SQL_DIAG_CONDITION_IDENTIFIER           (-50002)
#define SQL_DIAG_CONDITION_NUMBER               (14)
#define SQL_DIAG_CONSTRAINT_CATALOG             (15)
#define SQL_DIAG_CONSTRAINT_NAME                (17)
#define SQL_DIAG_CONSTRAINT_SCHEMA              (16)
#define SQL_DIAG_CURSOR_NAME                    (22)
#define SQL_DIAG_MESSAGE_LENGTH                 (23)
#define SQL_DIAG_MESSAGE_OCTET_LENGTH           (24)
#define SQL_DIAG_PARAMETER_MODE                 (37)
#define SQL_DIAG_PARAMETER_NAME                 (26)
#define SQL_DIAG_PARAMETER_ORDINAL_POSITION     (38)
#define SQL_DIAG_ROUTINE_CATALOG                (27)
#define SQL_DIAG_ROUTINE_NAME                   (29)
#define SQL_DIAG_ROUTINE_SCHEMA                 (28)
#define SQL_DIAG_SCHEMA_NAME                    (19)
#define SQL_DIAG_SPECIFIC_NAME                  (30)
#define SQL_DIAG_TABLE_NAME                     (20)
#define SQL_DIAG_TRIGGER_CATALOG                (31)
#define SQL_DIAG_TRIGGER_NAME                   (33)
#define SQL_DIAG_TRIGGER_SCHEMA                 (32)

#define SQL_DIAG_ERROR_LINE_NUMBER              (-5000)
#define SQL_DIAG_ERROR_COL_NUMBER               (-5001)

/* List of SQL Statements and Codes in SQL-92, but not in CLI */
#define TBI_DIAG_ALLOCATE_CURSOR                    (1)
#define TBI_DIAG_ALLOCATE_DESCRIPTOR                (2)
#define TBI_DIAG_CREATE_TRANSLATION                 (79)
#define TBI_DIAG_DEALLOCATE_DESCRIPTOR              (15)
#define TBI_DIAG_DEALLOCATE_PREPARE                 (16)
#define TBI_DIAG_DESCRIBE                           (20)
#define TBI_DIAG_DYNAMIC_CLOSE                      (37)
#define TBI_DIAG_DYNAMIC_DELETE_CURSOR_POSITIONED   (38)
#define TBI_DIAG_DYNAMIC_FETCH                      (39)
#define TBI_DIAG_DYNAMIC_OPEN                       (40)
#define TBI_DIAG_DYNAMIC_UPDATE_CURSOR_POSITIONED   (42)
#define TBI_DIAG_EXECUTE                            (44)
#define TBI_DIAG_EXECUTE_IMMEDIATE                  (43)
#define TBI_DIAG_FETCH                              (45)
#define TBI_DIAG_GET_DESCRIPTOR                     (47)
#define TBI_DIAG_PREPARE                            (56)
#define TBI_DIAG_SELECT_MULTIPLE_ROW                (21)
#define TBI_DIAG_SET_CURRENT_PATH                   (69)
#define TBI_DIAG_SET_DESCRIPTOR                     (70)

/* List of Additional SQL Statements and Codes in SQL-92 and CLI */
#define SQL_DIAG_CLOSE_CURSOR                   (9)
#define SQL_DIAG_COMMIT                         (11)
#define SQL_DIAG_CONNECT                        (13)
#define SQL_DIAG_DECLARE_CURSOR                 (101)
#define SQL_DIAG_DELETE_CURSOR                  (18)
#define SQL_DIAG_DISCONNECT                     (22)
#define SQL_DIAG_OPEN                           (53)
#define SQL_DIAG_ROLLBACK                       (62)
#define SQL_DIAG_SELECT_SINGLE_ROW              (65)
#define SQL_DIAG_SELECT                         (41)
#define SQL_DIAG_SET_CATALOG                    (66)
#define SQL_DIAG_SET_CONNECTION                 (67)
#define SQL_DIAG_SET_CONSTRAINT                 (68)
#define SQL_DIAG_SET_NAMES                      (72)
#define SQL_DIAG_SET_TIME_ZONE                  (71)
#define SQL_DIAG_SET_SESSION_AUTHORIZATION      (76)
#define SQL_DIAG_SET_SCHEMA                     (74)
#define SQL_DIAG_SET_TRANSACTION                (75)
#define SQL_DIAG_UPDATE_CURSOR                  (81)

/* List of Additional SQL Statements and Codes in SQL3, but not in CLI */
#define TBI_DIAG_ALTER_MODULE               (95)
#define TBI_DIAG_ALTER_ROUTINE              (17)
#define TBI_DIAG_ALTER_TYEP                 (60)
#define TBI_DIAG_ASSIGNMENT                 (5)
#define TBI_DIAG_BEGIN_END                  (12)
#define TBI_DIAG_CASE                       (86)
#define TBI_DIAG_CREATE_MODULE              (51)
#define TBI_DIAG_CREATE_ODERING             (114)
#define TBI_DIAG_CREATE_TRANSFORM           (117)
#define TBI_DIAG_DECLARE_VARIABLE           (96)
#define TBI_DIAG_DROP_MODULE                (28)
#define TBI_DIAG_FOR                        (46)
#define TBI_DIAG_FREE_LOCATOR               (98)
#define TBI_DIAG_HANDLER                    (87)
#define TBI_DIAG_HOLD_LOCATOR               (99)
#define TBI_DIAG_IF                         (88)
#define TBI_DIAG_LEAVE                      (89)
#define TBI_DIAG_LOOP                       (90)
#define TBI_DIAG_RESIGNLAL                  (91)
#define TBI_DIAG_SET_TRANSFORM_GROUP        (118)
#define TBI_DIAG_SIGNAL                     (92)
#define TBI_DIAG_TEMPORARY_TABLE            (93)
#define TBI_DIAG_WHILE                      (97)

/* List of Additional SQL Statements and Codes in SQL3 and CLI */
#define SQL_DIAG_CREATE_ROLE                    (61)
#define SQL_DIAG_CREATE_ROUTINE                 (14)
#define SQL_DIAG_CREATE_TRIGGER                 (80)
#define SQL_DIAG_CREATE_TYPE                    (83)
#define SQL_DIAG_DROP_ROLE                      (29)
#define SQL_DIAG_DROP_ROUTINE                   (30)
#define SQL_DIAG_DROP_TRANSFORM                 (116)
#define SQL_DIAG_DROP_TRIGGER                   (34)
#define SQL_DIAG_DROP_ORDERING                  (115)
#define SQL_DIAG_DROP_TYPE                      (35)
#define SQL_DIAG_GRANT_ROLE                     (49)
#define SQL_DIAG_RELEASE_SAVEPOINT              (57)
#define SQL_DIAG_RETURN                         (58)
#define SQL_DIAG_REVOKE_ROLE                    (60)
#define SQL_DIAG_SAVEPOINT                      (63)
#define SQL_DIAG_SET_ROLE                       (73)
#define SQL_DIAG_SET_SESSION_CHARACTERISTICS    (109)
#define SQL_DIAG_START_TRANSACTION              (111)

/* ODBC SQL Statements */
#define SQL_DIAG_CLASS_ORGIN                    (121)

/* SQL_DESC_TYPE 1002 (revised by jshwang) */

/* SQL_TYPE */
#define SQL_DEFAULT             (9999)  /* Tibero Specific */
#define SQL_UNKNOWN             (999)   /* Tibero specific */
#define SQL_NONE                (0)     /* Tibero compatibility */
#define SQL_BIT_VARYING         (15)
#define SQL_BOOLEAN             (16)
#define SQL_UDT                 (17)
#define SQL_UDT_LOCATOR         (18)
#define SQL_ROW_TYPE            (19)
#define SQL_REF                 (20)
#define SQL_BLOB                (30)
#define SQL_BLOB_LOCATOR        (31)
#define SQL_CLOB                (40)
#define SQL_CLOB_LOCATOR        (41)
#define SQL_ARRAY               (50)
#define SQL_ARRAY_LOCATOR       (51)
#define SQL_CSR                 (52)
#define SQL_TABLE               (54)
#define SQL_IDX_BY_TBL          (55)
#define SQL_BFLOAT              (56)
#define SQL_BDOUBLE             (57)
#define SQL_RECORD              (58)

/* -- SQL_C_TYPE */
#define SQL_C_INTERVAL_C        (22)
/* ODBC Compatibility */
#define SQL_C_CLOB_LOCATOR      (701)
#define SQL_C_BLOB_LOCATOR      (702)
#define SQL_C_INT               (703)
#define SQL_C_OCI_CHAR          (704)
#define SQL_C_UINT              (705)
#define SQL_C_TBNUMBER          (706)

/* for ESQL */
#define SQL_INT                 SQL_C_INT
#define SQL_C_CBL_COMP3         (707)
#define SQL_C_CBL_S9            (708)
#define SQL_C_CBL_S9V9          (709)
#define SQL_C_CBL_S9_COMP       (710)
#define SQL_C_CBL_S9_REAL_COMP  (711)
#define SQL_C_CBL_S9_DSL        (719)
#define SQL_C_CBL_S9V9_DSL      (720)
#define SQL_C_CBL_COMP1         (721)
#define SQL_C_CBL_COMP2         (722)

/* for OCI */
#define SQL_C_OCI_WCHAR           (712)
#define SQL_C_OCI_NUMBER          (713) /* SQLT_VNU */
#define SQL_C_OCI_DATE            (714) /* SQLT_ODT */
#define SQL_C_OCI_RAW             (715)
#define SQL_C_OCI_STRING          (716)
#define SQL_C_OCI_INTERNAL_NUMBER (717) /* SQLT_NUM */
#define SQL_C_OCI_OBJECT          (718) /* SQLT_NTY */
#define SQL_C_OCI_DAT             (723) /* SQLT_DAT */
#define SQL_C_OCI_VCS             (724) /* SQLT_VCS */

/* for dblink */
#define SQL_C_RECORD             (730)

/* date/time type subcodes */
#define SQL_CODE_TIME_ZONE          (4)
#define SQL_CODE_TIMESTAMP_ZONE     (5)

/* SQLFreeStmt() options */
#define SQL_REALLOCATE              (4)

/* SQL_ALTER_TABLE bitmasks */
#define SQL_AT_ALTER_COLUMN         (0x00000004L)
#define SQL_AT_DROP_CONSTRAINT      (0x00000010L)

/* SQL_OUTER_JOIN_CAPABILITIES bitmasks */
#define SQL_OUTER_JOIN_LEFT                     (0x00000001L)
#define SQL_OUTER_JOIN_RIGHT                    (0x00000002L)
#define SQL_OUTER_JOIN_FULL                     (0x00000004L)
#define SQL_OUTER_JOIN_NESTED                   (0x00000008L)
#define SQL_OUTER_JOIN_NOT_ORDERED              (0x00000010L)
#define SQL_OUTER_JOIN_INNER                    (0x00000020L)
#define SQL_OUTER_JOIN_ALL_COMPARISION_OPS      (0x00000040L)

#define SQL_TRANSACTION_READ_SERIALIZABLE       SQL_TXN_SERIALIZABLE

/* for SQLSetPos() */
#define SQL_NO_POSITION         (-1) /* Only Tibero Compatibility */

/************************************************/
/* Extended definitions for SQLGetInfo          */
/************************************************/

/* Information requested by SQLGetInfo() */
#if (ODBCVER >= 0x0300)
#define SQL_MAX_DRIVER_CONNECTIONS          0
#define SQL_MAXIMUM_DRIVER_CONNECTIONS      SQL_MAX_DRIVER_CONNECTIONS
#define SQL_MAX_CONCURRENT_ACTIVITIES       1
#define SQL_MAXIMUM_CONCURRENT_ACTIVITIES   SQL_MAX_CONCURRENT_ACTIVITIES
#endif
#define SQL_DATA_SOURCE_NAME                2
#define SQL_FETCH_DIRECTION                 8
#define SQL_SERVER_NAME                     13
#define SQL_SEARCH_PATTERN_ESCAPE           14
#define SQL_DBMS_NAME                       17
#define SQL_DBMS_VER                        18
#define SQL_ACCESSIBLE_TABLES               19
#define SQL_ACCESSIBLE_PROCEDURES           20
#define SQL_CURSOR_COMMIT_BEHAVIOR          23
#define SQL_DATA_SOURCE_READ_ONLY           25
#define SQL_DEFAULT_TXN_ISOLATION           26
#define SQL_IDENTIFIER_CASE                 28
#define SQL_IDENTIFIER_QUOTE_CHAR           29
#define SQL_MAX_COLUMN_NAME_LEN             30
#define SQL_MAXIMUM_COLUMN_NAME_LENGTH      SQL_MAX_COLUMN_NAME_LEN
#define SQL_MAX_CURSOR_NAME_LEN             31
#define SQL_MAXIMUM_CURSOR_NAME_LENGTH      SQL_MAX_CURSOR_NAME_LEN
#define SQL_MAX_SCHEMA_NAME_LEN             32
#define SQL_MAXIMUM_SCHEMA_NAME_LENGTH      SQL_MAX_SCHEMA_NAME_LEN
#define SQL_MAX_CATALOG_NAME_LEN            34
#define SQL_MAXIMUM_CATALOG_NAME_LENGTH     SQL_MAX_CATALOG_NAME_LEN
#define SQL_MAX_TABLE_NAME_LEN              35
#define SQL_SCROLL_CONCURRENCY              43
#define SQL_TXN_CAPABLE                     46
#define SQL_TRANSACTION_CAPABLE             SQL_TXN_CAPABLE
#define SQL_USER_NAME                       47
#define SQL_TXN_ISOLATION_OPTION            72
#define SQL_TRANSACTION_ISOLATION_OPTION    SQL_TXN_ISOLATION_OPTION
#define SQL_INTEGRITY                       73
#define SQL_GETDATA_EXTENSIONS              81
#define SQL_NULL_COLLATION                  85
#define SQL_ALTER_TABLE                     86
#define SQL_ORDER_BY_COLUMNS_IN_SELECT      90
#define SQL_SPECIAL_CHARACTERS              94
#define SQL_MAX_COLUMNS_IN_GROUP_BY         97
#define SQL_MAXIMUM_COLUMNS_IN_GROUP_BY     SQL_MAX_COLUMNS_IN_GROUP_BY
#define SQL_MAX_COLUMNS_IN_INDEX            98
#define SQL_MAXIMUM_COLUMNS_IN_INDEX        SQL_MAX_COLUMNS_IN_INDEX
#define SQL_MAX_COLUMNS_IN_ORDER_BY         99
#define SQL_MAXIMUM_COLUMNS_IN_ORDER_BY     SQL_MAX_COLUMNS_IN_ORDER_BY
#define SQL_MAX_COLUMNS_IN_SELECT           100
#define SQL_MAXIMUM_COLUMNS_IN_SELECT       SQL_MAX_COLUMNS_IN_SELECT
#define SQL_MAX_COLUMNS_IN_TABLE            101
#define SQL_MAX_INDEX_SIZE                  102
#define SQL_MAXIMUM_INDEX_SIZE              SQL_MAX_INDEX_SIZE
#define SQL_MAX_ROW_SIZE                    104
#define SQL_MAXIMUM_ROW_SIZE                SQL_MAX_ROW_SIZE
#define SQL_MAX_STATEMENT_LEN               105
#define SQL_MAXIMUM_STATEMENT_LENGTH        SQL_MAX_STATEMENT_LEN
#define SQL_MAX_TABLES_IN_SELECT            106
#define SQL_MAXIMUM_TABLES_IN_SELECT        SQL_MAX_TABLES_IN_SELECT
#define SQL_MAX_USER_NAME_LEN               107
#define SQL_MAXIMUM_USER_NAME_LENGTH        SQL_MAX_USER_NAME_LEN
#if (ODBCVER >= 0x0300)
#define SQL_OJ_CAPABILITIES                 115
#define SQL_OUTER_JOIN_CAPABILITIES         SQL_OJ_CAPABILITIES
#endif /* ODBCVER >= 0x0300 */

/* Direct Path Loading data save actions */
#define SQL_DPL_DATASAVE_SAVE_ONLY      0
#define SQL_DPL_DATASAVE_FINISH         1

/* Direct Path Loading flags */
#define SQL_DPL_FLAG_NOLOG              0
#define SQL_DPL_FLAG_LOG                1

/* Direct Path Parallel flags */
#define SQL_DPL_FLAG_NOPARALLEL         0
#define SQL_DPL_FLAG_PARALLEL           1

/* Direct Path Loading Patial Flags */
#define SQL_DPL_COL_COMPLETE            0
#define SQL_DPL_COL_PARTIAL             1
#define SQL_DPL_COL_NULL                2

/* 커서 타입 IN-OUT paramter를 가진 procedure의 파라미터를 OUT
   파라미터로만 사용할 경우 AP는 그 파라미터에 대해 아래 값을
   바인드해야만 한다. */
#define SQL_NULL_CSR                    4294967295U

/* callback event attribute */
#define SQL_FO_BEGIN                    0
#define SQL_FO_ABORT                    1
#define SQL_FO_END                      2
#define SQL_FO_REAUTH                   3
#define SQL_FO_ERROR                    4
#define SQL_FO_RETRY                    5

/* callback type attribute */
#define SQL_FO_SESSION                  0

/* endian */
#define SQL_SERVER_ENDIAN               1500
#define SQL_BIG_ENDIAN                  0
#define SQL_LITTLE_ENDIAN               1

#ifdef _GW_DRDA
/* gateway for drda */
#define SQL_ATTR_ENV_UNKNOWN_1064    1064
#define SQL_ATTR_DBC_UNKNOWN_1041    1041
#define SQL_ATTR_DBC_UNKNOWN_1042    1042
#endif

/* Data Types */
typedef long long           SQLBIGINT;      /* ODBC Compatibility */
typedef unsigned long long  SQLUBIGINT;     /* ODBC Compatibility */
typedef void               *SQLHDPCTX;      /* directpath context handle */

/* Failover Callback Function */
typedef SQLRETURN (*SQLCallbackFailover)(SQLHDBC connectionHandle,
                                         SQLPOINTER *foCtx,
                                         SQLSMALLINT foType,
                                         SQLSMALLINT foEvent);
typedef struct {
  SQLCallbackFailover callbackFunc;
  SQLPOINTER         *foCtx;
} SQLFocbkStruct;

/* Alloc handle with set thread state (ST/MT) */
SQLRETURN SQL_API SQLAllocHandle2(SQLSMALLINT   HandleType,
                                  SQLHANDLE     InputHandle,
                                  SQLHANDLE    *OutputHandle,
                                  SQLSMALLINT   UseMultiThread);

SQLRETURN SQL_API SQLAllocEnv2(SQLHENV     *EnvironmentHandle,
                               SQLSMALLINT  UseMultiThread);

/* Binding parameter (for named parameter) */
SQLRETURN SQL_API SQLBindParamByName(SQLHSTMT       StatementHandle,
                                     SQLCHAR       *PlaceHolder,
                                     SQLSMALLINT    NameLength,
                                     SQLSMALLINT    fParamType,
                                     SQLSMALLINT    fCType,
                                     SQLSMALLINT    fSqlType,
                                     SQLUINTEGER    cbColDef,
                                     SQLSMALLINT    ibScale,
                                     SQLPOINTER     rgbValue,
                                     SQLINTEGER     cbValueMax,
                                     SQLINTEGER    *pcbValue);

/* Result set API */
SQLRETURN SQL_API SQLRowCountFetched(SQLHSTMT       StatementHandle,
                                     SQLUINTEGER   *RowCount);

/* LOB API */
SQLRETURN SQL_API SQLLobClose(SQLHSTMT          StatementHandle,
                              SQLINTEGER        Locator,
                              SQLINTEGER       *Indicator);

SQLRETURN SQL_API SQLLobFreeLoc(SQLHSTMT StmtHandle, SQLINTEGER Locator);

SQLRETURN SQL_API SQLLobGetLocatorLength(SQLHSTMT        StatementHandle,
                                         SQLINTEGER      Locator,
                                         SQLINTEGER     *Length);

SQLRETURN SQL_API SQLLobGetLocator(SQLHSTMT      StatementHandle,
                                   SQLINTEGER    LocatorNum,
                                   SQLPOINTER    LocatorPtr);

SQLRETURN SQL_API SQLLobGetLength(SQLHSTMT      StatementHandle,
                                  SQLINTEGER    Locator,
                                  SQLBIGINT    *StringLength,
                                  SQLINTEGER   *Indicator);

SQLRETURN SQL_API SQLLobGetData(SQLHSTMT        StatementHandle,
                                SQLINTEGER      SourceLocator,
                                SQLBIGINT       Offset,
                                SQLINTEGER     *ByteLength,
                                SQLINTEGER     *CharLength,
                                SQLSMALLINT     TargetCType,
                                SQLPOINTER      Data,
                                SQLINTEGER      BufferLength,
                                SQLLEN         *Indicator);

SQLRETURN SQL_API SQLLobGetData2(SQLHSTMT       StatementHandle,
                                 SQLINTEGER     Locator,
                                 SQLBIGINT      Offset,
                                 SQLINTEGER    *ReadLength,
                                 SQLSMALLINT    TargetCType,
                                 SQLPOINTER     DataPtr,
                                 SQLINTEGER     BufferLength,
                                 SQLLEN        *Indicator);

SQLRETURN SQL_API SQLLobGetPosition(SQLHSTMT        StatementHandle,
                                    SQLINTEGER      SourceLocator,
                                    SQLINTEGER      SearchLocator,
                                    SQLCHAR        *SearchLiteral,
                                    SQLINTEGER      SearchLiteralLength,
                                    SQLBIGINT       FromPosition,
                                    SQLBIGINT      *LocatedAt,
                                    SQLINTEGER     *Indicator);

SQLRETURN SQL_API SQLLobOpen(SQLHSTMT           StatementHandle,
                             SQLINTEGER         Locator,
                             SQLSMALLINT        Mode,
                             SQLINTEGER        *Indicator);

SQLRETURN SQL_API SQLLobPutData(SQLHSTMT        StatementHandle,
                                SQLINTEGER     *DestLocator,
                                SQLBIGINT       DestOffset,
                                SQLSMALLINT     SourceCType,
                                SQLPOINTER      Source,
                                SQLINTEGER     *ByteLength,
                                SQLINTEGER     *CharLength,
                                SQLINTEGER     *Indicator);

SQLRETURN SQL_API SQLLobTruncate(SQLHSTMT       StatementHandle,
                                 SQLINTEGER    *Locator,
                                 SQLBIGINT      NewLength,
                                 SQLINTEGER    *Indicator);

/* tibero specified function */
SQLRETURN SQL_API SQLGetWarningMsgLength(SQLHSTMT       StatementHandle,
                                         SQLINTEGER    *Length);

SQLRETURN SQL_API SQLGetWarningMsg(SQLHSTMT     StatementHandle,
                                   SQLCHAR     *OutputString,
                                   SQLINTEGER   BufLength);

SQLRETURN SQL_API SQLRollBackToSavePoint(SQLHDBC    ConnectionHandle,
                                         SQLCHAR   *SavePoint);

SQLRETURN SQL_API SQLBindNativeParameter(SQLHSTMT       StatementHandle,
                                         SQLUSMALLINT   ParameterNumber,
                                         SQLSMALLINT    ParameterType,
                                         SQLSMALLINT    InputOutputType,
                                         SQLUINTEGER    ColumnSize,
                                         SQLSMALLINT    DecimalDigits,
                                         SQLPOINTER     ParameterValue,
                                         SQLINTEGER     BufferLength,
                                         SQLLEN        *StrLen_or_Ind);

/* direct path loading */
SQLRETURN SQL_API SQLAllocDirPathCtx(SQLHENV        EnvironmentHandle,
                                     SQLHDPCTX     *DirPathCtxHandle);

SQLRETURN SQL_API SQLSetDirPathAttr(SQLHDPCTX       DirPathCtxHandle,
                                    SQLINTEGER      Attribute,
                                    SQLPOINTER      Value,
                                    SQLINTEGER      StringLength);

SQLRETURN SQL_API SQLGetDirPathAttr(SQLHDPCTX       DirPathCtxHandle,
                                    SQLINTEGER      Attribute,
                                    SQLPOINTER      Value);

SQLRETURN SQL_API SQLDirPathPrepare(SQLHDPCTX   DirPathCtxHandle,
                                    SQLHDBC     ConnectionHandle);

SQLRETURN SQL_API SQLDirPathColArrayEntrySet(SQLHDPCTX      DirPathCtxHandle,
                                             SQLINTEGER     RowIndex,
                                             SQLSMALLINT    ColIndex,
                                             SQLPOINTER     ColumnValue,
                                             SQLINTEGER     ColumnLen,
                                             SQLLEN        *ReadLen,
                                             SQLSMALLINT    Flag);

SQLRETURN SQL_API SQLDirPathColArrayToStream(SQLHDPCTX      DirPathCtxHandle,
                                             SQLSMALLINT    RowNum);

SQLRETURN SQL_API SQLDirPathLoadStream(SQLHDPCTX DirPathCtxHandle);

SQLRETURN SQL_API SQLDirPathDataSave(SQLHDPCTX      DirPathCtxHandle,
                                     SQLSMALLINT    Action);

SQLRETURN SQL_API SQLDirPathAbort(SQLHDPCTX DirPathCtxHandle);

SQLRETURN SQL_API SQLDirPathFinish(SQLHDPCTX DirPathCtxHandle);

SQLRETURN SQL_API SQLDirPathFlushRow(SQLHDPCTX DirPathCtxHandle);

SQLRETURN SQL_API SQLDirPathStreamReset(SQLHDPCTX DirPathCtxHandle);

SQLRETURN SQL_API SQLGetLastSQLInfo(SQLHDBC         ConnectionHandle,
                                    SQLSMALLINT     SessionIdentifier,
                                    SQLCHAR        *SqlIdentifier,
                                    SQLCHAR        *HashVal);

SQLRETURN SQL_API SQLGetLastSQLInfo2(SQLHDBC         ConnectionHandle,
                                    SQLSMALLINT     SessionIdentifier,
                                    SQLCHAR        *SqlIdentifier,
                                    SQLCHAR        *ChildNumber);

SQLRETURN SQL_API SQLSesskill(SQLHENV   EnvironmentHandle,
                              SQLHDBC   ConnectionHandle);

SQLRETURN SQL_API SQLStmtCancel(SQLHSTMT StatementHandle);

SQLRETURN SQL_API SQLXAInit(SQLHDBC ConnectionHandle);
SQLRETURN SQL_API SQLXAInitWithRmid(SQLHDBC ConnectionHandle,
                                    SQLINTEGER rmid);

SQLRETURN SQL_API SQLDescribeCursor(SQLHANDLE       StatementHandle,
                                    SQLUINTEGER     CursorIdentifier);

/* external procedure utility */
SQLRETURN SQL_API SQLGetExtProcConnect(ExtProcContext  *Context,
                                       SQLHENV         *EnvironmentHandle,
                                       SQLHDBC         *ConnectionHandle,
                                       SQLSMALLINT     *ErrorHandleType,
                                       SQLHANDLE       *ErrorHandle);
SQLRETURN SQL_API SQLExtProcGetConnect(ExtProcContext  *Context,
                                       SQLHENV         *EnvironmentHandle,
                                       SQLHDBC         *ConnectionHandle,
                                       SQLSMALLINT     *ErrorHandleType,
                                       SQLHANDLE       *ErrorHandle);
#if 0
SQLRETURN SQL_API SQLExtProcGetStmtWithArgs(ExtProcContext  *Context,
                                            SQLHSTMT        *StatementHandle,
                                            SQLSMALLINT     *ErrorHandleType,
                                            SQLHANDLE       *ErrorHandle);
#endif
SQLRETURN SQL_API SQLExtProcLobGetLength(ExtProcContext    *Context,
                                         SQLPOINTER         Locator,
                                         SQLBIGINT         *Length,
                                         SQLSMALLINT       *ErrorHandleType,
                                         SQLHANDLE         *ErrorHandle);
SQLRETURN SQL_API SQLExtProcLobGetData(ExtProcContext  *Context,
                                       SQLPOINTER       Locator,
                                       SQLBIGINT        Offset,
                                       SQLINTEGER      *ReadLength,
                                       SQLSMALLINT      TargetCType,
                                       SQLPOINTER       DataPtr,
                                       SQLINTEGER       BufferLength,
                                       SQLLEN          *Indicator,
                                       SQLSMALLINT     *ErrorHandleType,
                                       SQLHANDLE       *ErrorHandle);
SQLRETURN SQL_API SQLExtProcLobPutData(ExtProcContext  *Context,
                                       SQLPOINTER       DestLocator,
                                       SQLBIGINT        DestOffset,
                                       SQLSMALLINT      SourceCType,
                                       SQLPOINTER       SourcePtr,
                                       SQLINTEGER      *ByteLength,
                                       SQLINTEGER      *CharLength,
                                       SQLSMALLINT     *ErrorHandleType,
                                       SQLHANDLE       *ErrorHandle);
SQLPOINTER SQL_API SQLExtProcAllocMemory(ExtProcContext    *Context,
                                         SQLUINTEGER        AllocSize);
void SQL_API SQLExtProcRaiseError(ExtProcContext *Context, int ErrorCode);
void SQL_API SQLExtProcRaiseErrorWithMsg(ExtProcContext    *Context,
                                         int                ErrorCode,
                                         char              *ErrorMessage);
SQLRETURN SQL_API SQLGetClientCharset(SQLHANDLE hdbd, SQLINTEGER *charset);

/* Tibero Queue API */
SQLRETURN SQL_API SQLAllocSubscription(SQLHENV        EnvironmentHandle,
                                       SQLHSUBS      *SubscriptionHandle);
SQLRETURN SQL_API SQLFreeSubscription(SQLHSUBS        SubscriptionHandle);
SQLRETURN SQL_API SQLSetSubscriptionAttr(SQLHSUBS     SubscriptionHandle,
                                         SQLSMALLINT  Attribute,
                                         SQLPOINTER   Value,
                                         SQLINTEGER   StringLength);

SQLRETURN SQL_API SQLGetSubscriptionAttr(SQLHSUBS     SubscriptionHandle,
                                         SQLSMALLINT  Attribute,
                                         SQLPOINTER   Value,
                                         SQLINTEGER   BufferLength,
                                         SQLINTEGER  *StringLengthPtr);

SQLRETURN SQL_API SQLSubscriptionRegister(SQLHDBC     ConnectionHandle,
                                          SQLHSUBS    SubscriptionHandle);

SQLRETURN SQL_API SQLSubscriptionUnRegister(SQLHDBC   ConnectionHandle,
                                            SQLHSUBS  SubscriptionHandle);

/* Tibero OS Auth API */
SQLRETURN SQL_API SQLConnectOSAuth(SQLHDBC  ConnectionHandle);

#ifdef __cplusplus
}
#endif

#endif /* _SQLCLI_TB_H */
