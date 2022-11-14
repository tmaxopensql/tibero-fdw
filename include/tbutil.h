/**
 * @file    tbutil.h
 * @brief   header file for tibero util api function.
 *
 * @author  gusami@tmax.co.kr
 * @version $Id$
 */

#ifndef _TBUTIL_H
#define _TBUTIL_H

#include "sqlca.h"
#include "sqlcli.h"

/* Exported file type */
#define SQL_DEL 0
#define SQL_WSF 1
#define SQL_IXF 2

typedef struct sqlstr
{
    SQLSMALLINT length;
    SQLTCHAR    *data;
} sqlstr;

typedef struct TBExpImpMeta
{
    SQLTCHAR     *fieldTerm;
    SQLSMALLINT  fieldTermLen;

    SQLTCHAR     *lineTerm;
    SQLSMALLINT  lineTermLen;

    SQLTCHAR     *enclStart;
    SQLSMALLINT  enclStartLen;

    SQLTCHAR     *enclEnd;
    SQLSMALLINT  enclEndLen;

    SQLTCHAR     *escape;            /* escape string: Only TBImport use it */
    SQLSMALLINT  escapeLen;
} TBExpImpMeta;

typedef struct TBExportIn
{
    TBExpImpMeta iMeta;
} TBExportIn;

typedef struct TBExportOut
{
    SQLINTEGER   oRowsExported;
} TBExportOut;

typedef struct TBExportStruct
{
    SQLTCHAR     *piDataFileName;    /* data file name with path : INPUT */
    SQLSMALLINT  iDataFileNameLen;   /* data file name length */

    /* TODO: struct definition */
    void        *piLobPathList;
    void        *piLobFileList;

    sqlstr      *piActionString;     /* action string : INPUT */
    SQLSMALLINT  iFileType;          /* file type
                                        - SQL_DEL, SQL_WSF, SQL_IXF : INPUT */

    SQLTCHAR     *piMsgFileName;     /* log file name with error,
                                        warning etc : INPUT */
    SQLSMALLINT  iMsgFileNameLen;    /* log file name length */

    TBExportIn  *piExportInfoIn;     /* information of Input */
    TBExportOut *poExportInfoOut;    /* information of Output */
} TBExportStruct;

typedef struct TBImportIn
{
    TBExpImpMeta iMeta;
    SQLINTEGER   iRowcount;
    SQLINTEGER   iSkipcount;
    SQLINTEGER   iCommitcount;
    SQLINTEGER   iErrorcount;        /* error data count that permitted */
} TBImportIn;

typedef struct TBImportOut
{
    SQLINTEGER   oRowsRead;
    SQLINTEGER   oRowsSkipped;
    SQLINTEGER   oRowsInserted;
    SQLINTEGER   oRowsUpdated;
    SQLINTEGER   oRowsRejected;
    SQLINTEGER   oRowsCommitted;
} TBImportOut;

typedef struct TBImportStruct
{
    SQLTCHAR     *piDataFileName;    /* data file name with path : INPUT */
    SQLSMALLINT  iDataFileNameLen;   /* data file name length */

    /* TODO: struct definition */
    void        *piLobPathList;
    void        *piLobFileList;

    sqlstr      *piActionString;     /* sql string : INPUT */
    SQLSMALLINT  iFileType;          /* file type
                                        - SQL_DEL, SQL_WSF, SQL_IXF : INPUT */

    SQLTCHAR     *piMsgFileName;     /* log file name with error,
                                        warning etc : INPUT */
    SQLSMALLINT  iMsgFileNameLen;    /* log file name length */

    SQLTCHAR     *piBadFileName;     /* bad data file name with rejected data
                                        : INPUT */
    SQLSMALLINT  iBadFileNameLen;    /* log file name length */

    BOOL         iDPL;               /* if DPL is used or not */
    BOOL         iTrailNullCols;     /* if missed columns is replaced by null */

    TBImportIn  *piImportInfoIn;     /* information of input */
    TBImportOut *poImportInfoOut;    /* information of output */
} TBImportStruct;

/*****************************************************************************
 *                  global function declaration                              *
 *****************************************************************************/
SQLRETURN SQL_API TBConnect(SQLTCHAR *dnsname, SQLTCHAR *username,
                            SQLTCHAR *pwd, struct sqlca *pSqlca);
SQLRETURN SQL_API TBSetConnect(SQLTCHAR *dnsname, SQLTCHAR *username,
                               SQLTCHAR *pwd, SQLHENV henv, SQLHDBC hdbc);
SQLRETURN SQL_API TBDisconnect(SQLTCHAR *dnsname, struct sqlca *pSqlca);

SQLRETURN SQL_API TBExport(SQLINTEGER versionNumber,
                           TBExportStruct *pParamStruct, struct sqlca *pSqlca);
SQLRETURN SQL_API TBImport(SQLINTEGER versionNumber,
                           TBImportStruct *pParamStruct, struct sqlca *pSqlca);

SQLRETURN SQL_API TBErrorToSqlCa(SQLINTEGER errcode, struct sqlca *pSqlca);
/*****************************************************************************
 *                  global variable declaration                              *
 *****************************************************************************/

#endif /* no _TBUTIL_H */
