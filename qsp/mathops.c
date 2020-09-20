/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
/*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "mathops.h"
#include "callbacks.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "objects.h"
#include "regexp.h"
#include "statements.h"
#include "text.h"
#include "time.h"
#include "variables.h"

QSPMathOperation qspOps[qspOpLast_Operation];
QSPMathOpName qspOpsNames[QSP_OPSLEVELS][QSP_MAXOPSNAMES];
int qspOpsNamesCounts[QSP_OPSLEVELS];
int qspOpMaxLen = 0;

INLINE void qspAddOperation(int, int, QSP_FUNCTION, int, int, int, ...);
INLINE void qspAddOpName(int, QSP_CHAR *, int);
INLINE int qspMathOpsCompare(const void *, const void *);
INLINE int qspMathOpStringFullCompare(const void *, const void *);
INLINE int qspMathOpStringCompare(const void *, const void *);
INLINE int qspGetNumber(QSPString *expr);
INLINE QSPString qspGetName(QSPString *expr);
INLINE int qspFunctionOpCode(QSPString funName);
INLINE int qspOperatorOpCode(QSPString *expr);
INLINE QSPString qspGetString(QSPString *expr);
INLINE QSPString qspGetQString(QSPString *expr);
INLINE QSPVariant qspValue(int, QSPVariant *, int *, int *);
INLINE QSP_BOOL qspCompileExprPushOpCode(int *opStack, int *argStack, int *opSp, int opCode);
INLINE QSP_BOOL qspAppendToCompiled(int opCode, int *itemsCount, QSPVariant *compValues, int *compOpCodes, int *compArgsCounts, int argsCount, QSPVariant v);
INLINE int qspCompileExpression(QSPString s, QSPVariant *compValues, int *compOpCodes, int *compArgsCounts);
INLINE void qspFunctionStrComp(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionStrFind(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionStrPos(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionRGB(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionMid(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionRand(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionDesc(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionGetObj(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionIsPlay(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionInstr(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionArrPos(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionArrComp(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionReplace(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionFunc(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionDynEval(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionMin(QSPVariant *, int, QSPVariant *);
INLINE void qspFunctionMax(QSPVariant *, int, QSPVariant *);

INLINE void qspAddOperation(int opCode, int priority, QSP_FUNCTION func, int resType, int minArgs, int maxArgs, ...)
{
    int i;
    va_list marker;
    qspOps[opCode].Priority = priority;
    qspOps[opCode].Func = func;
    qspOps[opCode].ResType = resType;
    qspOps[opCode].MinArgsCount = minArgs;
    qspOps[opCode].MaxArgsCount = maxArgs;
    if (maxArgs > 0)
    {
        va_start(marker, maxArgs);
        for (i = 0; i < maxArgs; ++i)
            qspOps[opCode].ArgsTypes[i] = va_arg(marker, int);
        va_end(marker);
    }
}

INLINE void qspAddOpName(int opCode, QSP_CHAR *opName, int level)
{
    int count, len;
    QSPString name = qspStringFromC(opName);
    count = qspOpsNamesCounts[level];
    qspOpsNames[level][count].Name = name;
    qspOpsNames[level][count].Code = opCode;
    qspOpsNamesCounts[level] = count + 1;
    /* Max length */
    len = qspStrLen(name);
    if (len > qspOpMaxLen) qspOpMaxLen = len;
}

INLINE int qspMathOpsCompare(const void *opName1, const void *opName2)
{
    return qspStrsComp(((QSPMathOpName *)opName1)->Name, ((QSPMathOpName *)opName2)->Name);
}

INLINE int qspMathOpStringFullCompare(const void *name, const void *compareTo)
{
    return qspStrsComp(*(QSPString *)name, ((QSPMathOpName *)compareTo)->Name);
}

INLINE int qspMathOpStringCompare(const void *name, const void *compareTo)
{
    QSPMathOpName *opName = (QSPMathOpName *)compareTo;
    return qspStrsNComp(*(QSPString *)name, opName->Name, qspStrLen(opName->Name));
}

void qspInitMath()
{
    /*
    Format:
        qspAddOperation(
            Operation,
            Priority,
            Function's Function,
            Result's Type,
            Minimum Arguments' Count,
            Maximum Arguments' Count,
            Arguments' Types [optional]
        );

        "Result's Type" and "Arguments' Types":
        -1 - Unknown / Any
        0 - Number
        1 - String
    */
    int i;
    for (i = 0; i < QSP_OPSLEVELS; ++i) qspOpsNamesCounts[i] = 0;
    qspOpMaxLen = 0;
    qspAddOperation(qspOpValue, 0, 0, 0, 0, 0);
    qspAddOperation(qspOpValueToFormat, 0, 0, 0, 0, 0);
    qspAddOperation(qspOpStart, 127, 0, -1, 0, 0);
    qspAddOperation(qspOpEnd, 0, 0, -1, 0, 0);
    qspAddOperation(qspOpOpenBracket, 127, 0, -1, 0, 0);
    qspAddOperation(qspOpCloseBracket, 0, 0, -1, 0, 0);
    qspAddOperation(qspOpOpenArrBracket, 127, 0, -1, 0, 0);
    qspAddOperation(qspOpCloseArrBracket, 0, 0, -1, 0, 0);
    qspAddOperation(qspOpNegation, 18, 0, 0, 1, 1, 0);
    qspAddOperation(qspOpAdd, 14, 0, -1, 2, 2, -1, -1);
    qspAddOperation(qspOpSub, 14, 0, 0, 2, 2, 0, 0);
    qspAddOperation(qspOpMul, 17, 0, 0, 2, 2, 0, 0);
    qspAddOperation(qspOpDiv, 17, 0, 0, 2, 2, 0, 0);
    qspAddOperation(qspOpMod, 16, 0, 0, 2, 2, 0, 0);
    qspAddOperation(qspOpNe, 10, 0, 0, 2, 2, -1, -1);
    qspAddOperation(qspOpLeq, 10, 0, 0, 2, 2, -1, -1);
    qspAddOperation(qspOpGeq, 10, 0, 0, 2, 2, -1, -1);
    qspAddOperation(qspOpEq, 10, 0, 0, 2, 2, -1, -1);
    qspAddOperation(qspOpLt, 10, 0, 0, 2, 2, -1, -1);
    qspAddOperation(qspOpGt, 10, 0, 0, 2, 2, -1, -1);
    qspAddOperation(qspOpAppend, 12, 0, 1, 2, 2, 1, 1);
    qspAddOperation(qspOpComma, 0, 0, 1, 2, 2, 1, 1);
    qspAddOperation(qspOpAnd, 7, 0, 0, 2, 2, 0, 0);
    qspAddOperation(qspOpOr, 6, 0, 0, 2, 2, 0, 0);
    qspAddOperation(qspOpLoc, 11, 0, 0, 1, 1, 1);
    qspAddOperation(qspOpObj, 11, 0, 0, 1, 1, 1);
    qspAddOperation(qspOpArrItem, 30, 0, -1, 1, 2, 1, -1);
    qspAddOperation(qspOpLastArrItem, 30, 0, -1, 1, 1, 1);
    qspAddOperation(qspOpNot, 8, 0, 0, 1, 1, 0);
    qspAddOperation(qspOpMin, 30, qspFunctionMin, -1, 1, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    qspAddOperation(qspOpMax, 30, qspFunctionMax, -1, 1, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    qspAddOperation(qspOpRand, 30, qspFunctionRand, 0, 1, 2, 0, 0);
    qspAddOperation(qspOpIIf, 30, 0, -1, 3, 3, 0, -1, -1);
    qspAddOperation(qspOpRGB, 30, qspFunctionRGB, 0, 3, 4, 0, 0, 0, 0);
    qspAddOperation(qspOpLen, 30, 0, 0, 1, 1, 1);
    qspAddOperation(qspOpIsNum, 30, 0, 0, 1, 1, -1);
    qspAddOperation(qspOpLCase, 30, 0, 1, 1, 1, 1);
    qspAddOperation(qspOpUCase, 30, 0, 1, 1, 1, 1);
    qspAddOperation(qspOpInput, 30, 0, 1, 1, 1, 1);
    qspAddOperation(qspOpStr, 30, 0, 1, 1, 1, 1);
    qspAddOperation(qspOpVal, 30, 0, 0, 1, 1, -1);
    qspAddOperation(qspOpArrSize, 30, 0, 0, 1, 1, 1);
    qspAddOperation(qspOpIsPlay, 30, qspFunctionIsPlay, 0, 1, 1, 1);
    qspAddOperation(qspOpDesc, 30, qspFunctionDesc, 1, 1, 1, 1);
    qspAddOperation(qspOpTrim, 30, 0, 1, 1, 1, 1);
    qspAddOperation(qspOpGetObj, 30, qspFunctionGetObj, 1, 1, 1, 0);
    qspAddOperation(qspOpStrComp, 30, qspFunctionStrComp, 0, 2, 2, 1, 1);
    qspAddOperation(qspOpStrFind, 30, qspFunctionStrFind, 1, 2, 3, 1, 1, 0);
    qspAddOperation(qspOpStrPos, 30, qspFunctionStrPos, 0, 2, 3, 1, 1, 0);
    qspAddOperation(qspOpMid, 30, qspFunctionMid, 1, 2, 3, 1, 0, 0);
    qspAddOperation(qspOpArrPos, 30, qspFunctionArrPos, 0, 2, 3, 1, -1, 0);
    qspAddOperation(qspOpArrComp, 30, qspFunctionArrComp, 0, 2, 3, 1, -1, 0);
    qspAddOperation(qspOpInstr, 30, qspFunctionInstr, 0, 2, 3, 1, 1, 0);
    qspAddOperation(qspOpReplace, 30, qspFunctionReplace, 1, 2, 3, 1, 1, 1);
    qspAddOperation(qspOpFunc, 30, qspFunctionFunc, -1, 1, 20, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    qspAddOperation(qspOpDynEval, 30, qspFunctionDynEval, -1, 1, 20, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    qspAddOperation(qspOpRnd, 30, 0, 0, 0, 0);
    qspAddOperation(qspOpCountObj, 30, 0, 0, 0, 0);
    qspAddOperation(qspOpMsecsCount, 30, 0, 0, 0, 0);
    qspAddOperation(qspOpQSPVer, 30, 0, 1, 0, 0);
    qspAddOperation(qspOpUserText, 30, 0, 1, 0, 0);
    qspAddOperation(qspOpCurLoc, 30, 0, 1, 0, 0);
    qspAddOperation(qspOpSelObj, 30, 0, 1, 0, 0);
    qspAddOperation(qspOpSelAct, 30, 0, 1, 0, 0);
    qspAddOperation(qspOpMainText, 30, 0, 1, 0, 0);
    qspAddOperation(qspOpStatText, 30, 0, 1, 0, 0);
    qspAddOperation(qspOpCurActs, 30, 0, 2, 0, 0);
    /* Names */
    qspAddOpName(qspOpCloseBracket, QSP_RRBRACK, 1);
    qspAddOpName(qspOpCloseArrBracket, QSP_RSBRACK, 1);
    qspAddOpName(qspOpAdd, QSP_ADD, 1);
    qspAddOpName(qspOpSub, QSP_SUB, 1);
    qspAddOpName(qspOpMul, QSP_MUL, 1);
    qspAddOpName(qspOpDiv, QSP_DIV, 1);
    qspAddOpName(qspOpMod, QSP_FMT("MOD"), 1);
    qspAddOpName(qspOpNe, QSP_NOTEQUAL1, 1);
    qspAddOpName(qspOpNe, QSP_NOTEQUAL2, 0);
    qspAddOpName(qspOpLeq, QSP_LESSEQ1, 0);
    qspAddOpName(qspOpLeq, QSP_LESSEQ2, 0);
    qspAddOpName(qspOpGeq, QSP_GREATEQ1, 0);
    qspAddOpName(qspOpGeq, QSP_GREATEQ2, 0);
    qspAddOpName(qspOpEq, QSP_EQUAL, 1);
    qspAddOpName(qspOpLt, QSP_LESS, 1);
    qspAddOpName(qspOpGt, QSP_GREAT, 1);
    qspAddOpName(qspOpAppend, QSP_APPEND, 1);
    qspAddOpName(qspOpComma, QSP_COMMA, 1);
    qspAddOpName(qspOpAnd, QSP_FMT("AND"), 1);
    qspAddOpName(qspOpOr, QSP_FMT("OR"), 1);
    qspAddOpName(qspOpLoc, QSP_FMT("LOC"), 1);
    qspAddOpName(qspOpObj, QSP_FMT("OBJ"), 1);
    qspAddOpName(qspOpNot, QSP_FMT("NO"), 1);
    qspAddOpName(qspOpMin, QSP_FMT("MIN"), 1);
    qspAddOpName(qspOpMin, QSP_STRCHAR QSP_FMT("MIN"), 1);
    qspAddOpName(qspOpMax, QSP_FMT("MAX"), 1);
    qspAddOpName(qspOpMax, QSP_STRCHAR QSP_FMT("MAX"), 1);
    qspAddOpName(qspOpRand, QSP_FMT("RAND"), 1);
    qspAddOpName(qspOpIIf, QSP_FMT("IIF"), 1);
    qspAddOpName(qspOpIIf, QSP_STRCHAR QSP_FMT("IIF"), 1);
    qspAddOpName(qspOpRGB, QSP_FMT("RGB"), 1);
    qspAddOpName(qspOpLen, QSP_FMT("LEN"), 1);
    qspAddOpName(qspOpIsNum, QSP_FMT("ISNUM"), 1);
    qspAddOpName(qspOpLCase, QSP_FMT("LCASE"), 1);
    qspAddOpName(qspOpLCase, QSP_STRCHAR QSP_FMT("LCASE"), 1);
    qspAddOpName(qspOpUCase, QSP_FMT("UCASE"), 1);
    qspAddOpName(qspOpUCase, QSP_STRCHAR QSP_FMT("UCASE"), 1);
    qspAddOpName(qspOpInput, QSP_FMT("INPUT"), 1);
    qspAddOpName(qspOpInput, QSP_STRCHAR QSP_FMT("INPUT"), 1);
    qspAddOpName(qspOpStr, QSP_FMT("STR"), 1);
    qspAddOpName(qspOpStr, QSP_STRCHAR QSP_FMT("STR"), 1);
    qspAddOpName(qspOpVal, QSP_FMT("VAL"), 1);
    qspAddOpName(qspOpArrSize, QSP_FMT("ARRSIZE"), 1);
    qspAddOpName(qspOpIsPlay, QSP_FMT("ISPLAY"), 1);
    qspAddOpName(qspOpDesc, QSP_FMT("DESC"), 1);
    qspAddOpName(qspOpDesc, QSP_STRCHAR QSP_FMT("DESC"), 1);
    qspAddOpName(qspOpTrim, QSP_FMT("TRIM"), 1);
    qspAddOpName(qspOpTrim, QSP_STRCHAR QSP_FMT("TRIM"), 1);
    qspAddOpName(qspOpGetObj, QSP_FMT("GETOBJ"), 1);
    qspAddOpName(qspOpGetObj, QSP_STRCHAR QSP_FMT("GETOBJ"), 1);
    qspAddOpName(qspOpStrComp, QSP_FMT("STRCOMP"), 1);
    qspAddOpName(qspOpStrFind, QSP_FMT("STRFIND"), 1);
    qspAddOpName(qspOpStrFind, QSP_STRCHAR QSP_FMT("STRFIND"), 1);
    qspAddOpName(qspOpStrPos, QSP_FMT("STRPOS"), 1);
    qspAddOpName(qspOpMid, QSP_FMT("MID"), 1);
    qspAddOpName(qspOpMid, QSP_STRCHAR QSP_FMT("MID"), 1);
    qspAddOpName(qspOpArrPos, QSP_FMT("ARRPOS"), 1);
    qspAddOpName(qspOpArrComp, QSP_FMT("ARRCOMP"), 1);
    qspAddOpName(qspOpInstr, QSP_FMT("INSTR"), 1);
    qspAddOpName(qspOpReplace, QSP_FMT("REPLACE"), 1);
    qspAddOpName(qspOpReplace, QSP_STRCHAR QSP_FMT("REPLACE"), 1);
    qspAddOpName(qspOpFunc, QSP_FMT("FUNC"), 1);
    qspAddOpName(qspOpFunc, QSP_STRCHAR QSP_FMT("FUNC"), 1);
    qspAddOpName(qspOpDynEval, QSP_FMT("DYNEVAL"), 1);
    qspAddOpName(qspOpDynEval, QSP_STRCHAR QSP_FMT("DYNEVAL"), 1);
    qspAddOpName(qspOpRnd, QSP_FMT("RND"), 1);
    qspAddOpName(qspOpCountObj, QSP_FMT("COUNTOBJ"), 1);
    qspAddOpName(qspOpMsecsCount, QSP_FMT("MSECSCOUNT"), 1);
    qspAddOpName(qspOpQSPVer, QSP_FMT("QSPVER"), 1);
    qspAddOpName(qspOpQSPVer, QSP_STRCHAR QSP_FMT("QSPVER"), 1);
    qspAddOpName(qspOpUserText, QSP_FMT("USER_TEXT"), 1);
    qspAddOpName(qspOpUserText, QSP_STRCHAR QSP_FMT("USER_TEXT"), 1);
    qspAddOpName(qspOpUserText, QSP_FMT("USRTXT"), 1);
    qspAddOpName(qspOpUserText, QSP_STRCHAR QSP_FMT("USRTXT"), 1);
    qspAddOpName(qspOpCurLoc, QSP_FMT("CURLOC"), 1);
    qspAddOpName(qspOpCurLoc, QSP_STRCHAR QSP_FMT("CURLOC"), 1);
    qspAddOpName(qspOpSelObj, QSP_FMT("SELOBJ"), 1);
    qspAddOpName(qspOpSelObj, QSP_STRCHAR QSP_FMT("SELOBJ"), 1);
    qspAddOpName(qspOpSelAct, QSP_FMT("SELACT"), 1);
    qspAddOpName(qspOpSelAct, QSP_STRCHAR QSP_FMT("SELACT"), 1);
    qspAddOpName(qspOpMainText, QSP_FMT("MAINTXT"), 1);
    qspAddOpName(qspOpMainText, QSP_STRCHAR QSP_FMT("MAINTXT"), 1);
    qspAddOpName(qspOpStatText, QSP_FMT("STATTXT"), 1);
    qspAddOpName(qspOpStatText, QSP_STRCHAR QSP_FMT("STATTXT"), 1);
    qspAddOpName(qspOpCurActs, QSP_FMT("CURACTS"), 1);
    qspAddOpName(qspOpCurActs, QSP_STRCHAR QSP_FMT("CURACTS"), 1);
    for (i = 0; i < QSP_OPSLEVELS; ++i)
        qsort(qspOpsNames[i], qspOpsNamesCounts[i], sizeof(QSPMathOpName), qspMathOpsCompare);
}

INLINE int qspGetNumber(QSPString *expr)
{
    int num = 0;
    QSP_CHAR *pos = expr->Str;
    while (pos < expr->End && qspIsDigit(*pos))
    {
        num = num * 10 + (*pos - QSP_FMT('0'));
        ++pos;
    }
    expr->Str = pos;
    if (num < 0) return INT_MAX; /* simple overflow protection */
    return num;
}

INLINE QSPString qspGetName(QSPString *expr)
{
    QSP_CHAR *startPos = expr->Str, *pos = startPos;
    while (++pos < expr->End)
    {
        if (qspIsInClass(*pos, QSP_CHAR_DELIM)) break;
    }
    expr->Str = pos;
    return qspStringFromPair(startPos, pos);
}

INLINE int qspFunctionOpCode(QSPString funName)
{
    QSPMathOpName *name = (QSPMathOpName *)bsearch(
        &funName,
        qspOpsNames[QSP_OPSLEVELS - 1],
        qspOpsNamesCounts[QSP_OPSLEVELS - 1],
        sizeof(QSPMathOpName),
        qspMathOpStringFullCompare);

    if (name) return name->Code;
    return qspOpUnknown;
}

INLINE int qspOperatorOpCode(QSPString *expr)
{
    int i;
    QSPMathOpName *name;
    if (qspIsEmpty(*expr)) return qspOpEnd;
    for (i = 0; i < QSP_OPSLEVELS; ++i)
    {
        name = (QSPMathOpName *)bsearch(expr, qspOpsNames[i], qspOpsNamesCounts[i], sizeof(QSPMathOpName), qspMathOpStringCompare);
        if (name)
        {
            expr->Str += qspStrLen(name->Name);
            return name->Code;
        }
    }
    return qspOpUnknown;
}

INLINE QSPString qspGetString(QSPString *expr)
{
    int strLen = 0, bufSize = 16;
    QSP_CHAR *buf, *pos = expr->Str, quot = *pos;
    buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
    while (1)
    {
        if (++pos >= expr->End)
        {
            qspSetError(QSP_ERR_QUOTNOTFOUND);
            free(buf);
            return qspNullString;
        }
        if (*pos == quot && (++pos >= expr->End || *pos != quot)) break;
        if (strLen >= bufSize)
        {
            bufSize = strLen + 128;
            buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
        }
        buf[strLen++] = *pos;
    }
    expr->Str = pos;
    return qspStringFromLen(buf, strLen);
}

INLINE QSPString qspGetQString(QSPString *expr)
{
    QSP_CHAR *pos, *buf = expr->Str;
    pos = qspDelimPos(*expr, QSP_RQUOT[0]);
    if (!pos)
    {
        qspSetError(QSP_ERR_QUOTNOTFOUND);
        return qspNullString;
    }
    expr->Str = pos + QSP_STATIC_LEN(QSP_RQUOT);
    return qspStringFromPair(buf + QSP_STATIC_LEN(QSP_LQUOT), pos);
}

INLINE QSPVariant qspValue(int itemsCount, QSPVariant *compValues, int *compOpCodes, int *compArgsCounts)
{
    int type;
    QSPVar *var;
    QSPVariant stack[QSP_STACKSIZE], args[QSP_OPMAXARGS], tos;
    QSPString name;
    int i, j, oldRefreshCount, opCode, argsCount, arrIndex, sp = -1, index = 0;
    tos.Type = QSP_TYPE_NUMBER;
    QSP_NUM(tos) = 0;
    oldRefreshCount = qspRefreshCount;
    while (1)
    {
        if (index == itemsCount) return tos;
        opCode = compOpCodes[index];
        argsCount = compArgsCounts[index];
        if (argsCount)
        {
            for (i = argsCount - 2, j = sp, args[i + 1] = tos; i >= 0; --i, --j)
                args[i] = stack[j];
            for (i = 0; i < argsCount; ++i)
            {
                type = qspOps[opCode].ArgsTypes[i];
                if (QSP_ISDEF(type) && !qspConvertVariantTo(args + i, type))
                {
                    qspSetError(QSP_ERR_TYPEMISMATCH);
                    break;
                }
            }
        }
        else
        {
            if (sp == QSP_STACKSIZE - 1)
            {
                qspSetError(QSP_ERR_STACKOVERFLOW);
                if (QSP_ISSTR(tos.Type)) qspFreeString(QSP_STR(tos));
                break;
            }
            stack[++sp] = tos;
        }
        if (!qspErrorNum)
        {
            type = qspOps[opCode].ResType;
            if (QSP_ISDEF(type)) tos.Type = type;
            switch (opCode)
            {
            case qspOpValue:
                tos = compValues[index];
                break;
            case qspOpValueToFormat:
                tos = compValues[index];
                if (QSP_ISSTR(tos.Type))
                {
                    name = QSP_STR(tos);
                    QSP_STR(tos) = qspFormatText(name, QSP_TRUE);
                    if (name.Str != QSP_STR(tos).Str) qspFreeString(name);
                }
                break;
            case qspOpArrItem:
            case qspOpLastArrItem:
                name = QSP_STR(args[0]);
                var = qspVarReference(name, QSP_FALSE);
                if (!var) break;
                if (opCode == qspOpLastArrItem)
                    arrIndex = var->ValsCount - 1;
                else if (argsCount == 2)
                    arrIndex = QSP_ISSTR(args[1].Type) ? qspGetVarTextIndex(var, QSP_STR(args[1]), QSP_FALSE) : QSP_NUM(args[1]);
                else
                    arrIndex = 0;
                if (!qspGetVarValueByReference(var, arrIndex, QSP_VARBASETYPE(name), &tos))
                {
                    qspSetError(QSP_ERR_TYPEMISMATCH);
                    qspFreeString(QSP_STR(tos));
                    break;
                }
                break;
            case qspOpMul:
                QSP_NUM(tos) = QSP_NUM(args[0]) * QSP_NUM(args[1]);
                break;
            case qspOpDiv:
                if (!QSP_NUM(args[1]))
                {
                    qspSetError(QSP_ERR_DIVBYZERO);
                    break;
                }
                QSP_NUM(tos) = QSP_NUM(args[0]) / QSP_NUM(args[1]);
                break;
            case qspOpAdd:
                if (QSP_ISNUM(args[0].Type) && QSP_ISNUM(args[1].Type)) /* tiny optimization for numbers */
                {
                    QSP_NUM(tos) = QSP_NUM(args[0]) + QSP_NUM(args[1]);
                    tos.Type = QSP_TYPE_NUMBER;
                }
                else if (QSP_ISSTR(args[0].Type) && QSP_ISSTR(args[1].Type))
                {
                    qspAddText(&QSP_STR(tos), QSP_STR(args[0]), QSP_TRUE);
                    qspAddText(&QSP_STR(tos), QSP_STR(args[1]), QSP_FALSE);
                    tos.Type = QSP_TYPE_STRING;
                }
                else if (qspIsCanConvertToNum(args) && qspIsCanConvertToNum(args + 1))
                {
                    qspConvertVariantTo(args, QSP_TYPE_NUMBER);
                    qspConvertVariantTo(args + 1, QSP_TYPE_NUMBER);
                    QSP_NUM(tos) = QSP_NUM(args[0]) + QSP_NUM(args[1]);
                    tos.Type = QSP_TYPE_NUMBER;
                }
                else
                {
                    /* Result is a string that can't be converted to a number */
                    qspConvertVariantTo(args, QSP_TYPE_STRING);
                    qspConvertVariantTo(args + 1, QSP_TYPE_STRING);
                    qspAddText(&QSP_STR(tos), QSP_STR(args[0]), QSP_TRUE);
                    qspAddText(&QSP_STR(tos), QSP_STR(args[1]), QSP_FALSE);
                    tos.Type = QSP_TYPE_STRING;
                }
                break;
            case qspOpSub:
                QSP_NUM(tos) = QSP_NUM(args[0]) - QSP_NUM(args[1]);
                break;
            case qspOpMod:
                if (!QSP_NUM(args[1]))
                {
                    qspSetError(QSP_ERR_DIVBYZERO);
                    break;
                }
                QSP_NUM(tos) = QSP_NUM(args[0]) % QSP_NUM(args[1]);
                break;
            case qspOpAppend:
                qspAddText(&QSP_STR(tos), QSP_STR(args[0]), QSP_TRUE);
                qspAddText(&QSP_STR(tos), QSP_STR(args[1]), QSP_FALSE);
                break;
            case qspOpComma:
                qspAddText(&QSP_STR(tos), QSP_STR(args[0]), QSP_TRUE);
                qspAddText(&QSP_STR(tos), QSP_STATIC_STR(QSP_VALSDELIM), QSP_FALSE);
                qspAddText(&QSP_STR(tos), QSP_STR(args[1]), QSP_FALSE);
                break;
            case qspOpEq:
                QSP_NUM(tos) = QSP_TOBOOL(!qspAutoConvertCompare(args, args + 1));
                break;
            case qspOpLt:
                QSP_NUM(tos) = QSP_TOBOOL(qspAutoConvertCompare(args, args + 1) < 0);
                break;
            case qspOpGt:
                QSP_NUM(tos) = QSP_TOBOOL(qspAutoConvertCompare(args, args + 1) > 0);
                break;
            case qspOpLeq:
                QSP_NUM(tos) = QSP_TOBOOL(qspAutoConvertCompare(args, args + 1) <= 0);
                break;
            case qspOpGeq:
                QSP_NUM(tos) = QSP_TOBOOL(qspAutoConvertCompare(args, args + 1) >= 0);
                break;
            case qspOpNe:
                QSP_NUM(tos) = QSP_TOBOOL(qspAutoConvertCompare(args, args + 1) != 0);
                break;
            case qspOpNegation:
                QSP_NUM(tos) = -QSP_NUM(args[0]);
                break;
            case qspOpNot:
                QSP_NUM(tos) = ~QSP_NUM(args[0]);
                break;
            case qspOpAnd:
                QSP_NUM(tos) = QSP_NUM(args[0]) & QSP_NUM(args[1]);
                break;
            case qspOpOr:
                QSP_NUM(tos) = QSP_NUM(args[0]) | QSP_NUM(args[1]);
                break;
            /* Embedded functions -------------------------------------------------------------- */
            case qspOpLoc:
                QSP_NUM(tos) = QSP_TOBOOL(qspLocIndex(QSP_STR(args[0])) >= 0);
                break;
            case qspOpObj:
                QSP_NUM(tos) = QSP_TOBOOL(qspObjIndex(QSP_STR(args[0])) >= 0);
                break;
            case qspOpIIf:
                qspCopyToNewVariant(&tos, QSP_NUM(args[0]) ? args + 1 : args + 2);
                break;
            case qspOpLen:
                QSP_NUM(tos) = qspStrLen(QSP_STR(args[0]));
                break;
            case qspOpIsNum:
                if (QSP_ISSTR(args[0].Type))
                    QSP_NUM(tos) = QSP_TOBOOL(qspIsNumber(QSP_STR(args[0])));
                else
                    QSP_NUM(tos) = QSP_TOBOOL(QSP_TRUE);
                break;
            case qspOpLCase:
                QSP_STR(tos) = qspGetNewText(QSP_STR(args[0]));
                qspLowerStr(&QSP_STR(tos));
                break;
            case qspOpUCase:
                QSP_STR(tos) = qspGetNewText(QSP_STR(args[0]));
                qspUpperStr(&QSP_STR(tos));
                break;
            case qspOpStr:
                QSP_STR(tos) = qspGetNewText(QSP_STR(args[0]));
                break;
            case qspOpVal:
                if (qspConvertVariantTo(args, QSP_TYPE_NUMBER))
                    QSP_NUM(tos) = QSP_NUM(args[0]);
                else
                    QSP_NUM(tos) = 0;
                break;
            case qspOpArrSize:
                QSP_NUM(tos) = qspArraySize(QSP_STR(args[0]));
                break;
            case qspOpTrim:
                QSP_STR(tos) = qspGetNewText(qspDelSpc(QSP_STR(args[0])));
                break;
            case qspOpInput:
                QSP_STR(tos) = qspCallInputBox(QSP_STR(args[0]));
                break;
            case qspOpRnd:
                QSP_NUM(tos) = qspRand() % 1000 + 1;
                break;
            case qspOpCountObj:
                QSP_NUM(tos) = qspCurObjectsCount;
                break;
            case qspOpMsecsCount:
                QSP_NUM(tos) = qspGetTime();
                break;
            case qspOpQSPVer:
                QSP_STR(tos) = qspGetNewText(QSP_STATIC_STR(QSP_VER));
                break;
            case qspOpUserText:
                QSP_STR(tos) = (qspCurInput.Str ? qspGetNewText(qspCurInput) : qspNullString);
                break;
            case qspOpCurLoc:
                QSP_STR(tos) = (qspCurLoc >= 0 ? qspGetNewText(qspLocs[qspCurLoc].Name) : qspNullString);
                break;
            case qspOpSelObj:
                QSP_STR(tos) = (qspCurSelObject >= 0 ? qspGetNewText(qspCurObjects[qspCurSelObject].Desc) : qspNullString);
                break;
            case qspOpSelAct:
                QSP_STR(tos) = (qspCurSelAction >= 0 ? qspGetNewText(qspCurActions[qspCurSelAction].Desc) : qspNullString);
                break;
            case qspOpMainText:
                QSP_STR(tos) = (qspCurDesc.Str ? qspGetNewText(qspCurDesc) : qspNullString);
                break;
            case qspOpStatText:
                QSP_STR(tos) = (qspCurVars.Str ? qspGetNewText(qspCurVars) : qspNullString);
                break;
            case qspOpCurActs:
                QSP_STR(tos) = qspGetAllActionsAsCode();
                break;
            /* External functions -------------------------------------------------------------- */
            default:
                qspOps[opCode].Func(args, argsCount, &tos);
                break;
            }
        }
        if (argsCount)
        {
            qspFreeVariants(args, argsCount);
            sp -= argsCount - 1;
        }
        if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
        ++index;
    }
    qspFreeVariants(stack, sp + 1);
    while (index < itemsCount)
    {
        switch (compOpCodes[index])
        {
            case qspOpValue:
            case qspOpValueToFormat:
                if (QSP_ISSTR(compValues[index].Type)) qspFreeString(QSP_STR(compValues[index]));
                break;
        }
        ++index;
    }
    return qspGetEmptyVariant(QSP_TYPE_UNDEFINED);
}

INLINE QSP_BOOL qspCompileExprPushOpCode(int *opStack, int *argStack, int *opSp, int opCode)
{
    if (*opSp == QSP_STACKSIZE - 1)
    {
        qspSetError(QSP_ERR_STACKOVERFLOW);
        return QSP_FALSE;
    }
    ++(*opSp);
    opStack[*opSp] = opCode;
    argStack[*opSp] = (opCode < qspOpFirst_Function ? qspOps[opCode].MinArgsCount : 0);
    return QSP_TRUE;
}

/* N.B. We can safely add operations with the highest priority directly to the output w/o intermediate stack */
INLINE QSP_BOOL qspAppendToCompiled(int opCode, int *itemsCount, QSPVariant *compValues, int *compOpCodes, int *compArgsCounts, int argsCount, QSPVariant v)
{
    if (*itemsCount == QSP_MAXITEMS)
    {
        qspSetError(QSP_ERR_TOOMANYITEMS);
        return QSP_FALSE;
    }
    compOpCodes[*itemsCount] = opCode;
    compArgsCounts[*itemsCount] = argsCount;
    switch (opCode)
    {
        case qspOpValue:
        case qspOpValueToFormat:
            compValues[*itemsCount] = v;
            break;
    }
    ++(*itemsCount);
    return QSP_TRUE;
}

INLINE int qspCompileExpression(QSPString s, QSPVariant *compValues, int *compOpCodes, int *compArgsCounts)
{
    QSPVariant v;
    QSPString name;
    QSP_BOOL waitForOperator = QSP_FALSE;
    int opStack[QSP_STACKSIZE], argStack[QSP_STACKSIZE], opCode, itemsCount = 0, opSp = -1;
    if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpStart)) return 0;
    while (1)
    {
        qspSkipSpaces(&s);
        if (waitForOperator)
        {
            opCode = qspOperatorOpCode(&s);
            if (opCode == qspOpUnknown || opCode >= qspOpFirst_Function)
            {
                qspSetError(QSP_ERR_UNKNOWNACTION);
                break;
            }
            /* We want to separate keywords */
            if ((opCode == qspOpAnd || opCode == qspOpOr || opCode == qspOpMod) && (qspIsEmpty(s) || !qspIsInClass(*s.Str, QSP_CHAR_SPACE | QSP_CHAR_QUOT | QSP_CHAR_EXPSTART)))
            {
                qspSetError(QSP_ERR_SYNTAX);
                break;
            }
            while (qspOps[opCode].Priority <= qspOps[opStack[opSp]].Priority && qspOps[opStack[opSp]].Priority != 127)
            {
                if (!qspAppendToCompiled(opStack[opSp], &itemsCount, compValues, compOpCodes, compArgsCounts, argStack[opSp], v)) break;
                if (--opSp < 0)
                {
                    qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
            }
            if (qspErrorNum) break;
            switch (opCode)
            {
            case qspOpEnd:
                if (opSp)
                {
                    qspSetError(QSP_ERR_BRACKNOTFOUND);
                    break;
                }
                return itemsCount;
            case qspOpCloseBracket:
                if (opStack[opSp] != qspOpOpenBracket)
                {
                    qspSetError(QSP_ERR_BRACKNOTFOUND);
                    break;
                }
                opCode = opStack[--opSp];
                if (opCode >= qspOpFirst_Function)
                {
                    ++argStack[opSp];
                    if (argStack[opSp] < qspOps[opCode].MinArgsCount || argStack[opSp] > qspOps[opCode].MaxArgsCount)
                        qspSetError(QSP_ERR_ARGSCOUNT);
                }
                break;
            case qspOpCloseArrBracket:
                if (opStack[opSp] != qspOpOpenArrBracket)
                {
                    qspSetError(QSP_ERR_BRACKNOTFOUND);
                    break;
                }
                opCode = opStack[--opSp];
                if (opCode != qspOpArrItem)
                {
                    qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                ++argStack[opSp];
                break;
            case qspOpComma:
                if (opStack[opSp] == qspOpOpenBracket && opStack[opSp - 1] >= qspOpFirst_Function)
                {
                    if (++argStack[opSp - 1] > qspOps[opStack[opSp - 1]].MaxArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                }
                else
                {
                    if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpComma)) break;
                }
                waitForOperator = QSP_FALSE;
                break;
            default:
                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, opCode)) break;
                waitForOperator = QSP_FALSE;
                break;
            }
            if (qspErrorNum) break;
        }
        else
        {
            if (qspIsEmpty(s))
            {
                if (opStack[opSp] >= qspOpFirst_Function)
                    qspSetError(QSP_ERR_ARGSCOUNT);
                else
                    qspSetError(QSP_ERR_SYNTAX);
                break;
            }
            else if (qspIsDigit(*s.Str))
            {
                v.Type = QSP_TYPE_NUMBER;
                QSP_NUM(v) = qspGetNumber(&s);
                if (opStack[opSp] == qspOpNegation)
                {
                    QSP_NUM(v) = -QSP_NUM(v);
                    --opSp;
                }
                if (!qspAppendToCompiled(qspOpValue, &itemsCount, compValues, compOpCodes, compArgsCounts, 0, v)) break;
                waitForOperator = QSP_TRUE;
            }
            else if (qspIsInClass(*s.Str, QSP_CHAR_QUOT))
            {
                name = qspGetString(&s);
                if (qspErrorNum) break;
                v.Type = QSP_TYPE_STRING;
                QSP_STR(v) = name;
                if (!qspAppendToCompiled(qspOpValueToFormat, &itemsCount, compValues, compOpCodes, compArgsCounts, 0, v))
                {
                    qspFreeString(QSP_STR(v));
                    break;
                }
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_LQUOT[0])
            {
                name = qspGetQString(&s);
                if (qspErrorNum) break;
                v.Type = QSP_TYPE_CODE;
                QSP_STR(v) = qspGetNewText(name);
                if (!qspAppendToCompiled(qspOpValue, &itemsCount, compValues, compOpCodes, compArgsCounts, 0, v))
                {
                    qspFreeString(QSP_STR(v));
                    break;
                }
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_NEGATION[0])
            {
                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpNegation)) break;
                s.Str += QSP_STATIC_LEN(QSP_NEGATION);
            }
            else if (*s.Str == QSP_LRBRACK[0])
            {
                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpOpenBracket)) break;
                s.Str += QSP_STATIC_LEN(QSP_LRBRACK);
            }
            else if (*s.Str == QSP_RRBRACK[0])
            {
                opCode = opStack[opSp];
                if (opCode != qspOpOpenBracket)
                {
                    if (opCode >= qspOpFirst_Function)
                        qspSetError(QSP_ERR_ARGSCOUNT);
                    else
                        qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                opCode = opStack[--opSp];
                if (opCode < qspOpFirst_Function)
                {
                    qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                if (argStack[opSp] < qspOps[opCode].MinArgsCount)
                {
                    qspSetError(QSP_ERR_ARGSCOUNT);
                    break;
                }
                s.Str += QSP_STATIC_LEN(QSP_RRBRACK);
                waitForOperator = QSP_TRUE;
            }
            else if (!qspIsInClass(*s.Str, QSP_CHAR_DELIM))
            {
                name = qspGetName(&s);
                if (qspErrorNum) break;
                qspSkipSpaces(&s);
                if (*name.Str == QSP_USERFUNC[0])
                {
                    /* Ignore a @ symbol */
                    name.Str += QSP_STATIC_LEN(QSP_USERFUNC);
                    /* Add the loc name */
                    v.Type = QSP_TYPE_STRING;
                    QSP_STR(v) = qspGetNewText(name);
                    if (!qspAppendToCompiled(qspOpValue, &itemsCount, compValues, compOpCodes, compArgsCounts, 0, v))
                    {
                        qspFreeString(QSP_STR(v));
                        break;
                    }
                    /* Add a function call */
                    if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpFunc)) break;
                    ++argStack[opSp];
                    if (!qspIsEmpty(s) && *s.Str == QSP_LRBRACK[0])
                    {
                        if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpOpenBracket)) break;
                        s.Str += QSP_STATIC_LEN(QSP_LRBRACK);
                    }
                    else
                    {
                        waitForOperator = QSP_TRUE;
                    }
                }
                else
                {
                    opCode = qspFunctionOpCode(name);
                    if (opCode >= qspOpFirst_Function)
                    {
                        if (!qspIsEmpty(s) && *s.Str == QSP_LRBRACK[0])
                        {
                            if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, opCode)) break;
                            if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpOpenBracket)) break;
                            s.Str += QSP_STATIC_LEN(QSP_LRBRACK);
                        }
                        else if (qspOps[opCode].MinArgsCount < 2)
                        {
                            if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, opCode)) break;
                            if (qspOps[opCode].MinArgsCount)
                            {
                                /* The function has a single argument */
                                ++argStack[opSp];
                            }
                            else
                            {
                                /* The function has no arguments */
                                waitForOperator = QSP_TRUE;
                            }
                        }
                        else
                        {
                            qspSetError(QSP_ERR_BRACKSNOTFOUND);
                            break;
                        }
                    }
                    else
                    {
                        v.Type = QSP_TYPE_STRING;
                        QSP_STR(v) = qspGetNewText(name);
                        if (!qspAppendToCompiled(qspOpValue, &itemsCount, compValues, compOpCodes, compArgsCounts, 0, v))
                        {
                            qspFreeString(QSP_STR(v));
                            break;
                        }
                        if (!qspIsEmpty(s) && *s.Str == QSP_LSBRACK[0])
                        {
                            s.Str += QSP_STATIC_LEN(QSP_LSBRACK);
                            qspSkipSpaces(&s);
                            if (!qspIsEmpty(s) && *s.Str == QSP_RSBRACK[0])
                            {
                                s.Str += QSP_STATIC_LEN(QSP_RSBRACK);
                                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpLastArrItem)) break;
                                waitForOperator = QSP_TRUE;
                            }
                            else
                            {
                                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpArrItem)) break;
                                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpOpenArrBracket)) break;
                            }
                        }
                        else
                        {
                            if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpArrItem)) break;
                            waitForOperator = QSP_TRUE;
                        }
                    }
                }
            }
            else
            {
                qspSetError(QSP_ERR_SYNTAX);
                break;
            }
        }
    }
    while (--itemsCount >= 0)
    {
        switch (compOpCodes[itemsCount])
        {
            case qspOpValue:
            case qspOpValueToFormat:
                if (QSP_ISSTR(compValues[itemsCount].Type)) qspFreeString(QSP_STR(compValues[itemsCount]));
                break;
        }
    }
    return 0;
}

QSPVariant qspExprValue(QSPString expr)
{
    QSPVariant compValues[QSP_MAXITEMS];
    int compOpCodes[QSP_MAXITEMS], compArgsCounts[QSP_MAXITEMS], itemsCount;
    if (!(itemsCount = qspCompileExpression(expr, compValues, compOpCodes, compArgsCounts)))
        return qspGetEmptyVariant(QSP_TYPE_UNDEFINED);
    return qspValue(itemsCount, compValues, compOpCodes, compArgsCounts);
}

INLINE void qspFunctionStrComp(QSPVariant *args, int count, QSPVariant *tos)
{
    regex_t *regExp = qspRegExpGetCompiled(QSP_STR(args[1]));
    if (!regExp) return;
    QSP_PNUM(tos) = QSP_TOBOOL(qspRegExpStrMatch(regExp, QSP_STR(args[0])));
}

INLINE void qspFunctionStrFind(QSPVariant *args, int count, QSPVariant *tos)
{
    regex_t *regExp = qspRegExpGetCompiled(QSP_STR(args[1]));
    if (!regExp) return;
    if (count == 3 && QSP_NUM(args[2]) >= 0)
        QSP_PSTR(tos) = qspRegExpStrFind(regExp, QSP_STR(args[0]), QSP_NUM(args[2]));
    else
        QSP_PSTR(tos) = qspRegExpStrFind(regExp, QSP_STR(args[0]), 0);
}

INLINE void qspFunctionStrPos(QSPVariant *args, int count, QSPVariant *tos)
{
    regex_t *regExp = qspRegExpGetCompiled(QSP_STR(args[1]));
    if (!regExp) return;
    if (count == 3 && QSP_NUM(args[2]) >= 0)
        QSP_PNUM(tos) = qspRegExpStrPos(regExp, QSP_STR(args[0]), QSP_NUM(args[2]));
    else
        QSP_PNUM(tos) = qspRegExpStrPos(regExp, QSP_STR(args[0]), 0);
}

INLINE void qspFunctionRGB(QSPVariant *args, int count, QSPVariant *tos)
{
    int r, g, b, a = 255;
    r = QSP_NUM(args[0]);
    g = QSP_NUM(args[1]);
    b = QSP_NUM(args[2]);
    if (count == 4)
    {
        a = QSP_NUM(args[3]);
        if (a < 0)
            a = 0;
        else if (a > 255)
            a = 255;
    }
    if (r < 0)
        r = 0;
    else if (r > 255)
        r = 255;
    if (g < 0)
        g = 0;
    else if (g > 255)
        g = 255;
    if (b < 0)
        b = 0;
    else if (b > 255)
        b = 255;
    QSP_PNUM(tos) = (a << 24) | (b << 16) | (g << 8) | r;
}

INLINE void qspFunctionMid(QSPVariant *args, int count, QSPVariant *tos)
{
    int len, subLen, beg = QSP_NUM(args[1]) - 1;
    if (beg < 0) beg = 0;
    len = qspStrLen(QSP_STR(args[0]));
    if (beg < len)
    {
        len -= beg;
        if (count == 3)
        {
            subLen = QSP_NUM(args[2]);
            if (subLen < 0)
                len = 0;
            else if (subLen < len)
                len = subLen;
        }
        QSP_PSTR(tos) = qspGetNewText(qspStringFromLen(QSP_STR(args[0]).Str + beg, len));
    }
    else
        QSP_PSTR(tos) = qspNullString;
}

INLINE void qspFunctionRand(QSPVariant *args, int count, QSPVariant *tos)
{
    int min, max;
    min = QSP_NUM(args[0]);
    max = (count == 2 ? QSP_NUM(args[1]) : 1);
    if (min > max)
    {
        min = max;
        max = QSP_NUM(args[0]);
    }
    QSP_PNUM(tos) = qspRand() % (max - min + 1) + min;
}

INLINE void qspFunctionDesc(QSPVariant *args, int count, QSPVariant *tos)
{
    QSPString desc;
    int oldRefreshCount, index = qspLocIndex(QSP_STR(args[0]));
    if (index < 0)
    {
        qspSetError(QSP_ERR_LOCNOTFOUND);
        return;
    }
    oldRefreshCount = qspRefreshCount;
    desc = qspFormatText(qspLocs[index].Desc, QSP_FALSE);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum) return;
    QSP_PSTR(tos) = desc;
}

INLINE void qspFunctionGetObj(QSPVariant *args, int count, QSPVariant *tos)
{
    int ind = QSP_NUM(args[0]) - 1;
    if (ind >= 0 && ind < qspCurObjectsCount)
        QSP_PSTR(tos) = qspGetNewText(qspCurObjects[ind].Desc);
    else
        QSP_PSTR(tos) = qspNullString;
}

INLINE void qspFunctionIsPlay(QSPVariant *args, int count, QSPVariant *tos)
{
    if (qspIsAnyString(QSP_STR(args[0])))
        QSP_PNUM(tos) = QSP_TOBOOL(qspCallIsPlayingFile(QSP_STR(args[0])) != 0);
    else
        QSP_PNUM(tos) = QSP_TOBOOL(QSP_FALSE);
}

INLINE void qspFunctionInstr(QSPVariant *args, int count, QSPVariant *tos)
{
    int beg;
    QSP_CHAR *pos;
    QSPString subString;
    if (count == 2)
        beg = 0;
    else
    {
        beg = QSP_NUM(args[2]) - 1;
        if (beg < 0) beg = 0;
    }
    if (beg < qspStrLen(QSP_STR(args[0])))
    {
        subString = QSP_STR(args[0]);
        subString.Str += beg;
        pos = qspStrStr(subString, QSP_STR(args[1]));
        QSP_PNUM(tos) = (pos ? (int)(pos - QSP_STR(args[0]).Str) + 1 : 0);
    }
    else
        QSP_PNUM(tos) = 0;
}

INLINE void qspFunctionArrPos(QSPVariant *args, int count, QSPVariant *tos)
{
    if (count == 2)
        QSP_PNUM(tos) = qspArrayPos(QSP_STR(args[0]), args + 1, 0, QSP_FALSE);
    else
        QSP_PNUM(tos) = qspArrayPos(QSP_STR(args[0]), args + 1, QSP_NUM(args[2]), QSP_FALSE);
}

INLINE void qspFunctionArrComp(QSPVariant *args, int count, QSPVariant *tos)
{
    if (count == 2)
        QSP_PNUM(tos) = qspArrayPos(QSP_STR(args[0]), args + 1, 0, QSP_TRUE);
    else
        QSP_PNUM(tos) = qspArrayPos(QSP_STR(args[0]), args + 1, QSP_NUM(args[2]), QSP_TRUE);
}

INLINE void qspFunctionReplace(QSPVariant *args, int count, QSPVariant *tos)
{
    QSPString searchTxt = QSP_STR(args[1]);
    if (qspIsEmpty(searchTxt))
        QSP_PSTR(tos) = qspGetNewText(QSP_STR(args[0]));
    else if (count == 2)
        QSP_PSTR(tos) = qspReplaceText(QSP_STR(args[0]), searchTxt, qspNullString);
    else
        QSP_PSTR(tos) = qspReplaceText(QSP_STR(args[0]), searchTxt, QSP_STR(args[2]));
}

INLINE void qspFunctionFunc(QSPVariant *args, int count, QSPVariant *tos)
{
    qspExecLocByNameWithArgs(QSP_STR(args[0]), args + 1, count - 1, tos);
}

INLINE void qspFunctionDynEval(QSPVariant *args, int count, QSPVariant *tos)
{
    qspExecStringAsCodeWithArgs(QSP_STR(args[0]), args + 1, count - 1, tos);
}

INLINE void qspFunctionMin(QSPVariant *args, int count, QSPVariant *tos)
{
    int i, minInd;
    if (count == 1)
    {
        qspConvertVariantTo(args, QSP_TYPE_STRING);
        *tos = qspArrayMinMaxItem(QSP_STR(args[0]), QSP_TRUE);
    }
    else
    {
        minInd = 0;
        for (i = 1; i < count; ++i)
        {
            if (qspAutoConvertCompare(args + i, args + minInd) < 0)
                minInd = i;
        }
        qspCopyToNewVariant(tos, args + minInd);
    }
}

INLINE void qspFunctionMax(QSPVariant *args, int count, QSPVariant *tos)
{
    int i, maxInd;
    if (count == 1)
    {
        qspConvertVariantTo(args, QSP_TYPE_STRING);
        *tos = qspArrayMinMaxItem(QSP_STR(args[0]), QSP_FALSE);
    }
    else
    {
        maxInd = 0;
        for (i = 1; i < count; ++i)
        {
            if (qspAutoConvertCompare(args + i, args + maxInd) > 0)
                maxInd = i;
        }
        qspCopyToNewVariant(tos, args + maxInd);
    }
}
