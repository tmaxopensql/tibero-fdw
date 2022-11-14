#ifndef _SQLCA_H
#define _SQLCA_H

#define SQLERRMC_LEN	70

struct sqlca {
    char    sqlcaid[8];              /* "SQLCA" id */
    int     sqlabc;                  /* byte length of SQL structure */
    int     sqlcode;                 /* status code of SQL execution */

    struct {
        unsigned short sqlerrml;     /* message length */
        char           sqlerrmc[SQLERRMC_LEN]; /* error message */
    } sqlerrm;

    char    sqlerrp[8];              /* RESERVED */
    int     sqlerrd[6];              /* error */
	/* Element 0: empty						*/
	/* 1: OID of processed tuple if applicable			*/
	/* 2: number of rows affected after an INSERT, UPDATE, DELETE	*/
	/* 3: empty							*/
	/* 4: empty							*/
	/* 5: empty							*/

    char    sqlwarn[8];              /* warning */
  	/* Element 0: set to 'W' if at least one other is 'W'		*/
	/* 1: if 'W' at least one character string			*/
	/* value was truncated when it was stored into a host variable.	*/
	/* 2: if 'W' a (hopefully) non-fatal notice occured		*/
	/* 3: empty 							*/
	/* 4: empty							*/
	/* 5: empty							*/
	/* 6: empty							*/
	/* 7: empty							*/
    char    sqlext[8];               /* RESERVED */
};

#ifndef _WIN32
extern struct sqlca sqlca;
#else   /* _WIN32 */
#   ifdef __cplusplus
extern "C" struct sqlca __declspec(dllimport) sqlca;
#   else
extern struct sqlca __declspec(dllimport) sqlca;
#   endif
#endif  /* _WIN32 */

#endif  /* _SQLCA_H */
