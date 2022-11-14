
#ifndef _SQLCPR_H
#define _SQLCPR_H

#include <string.h>
#include <sqlcli.h>

#define SQL_SINGLE_RCTX (void *)0

#define SQLErrorGetText(arg1, arg2, arg3, arg4) sqlglmt(arg1, arg2, arg3, arg4)
#define SQLStmtGetText(arg1, arg2, arg3, arg4) sqlglst(arg1, arg2, arg3, arg4)
#define SQLColumnNullCheck(arg1, arg2, arg3, arg4) \
        sqlnult(arg1, arg2, arg3, arg4)
#define SQLNumberPrecV6(arg1, arg2, arg3, arg4) sqlprct(arg1, arg2, arg3, arg4)
#define SQLNUmberPrecv7(arg1, arg2, arg3, arg4) sqlpr2t(arg1, arg2, arg3, arg4)
#define SQLNumberPrecV7(arg1, arg2, arg3, arg4) sqlpr2t(arg1, arg2, arg3, arg4)
#define SQLNumberPrecv7(arg1, arg2, arg3, arg4) sqlpr2t(arg1, arg2, arg3, arg4)
#define SQLVarcharGetLength(arg1, arg2, arg3) sqlvcpt(arg1, arg2, arg3)

#ifdef  __cplusplus
extern "C" {
#endif

/* non-threaded API */
extern void sqlglm( char *, size_t *, size_t * );
extern void sqlgls( char *, size_t *, size_t * );
extern void sqliem( char *, int * );
extern void sqlnul( unsigned short *, unsigned short *, int * );
extern void sqlprc( unsigned int *, int *, int * );
extern void sqlpr2( unsigned int *, int *, int * );
extern void sqlvcp( unsigned int *, unsigned int * );
extern struct SQLDA *sqlald( int, size_t, size_t );
extern void sqlclu( struct SQLDA *);

/* thread safe API */
extern void sqlglmt( void *, char *, size_t *, size_t * );
extern void sqlglst( void *, char *, size_t *, size_t * );
extern void sqliemt( void *, char *, int* );
extern void sqlnult( void *, unsigned short *, unsigned short *, int * );
extern void sqlprct( void *, unsigned int *, int *, int * );
extern void sqlpr2t( void *, unsigned int *, int *, int *);
extern void sqlvcpt( void *, unsigned int *, unsigned int * );
extern struct SQLDA *sqlaldt( void *, int, size_t, size_t );
extern void sqlclut( void *, struct SQLDA *);
extern SQLRETURN SQLCtxGetHStmt(void *sqlctx,
                           char *dbname, int dbname_len, void *hstmt);
extern SQLRETURN SQLCtxGetHEnv(void *sqlctx, void *henv);
extern SQLRETURN SQLCtxGetHDbc(void *sqlctx,
                               char *dbname, int dbname_len, void *hdbc);

#ifdef  __cplusplus
}
#endif

/* k&r C - not ANSI standard */
/* TODO */

#endif 	/* _SQLCPR_H */

