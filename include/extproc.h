/**
 * @file    extproc.h
 * @brief   The Include File for PSM External Procedure
 *
 * @author  chhwang
 * @version $Id$
 */

#ifndef _EXTPROC_H
#define _EXTPROC_H

typedef void ExtProcContext;

extern void *SQLExtProcDirectAllocMemory(ExtProcContext *Context,
                                         size_t AllocSize);
extern void SQLExtProcDirectRaiseError(ExtProcContext *Context,
                                       int ErrorCode);
extern void SQLExtProcDirectRaiseErrorWithMsg(ExtProcContext *Context,
                                              int ErrorCode,
                                              char *ErrorMessage);

#endif  /* _EXTPROC_H */
