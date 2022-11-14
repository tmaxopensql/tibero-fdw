/**
 * @file    sqlcli_epl.h
 * @brief   TODO brief documentation here.
 *
 * @author
 * @version $Id$
 */

#ifndef _SQLCLI_EPL_H
#define _SQLCLI_EPL_H

/*{{{ Headers ----------------------------------------------------------------*/
/*---------------------------------------------------------------- Headers }}}*/


void EPLGetUserenv(char *buf, int buf_len, char *field);

int EPLGetCurSID();

int EPLGetMyTID();

#endif /* no _SQLCLI_EPL_H */
