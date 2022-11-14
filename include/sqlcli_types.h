/**
 * @file    sqlcli_types.h
 * @brief   define the types used in ODBC and CLI
 *
 * @author  eacho, wrpark
 * @version \$Id$
 */

#ifndef _SQLCLI_TYPES_H
#define _SQLCLI_TYPES_H

/*
 * if ODBCVER is not defined, assume version 3.51
 */
#ifndef ODBCVER
#define ODBCVER 0x0351
#endif  /* ODBCVER */

/*
 * environment specific definitions
 */
#ifndef EXPORT
#define EXPORT
#endif  /* EXPORT */

#ifdef  _WIN32
#define SQL_API  __stdcall
#else
#define SQL_API
#endif  /* _WIN32 */

#ifndef RC_INVOKED

/*
 *  API declaration data types
 */
typedef unsigned char   SQLCHAR;
#if (ODBCVER >= 0x0300)
typedef signed char     SQLSCHAR;
typedef unsigned char   SQLDATE;
typedef unsigned char   SQLDECIMAL;
typedef double          SQLDOUBLE;
typedef double          SQLFLOAT;
#endif  /* ODBCVER >= 0x0300 */

/* SQLINTEGER / SQLUINTEGER */
#ifdef TB_ODBC
#if (__SIZEOF_LONG__ == 8) || (SIZEOF_LONG == 8)
typedef int                     SQLINTEGER;
typedef unsigned int            SQLUINTEGER;
#else
typedef long                    SQLINTEGER;
typedef unsigned long           SQLUINTEGER;
#endif  /* (SIZEOF_LONG == 8) */
#else
typedef long                    SQLINTEGER;
typedef unsigned long           SQLUINTEGER;
#endif  /* TB_ODBC */

/* SQLLEN / SQLULEN */
#ifdef _WIN64
typedef INT64                   SQLLEN;
typedef UINT64                  SQLULEN;
typedef UINT64                  SQLSETPOSIROW;
#else
#define SQLLEN                  long
#define SQLULEN                 unsigned long
#define SQLSETPOSIROW           unsigned short
#endif  /* _WIN64 */

/* other data types */
#if (ODBCVER >= 0x0300)
typedef unsigned char   SQLNUMERIC;
#endif  /* ODBCVER >= 0x0300 */
typedef void *          SQLPOINTER;
#if (ODBCVER >= 0x0300)
typedef float           SQLREAL;
#endif  /* ODBCVER >= 0x0300 */

typedef short           SQLSMALLINT;
typedef unsigned short  SQLUSMALLINT;

#if (ODBCVER >= 0x0300)
typedef unsigned char   SQLTIME;
typedef unsigned char   SQLTIMESTAMP;
typedef unsigned char   SQLVARCHAR;
#endif  /* ODBCVER >= 0x0300 */

/*
 * function return type
 */
typedef SQLSMALLINT     SQLRETURN;
#ifdef  TB_ODBC
typedef long            SQLHANDLE;
#else
typedef SQLINTEGER      SQLHANDLE;
#endif  /* TB_ODBC */
typedef SQLHANDLE       SQLHENV;
typedef SQLHANDLE       SQLHDBC;
typedef SQLHANDLE       SQLHSTMT;
typedef SQLHANDLE       SQLHDESC;
typedef SQLHANDLE       SQLHSUBS;

#ifdef __FUTURE_WORK
/* generic data structures */
#if (ODBCVER >= 0x0300)
#ifdef  _WIN32
typedef void *          SQLHANDLE;
#else
typedef SQLINTEGER      SQLHANDLE;
#endif  /* _WIN32 */
typedef SQLHANDLE       SQLHENV;
typedef SQLHANDLE       SQLHDBC;
typedef SQLHANDLE       SQLHSTMT;
typedef SQLHANDLE       SQLHDESC;
typedef SQLHANDLE       SQLHSUBS;
#else
#ifdef  _WIN32
typedef void *          SQLHENV;
typedef void *          SQLHDBC;
typedef void *          SQLHSTMT;
typedef void *          SQLHSUBS;
#else
typedef SQLINTEGER      SQLHENV;
typedef SQLINTEGER      SQLHDBC;
typedef SQLINTEGER      SQLHSTMT;
typedef SQLINTEGER      SQLHSUBS;
#endif  /* _WIN32 */
#endif  /* ODBCVER >= 0x0300 */
#endif  /* __FUTURE_WORK */

/*
 * SQL portable types for C
 */
typedef unsigned char           UCHAR;
typedef signed char             SCHAR;
typedef long int                SDWORD;
typedef short int               SWORD;
typedef unsigned long int       UDWORD;
typedef unsigned short int      UWORD;

typedef signed long             SLONG;
typedef signed short            SSHORT;
typedef unsigned long           ULONG;
typedef unsigned short          USHORT;
typedef double                  SDOUBLE;
typedef double                  LDOUBLE;
typedef float                   SFLOAT;

/*
 * for PHP 4.x support
 */
#ifndef FAR
#define FAR
#endif  /* FAR */

typedef void FAR *              PTR;
typedef void FAR *              HENV;
typedef void FAR *              HDBC;
typedef void FAR *              HSTMT;
typedef void FAR *              HSUBS;
typedef signed short            RETCODE;

#if defined(WIN32) || defined(OS2)
typedef HWND                    SQLHWND;
#else
/* placehold for future O/S GUI window handle definition */
typedef SQLPOINTER              SQLHWND;
#endif

#ifndef __SQLDATE
#define __SQLDATE
/* transfer types for DATE, TIME, TIMESTAMP */
typedef struct tagDATE_STRUCT {
    SQLSMALLINT    year;
    SQLUSMALLINT   month;
    SQLUSMALLINT   day;
} DATE_STRUCT;

#if (ODBCVER >= 0x0300)
typedef DATE_STRUCT SQL_DATE_STRUCT;
#endif  /* ODBCVER >= 0x0300 */

typedef struct tagTIME_STRUCT {
    SQLUSMALLINT   hour;
    SQLUSMALLINT   minute;
    SQLUSMALLINT   second;
} TIME_STRUCT;

#if (ODBCVER >= 0x0300)
typedef TIME_STRUCT SQL_TIME_STRUCT;
#endif /* ODBCVER >= 0x0300 */

typedef struct tagTIMESTAMP_STRUCT {
    SQLSMALLINT    year;
    SQLUSMALLINT   month;
    SQLUSMALLINT   day;
    SQLUSMALLINT   hour;
    SQLUSMALLINT   minute;
    SQLUSMALLINT   second;
    SQLUINTEGER    fraction;
} TIMESTAMP_STRUCT;

#if (ODBCVER >= 0x0300)
typedef TIMESTAMP_STRUCT    SQL_TIMESTAMP_STRUCT;
#endif  /* ODBCVER >= 0x0300 */

/*
 * enumerations for DATETIME_INTERVAL_SUBCODE values for interval data types
 * these values are from SQL-92
 */

#if (ODBCVER >= 0x0300)
typedef enum {
    SQL_IS_YEAR                 = 1,
    SQL_IS_MONTH                = 2,
    SQL_IS_DAY                  = 3,
    SQL_IS_HOUR                 = 4,
    SQL_IS_MINUTE               = 5,
    SQL_IS_SECOND               = 6,
    SQL_IS_YEAR_TO_MONTH        = 7,
    SQL_IS_DAY_TO_HOUR          = 8,
    SQL_IS_DAY_TO_MINUTE        = 9,
    SQL_IS_DAY_TO_SECOND        = 10,
    SQL_IS_HOUR_TO_MINUTE       = 11,
    SQL_IS_HOUR_TO_SECOND       = 12,
    SQL_IS_MINUTE_TO_SECOND     = 13
} SQLINTERVAL;

#endif  /* ODBCVER >= 0x0300 */

#if (ODBCVER >= 0x0300)
typedef struct tagSQL_YEAR_MONTH {
    SQLUINTEGER     year;
    SQLUINTEGER     month;
} SQL_YEAR_MONTH_STRUCT;

typedef struct tagSQL_DAY_SECOND {
    SQLUINTEGER     day;
    SQLUINTEGER     hour;
    SQLUINTEGER     minute;
    SQLUINTEGER     second;
    SQLUINTEGER     fraction;
} SQL_DAY_SECOND_STRUCT;

typedef struct tagSQL_INTERVAL_STRUCT {
    SQLINTERVAL     interval_type;
    SQLSMALLINT     interval_sign;

    union {
        SQL_YEAR_MONTH_STRUCT       year_month;
        SQL_DAY_SECOND_STRUCT       day_second;
    } intval;
} SQL_INTERVAL_STRUCT;
#endif  /* ODBCVER >= 0x0300 */

#endif  /* __SQLDATE    */

/* the ODBC C types for SQL_C_SBIGINT and SQL_C_UBIGINT */
#if (ODBCVER >= 0x0300)
#if (_MSC_VER >= 900)
#define ODBCINT64   __int64
#endif

/* If using other compilers, define ODBCINT64 to the
    approriate 64 bit integer type */
#ifdef  ODBCINT64
typedef ODBCINT64           SQLBIGINT;
typedef unsigned ODBCINT64  SQLUBIGINT;
#endif  /* ODBCINT64 */
#endif  /* ODBCVER >= 0x0300 */

/* internal representation of numeric data type */
#if (ODBCVER >= 0x0300)
#define SQL_MAX_NUMERIC_LEN     16
typedef struct tagSQL_NUMERIC_STRUCT {
    SQLCHAR     precision;
    SQLSCHAR    scale;
    SQLCHAR     sign;   /* 1 if positive, 0 if negative */
    SQLCHAR     val[SQL_MAX_NUMERIC_LEN];
} SQL_NUMERIC_STRUCT;
#endif  /* ODBCVER >= 0x0300 */

#if (ODBCVER >= 0x0350)
#ifdef GUID_DEFINED
typedef GUID    SQLGUID;
#else
#ifdef _WIN32
/* size is 16 */
typedef struct  tagSQLGUID {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[ 8 ];
} SQLGUID;
#else
typedef struct  tagSQLGUID {
    SQLCHAR Data1;
    SQLCHAR Data2;
    SQLCHAR Data3;
    SQLCHAR Data4[8];
} SQLGUID;
#endif

#endif  /* GUID_DEFINED */
#endif  /* ODBCVER >= 0x0350 */

typedef unsigned long int       BOOKMARK;

#if defined(__cplusplus) || defined(_WCHAR_T_DEFINED) || defined(_WCHAR_T)
typedef wchar_t SQLWCHAR;
#else
typedef unsigned short SQLWCHAR;
#endif

#ifdef UNICODE
typedef SQLWCHAR        SQLTCHAR;
#else
typedef SQLCHAR         SQLTCHAR;
#endif  /* UNICODE */

/* These make up for having no windows.h */
#ifndef _WIN32

#define BOOL                int
typedef void*               HWND;
typedef char                CHAR;

#ifdef  UNICODE

#if defined(__cplusplus) || defined(_WCHAR_T_DEFINED)
typedef wchar_t             TCHAR;
#else
typedef signed short        TCHAR;
#endif  /* _WCHAR_T_DEFINED */

#else

typedef char                TCHAR;

#endif  /* UNICODE */

typedef void                VOID;
typedef unsigned short      WORD;

#if (__SIZEOF_LONG__ == 4) || (SIZEOF_LONG == 4)
typedef unsigned long       DWORD;
#else
typedef unsigned int        DWORD;
#endif

typedef unsigned char       BYTE;

#if defined(__cplusplus) || defined(_WCHAR_T_DEFINED)
typedef wchar_t             WCHAR;
#else
typedef unsigned short      WCHAR;
#endif  /* _WCHAR_T_DEFINED */

typedef WCHAR *             LPWSTR;
typedef const char *        LPCSTR;
typedef const WCHAR *       LPCWSTR;
typedef TCHAR *             LPTSTR;
typedef char *              LPSTR;
typedef DWORD *             LPDWORD;

typedef void*               HINSTANCE;
#endif  /* _WIN32 */

#endif  /* RC_INVOKED */

#endif  /* _SQLCLI_TYPES_H */
