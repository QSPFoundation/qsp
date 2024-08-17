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
#include "tuples.h"
#include "variables.h"
#include "variant.h"

QSPMathOperation qspOps[qspOpLast_Operation];
QSPMathOpName qspOpsNames[QSP_OPSLEVELS][QSP_MAXOPSNAMES];
int qspOpsNamesCounts[QSP_OPSLEVELS];
int qspOpMaxLen = 0;

INLINE void qspAddOperation(QSP_TINYINT opCode, QSP_TINYINT priority, QSP_FUNCTION func, QSP_TINYINT resType, QSP_TINYINT minArgs, QSP_TINYINT maxArgs, ...);
INLINE void qspAddSingleOpName(QSP_TINYINT opCode, QSPString opName, QSP_TINYINT type, int level);
INLINE void qspAddOpName(QSP_TINYINT opCode, QSP_CHAR *opName, int level, QSP_BOOL isFunc);
INLINE int qspMathOpsCompare(const void *opName1, const void *opName2);
INLINE int qspMathOpStringFullCompare(const void *name, const void *compareTo);
INLINE int qspMathOpStringCompare(const void *name, const void *compareTo);
INLINE QSP_TINYINT qspFunctionOpCode(QSPString funName);
INLINE int qspGetNumber(QSPString *expr);
INLINE QSPString qspGetName(QSPString *expr);
INLINE QSP_TINYINT qspOperatorOpCode(QSPString *expr);
INLINE QSPString qspGetString(QSPString *expr);
INLINE QSPString qspGetQString(QSPString *expr);
INLINE int qspSkipValue(QSPMathExpression *expression, int valueIndex);
INLINE QSPVariant qspArgumentValue(QSPMathExpression *expression, int valueIndex, QSP_TINYINT type);
INLINE QSP_BOOL qspCompileExprPushOpCode(QSP_TINYINT *opStack, QSP_TINYINT *argStack, int *opSp, QSP_TINYINT opCode);
INLINE QSP_BOOL qspAppendValueToCompiled(QSPMathExpression* expression, QSP_TINYINT opCode, QSPVariant v);
INLINE QSP_BOOL qspAppendToCompiled(QSPMathExpression* expression, QSP_TINYINT opCode, QSP_TINYINT argsCount);
INLINE void qspFunctionLen(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionIsNum(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionStrComp(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionStrFind(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionStrPos(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionRGB(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionMid(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionRand(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionDesc(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionGetObj(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionIsPlay(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionInstr(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionArrPos(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionArrComp(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionReplace(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionFunc(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionDynEval(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionMin(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionMax(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);

INLINE void qspAddOperation(QSP_TINYINT opCode, QSP_TINYINT priority, QSP_FUNCTION func, QSP_TINYINT resType, QSP_TINYINT minArgs, QSP_TINYINT maxArgs, ...)
{
    qspOps[opCode].Priority = priority;
    qspOps[opCode].Func = func;
    qspOps[opCode].ResType = resType;
    qspOps[opCode].MinArgsCount = minArgs;
    qspOps[opCode].MaxArgsCount = maxArgs;
    if (maxArgs > 0)
    {
        int i;
        va_list marker;
        QSP_BOOL isFinished = QSP_FALSE;
        QSP_TINYINT lastType = QSP_TYPE_UNDEF;
        va_start(marker, maxArgs);
        for (i = 0; i < maxArgs; ++i)
        {
            if (!isFinished)
            {
                QSP_TINYINT curType = (QSP_TINYINT)va_arg(marker, int);
                if (curType >= 0)
                    lastType = curType;
                else
                    isFinished = QSP_TRUE; /* use lastType for the rest of arguments */
            }
            qspOps[opCode].ArgsTypes[i] = lastType;
        }
        va_end(marker);
    }
}

INLINE void qspAddSingleOpName(QSP_TINYINT opCode, QSPString opName, QSP_TINYINT type, int level)
{
    int itemInd;
    QSPBufString buf = qspNewBufString(16);
    if (QSP_ISDEF(type))
    {
        switch (QSP_BASETYPE(type))
        {
        case QSP_TYPE_TUPLE:
            qspAddBufText(&buf, QSP_STATIC_STR(QSP_TUPLECHAR));
            break;
        case QSP_TYPE_STR:
            qspAddBufText(&buf, QSP_STATIC_STR(QSP_STRCHAR));
            break;
        }
    }
    qspAddBufText(&buf, opName);

    itemInd = qspOpsNamesCounts[level]++;
    qspOpsNames[level][itemInd].Name = qspBufTextToString(buf);
    qspOpsNames[level][itemInd].Code = opCode;
}

INLINE void qspAddOpName(QSP_TINYINT opCode, QSP_CHAR *opName, int level, QSP_BOOL isFunc)
{
    int len;
    QSPString name = qspStringFromC(opName);
    /* Add base name */
    qspAddSingleOpName(opCode, name, QSP_TYPE_UNDEF, level);
    if (isFunc)
    {
        if (QSP_ISDEF(qspOps[opCode].ResType))
        {
            /* Add type-specific name */
            qspAddSingleOpName(opCode, name, qspOps[opCode].ResType, level);
        }
        else
        {
            /* Add all possible type-specific names */
            qspAddSingleOpName(opCode, name, QSP_TYPE_TUPLE, level);
            qspAddSingleOpName(opCode, name, QSP_TYPE_STR, level);
        }
    }
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
            Handler,
            Result type,
            Minimum arguments' count,
            Maximum arguments' count,
            Arguments' types [optional, -1 to use the last known type for the rest of arguments]
        );
    */
    int i;
    for (i = 0; i < QSP_OPSLEVELS; ++i) qspOpsNamesCounts[i] = 0;
    qspOpMaxLen = 0;
    qspAddOperation(qspOpValue, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpValueToFormat, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpStart, 127, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpEnd, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpTuple, 127, 0, QSP_TYPE_TUPLE, 0, QSP_OPMAXARGS, QSP_TYPE_UNDEF, -1);
    qspAddOperation(qspOpOpenRoundBracket, 127, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpCloseRoundBracket, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpOpenSquareBracket, 127, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpCloseSquareBracket, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpNegation, 18, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_NUM);
    qspAddOperation(qspOpAdd, 14, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpSub, 14, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpMul, 17, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpDiv, 17, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpMod, 16, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(qspOpNe, 10, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpLeq, 10, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpGeq, 10, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpEq, 10, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpLt, 10, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpGt, 10, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpAppend, 12, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpAnd, 7, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(qspOpOr, 6, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(qspOpLoc, 11, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpObj, 11, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpArrItem, 30, 0, QSP_TYPE_UNDEF, 1, 2, QSP_TYPE_VARREF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpLastArrItem, 30, 0, QSP_TYPE_UNDEF, 1, 1, QSP_TYPE_VARREF);
    qspAddOperation(qspOpNot, 8, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_NUM);
    qspAddOperation(qspOpMin, 30, qspFunctionMin, QSP_TYPE_UNDEF, 1, QSP_OPMAXARGS, QSP_TYPE_UNDEF, -1);
    qspAddOperation(qspOpMax, 30, qspFunctionMax, QSP_TYPE_UNDEF, 1, QSP_OPMAXARGS, QSP_TYPE_UNDEF, -1);
    qspAddOperation(qspOpRand, 30, qspFunctionRand, QSP_TYPE_NUM, 1, 2, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(qspOpIIf, 30, 0, QSP_TYPE_UNDEF, 3, 3, QSP_TYPE_NUM, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpRGB, 30, qspFunctionRGB, QSP_TYPE_NUM, 3, 4, QSP_TYPE_NUM, QSP_TYPE_NUM, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(qspOpLen, 30, qspFunctionLen, QSP_TYPE_NUM, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpIsNum, 30, qspFunctionIsNum, QSP_TYPE_NUM, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpLCase, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpUCase, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpInput, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpStr, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpVal, 30, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpArrSize, 30, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_VARREF);
    qspAddOperation(qspOpIsPlay, 30, qspFunctionIsPlay, QSP_TYPE_NUM, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpDesc, 30, qspFunctionDesc, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpTrim, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpGetObj, 30, qspFunctionGetObj, QSP_TYPE_STR, 1, 1, QSP_TYPE_NUM);
    qspAddOperation(qspOpStrComp, 30, qspFunctionStrComp, QSP_TYPE_NUM, 2, 2, QSP_TYPE_STR, QSP_TYPE_STR);
    qspAddOperation(qspOpStrFind, 30, qspFunctionStrFind, QSP_TYPE_STR, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddOperation(qspOpStrPos, 30, qspFunctionStrPos, QSP_TYPE_NUM, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddOperation(qspOpMid, 30, qspFunctionMid, QSP_TYPE_STR, 2, 3, QSP_TYPE_STR, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(qspOpArrPos, 30, qspFunctionArrPos, QSP_TYPE_NUM, 2, 3, QSP_TYPE_VARREF, QSP_TYPE_UNDEF, QSP_TYPE_NUM);
    qspAddOperation(qspOpArrComp, 30, qspFunctionArrComp, QSP_TYPE_NUM, 2, 3, QSP_TYPE_VARREF, QSP_TYPE_UNDEF, QSP_TYPE_NUM);
    qspAddOperation(qspOpInstr, 30, qspFunctionInstr, QSP_TYPE_NUM, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddOperation(qspOpReplace, 30, qspFunctionReplace, QSP_TYPE_STR, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_STR);
    qspAddOperation(qspOpFunc, 30, qspFunctionFunc, QSP_TYPE_UNDEF, 1, QSP_OPMAXARGS, QSP_TYPE_STR, QSP_TYPE_UNDEF, -1);
    qspAddOperation(qspOpDynEval, 30, qspFunctionDynEval, QSP_TYPE_UNDEF, 1, QSP_OPMAXARGS, QSP_TYPE_CODE, QSP_TYPE_UNDEF, -1);
    qspAddOperation(qspOpRnd, 30, 0, QSP_TYPE_NUM, 0, 0);
    qspAddOperation(qspOpCountObj, 30, 0, QSP_TYPE_NUM, 0, 0);
    qspAddOperation(qspOpMsecsCount, 30, 0, QSP_TYPE_NUM, 0, 0);
    qspAddOperation(qspOpQSPVer, 30, 0, QSP_TYPE_STR, 0, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpUserText, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpCurLoc, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpSelObj, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpSelAct, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpMainText, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpStatText, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpCurActs, 30, 0, QSP_TYPE_CODE, 0, 0);
    qspAddOperation(qspOpCurObjs, 30, 0, QSP_TYPE_CODE, 0, 0);
    /* Names */
    qspAddOpName(qspOpCloseRoundBracket, QSP_RRBRACK, 1, QSP_FALSE);
    qspAddOpName(qspOpCloseSquareBracket, QSP_RSBRACK, 1, QSP_FALSE);
    qspAddOpName(qspOpAdd, QSP_ADD, 1, QSP_FALSE);
    qspAddOpName(qspOpSub, QSP_SUB, 1, QSP_FALSE);
    qspAddOpName(qspOpMul, QSP_MUL, 1, QSP_FALSE);
    qspAddOpName(qspOpDiv, QSP_DIV, 1, QSP_FALSE);
    qspAddOpName(qspOpMod, QSP_FMT("MOD"), 1, QSP_FALSE);
    qspAddOpName(qspOpNe, QSP_NOTEQUAL1, 1, QSP_FALSE);
    qspAddOpName(qspOpNe, QSP_NOTEQUAL2, 0, QSP_FALSE);
    qspAddOpName(qspOpLeq, QSP_LESSEQ1, 0, QSP_FALSE);
    qspAddOpName(qspOpLeq, QSP_LESSEQ2, 0, QSP_FALSE);
    qspAddOpName(qspOpGeq, QSP_GREATEQ1, 0, QSP_FALSE);
    qspAddOpName(qspOpGeq, QSP_GREATEQ2, 0, QSP_FALSE);
    qspAddOpName(qspOpEq, QSP_EQUAL, 1, QSP_FALSE);
    qspAddOpName(qspOpLt, QSP_LESS, 1, QSP_FALSE);
    qspAddOpName(qspOpGt, QSP_GREAT, 1, QSP_FALSE);
    qspAddOpName(qspOpAppend, QSP_APPEND, 1, QSP_FALSE);
    qspAddOpName(qspOpComma, QSP_COMMA, 1, QSP_FALSE);
    qspAddOpName(qspOpAnd, QSP_FMT("AND"), 1, QSP_FALSE);
    qspAddOpName(qspOpOr, QSP_FMT("OR"), 1, QSP_FALSE);
    qspAddOpName(qspOpNot, QSP_FMT("NO"), 1, QSP_FALSE);
    qspAddOpName(qspOpArrItem, QSP_FMT("ARRITEM"), 1, QSP_TRUE);
    qspAddOpName(qspOpLoc, QSP_FMT("LOC"), 1, QSP_TRUE);
    qspAddOpName(qspOpObj, QSP_FMT("OBJ"), 1, QSP_TRUE);
    qspAddOpName(qspOpMin, QSP_FMT("MIN"), 1, QSP_TRUE);
    qspAddOpName(qspOpMax, QSP_FMT("MAX"), 1, QSP_TRUE);
    qspAddOpName(qspOpRand, QSP_FMT("RAND"), 1, QSP_TRUE);
    qspAddOpName(qspOpIIf, QSP_FMT("IIF"), 1, QSP_TRUE);
    qspAddOpName(qspOpRGB, QSP_FMT("RGB"), 1, QSP_TRUE);
    qspAddOpName(qspOpLen, QSP_FMT("LEN"), 1, QSP_TRUE);
    qspAddOpName(qspOpIsNum, QSP_FMT("ISNUM"), 1, QSP_TRUE);
    qspAddOpName(qspOpLCase, QSP_FMT("LCASE"), 1, QSP_TRUE);
    qspAddOpName(qspOpUCase, QSP_FMT("UCASE"), 1, QSP_TRUE);
    qspAddOpName(qspOpInput, QSP_FMT("INPUT"), 1, QSP_TRUE);
    qspAddOpName(qspOpStr, QSP_FMT("STR"), 1, QSP_TRUE);
    qspAddOpName(qspOpVal, QSP_FMT("VAL"), 1, QSP_TRUE);
    qspAddOpName(qspOpArrSize, QSP_FMT("ARRSIZE"), 1, QSP_TRUE);
    qspAddOpName(qspOpIsPlay, QSP_FMT("ISPLAY"), 1, QSP_TRUE);
    qspAddOpName(qspOpDesc, QSP_FMT("DESC"), 1, QSP_TRUE);
    qspAddOpName(qspOpTrim, QSP_FMT("TRIM"), 1, QSP_TRUE);
    qspAddOpName(qspOpGetObj, QSP_FMT("GETOBJ"), 1, QSP_TRUE);
    qspAddOpName(qspOpStrComp, QSP_FMT("STRCOMP"), 1, QSP_TRUE);
    qspAddOpName(qspOpStrFind, QSP_FMT("STRFIND"), 1, QSP_TRUE);
    qspAddOpName(qspOpStrPos, QSP_FMT("STRPOS"), 1, QSP_TRUE);
    qspAddOpName(qspOpMid, QSP_FMT("MID"), 1, QSP_TRUE);
    qspAddOpName(qspOpArrPos, QSP_FMT("ARRPOS"), 1, QSP_TRUE);
    qspAddOpName(qspOpArrComp, QSP_FMT("ARRCOMP"), 1, QSP_TRUE);
    qspAddOpName(qspOpInstr, QSP_FMT("INSTR"), 1, QSP_TRUE);
    qspAddOpName(qspOpReplace, QSP_FMT("REPLACE"), 1, QSP_TRUE);
    qspAddOpName(qspOpFunc, QSP_FMT("FUNC"), 1, QSP_TRUE);
    qspAddOpName(qspOpDynEval, QSP_FMT("DYNEVAL"), 1, QSP_TRUE);
    qspAddOpName(qspOpRnd, QSP_FMT("RND"), 1, QSP_TRUE);
    qspAddOpName(qspOpCountObj, QSP_FMT("COUNTOBJ"), 1, QSP_TRUE);
    qspAddOpName(qspOpMsecsCount, QSP_FMT("MSECSCOUNT"), 1, QSP_TRUE);
    qspAddOpName(qspOpQSPVer, QSP_FMT("QSPVER"), 1, QSP_TRUE);
    qspAddOpName(qspOpUserText, QSP_FMT("USER_TEXT"), 1, QSP_TRUE);
    qspAddOpName(qspOpUserText, QSP_FMT("USRTXT"), 1, QSP_TRUE);
    qspAddOpName(qspOpCurLoc, QSP_FMT("CURLOC"), 1, QSP_TRUE);
    qspAddOpName(qspOpSelObj, QSP_FMT("SELOBJ"), 1, QSP_TRUE);
    qspAddOpName(qspOpSelAct, QSP_FMT("SELACT"), 1, QSP_TRUE);
    qspAddOpName(qspOpMainText, QSP_FMT("MAINTXT"), 1, QSP_TRUE);
    qspAddOpName(qspOpStatText, QSP_FMT("STATTXT"), 1, QSP_TRUE);
    qspAddOpName(qspOpCurActs, QSP_FMT("CURACTS"), 1, QSP_TRUE);
    qspAddOpName(qspOpCurObjs, QSP_FMT("CUROBJS"), 1, QSP_TRUE);
    for (i = 0; i < QSP_OPSLEVELS; ++i)
        qsort(qspOpsNames[i], qspOpsNamesCounts[i], sizeof(QSPMathOpName), qspMathOpsCompare);
}

void qspDeinitMath()
{
    int i, j, count;
    for (i = 0; i < QSP_OPSLEVELS; ++i)
    {
        count = qspOpsNamesCounts[i];
        for (j = 0; j < count; ++j)
            qspFreeString(&qspOpsNames[i][j].Name);
    }
}

INLINE QSP_TINYINT qspFunctionOpCode(QSPString funName)
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

INLINE int qspGetNumber(QSPString *expr)
{
    int num = 0;
    QSP_CHAR *pos = expr->Str;
    while (pos < expr->End && qspIsInClass(*pos, QSP_CHAR_DIGIT))
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

INLINE QSP_TINYINT qspOperatorOpCode(QSPString *expr)
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
        if (*pos == quot)
        {
            ++pos;
            if (pos >= expr->End || *pos != quot) break;
        }
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
    return qspCopyToNewText(qspStringFromPair(buf + QSP_STATIC_LEN(QSP_LQUOT), pos));
}

INLINE int qspSkipValue(QSPMathExpression *expression, int valueIndex)
{
    QSP_TINYINT argsCount;
    if (valueIndex < 0) return -1;
    argsCount = expression->CompArgsCounts[valueIndex];
    --valueIndex;
    if (argsCount)
    {
        int i;
        for (i = 0; i < argsCount; ++i)
            valueIndex = qspSkipValue(expression, valueIndex);
    }
    return valueIndex;
}

INLINE QSPVariant qspArgumentValue(QSPMathExpression *expression, int valueIndex, QSP_TINYINT type)
{
    int oldRefreshCount = qspRefreshCount;
    QSPVariant res = qspValue(expression, valueIndex);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum)
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    if (QSP_ISDEF(type) && !qspConvertVariantTo(&res, type))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        qspFreeVariant(&res);
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    }
    return res;
}

INLINE QSP_BOOL qspCompileExprPushOpCode(QSP_TINYINT *opStack, QSP_TINYINT *argStack, int *opSp, QSP_TINYINT opCode)
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

INLINE QSP_BOOL qspAppendValueToCompiled(QSPMathExpression* expression, QSP_TINYINT opCode, QSPVariant v)
{
    int opIndex = expression->ItemsCount;
    if (opIndex == QSP_MAXITEMS)
    {
        qspSetError(QSP_ERR_TOOMANYITEMS);
        return QSP_FALSE;
    }
    expression->CompOpCodes[opIndex] = opCode;
    expression->CompArgsCounts[opIndex] = 0;
    expression->CompValues[opIndex] = v;
    ++expression->ItemsCount;
    return QSP_TRUE;
}

/* N.B. We can safely add operations with the highest priority directly to the output w/o intermediate stack */
INLINE QSP_BOOL qspAppendToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, QSP_TINYINT argsCount)
{
    int opIndex = expression->ItemsCount;
    if (opIndex == QSP_MAXITEMS)
    {
        qspSetError(QSP_ERR_TOOMANYITEMS);
        return QSP_FALSE;
    }
    expression->CompOpCodes[opIndex] = opCode;
    expression->CompArgsCounts[opIndex] = argsCount;
    ++expression->ItemsCount;
    return QSP_TRUE;
}

QSP_BOOL qspCompileExpression(QSPString s, QSP_BOOL isReusable, QSPMathExpression *expression)
{
    QSPVariant v;
    QSPString name;
    QSP_TINYINT opCode, opStack[QSP_STACKSIZE], argStack[QSP_STACKSIZE];
    QSP_BOOL waitForOperator = QSP_FALSE;
    int opSp = -1;
    expression->ItemsCount = 0;
    expression->IsReusable = isReusable;
    if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpStart)) return QSP_FALSE;
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
            switch (opCode)
            {
            case qspOpAnd:
            case qspOpOr:
            case qspOpMod:
                if (qspIsEmpty(s) || !qspIsInClass(*s.Str, QSP_CHAR_SPACE | QSP_CHAR_QUOT | QSP_CHAR_EXPSTART))
                    qspSetError(QSP_ERR_SYNTAX);
                break;
            }
            while (qspOps[opCode].Priority <= qspOps[opStack[opSp]].Priority && qspOps[opStack[opSp]].Priority != 127)
            {
                if (!qspAppendToCompiled(expression, opStack[opSp], argStack[opSp])) break;
                --opSp; /* it's always positive */
            }
            if (qspErrorNum) break;
            switch (opCode)
            {
            case qspOpEnd:
            case qspOpCloseRoundBracket:
            case qspOpCloseSquareBracket:
                if (opStack[opSp] == qspOpTuple)
                {
                    if (++argStack[opSp] > qspOps[qspOpTuple].MaxArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                    if (!qspAppendToCompiled(expression, qspOpTuple, argStack[opSp])) break;
                    --opSp; /* it's always positive */
                }
                break;
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
                return QSP_TRUE;
            case qspOpCloseRoundBracket:
                if (opStack[opSp] != qspOpOpenRoundBracket)
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
            case qspOpCloseSquareBracket:
                if (opStack[opSp] != qspOpOpenSquareBracket)
                {
                    qspSetError(QSP_ERR_BRACKNOTFOUND);
                    break;
                }
                --opSp; /* it's always positive */
                if (opStack[opSp] == qspOpArrItem)
                {
                    ++argStack[opSp]; /* we don't need to check for max arguments */
                }
                break;
            case qspOpComma:
                if (opStack[opSp] == qspOpOpenRoundBracket && opStack[opSp - 1] >= qspOpFirst_Function)
                {
                    if (++argStack[opSp - 1] > qspOps[opStack[opSp - 1]].MaxArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                }
                else
                {
                    if (opStack[opSp] != qspOpTuple)
                    {
                        if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpTuple)) break;
                    }
                    if (++argStack[opSp] > qspOps[opStack[opSp]].MaxArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
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
            else if (qspIsInClass(*s.Str, QSP_CHAR_DIGIT))
            {
                v.Type = QSP_TYPE_NUM;
                QSP_NUM(v) = qspGetNumber(&s);
                if (opStack[opSp] == qspOpNegation)
                {
                    QSP_NUM(v) = -QSP_NUM(v);
                    --opSp;
                }
                if (!qspAppendValueToCompiled(expression, qspOpValue, v)) break;
                waitForOperator = QSP_TRUE;
            }
            else if (qspIsInClass(*s.Str, QSP_CHAR_QUOT))
            {
                name = qspGetString(&s);
                if (qspErrorNum) break;
                v.Type = QSP_TYPE_STR;
                QSP_STR(v) = name;
                opCode = qspIsEmpty(name) ? qspOpValue : qspOpValueToFormat;
                if (!qspAppendValueToCompiled(expression, opCode, v))
                {
                    qspFreeString(&QSP_STR(v));
                    break;
                }
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_LQUOT[0])
            {
                name = qspGetQString(&s);
                if (qspErrorNum) break;
                v.Type = QSP_TYPE_CODE;
                QSP_STR(v) = name;
                if (!qspAppendValueToCompiled(expression, qspOpValue, v))
                {
                    qspFreeString(&QSP_STR(v));
                    break;
                }
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_NEGATION[0])
            {
                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpNegation)) break;
                s.Str += QSP_STATIC_LEN(QSP_NEGATION);
            }
            else if (*s.Str == QSP_LRBRACK[0]) /* a subexpression OR a tuple */
            {
                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpOpenRoundBracket)) break;
                s.Str += QSP_STATIC_LEN(QSP_LRBRACK);
            }
            else if (*s.Str == QSP_RRBRACK[0]) /* happens when "(" gets closed with ")" without any values */
            {
                opCode = opStack[opSp];
                if (opCode != qspOpOpenRoundBracket)
                {
                    if (opCode >= qspOpFirst_Function)
                        qspSetError(QSP_ERR_ARGSCOUNT);
                    else
                        qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                opCode = opStack[--opSp];
                if (opCode >= qspOpFirst_Function)
                {
                    if (argStack[opSp] < qspOps[opCode].MinArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                }
                else
                {
                    if (!qspAppendToCompiled(expression, qspOpTuple, 0)) break;
                }
                s.Str += QSP_STATIC_LEN(QSP_RRBRACK);
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_LSBRACK[0]) /* a tuple */
            {
                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpOpenSquareBracket)) break;
                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpTuple)) break;
                s.Str += QSP_STATIC_LEN(QSP_LSBRACK);
            }
            else if (*s.Str == QSP_RSBRACK[0]) /* happens when "[" gets closed with "]" without any values */
            {
                if (opStack[opSp] != qspOpTuple)
                {
                    qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                if (!qspAppendToCompiled(expression, qspOpTuple, 0)) break;
                --opSp; /* it's always positive */
                if (opStack[opSp] != qspOpOpenSquareBracket)
                {
                    qspSetError(QSP_ERR_BRACKNOTFOUND);
                    break;
                }
                --opSp; /* it's always positive */
                s.Str += QSP_STATIC_LEN(QSP_RSBRACK);
                waitForOperator = QSP_TRUE;
            }
            else if (!qspIsInClass(*s.Str, QSP_CHAR_DELIM))
            {
                name = qspGetName(&s);
                if (qspErrorNum) break;
                qspSkipSpaces(&s);
                if (*name.Str == QSP_USERFUNC[0])
                {
                    /* Ignore @ symbol */
                    name.Str += QSP_STATIC_LEN(QSP_USERFUNC);
                    /* Add the loc name */
                    v.Type = QSP_TYPE_STR;
                    QSP_STR(v) = qspCopyToNewText(name);
                    if (!qspAppendValueToCompiled(expression, qspOpValue, v))
                    {
                        qspFreeString(&QSP_STR(v));
                        break;
                    }
                    /* Add a function call */
                    if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpFunc)) break;
                    ++argStack[opSp]; /* added the function name already */
                    if (!qspIsEmpty(s) && *s.Str == QSP_LRBRACK[0])
                    {
                        if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpOpenRoundBracket)) break;
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
                            if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpOpenRoundBracket)) break;
                            s.Str += QSP_STATIC_LEN(QSP_LRBRACK);
                        }
                        else if (qspOps[opCode].MinArgsCount < 2)
                        {
                            if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, opCode)) break;
                            if (qspOps[opCode].MinArgsCount)
                            {
                                /* The function has single argument */
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
                        v.Type = QSP_TYPE_VARREF;
                        QSP_STR(v) = qspCopyToNewText(name);
                        if (!qspAppendValueToCompiled(expression, qspOpValue, v))
                        {
                            qspFreeString(&QSP_STR(v));
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
                                ++argStack[opSp]; /* added the var name already */
                                waitForOperator = QSP_TRUE;
                            }
                            else
                            {
                                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpArrItem)) break;
                                ++argStack[opSp]; /* added the var name already */
                                if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpOpenSquareBracket)) break;
                            }
                        }
                        else
                        {
                            if (!qspCompileExprPushOpCode(opStack, argStack, &opSp, qspOpArrItem)) break;
                            ++argStack[opSp]; /* added the var name already */
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
    if (expression->ItemsCount > 0)
    {
        int i;
        for (i = expression->ItemsCount - 1; i >= 0; --i)
        {
            switch (expression->CompOpCodes[i])
            {
                case qspOpValue:
                case qspOpValueToFormat:
                    qspFreeVariant(expression->CompValues + i);
                    break;
            }
        }
        expression->ItemsCount = 0;
    }
    return QSP_FALSE;
}

int qspFreeValue(QSPMathExpression *expression, int valueIndex) /* the last item represents a whole expression */
{
    QSP_TINYINT argsCount;
    if (valueIndex < 0) return -1;
    argsCount = expression->CompArgsCounts[valueIndex];
    if (argsCount)
    {
        int i;
        --valueIndex;
        for (i = 0; i < argsCount; ++i)
            valueIndex = qspFreeValue(expression, valueIndex);
    }
    else
    {
        switch (expression->CompOpCodes[valueIndex])
        {
        case qspOpValue:
        case qspOpValueToFormat:
            qspFreeVariant(expression->CompValues + valueIndex);
            break;
        }
        --valueIndex;
    }
    return valueIndex;
}

QSPVariant qspValue(QSPMathExpression *expression, int valueIndex) /* the last item represents the whole expression */
{
    QSPVariant args[QSP_OPMAXARGS], tos;
    QSP_TINYINT opCode, argsCount, type;
    int oldRefreshCount;
    if (valueIndex < 0)
    {
        qspSetError(QSP_ERR_SYNTAX);
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    }
    oldRefreshCount = qspRefreshCount;
    opCode = expression->CompOpCodes[valueIndex];
    argsCount = expression->CompArgsCounts[valueIndex];
    type = qspOps[opCode].ResType;
    if (QSP_ISDEF(type)) tos.Type = type;
    if (argsCount)
    {
        int i, argIndices[QSP_OPMAXARGS];
        /* Find positions of arguments */
        --valueIndex; /* move to the last argument */
        for (i = argsCount - 1; i >= 0; --i)
        {
            argIndices[i] = valueIndex;
            valueIndex = qspSkipValue(expression, valueIndex);
        }
        switch (opCode)
        {
        case qspOpAnd: /* logical AND operator, we don't pre-evaluate arguments */
            args[0] = qspArgumentValue(expression, argIndices[0], QSP_TYPE_NUM);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                return qspGetEmptyVariant(QSP_TYPE_UNDEF);
            if (QSP_ISTRUE(QSP_NUM(args[0])))
            {
                args[1] = qspArgumentValue(expression, argIndices[1], QSP_TYPE_NUM);
                if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                QSP_NUM(tos) = QSP_TOBOOL(QSP_NUM(args[1]));
            }
            else
            {
                QSP_NUM(tos) = QSP_TOBOOL(QSP_FALSE);
            }
            return tos;
        case qspOpOr: /* logical OR operator, we don't pre-evaluate arguments */
            args[0] = qspArgumentValue(expression, argIndices[0], QSP_TYPE_NUM);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                return qspGetEmptyVariant(QSP_TYPE_UNDEF);
            if (QSP_ISTRUE(QSP_NUM(args[0])))
            {
                QSP_NUM(tos) = QSP_TOBOOL(QSP_TRUE);
            }
            else
            {
                args[1] = qspArgumentValue(expression, argIndices[1], QSP_TYPE_NUM);
                if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                QSP_NUM(tos) = QSP_TOBOOL(QSP_NUM(args[1]));
            }
            return tos;
        case qspOpNot: /* logical NOT operator, we don't pre-evaluate arguments */
            args[0] = qspArgumentValue(expression, argIndices[0], QSP_TYPE_NUM);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                return qspGetEmptyVariant(QSP_TYPE_UNDEF);
            QSP_NUM(tos) = QSP_TOBOOL(!QSP_NUM(args[0]));
            return tos;
        case qspOpIIf: /* inline IF operator, we don't pre-evaluate arguments */
            args[0] = qspArgumentValue(expression, argIndices[0], QSP_TYPE_NUM);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                return qspGetEmptyVariant(QSP_TYPE_UNDEF);
            tos = qspArgumentValue(expression, (QSP_ISTRUE(QSP_NUM(args[0])) ? argIndices[1] : argIndices[2]), QSP_TYPE_UNDEF);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                return qspGetEmptyVariant(QSP_TYPE_UNDEF);
            return tos;
        default:
            for (i = 0; i < argsCount; ++i)
            {
                args[i] = qspArgumentValue(expression, argIndices[i], qspOps[opCode].ArgsTypes[i]);
                if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                {
                    /* We have to cleanup collected arguments */
                    qspFreeVariants(args, i);
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                }
            }
            break;
        }
    }
    switch (opCode)
    {
    case qspOpValue:
        if (expression->IsReusable)
            qspCopyToNewVariant(&tos, expression->CompValues + valueIndex);
        else
            qspMoveToNewVariant(&tos, expression->CompValues + valueIndex);
        break;
    case qspOpValueToFormat:
        if (expression->IsReusable)
            qspCopyToNewVariant(&tos, expression->CompValues + valueIndex);
        else
            qspMoveToNewVariant(&tos, expression->CompValues + valueIndex);
        if (QSP_ISSTR(tos.Type))
        {
            QSPString textToFormat = QSP_STR(tos);
            QSP_STR(tos) = qspFormatText(textToFormat, QSP_TRUE);
            if (QSP_STR(tos).Str != textToFormat.Str) qspFreeString(&textToFormat);
        }
        break;
    case qspOpArrItem:
    case qspOpLastArrItem:
    {
        QSPVar *var;
        int arrIndex;
        QSPString varName = QSP_STR(args[0]);
        var = qspVarReference(varName, QSP_FALSE);
        if (!var) break;
        if (opCode == qspOpLastArrItem)
            arrIndex = var->ValsCount - 1;
        else if (argsCount == 2)
            arrIndex = qspGetVarIndex(var, args[1], QSP_FALSE);
        else
            arrIndex = 0;
        type = qspGetVarType(varName);
        qspGetVarValueByReference(var, arrIndex, type, &tos);
        break;
    }
    case qspOpNegation:
        QSP_NUM(tos) = -QSP_NUM(args[0]);
        break;
    case qspOpMul:
        qspAutoConvertCombine(args, args + 1, QSP_MUL[0], &tos);
        break;
    case qspOpDiv:
        qspAutoConvertCombine(args, args + 1, QSP_DIV[0], &tos);
        break;
    case qspOpAdd:
        qspAutoConvertCombine(args, args + 1, QSP_ADD[0], &tos);
        break;
    case qspOpSub:
        qspAutoConvertCombine(args, args + 1, QSP_SUB[0], &tos);
        break;
    case qspOpMod:
        if (QSP_NUM(args[1]) == 0)
        {
            qspSetError(QSP_ERR_DIVBYZERO);
            break;
        }
        QSP_NUM(tos) = QSP_NUM(args[0]) % QSP_NUM(args[1]);
        break;
    case qspOpAppend:
        qspAutoConvertAppend(args, args + 1, &tos);
        break;
    case qspOpTuple:
        QSP_TUPLE(tos) = qspMoveToNewTuple(args, argsCount);
        break;
    case qspOpEq:
        QSP_NUM(tos) = QSP_TOBOOL(qspAutoConvertCompare(args, args + 1) == 0);
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
    /* Embedded functions -------------------------------------------------------------- */
    case qspOpLoc:
        QSP_NUM(tos) = QSP_TOBOOL(qspLocIndex(QSP_STR(args[0])) >= 0);
        break;
    case qspOpObj:
        QSP_NUM(tos) = QSP_TOBOOL(qspObjIndex(QSP_STR(args[0])) >= 0);
        break;
    case qspOpLCase:
        qspMoveToNewVariant(&tos, args);
        qspLowerStr(&QSP_STR(tos));
        break;
    case qspOpUCase:
        qspMoveToNewVariant(&tos, args);
        qspUpperStr(&QSP_STR(tos));
        break;
    case qspOpStr:
        qspMoveToNewVariant(&tos, args);
        break;
    case qspOpVal:
        QSP_NUM(tos) = qspGetVariantAsNum(args, 0);
        break;
    case qspOpArrSize:
        QSP_NUM(tos) = qspArraySize(QSP_STR(args[0]));
        break;
    case qspOpTrim:
        QSP_STR(tos) = qspCopyToNewText(qspDelSpc(QSP_STR(args[0])));
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
        QSP_STR(tos) = (argsCount > 0 ? qspCallVersion(QSP_STR(args[0])) : qspCallVersion(qspNullString));
        break;
    case qspOpUserText:
        QSP_STR(tos) = (qspCurInput.Str ? qspCopyToNewText(qspCurInput) : qspNullString);
        break;
    case qspOpCurLoc:
        QSP_STR(tos) = (qspCurLoc >= 0 ? qspCopyToNewText(qspLocs[qspCurLoc].Name) : qspNullString);
        break;
    case qspOpSelObj:
        QSP_STR(tos) = (qspCurSelObject >= 0 ? qspCopyToNewText(qspCurObjects[qspCurSelObject].Desc) : qspNullString);
        break;
    case qspOpSelAct:
        QSP_STR(tos) = (qspCurSelAction >= 0 ? qspCopyToNewText(qspCurActions[qspCurSelAction].Desc) : qspNullString);
        break;
    case qspOpMainText:
        QSP_STR(tos) = (qspCurDesc.Len > 0 ? qspCopyToNewText(qspBufTextToString(qspCurDesc)) : qspNullString);
        break;
    case qspOpStatText:
        QSP_STR(tos) = (qspCurVars.Len > 0 ? qspCopyToNewText(qspBufTextToString(qspCurVars)) : qspNullString);
        break;
    case qspOpCurActs:
        QSP_STR(tos) = qspGetAllActionsAsCode();
        break;
    case qspOpCurObjs:
        QSP_STR(tos) = qspGetAllObjectsAsCode();
        break;
    /* External functions -------------------------------------------------------------- */
    default:
        qspOps[opCode].Func(args, argsCount, &tos);
        break;
    }
    if (argsCount) qspFreeVariants(args, argsCount);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum) return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    return tos;
}

QSPVariant qspExprValue(QSPString expr)
{
    QSPVariant res;
    QSPMathExpression expression;
    if (!qspCompileExpression(expr, QSP_FALSE, &expression))
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    res = qspValue(&expression, expression.ItemsCount - 1);
    qspFreeValue(&expression, expression.ItemsCount - 1);
    return res;
}

INLINE void qspFunctionLen(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    switch (QSP_BASETYPE(args[0].Type))
    {
        case QSP_TYPE_TUPLE:
            QSP_PNUM(res) = QSP_TUPLE(args[0]).Items;
            break;
        case QSP_TYPE_NUM:
        {
            QSP_CHAR buf[QSP_NUMTOSTRBUF];
            QSP_PNUM(res) = qspStrLen(qspNumToStr(buf, QSP_NUM(args[0])));
            break;
        }
        case QSP_TYPE_STR:
            QSP_PNUM(res) = qspStrLen(QSP_STR(args[0]));
            break;
    }
}

INLINE void qspFunctionIsNum(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    switch (QSP_BASETYPE(args[0].Type))
    {
        case QSP_TYPE_TUPLE:
            QSP_PNUM(res) = QSP_TOBOOL(QSP_FALSE);
            break;
        case QSP_TYPE_NUM:
            QSP_PNUM(res) = QSP_TOBOOL(QSP_TRUE);
            break;
        case QSP_TYPE_STR:
            QSP_PNUM(res) = QSP_TOBOOL(qspIsNumber(QSP_STR(args[0])));
            break;
    }
}

INLINE void qspFunctionStrComp(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    QSPRegExp *regExp = qspRegExpGetCompiled(QSP_STR(args[1]));
    if (!regExp) return;
    QSP_PNUM(res) = QSP_TOBOOL(qspRegExpStrMatch(regExp, QSP_STR(args[0])));
}

INLINE void qspFunctionStrFind(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    QSPRegExp *regExp = qspRegExpGetCompiled(QSP_STR(args[1]));
    if (!regExp) return;
    if (count == 3 && QSP_NUM(args[2]) >= 0)
        QSP_PSTR(res) = qspRegExpStrFind(regExp, QSP_STR(args[0]), QSP_NUM(args[2]));
    else
        QSP_PSTR(res) = qspRegExpStrFind(regExp, QSP_STR(args[0]), 0);
}

INLINE void qspFunctionStrPos(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    QSPRegExp *regExp = qspRegExpGetCompiled(QSP_STR(args[1]));
    if (!regExp) return;
    if (count == 3 && QSP_NUM(args[2]) >= 0)
        QSP_PNUM(res) = qspRegExpStrPos(regExp, QSP_STR(args[0]), QSP_NUM(args[2]));
    else
        QSP_PNUM(res) = qspRegExpStrPos(regExp, QSP_STR(args[0]), 0);
}

INLINE void qspFunctionRGB(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
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
    QSP_PNUM(res) = (a << 24) | (b << 16) | (g << 8) | r;
}

INLINE void qspFunctionMid(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int len, beg = QSP_NUM(args[1]) - 1;
    if (beg < 0) beg = 0;
    len = qspStrLen(QSP_STR(args[0]));
    if (beg < len)
    {
        len -= beg;
        if (count == 3)
        {
            int subLen = QSP_NUM(args[2]);
            if (subLen < 0)
                len = 0;
            else if (subLen < len)
                len = subLen;
        }
        QSP_PSTR(res) = qspCopyToNewText(qspStringFromLen(QSP_STR(args[0]).Str + beg, len));
    }
    else
        QSP_PSTR(res) = qspNullString;
}

INLINE void qspFunctionRand(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int min, max;
    min = QSP_NUM(args[0]);
    max = (count == 2 ? QSP_NUM(args[1]) : 1);
    if (min > max)
    {
        min = max;
        max = QSP_NUM(args[0]);
    }
    QSP_PNUM(res) = qspRand() % (max - min + 1) + min;
}

INLINE void qspFunctionDesc(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int index = qspLocIndex(QSP_STR(args[0]));
    if (index < 0)
    {
        qspSetError(QSP_ERR_LOCNOTFOUND);
        return;
    }
    QSP_PSTR(res) = qspFormatText(qspLocs[index].Desc, QSP_FALSE);
}

INLINE void qspFunctionGetObj(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int ind = QSP_NUM(args[0]) - 1;
    if (ind >= 0 && ind < qspCurObjectsCount)
        QSP_PSTR(res) = qspCopyToNewText(qspCurObjects[ind].Desc);
    else
        QSP_PSTR(res) = qspNullString;
}

INLINE void qspFunctionIsPlay(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    if (qspIsAnyString(QSP_STR(args[0])))
        QSP_PNUM(res) = QSP_TOBOOL(qspCallIsPlayingFile(QSP_STR(args[0])) != 0);
    else
        QSP_PNUM(res) = QSP_TOBOOL(QSP_FALSE);
}

INLINE void qspFunctionInstr(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int beg;
    if (count == 2)
        beg = 0;
    else
    {
        beg = QSP_NUM(args[2]) - 1;
        if (beg < 0) beg = 0;
    }
    if (beg < qspStrLen(QSP_STR(args[0])))
    {
        QSP_CHAR *pos;
        QSPString subString = QSP_STR(args[0]);
        subString.Str += beg;
        pos = qspStrStr(subString, QSP_STR(args[1]));
        QSP_PNUM(res) = (pos ? (int)(pos - QSP_STR(args[0]).Str) + 1 : 0);
    }
    else
        QSP_PNUM(res) = 0;
}

INLINE void qspFunctionArrPos(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    if (count == 2)
        QSP_PNUM(res) = qspArrayPos(QSP_STR(args[0]), args + 1, 0, QSP_FALSE);
    else
        QSP_PNUM(res) = qspArrayPos(QSP_STR(args[0]), args + 1, QSP_NUM(args[2]), QSP_FALSE);
}

INLINE void qspFunctionArrComp(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    if (count == 2)
        QSP_PNUM(res) = qspArrayPos(QSP_STR(args[0]), args + 1, 0, QSP_TRUE);
    else
        QSP_PNUM(res) = qspArrayPos(QSP_STR(args[0]), args + 1, QSP_NUM(args[2]), QSP_TRUE);
}

INLINE void qspFunctionReplace(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    QSPString searchTxt = QSP_STR(args[1]);
    if (qspIsEmpty(searchTxt))
        QSP_PSTR(res) = qspCopyToNewText(QSP_STR(args[0]));
    else if (count == 2)
        QSP_PSTR(res) = qspReplaceText(QSP_STR(args[0]), searchTxt, qspNullString);
    else
        QSP_PSTR(res) = qspReplaceText(QSP_STR(args[0]), searchTxt, QSP_STR(args[2]));
}

INLINE void qspFunctionFunc(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    qspExecLocByNameWithArgs(QSP_STR(args[0]), args + 1, count - 1, res);
}

INLINE void qspFunctionDynEval(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    qspExecStringAsCodeWithArgs(QSP_STR(args[0]), args + 1, count - 1, 0, res);
}

INLINE void qspFunctionMin(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    if (count == 1)
    {
        qspConvertVariantTo(args, QSP_TYPE_VARREF);
        *res = qspArrayMinMaxItem(QSP_STR(args[0]), QSP_TRUE);
    }
    else
    {
        int i, minInd = 0;
        for (i = 1; i < count; ++i)
        {
            if (qspAutoConvertCompare(args + i, args + minInd) < 0)
                minInd = i;
        }
        qspMoveToNewVariant(res, args + minInd);
    }
}

INLINE void qspFunctionMax(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    if (count == 1)
    {
        qspConvertVariantTo(args, QSP_TYPE_VARREF);
        *res = qspArrayMinMaxItem(QSP_STR(args[0]), QSP_FALSE);
    }
    else
    {
        int i, maxInd = 0;
        for (i = 1; i < count; ++i)
        {
            if (qspAutoConvertCompare(args + i, args + maxInd) > 0)
                maxInd = i;
        }
        qspMoveToNewVariant(res, args + maxInd);
    }
}
