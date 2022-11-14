#ifndef _SQLDA_H
#define _SQLDA_H

struct SQLDA {
    int        N;
    char     **V;
    int       *L;
    short     *T;
    short    **I;
    int        F;
    char     **S;
    short     *M;
    short     *C;
    char     **X;
    short     *Y;
    short     *Z;
};

typedef struct SQLDA SQLDA;

#define SQLSQLDAAlloc(arg1, arg2, arg3, arg4) sqlaldt(arg1, arg2, arg3, arg4)
#define SQLSQLDAFree(arg1, arg2) sqlclut(arg1, arg2)

#endif /* no _SQLDA_H */
