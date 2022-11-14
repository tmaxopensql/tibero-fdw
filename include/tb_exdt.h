/**
 * @file    tb_exdt.h
 * @brief   The Include File for external datatype
 *
 * @author  kskim
 * @version $Id$
 *
 */

#ifndef _TB_EXDT_H
#define _TB_EXDT_H

#define TB_NUMBER_SIZE          23

#define TB_NUMBER_UNSIGNED      0
#define TB_NUMBER_SIGNED        1

struct TBNUMBER {
    unsigned char data[TB_NUMBER_SIZE];
};
typedef struct TBNUMBER TBNUMBER;

SQLRETURN TBNumberFromInt(SQLHENV EnvironmentHandle,
                          const SQLPOINTER IntegerValuePtr,
                          SQLSMALLINT IntegerValueSize,
                          SQLSMALLINT IntegerValueSign, TBNUMBER *NumberPtr);

SQLRETURN TBNumberFromReal(SQLHENV EnvironmentHandle,
                           const SQLPOINTER RealValuePtr,
                           SQLSMALLINT RealValueSize, TBNUMBER *NumberPtr);

SQLRETURN TBNumberFromText(SQLHENV EnvironmentHandle, const SQLCHAR *Text,
                           SQLINTEGER TextLength, const SQLCHAR *Format,
                           SQLINTEGER FormatLength, TBNUMBER *NumberPtr);

SQLRETURN TBNumberToInt(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr,
                        SQLSMALLINT IntegerValueSize,
                        SQLSMALLINT IntegerValueSign,
                        SQLPOINTER IntegerValuePtr);

SQLRETURN TBNumberToReal(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr,
                         SQLSMALLINT RealValueSize, SQLPOINTER RealValuePtr);

SQLRETURN TBNumberToText(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr,
                         const SQLCHAR *Format, SQLINTEGER FormatLength,
                         SQLINTEGER BufferSize, SQLCHAR *BufferPtr);

SQLRETURN TBNumberAbs(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr,
                      TBNUMBER *ValuePtr);

SQLRETURN TBNumberAdd(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr1,
                      const TBNUMBER *NumberPtr2, TBNUMBER *ValuePtr);

SQLRETURN TBNumberCeil(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr,
                       TBNUMBER *ValuePtr);

SQLRETURN TBNumberCmp(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr1,
                      const TBNUMBER *NumberPtr2, SQLSMALLINT *ValuePtr);

SQLRETURN TBNumberDiv(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr1,
                      const TBNUMBER *NumberPtr2, TBNUMBER *ValuePtr);

SQLRETURN TBNumberFloor(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr,
                        TBNUMBER *ValuePtr);

SQLRETURN TBNumberMod(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr1,
                      const TBNUMBER *NumberPtr2, TBNUMBER *ValuePtr);

SQLRETURN TBNumberMul(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr1,
                      const TBNUMBER *NumberPtr2, TBNUMBER *ValuePtr);

SQLRETURN TBNumberPower(SQLHENV EnvironmentHandle, const TBNUMBER *ExponentBase,
                        const TBNUMBER *NumberPtr, TBNUMBER *ValuePtr);

SQLRETURN TBNumberPrec(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr,
                       SQLSMALLINT Precision, TBNUMBER *ValuePtr);

SQLRETURN TBNumberRound(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr,
                        SQLSMALLINT DecimalDigits, TBNUMBER *ValuePtr);

SQLRETURN TBNumberSqrt(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr,
                       TBNUMBER *ValuePtr);

SQLRETURN TBNumberSub(SQLHENV EnvironmentHandle, const TBNUMBER *NumberPtr1,
                      const TBNUMBER *NumberPtr2, TBNUMBER *ValuePtr);

#endif  /* _TB_EXDT_H */
