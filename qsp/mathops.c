/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "mathops.h"
#include "actions.h"
#include "callbacks.h"
#include "codetools.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "memory.h"
#include "objects.h"
#include "regexp.h"
#include "statements.h"
#include "text.h"
#include "time.h"
#include "tuples.h"
#include "variables.h"
#include "variant.h"

QSPMathOperation qspOps[QSP_OP_LASTOPERATION];
QSPMathOpName qspOpsNames[QSP_MATHOPSLEVELS][QSP_MAXMATHOPSNAMES];
int qspOpsNamesCounts[QSP_MATHOPSLEVELS];
int qspOpMaxLen = 0;

INLINE void qspAddOperation(QSP_TINYINT opCode, QSP_TINYINT priority, QSP_FUNCTION func, QSP_TINYINT resType, QSP_TINYINT minArgs, int maxArgs, ...);
INLINE void qspAddSingleOpName(QSP_TINYINT opCode, QSPString opName, QSP_TINYINT type, int level);
INLINE void qspAddOpName(QSP_TINYINT opCode, QSP_CHAR *opName, int level, QSP_BOOL isFunc);
INLINE int qspMathOpsCompare(const void *opName1, const void *opName2);
INLINE int qspMathOpStringFullCompare(const void *name, const void *compareTo);
INLINE int qspMathOpStringCompare(const void *name, const void *compareTo);
INLINE QSP_TINYINT qspFunctionOpCode(QSPString funName);
INLINE QSP_TINYINT qspOperatorOpCode(QSPString *expr);
INLINE QSPString qspGetString(QSPString *expr);
INLINE QSPString qspGetCodeBlock(QSPString *expr);
INLINE QSP_BOOL qspPushOperationToStack(QSP_TINYINT *opStack, QSP_TINYINT *argStack, int *opSp, QSP_TINYINT opCode);
INLINE int qspStoreValue(QSPMathExpression *expression, QSPVariant v);
INLINE QSPMathCompiledOp *qspAppendValueToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, QSPVariant v);
INLINE QSPMathCompiledOp *qspAppendOperationToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, QSP_TINYINT argsCount);
INLINE QSP_BOOL qspStartFormatPart(QSPMathExpression *expression, QSP_TINYINT *partsCount);
INLINE QSP_BOOL qspAppendFormatText(QSPMathExpression *expression, QSPString text, QSP_TINYINT *partsCount);
INLINE QSP_BOOL qspAppendFormatToCompiled(QSPMathExpression *expression, QSPString text);
INLINE QSP_BOOL qspAppendExpressionToCompiled(QSPMathExpression *expression, QSPString s);
INLINE int qspSkipMathValue(QSPMathExpression *expression, int itemIndex);
INLINE QSPVariant qspCalculateArgumentValue(QSPMathExpression *expression, int itemIndex, QSP_TINYINT type);
INLINE QSPVariant qspCalculateFormatValue(QSPMathExpression *expression, int *argIndices, QSP_TINYINT argsCount);
INLINE void qspFunctionLen(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionIsNum(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionStrComp(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionStrFind(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionStrPos(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionInstr(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionMid(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionReplace(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionArrType(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionArrKey(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionArrPos(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionArrComp(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionArrPack(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionMin(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionMax(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionRand(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionRGB(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionDesc(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionGetObj(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionIsPlay(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionFunc(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
INLINE void qspFunctionDynEval(QSPVariant *args, QSP_TINYINT count, QSPVariant *res);

INLINE void qspAddOperation(QSP_TINYINT opCode, QSP_TINYINT priority, QSP_FUNCTION func, QSP_TINYINT resType, QSP_TINYINT minArgs, int maxArgs, ...)
{
    qspOps[opCode].Priority = priority;
    qspOps[opCode].Func = func;
    qspOps[opCode].ResType = resType;
    qspOps[opCode].MinArgsCount = minArgs;
    qspOps[opCode].MaxArgsCount = (QSP_TINYINT)maxArgs;
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
                if (curType == QSP_TYPE_TERM)
                    isFinished = QSP_TRUE; /* use lastType for the rest of arguments */
                else
                    lastType = curType;
            }
            qspOps[opCode].ArgsTypes[i] = lastType;
        }
        va_end(marker);
    }
}

INLINE void qspAddSingleOpName(QSP_TINYINT opCode, QSPString opName, QSP_TINYINT type, int level)
{
    int itemInd;
    QSPBufString buf = qspNewBufString(0, 16);
    switch (QSP_BASETYPE(type))
    {
    case QSP_TYPE_TUPLE:
        qspAddBufText(&buf, QSP_STATIC_STR(QSP_TUPLETYPE));
        break;
    case QSP_TYPE_NUM:
        qspAddBufText(&buf, QSP_STATIC_STR(QSP_NUMTYPE));
        break;
    case QSP_TYPE_STR:
        qspAddBufText(&buf, QSP_STATIC_STR(QSP_STRTYPE));
        break;
    }
    qspAddBufText(&buf, opName);
    if (buf.Len > qspOpMaxLen) qspOpMaxLen = buf.Len;

    itemInd = qspOpsNamesCounts[level]++;
    qspOpsNames[level][itemInd].Name = qspBufStringToString(buf);
    qspOpsNames[level][itemInd].Code = opCode;
}

INLINE void qspAddOpName(QSP_TINYINT opCode, QSP_CHAR *opName, int level, QSP_BOOL isFunc)
{
    QSPString name = qspStringFromC(opName);
    /* Add the base name */
    qspAddSingleOpName(opCode, name, QSP_TYPE_UNDEF, level);
    if (isFunc)
    {
        QSP_TINYINT type = qspOps[opCode].ResType;
        if (QSP_ISDEF(type))
        {
            /* Add type-specific name */
            qspAddSingleOpName(opCode, name, type, level);
        }
        else
        {
            /* Add all possible type-specific names */
            qspAddSingleOpName(opCode, name, QSP_TYPE_TUPLE, level);
            qspAddSingleOpName(opCode, name, QSP_TYPE_NUM, level);
            qspAddSingleOpName(opCode, name, QSP_TYPE_STR, level);
        }
    }
}

INLINE int qspMathOpsCompare(const void *opName1, const void *opName2)
{
    return qspStrsCompare(((QSPMathOpName *)opName1)->Name, ((QSPMathOpName *)opName2)->Name);
}

INLINE int qspMathOpStringFullCompare(const void *name, const void *compareTo)
{
    return qspStrsCompare(*(QSPString *)name, ((QSPMathOpName *)compareTo)->Name);
}

INLINE int qspMathOpStringCompare(const void *name, const void *compareTo)
{
    QSPMathOpName *opName = (QSPMathOpName *)compareTo;
    return qspStrsPartCompare(*(QSPString *)name, opName->Name);
}

void qspInitMath(void)
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
            Arguments' types [optional, QSP_TYPE_TERM to use the last known type for the rest of arguments]
        );
    */
    int i;
    for (i = 0; i < QSP_MATHOPSLEVELS; ++i) qspOpsNamesCounts[i] = 0;
    qspOpMaxLen = 0;
    qspAddOperation(QSP_OP_START, 127, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(QSP_OP_END, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(QSP_OP_OPENROUNDBRACKET, 127, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(QSP_OP_CLOSEROUNDBRACKET, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(QSP_OP_OPENSQUAREBRACKET, 127, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(QSP_OP_CLOSESQUAREBRACKET, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(QSP_OP_TUPLE, 127, 0, QSP_TYPE_TUPLE, 0, QSP_MAXMATHOPARGS, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddOperation(QSP_OP_VALUE, 0, 0, QSP_TYPE_UNDEF, 0, 0);

    qspAddOperation(QSP_OP_NEGATION, 18, 0, QSP_TYPE_UNDEF, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_AFFIRMATION, 18, 0, QSP_TYPE_UNDEF, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_APPEND, 12, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_ADD, 14, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_SUB, 14, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_MUL, 17, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_DIV, 17, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_MOD, 16, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_NUM, QSP_TYPE_NUM);

    qspAddOperation(QSP_OP_AND, 7, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_BOOL, QSP_TYPE_BOOL);
    qspAddOperation(QSP_OP_OR, 6, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_BOOL, QSP_TYPE_BOOL);
    qspAddOperation(QSP_OP_NOT, 8, 0, QSP_TYPE_BOOL, 1, 1, QSP_TYPE_BOOL);
    qspAddOperation(QSP_OP_NE, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_LEQ, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_GEQ, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_EQ, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_LT, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_GT, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_IIF, 30, 0, QSP_TYPE_UNDEF, 3, 3, QSP_TYPE_BOOL, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);

    qspAddOperation(QSP_OP_MIN, 30, qspFunctionMin, QSP_TYPE_UNDEF, 1, QSP_MAXMATHOPARGS, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddOperation(QSP_OP_MAX, 30, qspFunctionMax, QSP_TYPE_UNDEF, 1, QSP_MAXMATHOPARGS, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddOperation(QSP_OP_RAND, 30, qspFunctionRand, QSP_TYPE_NUM, 1, 3, QSP_TYPE_NUM, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_RND, 30, 0, QSP_TYPE_NUM, 0, 0);

    qspAddOperation(QSP_OP_ARRSIZE, 30, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_VARREF);
    qspAddOperation(QSP_OP_ARRTYPE, 30, qspFunctionArrType, QSP_TYPE_STR, 1, 2, QSP_TYPE_VARREF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_ARRKEY, 30, qspFunctionArrKey, QSP_TYPE_UNDEF, 1, 2, QSP_TYPE_VARREF, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_ARRITEM, 30, 0, QSP_TYPE_UNDEF, 1, 2, QSP_TYPE_VARREF, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_INLINEFIRSTARRITEM, 30, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(QSP_OP_INLINELASTARRITEM, 30, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(QSP_OP_INLINEINDEXEDARRITEM, 30, 0, QSP_TYPE_UNDEF, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_ARRPACK, 30, qspFunctionArrPack, QSP_TYPE_TUPLE, 1, 3, QSP_TYPE_VARREF, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_ARRPOS, 30, qspFunctionArrPos, QSP_TYPE_NUM, 2, 3, QSP_TYPE_VARREF, QSP_TYPE_UNDEF, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_ARRCOMP, 30, qspFunctionArrComp, QSP_TYPE_NUM, 2, 3, QSP_TYPE_VARREF, QSP_TYPE_STR, QSP_TYPE_NUM);

    qspAddOperation(QSP_OP_STR, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(QSP_OP_FORMAT, 30, 0, QSP_TYPE_STR, 1, QSP_MAXMATHOPARGS, QSP_TYPE_STR, QSP_TYPE_TERM);
    qspAddOperation(QSP_OP_VAL, 30, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_ISNUM, 30, qspFunctionIsNum, QSP_TYPE_BOOL, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_LEN, 30, qspFunctionLen, QSP_TYPE_NUM, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(QSP_OP_LCASE, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(QSP_OP_UCASE, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(QSP_OP_TRIM, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(QSP_OP_MID, 30, qspFunctionMid, QSP_TYPE_STR, 2, 3, QSP_TYPE_STR, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_INSTR, 30, qspFunctionInstr, QSP_TYPE_NUM, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_REPLACE, 30, qspFunctionReplace, QSP_TYPE_STR, 2, 4, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_STRCOMP, 30, qspFunctionStrComp, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_STR, QSP_TYPE_STR);
    qspAddOperation(QSP_OP_STRFIND, 30, qspFunctionStrFind, QSP_TYPE_STR, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_STRPOS, 30, qspFunctionStrPos, QSP_TYPE_NUM, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);

    qspAddOperation(QSP_OP_FUNC, 30, qspFunctionFunc, QSP_TYPE_UNDEF, 1, QSP_MAXMATHOPARGS, QSP_TYPE_STR, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddOperation(QSP_OP_DYNEVAL, 30, qspFunctionDynEval, QSP_TYPE_UNDEF, 1, QSP_MAXMATHOPARGS, QSP_TYPE_CODE, QSP_TYPE_UNDEF, QSP_TYPE_TERM);

    qspAddOperation(QSP_OP_LOC, 11, 0, QSP_TYPE_BOOL, 1, 1, QSP_TYPE_STR);
    qspAddOperation(QSP_OP_CURLOC, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(QSP_OP_DESC, 30, qspFunctionDesc, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);

    qspAddOperation(QSP_OP_MAINTEXT, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(QSP_OP_STATTEXT, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(QSP_OP_USERTEXT, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(QSP_OP_INPUT, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);

    qspAddOperation(QSP_OP_OBJ, 11, 0, QSP_TYPE_BOOL, 1, 1, QSP_TYPE_STR);
    qspAddOperation(QSP_OP_SELOBJ, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(QSP_OP_GETOBJ, 30, qspFunctionGetObj, QSP_TYPE_STR, 1, 1, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_COUNTOBJ, 30, 0, QSP_TYPE_NUM, 0, 1, QSP_TYPE_STR);
    qspAddOperation(QSP_OP_CUROBJS, 30, 0, QSP_TYPE_CODE, 0, 0);

    qspAddOperation(QSP_OP_SELACT, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(QSP_OP_CURACTS, 30, 0, QSP_TYPE_CODE, 0, 0);

    qspAddOperation(QSP_OP_RGB, 30, qspFunctionRGB, QSP_TYPE_NUM, 3, 4, QSP_TYPE_NUM, QSP_TYPE_NUM, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_ISPLAY, 30, qspFunctionIsPlay, QSP_TYPE_BOOL, 1, 1, QSP_TYPE_STR);
    qspAddOperation(QSP_OP_MSECSCOUNT, 30, 0, QSP_TYPE_NUM, 0, 0);
    qspAddOperation(QSP_OP_QSPVER, 30, 0, QSP_TYPE_STR, 0, 1, QSP_TYPE_STR);

    /* Names */
    qspAddOpName(QSP_OP_CLOSEROUNDBRACKET, QSP_RRBRACK, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_CLOSESQUAREBRACKET, QSP_RSBRACK, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_COMMA, QSP_COMMA, 1, QSP_FALSE);

    qspAddOpName(QSP_OP_APPEND, QSP_APPEND, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_ADD, QSP_ADD, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_SUB, QSP_SUB, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_MUL, QSP_MUL, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_DIV, QSP_DIV, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_MOD, QSP_FMT("MOD"), 1, QSP_FALSE);

    qspAddOpName(QSP_OP_AND, QSP_FMT("AND"), 1, QSP_FALSE);
    qspAddOpName(QSP_OP_OR, QSP_FMT("OR"), 1, QSP_FALSE);
    qspAddOpName(QSP_OP_NOT, QSP_FMT("NO"), 1, QSP_FALSE);
    qspAddOpName(QSP_OP_NE, QSP_NOTEQUAL1, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_NE, QSP_NOTEQUAL2, 0, QSP_FALSE);
    qspAddOpName(QSP_OP_LEQ, QSP_LESSEQ1, 0, QSP_FALSE);
    qspAddOpName(QSP_OP_LEQ, QSP_LESSEQ2, 0, QSP_FALSE);
    qspAddOpName(QSP_OP_GEQ, QSP_GREATEQ1, 0, QSP_FALSE);
    qspAddOpName(QSP_OP_GEQ, QSP_GREATEQ2, 0, QSP_FALSE);
    qspAddOpName(QSP_OP_EQ, QSP_EQUAL, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_LT, QSP_LESS, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_GT, QSP_GREAT, 1, QSP_FALSE);
    qspAddOpName(QSP_OP_IIF, QSP_FMT("IIF"), 1, QSP_TRUE);

    qspAddOpName(QSP_OP_MIN, QSP_FMT("MIN"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_MAX, QSP_FMT("MAX"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_RAND, QSP_FMT("RAND"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_RND, QSP_FMT("RND"), 1, QSP_TRUE);

    qspAddOpName(QSP_OP_ARRSIZE, QSP_FMT("ARRSIZE"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_ARRTYPE, QSP_FMT("ARRTYPE"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_ARRKEY, QSP_FMT("ARRKEY"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_ARRITEM, QSP_FMT("ARRITEM"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_ARRPACK, QSP_FMT("ARRPACK"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_ARRPOS, QSP_FMT("ARRPOS"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_ARRCOMP, QSP_FMT("ARRCOMP"), 1, QSP_TRUE);

    qspAddOpName(QSP_OP_STR, QSP_FMT("STR"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_VAL, QSP_FMT("VAL"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_ISNUM, QSP_FMT("ISNUM"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_LEN, QSP_FMT("LEN"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_LCASE, QSP_FMT("LCASE"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_UCASE, QSP_FMT("UCASE"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_TRIM, QSP_FMT("TRIM"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_MID, QSP_FMT("MID"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_INSTR, QSP_FMT("INSTR"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_REPLACE, QSP_FMT("REPLACE"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_STRCOMP, QSP_FMT("STRCOMP"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_STRFIND, QSP_FMT("STRFIND"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_STRPOS, QSP_FMT("STRPOS"), 1, QSP_TRUE);

    qspAddOpName(QSP_OP_FUNC, QSP_FMT("FUNC"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_DYNEVAL, QSP_FMT("DYNEVAL"), 1, QSP_TRUE);

    qspAddOpName(QSP_OP_LOC, QSP_FMT("LOC"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_CURLOC, QSP_FMT("CURLOC"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_DESC, QSP_FMT("DESC"), 1, QSP_TRUE);

    qspAddOpName(QSP_OP_MAINTEXT, QSP_FMT("MAINTXT"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_STATTEXT, QSP_FMT("STATTXT"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_USERTEXT, QSP_FMT("USER_TEXT"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_USERTEXT, QSP_FMT("USRTXT"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_INPUT, QSP_FMT("INPUT"), 1, QSP_TRUE);

    qspAddOpName(QSP_OP_OBJ, QSP_FMT("OBJ"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_SELOBJ, QSP_FMT("SELOBJ"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_GETOBJ, QSP_FMT("GETOBJ"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_COUNTOBJ, QSP_FMT("COUNTOBJ"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_CUROBJS, QSP_FMT("CUROBJS"), 1, QSP_TRUE);

    qspAddOpName(QSP_OP_SELACT, QSP_FMT("SELACT"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_CURACTS, QSP_FMT("CURACTS"), 1, QSP_TRUE);

    qspAddOpName(QSP_OP_RGB, QSP_FMT("RGB"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_ISPLAY, QSP_FMT("ISPLAY"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_MSECSCOUNT, QSP_FMT("MSECSCOUNT"), 1, QSP_TRUE);
    qspAddOpName(QSP_OP_QSPVER, QSP_FMT("QSPVER"), 1, QSP_TRUE);

    for (i = 0; i < QSP_MATHOPSLEVELS; ++i)
        qsort(qspOpsNames[i], qspOpsNamesCounts[i], sizeof(QSPMathOpName), qspMathOpsCompare);
}

void qspTerminateMath(void)
{
    int i, j, count;
    for (i = 0; i < QSP_MATHOPSLEVELS; ++i)
    {
        count = qspOpsNamesCounts[i];
        for (j = 0; j < count; ++j)
            qspFreeString(&qspOpsNames[i][j].Name);
    }
}

INLINE QSP_TINYINT qspFunctionOpCode(QSPString funName)
{
    /* All functions are in one group because we compare full names */
    QSPMathOpName *name = (QSPMathOpName *)bsearch(
        &funName,
        qspOpsNames[QSP_MATHOPSLEVELS - 1],
        qspOpsNamesCounts[QSP_MATHOPSLEVELS - 1],
        sizeof(QSPMathOpName),
        qspMathOpStringFullCompare);

    if (name) return name->Code;
    return QSP_OP_UNKNOWN;
}

INLINE QSP_TINYINT qspOperatorOpCode(QSPString *expr)
{
    int i;
    QSPMathOpName *name;
    if (qspIsEmpty(*expr)) return QSP_OP_END;
    for (i = 0; i < QSP_MATHOPSLEVELS; ++i)
    {
        name = (QSPMathOpName *)bsearch(expr, qspOpsNames[i], qspOpsNamesCounts[i], sizeof(QSPMathOpName), qspMathOpStringCompare);
        if (name)
        {
            expr->Str += qspStrLen(name->Name);
            return name->Code;
        }
    }
    return QSP_OP_UNKNOWN;
}

INLINE QSPString qspGetString(QSPString *expr)
{
    QSPString text, res;
    QSP_CHAR *dest, *pos = expr->Str, *endPos = expr->End, quote = *pos;
    int len, doubledQuotesCount = 0;
    while (1)
    {
        if (++pos >= endPos)
        {
            qspSetError(QSP_ERR_QUOTNOTFOUND);
            return qspNullString;
        }
        if (*pos == quote)
        {
            ++pos;
            if (pos >= endPos || *pos != quote) break;
            ++doubledQuotesCount;
        }
    }
    text = qspStringFromPair(expr->Str + QSP_CHARLEN, pos - QSP_CHARLEN);
    expr->Str = pos;
    len = qspStrLen(text) - doubledQuotesCount;
    dest = (QSP_CHAR *)malloc(len * sizeof(QSP_CHAR));
    res = qspStringFromLen(dest, len);
    for (pos = text.Str; pos < text.End; ++pos, ++dest)
    {
        *dest = *pos;
        if (*pos == quote) ++pos; /* skip the second quote */
    }
    return res;
}

INLINE QSPString qspGetCodeBlock(QSPString *expr)
{
    QSP_CHAR *endPos, *startPos = expr->Str;
    endPos = qspDelimPos(*expr, QSP_RCODE_CHAR);
    if (!endPos)
    {
        qspSetError(QSP_ERR_QUOTNOTFOUND);
        return qspNullString;
    }
    expr->Str = endPos + QSP_CHARLEN;
    return qspStringFromPair(startPos + QSP_CHARLEN, endPos);
}

INLINE QSP_BOOL qspPushOperationToStack(QSP_TINYINT *opStack, QSP_TINYINT *argStack, int *opSp, QSP_TINYINT opCode)
{
    if (*opSp >= QSP_MATHSTACKSIZE - 1)
    {
        qspSetError(QSP_ERR_COMPLEXEXPRESSION);
        return QSP_FALSE;
    }
    ++(*opSp);
    opStack[*opSp] = opCode;
    argStack[*opSp] = (opCode < QSP_OP_FIRSTFUNCTION ? qspOps[opCode].MinArgsCount : 0);
    return QSP_TRUE;
}

INLINE int qspStoreValue(QSPMathExpression *expression, QSPVariant v)
{
    int valueIndex = expression->ValsCount;
    if (valueIndex >= expression->ValsCapacity)
    {
        expression->ValsCapacity = valueIndex + 8;
        expression->Values = (QSPVariant *)realloc(expression->Values, expression->ValsCapacity * sizeof(QSPVariant));
    }
    expression->Values[valueIndex] = v;
    ++expression->ValsCount;
    return valueIndex;
}

INLINE QSPMathCompiledOp *qspAppendValueToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, QSPVariant v)
{
    QSPMathCompiledOp *compiledOp;
    int opIndex = expression->ItemsCount;
    if (opIndex >= QSP_MAXMATHITEMS)
    {
        qspSetError(QSP_ERR_COMPLEXEXPRESSION);
        return 0;
    }
    if (opIndex >= expression->ItemsCapacity)
    {
        expression->ItemsCapacity = opIndex + 16;
        expression->CompItems = (QSPMathCompiledOp *)realloc(expression->CompItems, expression->ItemsCapacity * sizeof(QSPMathCompiledOp));
    }
    compiledOp = expression->CompItems + opIndex;
    compiledOp->OpCode = opCode;
    compiledOp->ArgsCount = 0;
    compiledOp->ValueIndex = (unsigned short)qspStoreValue(expression, v);
    ++expression->ItemsCount;
    return compiledOp;
}

/* N.B. We can safely add operations with the highest priority directly to the output w/o intermediate stack */
INLINE QSPMathCompiledOp *qspAppendOperationToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, QSP_TINYINT argsCount)
{
    QSPMathCompiledOp *compiledOp;
    int opIndex = expression->ItemsCount;
    if (opIndex >= QSP_MAXMATHITEMS)
    {
        qspSetError(QSP_ERR_COMPLEXEXPRESSION);
        return 0;
    }
    if (opIndex >= expression->ItemsCapacity)
    {
        expression->ItemsCapacity = opIndex + 16;
        expression->CompItems = (QSPMathCompiledOp *)realloc(expression->CompItems, expression->ItemsCapacity * sizeof(QSPMathCompiledOp));
    }
    compiledOp = expression->CompItems + opIndex;
    compiledOp->OpCode = opCode;
    compiledOp->ArgsCount = argsCount;
    ++expression->ItemsCount;
    return compiledOp;
}

INLINE QSP_BOOL qspStartFormatPart(QSPMathExpression *expression, QSP_TINYINT *partsCount)
{
    /* A full list of parts becomes the first part of the next list */
    if (*partsCount == QSP_MAXMATHOPARGS)
    {
        if (!qspAppendOperationToCompiled(expression, QSP_OP_FORMAT, QSP_MAXMATHOPARGS))
            return QSP_FALSE;
        *partsCount = 1;
    }
    ++(*partsCount);
    return QSP_TRUE;
}

INLINE QSP_BOOL qspAppendFormatText(QSPMathExpression *expression, QSPString text, QSP_TINYINT *partsCount)
{
    if (!qspIsEmpty(text))
    {
        QSPVariant v;
        if (!qspStartFormatPart(expression, partsCount)) return QSP_FALSE;
        v = qspStrVariant(qspCopyToNewText(text), QSP_TYPE_STR);
        if (!qspAppendValueToCompiled(expression, QSP_OP_VALUE, v))
        {
            qspFreeVariant(&v);
            return QSP_FALSE;
        }
    }
    return QSP_TRUE;
}

INLINE QSP_BOOL qspAppendFormatToCompiled(QSPMathExpression *expression, QSPString text)
{
    /* 'a<<x>>b' joins 'a', x & 'b' */
    QSPString part;
    QSP_TINYINT partsCount = 0;
    QSP_CHAR *pos = qspStrStr(text, QSP_STATIC_STR(QSP_LSUBEX));
    while (pos)
    {
        if (!qspAppendFormatText(expression, qspStringFromPair(text.Str, pos), &partsCount))
            return QSP_FALSE;
        text.Str = pos + QSP_STATIC_LEN(QSP_LSUBEX);
        pos = qspKeywordPos(text, QSP_STATIC_STR(QSP_RSUBEX), QSP_FALSE);
        if (!pos)
        {
            qspSetError(QSP_ERR_BRACKETNOTFOUND);
            return QSP_FALSE;
        }
        if (!qspStartFormatPart(expression, &partsCount)) return QSP_FALSE;
        part = qspStringFromPair(text.Str, pos);
        qspPrepareStringToExecution(&part);
        if (!qspAppendExpressionToCompiled(expression, part)) return QSP_FALSE;
        text.Str = pos + QSP_STATIC_LEN(QSP_RSUBEX);
        pos = qspStrStr(text, QSP_STATIC_STR(QSP_LSUBEX));
    }
    if (!qspAppendFormatText(expression, text, &partsCount)) return QSP_FALSE;
    if (!qspAppendOperationToCompiled(expression, QSP_OP_FORMAT, partsCount)) return QSP_FALSE;
    return QSP_TRUE;
}

INLINE QSP_BOOL qspAppendExpressionToCompiled(QSPMathExpression *expression, QSPString s)
{
    QSPVariant v;
    QSPString name;
    QSPMathCompiledOp *compiledOp;
    QSP_TINYINT opCode, opStack[QSP_MATHSTACKSIZE], argStack[QSP_MATHSTACKSIZE];
    unsigned short nameStack[QSP_MATHSTACKSIZE]; /* var names of the pending QSP_OP_INLINEINDEXEDARRITEM operations */
    QSP_BOOL waitForOperator = QSP_FALSE;
    int opSp = -1;
    if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_START)) return QSP_FALSE;
    while (1)
    {
        qspSkipSpaces(&s);
        if (waitForOperator)
        {
            opCode = qspOperatorOpCode(&s);
            if (opCode == QSP_OP_UNKNOWN || opCode >= QSP_OP_FIRSTFUNCTION)
            {
                qspSetError(QSP_ERR_UNKNOWNACTION);
                break;
            }
            /* We want to separate keywords */
            switch (opCode)
            {
            case QSP_OP_AND:
            case QSP_OP_OR:
            case QSP_OP_MOD:
                if (qspIsEmpty(s) || !qspIsInClass(*s.Str, QSP_CHAR_DELIM))
                    qspSetError(QSP_ERR_SYNTAX);
                break;
            }
            if (qspErrorNum) break;
            while (qspOps[opCode].Priority <= qspOps[opStack[opSp]].Priority && qspOps[opStack[opSp]].Priority != 127)
            {
                if (!qspAppendOperationToCompiled(expression, opStack[opSp], argStack[opSp])) break;
                --opSp; /* it's always positive */
            }
            if (qspErrorNum) break;
            switch (opCode)
            {
            case QSP_OP_END:
            case QSP_OP_CLOSEROUNDBRACKET:
            case QSP_OP_CLOSESQUAREBRACKET:
                if (opStack[opSp] == QSP_OP_TUPLE)
                {
                    if (++argStack[opSp] > qspOps[QSP_OP_TUPLE].MaxArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                    if (!qspAppendOperationToCompiled(expression, QSP_OP_TUPLE, argStack[opSp])) break;
                    --opSp; /* it's always positive */
                }
                break;
            }
            if (qspErrorNum) break;
            switch (opCode)
            {
            case QSP_OP_END:
                if (opSp)
                {
                    qspSetError(QSP_ERR_BRACKETNOTFOUND);
                    break;
                }
                return QSP_TRUE;
            case QSP_OP_CLOSEROUNDBRACKET:
                if (opStack[opSp] != QSP_OP_OPENROUNDBRACKET)
                {
                    qspSetError(QSP_ERR_BRACKETNOTFOUND);
                    break;
                }
                opCode = opStack[--opSp];
                if (opCode >= QSP_OP_FIRSTFUNCTION)
                {
                    ++argStack[opSp];
                    if (argStack[opSp] < qspOps[opCode].MinArgsCount || argStack[opSp] > qspOps[opCode].MaxArgsCount)
                        qspSetError(QSP_ERR_ARGSCOUNT);
                }
                break;
            case QSP_OP_CLOSESQUAREBRACKET:
                if (opStack[opSp] != QSP_OP_OPENSQUAREBRACKET)
                {
                    qspSetError(QSP_ERR_BRACKETNOTFOUND);
                    break;
                }
                --opSp; /* it's always positive */
                if (opStack[opSp] == QSP_OP_INLINEINDEXEDARRITEM)
                {
                    /* The index is complete, the name waited on the stack */
                    compiledOp = qspAppendOperationToCompiled(expression, QSP_OP_INLINEINDEXEDARRITEM, 1);
                    if (!compiledOp) break;
                    compiledOp->ValueIndex = nameStack[opSp];
                    --opSp;
                }
                break;
            case QSP_OP_COMMA:
                if (opStack[opSp] == QSP_OP_OPENROUNDBRACKET && opStack[opSp - 1] >= QSP_OP_FIRSTFUNCTION)
                {
                    if (++argStack[opSp - 1] > qspOps[opStack[opSp - 1]].MaxArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                }
                else
                {
                    if (opStack[opSp] != QSP_OP_TUPLE)
                    {
                        if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_TUPLE)) break;
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
                if (!qspPushOperationToStack(opStack, argStack, &opSp, opCode)) break;
                waitForOperator = QSP_FALSE;
                break;
            }
            if (qspErrorNum) break;
        }
        else
        {
            if (qspIsEmpty(s))
            {
                if (opStack[opSp] >= QSP_OP_FIRSTFUNCTION)
                    qspSetError(QSP_ERR_ARGSCOUNT);
                else
                    qspSetError(QSP_ERR_SYNTAX);
                break;
            }
            else if (qspIsInClass(*s.Str, QSP_CHAR_DIGIT))
            {
                v = qspNumVariant(qspParseUnsignedNumber(&s));
                switch (opStack[opSp])
                {
                case QSP_OP_NEGATION:
                    QSP_NUM(v) = -QSP_NUM(v);
                    --opSp;
                    break;
                case QSP_OP_AFFIRMATION:
                    --opSp;
                    break;
                }
                if (!qspAppendValueToCompiled(expression, QSP_OP_VALUE, v)) break;
                waitForOperator = QSP_TRUE;
            }
            else if (qspIsInClass(*s.Str, QSP_CHAR_QUOT))
            {
                name = qspGetString(&s);
                if (qspErrorNum) break;
                if (qspStrStr(name, QSP_STATIC_STR(QSP_LSUBEX)))
                {
                    if (!qspAppendFormatToCompiled(expression, name))
                    {
                        qspFreeString(&name);
                        break;
                    }
                    qspFreeString(&name);
                }
                else
                {
                    v = qspStrVariant(name, QSP_TYPE_STR);
                    if (!qspAppendValueToCompiled(expression, QSP_OP_VALUE, v))
                    {
                        qspFreeVariant(&v);
                        break;
                    }
                }
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_LCODE_CHAR)
            {
                name = qspGetCodeBlock(&s);
                if (qspErrorNum) break;
                v = qspStrVariant(qspCopyToNewText(name), QSP_TYPE_CODE);
                if (!qspAppendValueToCompiled(expression, QSP_OP_VALUE, v))
                {
                    qspFreeVariant(&v);
                    break;
                }
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_UNDEFTYPE_CHAR)
            {
                qspInitVariant(&v, QSP_TYPE_UNDEF);
                if (!qspAppendValueToCompiled(expression, QSP_OP_VALUE, v)) break;
                s.Str += QSP_CHARLEN;
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_NEGATION_CHAR)
            {
                if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_NEGATION)) break;
                s.Str += QSP_CHARLEN;
            }
            else if (*s.Str == QSP_AFFIRMATION_CHAR)
            {
                if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_AFFIRMATION)) break;
                s.Str += QSP_CHARLEN;
            }
            else if (*s.Str == QSP_LRBRACK_CHAR) /* a subexpression OR a tuple */
            {
                if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_OPENROUNDBRACKET)) break;
                s.Str += QSP_CHARLEN;
            }
            else if (*s.Str == QSP_RRBRACK_CHAR) /* happens when "(" gets closed with ")" without any values */
            {
                opCode = opStack[opSp];
                if (opCode != QSP_OP_OPENROUNDBRACKET)
                {
                    if (opCode >= QSP_OP_FIRSTFUNCTION)
                        qspSetError(QSP_ERR_ARGSCOUNT);
                    else
                        qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                opCode = opStack[--opSp];
                if (opCode >= QSP_OP_FIRSTFUNCTION)
                {
                    if (argStack[opSp] < qspOps[opCode].MinArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                }
                else
                {
                    if (!qspAppendOperationToCompiled(expression, QSP_OP_TUPLE, 0)) break;
                }
                s.Str += QSP_CHARLEN;
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_LSBRACK_CHAR) /* a tuple */
            {
                if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_OPENSQUAREBRACKET)) break;
                if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_TUPLE)) break;
                s.Str += QSP_CHARLEN;
            }
            else if (*s.Str == QSP_RSBRACK_CHAR) /* happens when "[" gets closed with "]" without any values */
            {
                if (opStack[opSp] != QSP_OP_TUPLE)
                {
                    qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                if (!qspAppendOperationToCompiled(expression, QSP_OP_TUPLE, 0)) break;
                --opSp; /* it's always positive */
                if (opStack[opSp] != QSP_OP_OPENSQUAREBRACKET)
                {
                    qspSetError(QSP_ERR_BRACKETNOTFOUND);
                    break;
                }
                --opSp; /* it's always positive */
                s.Str += QSP_CHARLEN;
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_USERFUNC_CHAR) /* a user function call */
            {
                s.Str += QSP_CHARLEN;
                qspSkipSpaces(&s);
                name = qspParseName(&s);
                if (qspIsEmpty(name))
                {
                    qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                qspSkipSpaces(&s);
                /* Add the loc name */
                v = qspStrVariant(qspCopyToNewText(name), QSP_TYPE_STR);
                if (!qspAppendValueToCompiled(expression, QSP_OP_VALUE, v))
                {
                    qspFreeVariant(&v);
                    break;
                }
                /* Add a function call */
                if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_FUNC)) break;
                ++argStack[opSp]; /* added the function name already */
                if (!qspIsEmpty(s) && *s.Str == QSP_LRBRACK_CHAR)
                {
                    if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_OPENROUNDBRACKET)) break;
                    s.Str += QSP_CHARLEN;
                }
                else
                {
                    waitForOperator = QSP_TRUE;
                }
            }
            else if (!qspIsInClass(*s.Str, QSP_CHAR_DELIM))
            {
                name = qspParseName(&s);
                qspSkipSpaces(&s);
                opCode = qspFunctionOpCode(name);
                if (opCode >= QSP_OP_FIRSTFUNCTION)
                {
                    if (!qspIsEmpty(s) && *s.Str == QSP_LRBRACK_CHAR)
                    {
                        if (!qspPushOperationToStack(opStack, argStack, &opSp, opCode)) break;
                        if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_OPENROUNDBRACKET)) break;
                        s.Str += QSP_CHARLEN;
                    }
                    else if (qspOps[opCode].MinArgsCount < 2)
                    {
                        if (!qspPushOperationToStack(opStack, argStack, &opSp, opCode)) break;
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
                        qspSetError(QSP_ERR_BRACKETNOTFOUND);
                        break;
                    }
                }
                else
                {
                    v = qspStrVariant(qspCopyToNewText(name), QSP_TYPE_VARREF);
                    if (!qspIsEmpty(s) && *s.Str == QSP_LSBRACK_CHAR)
                    {
                        s.Str += QSP_CHARLEN;
                        qspSkipSpaces(&s);
                        if (!qspIsEmpty(s) && *s.Str == QSP_RSBRACK_CHAR)
                        {
                            s.Str += QSP_CHARLEN;
                            if (!qspAppendValueToCompiled(expression, QSP_OP_INLINELASTARRITEM, v))
                            {
                                qspFreeVariant(&v);
                                break;
                            }
                            waitForOperator = QSP_TRUE;
                        }
                        else
                        {
                            /* The operation gets appended after its index, so the name waits on the stack */
                            if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_INLINEINDEXEDARRITEM))
                            {
                                qspFreeVariant(&v);
                                break;
                            }
                            nameStack[opSp] = (unsigned short)qspStoreValue(expression, v);
                            if (!qspPushOperationToStack(opStack, argStack, &opSp, QSP_OP_OPENSQUAREBRACKET)) break;
                        }
                    }
                    else
                    {
                        if (!qspAppendValueToCompiled(expression, QSP_OP_INLINEFIRSTARRITEM, v))
                        {
                            qspFreeVariant(&v);
                            break;
                        }
                        waitForOperator = QSP_TRUE;
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
    return QSP_FALSE;
}

QSPMathExpression *qspCompileMathExpression(QSPString s)
{
    QSPMathExpression *expression = (QSPMathExpression *)malloc(sizeof(QSPMathExpression));
    expression->CompItems = 0;
    expression->ItemsCount = 0;
    expression->ItemsCapacity = 0;
    expression->Values = 0;
    expression->ValsCount = 0;
    expression->ValsCapacity = 0;
    if (!qspAppendExpressionToCompiled(expression, s))
    {
        qspFreeMathExpression(expression);
        return 0;
    }
    /* Release the spare capacity */
    if (expression->ItemsCount < expression->ItemsCapacity)
    {
        expression->ItemsCapacity = expression->ItemsCount;
        expression->CompItems = (QSPMathCompiledOp *)realloc(expression->CompItems, expression->ItemsCount * sizeof(QSPMathCompiledOp));
    }
    if (expression->ValsCount < expression->ValsCapacity)
    {
        expression->ValsCapacity = expression->ValsCount;
        expression->Values = (QSPVariant *)realloc(expression->Values, expression->ValsCount * sizeof(QSPVariant));
    }
    return expression;
}

INLINE int qspSkipMathValue(QSPMathExpression *expression, int itemIndex)
{
    int skipItems = 1;
    QSPMathCompiledOp *expItems = expression->CompItems;
    do
    {
        skipItems += expItems[itemIndex].ArgsCount - 1; /* reduces the number of items to skip */
        --itemIndex;
    } while (skipItems > 0);
    return itemIndex;
}

void qspFreeMathExpression(QSPMathExpression *expression)
{
    if (expression->Values)
    {
        qspFreeVariants(expression->Values, expression->ValsCount);
        free(expression->Values);
    }
    if (expression->CompItems) free(expression->CompItems);
    free(expression);
}

INLINE QSPVariant qspCalculateArgumentValue(QSPMathExpression *expression, int itemIndex, QSP_TINYINT type)
{
    QSPVariant res;
    QSPMathCompiledOp *item = expression->CompItems + itemIndex;
    if (item->OpCode == QSP_OP_VALUE)
    {
        /* Fast track */
        qspCopyToNewVariant(&res, expression->Values + item->ValueIndex);
    }
    else
    {
        int oldLocationState = qspLocationState;
        res = qspCalculateValue(expression, itemIndex);
        if (qspLocationState != oldLocationState)
            return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    }
    if (QSP_ISDEF(type) && !qspConvertVariantTo(&res, type))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        qspFreeVariant(&res);
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    }
    return res;
}

INLINE QSPVariant qspCalculateFormatValue(QSPMathExpression *expression, int *argIndices, QSP_TINYINT argsCount)
{
    QSPVariant part;
    QSPMathCompiledOp *item;
    int i, oldLocationState = qspLocationState;
    QSPBufString buf = qspNewBufString(0, 128);
    for (i = 0; i < argsCount; ++i)
    {
        item = expression->CompItems + argIndices[i];
        if (item->OpCode == QSP_OP_VALUE)
        {
            /* Fast track */
            qspAppendVariantToString(expression->Values + item->ValueIndex, &buf);
        }
        else
        {
            part = qspCalculateValue(expression, argIndices[i]);
            if (qspLocationState != oldLocationState)
            {
                qspFreeBufString(&buf);
                return qspGetEmptyVariant(QSP_TYPE_UNDEF);
            }
            qspAppendVariantToString(&part, &buf);
            qspFreeVariant(&part);
        }
    }
    return qspStrVariant(qspBufStringToString(buf), QSP_TYPE_STR);
}

QSPVariant qspCalculateValue(QSPMathExpression *expression, int itemIndex) /* the last item represents the whole expression */
{
    QSPVariant *args, tos;
    QSPMathCompiledOp *item;
    QSP_TINYINT opCode, argsCount, type;
    int oldLocationState;
    if (itemIndex < 0)
    {
        qspSetError(QSP_ERR_INTERNAL);
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    }
    oldLocationState = qspLocationState;
    item = expression->CompItems + itemIndex;
    opCode = item->OpCode;
    argsCount = item->ArgsCount;
    type = qspOps[opCode].ResType;
    if (QSP_ISDEF(type)) tos.Type = type;
    if (argsCount)
    {
        int i, argIndices[QSP_MAXMATHOPARGS];
        /* Find positions of the arguments */
        --itemIndex; /* move to the last argument */
        for (i = argsCount - 1; i > 0; --i)
        {
            argIndices[i] = itemIndex;
            itemIndex = qspSkipMathValue(expression, itemIndex);
        }
        argIndices[0] = itemIndex;
        switch (opCode)
        {
        case QSP_OP_AND: /* logical AND operator, we don't pre-evaluate arguments */
            {
                QSPVariant arg = qspCalculateArgumentValue(expression, argIndices[0], QSP_TYPE_BOOL);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                if (QSP_ISTRUE(QSP_NUM(arg)))
                {
                    arg = qspCalculateArgumentValue(expression, argIndices[1], QSP_TYPE_BOOL);
                    if (qspLocationState != oldLocationState)
                        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                    QSP_NUM(tos) = QSP_TOBOOL(QSP_NUM(arg));
                }
                else
                {
                    QSP_NUM(tos) = QSP_TOBOOL(QSP_FALSE);
                }
                return tos;
            }
        case QSP_OP_OR: /* logical OR operator, we don't pre-evaluate arguments */
            {
                QSPVariant arg = qspCalculateArgumentValue(expression, argIndices[0], QSP_TYPE_BOOL);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                if (QSP_ISTRUE(QSP_NUM(arg)))
                {
                    QSP_NUM(tos) = QSP_TOBOOL(QSP_TRUE);
                }
                else
                {
                    arg = qspCalculateArgumentValue(expression, argIndices[1], QSP_TYPE_BOOL);
                    if (qspLocationState != oldLocationState)
                        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                    QSP_NUM(tos) = QSP_TOBOOL(QSP_NUM(arg));
                }
                return tos;
            }
        case QSP_OP_NOT: /* logical NOT operator, we don't pre-evaluate arguments */
            {
                QSPVariant arg = qspCalculateArgumentValue(expression, argIndices[0], QSP_TYPE_BOOL);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                QSP_NUM(tos) = QSP_TOBOOL(!QSP_NUM(arg));
                return tos;
            }
        case QSP_OP_IIF: /* inline IF operator, we don't pre-evaluate arguments */
            {
                QSPVariant arg = qspCalculateArgumentValue(expression, argIndices[0], QSP_TYPE_BOOL);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                tos = qspCalculateArgumentValue(expression, (QSP_ISTRUE(QSP_NUM(arg)) ? argIndices[1] : argIndices[2]), QSP_TYPE_UNDEF);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                return tos;
            }
        case QSP_OP_INLINEINDEXEDARRITEM: /* array item by index, we don't pre-evaluate arguments */
            {
                QSPVariant index = qspCalculateArgumentValue(expression, argIndices[0], QSP_TYPE_UNDEF);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                qspGetVarValueByIndex(QSP_STR(expression->Values[item->ValueIndex]), index, &tos);
                qspFreeVariant(&index);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                return tos;
            }
        case QSP_OP_FORMAT: /* string with subexpressions, we don't pre-evaluate arguments */
            return qspCalculateFormatValue(expression, argIndices, argsCount);
        default:
            args = (QSPVariant *)qspAllocateMemory(argsCount * sizeof(QSPVariant));
            for (i = 0; i < argsCount; ++i)
            {
                args[i] = qspCalculateArgumentValue(expression, argIndices[i], qspOps[opCode].ArgsTypes[i]);
                if (qspLocationState != oldLocationState)
                {
                    /* We have to clean up collected arguments */
                    qspFreeVariants(args, i);
                    qspReleaseMemory(args);
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                }
            }
            break;
        }
    }
    switch (opCode)
    {
    case QSP_OP_VALUE:
        /* Copy the value instead of moving it because it has to be possible to reuse the compiled expression */
        qspCopyToNewVariant(&tos, expression->Values + item->ValueIndex);
        break;
    case QSP_OP_ARRITEM:
        if (argsCount == 2)
            qspGetVarValueByIndex(QSP_STR(args[0]), args[1], &tos);
        else
            qspGetFirstVarValue(QSP_STR(args[0]), &tos);
        break;
    case QSP_OP_INLINEFIRSTARRITEM:
        qspGetFirstVarValue(QSP_STR(expression->Values[item->ValueIndex]), &tos);
        break;
    case QSP_OP_INLINELASTARRITEM:
        qspGetLastVarValue(QSP_STR(expression->Values[item->ValueIndex]), &tos);
        break;
    case QSP_OP_ADD:
        qspAutoConvertCombine(args, args + 1, QSP_ADD_CHAR, &tos);
        break;
    case QSP_OP_SUB:
        qspAutoConvertCombine(args, args + 1, QSP_SUB_CHAR, &tos);
        break;
    case QSP_OP_MUL:
        qspAutoConvertCombine(args, args + 1, QSP_MUL_CHAR, &tos);
        break;
    case QSP_OP_DIV:
        qspAutoConvertCombine(args, args + 1, QSP_DIV_CHAR, &tos);
        break;
    case QSP_OP_MOD:
        if (QSP_NUM(args[1]) == 0)
        {
            qspSetError(QSP_ERR_DIVBYZERO);
            break;
        }
        QSP_NUM(tos) = QSP_NUM(args[0]) % QSP_NUM(args[1]);
        break;
    case QSP_OP_NEGATION:
        qspMultiplyVariantByNum(args, -1, &tos);
        break;
    case QSP_OP_AFFIRMATION:
        qspMultiplyVariantByNum(args, 1, &tos);
        break;
    case QSP_OP_TUPLE:
        QSP_TUPLE(tos) = qspMoveToNewTuple(args, argsCount);
        break;
    case QSP_OP_APPEND:
        qspAutoConvertAppend(args, args + 1, &tos);
        break;
    case QSP_OP_EQ:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) == 0);
        break;
    case QSP_OP_NE:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) != 0);
        break;
    case QSP_OP_LT:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) < 0);
        break;
    case QSP_OP_GT:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) > 0);
        break;
    case QSP_OP_LEQ:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) <= 0);
        break;
    case QSP_OP_GEQ:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) >= 0);
        break;
    /* Embedded functions -------------------------------------------------------------- */
    case QSP_OP_LOC:
        QSP_NUM(tos) = QSP_TOBOOL(qspLocByName(QSP_STR(args[0])) != 0);
        break;
    case QSP_OP_OBJ:
        QSP_NUM(tos) = QSP_TOBOOL(qspObjsCountByName(QSP_STR(args[0])) > 0);
        break;
    case QSP_OP_LCASE:
        qspMoveToNewVariant(&tos, args);
        qspLowerStr(&QSP_STR(tos));
        break;
    case QSP_OP_UCASE:
        qspMoveToNewVariant(&tos, args);
        qspUpperStr(&QSP_STR(tos));
        break;
    case QSP_OP_STR:
        qspMoveToNewVariant(&tos, args);
        break;
    case QSP_OP_VAL:
        QSP_NUM(tos) = qspGetVariantAsNum(args, 0);
        break;
    case QSP_OP_ARRSIZE:
        QSP_NUM(tos) = qspArraySize(QSP_STR(args[0]));
        break;
    case QSP_OP_TRIM:
        QSP_STR(tos) = qspCopyToNewText(qspDelSpc(QSP_STR(args[0])));
        break;
    case QSP_OP_INPUT:
        QSP_STR(tos) = qspCallInputBox(QSP_STR(args[0]));
        break;
    case QSP_OP_RND:
        QSP_NUM(tos) = qspUniformRand(1, 1000);
        break;
    case QSP_OP_COUNTOBJ:
        QSP_NUM(tos) = (argsCount ? qspObjsCountByName(QSP_STR(args[0])) : qspCurObjsCount);
        break;
    case QSP_OP_MSECSCOUNT:
        QSP_NUM(tos) = qspGetTime();
        break;
    case QSP_OP_QSPVER:
        QSP_STR(tos) = (argsCount ? qspCallVersion(QSP_STR(args[0])) : qspCallVersion(qspNullString));
        break;
    case QSP_OP_USERTEXT:
        QSP_STR(tos) = (qspCurInput.Str ? qspCopyToNewText(qspCurInput) : qspNullString);
        break;
    case QSP_OP_CURLOC:
        QSP_STR(tos) = (qspCurLoc ? qspCopyToNewText(qspCurLoc->Name) : qspNullString);
        break;
    case QSP_OP_SELOBJ:
        QSP_STR(tos) = (qspCurSelObject >= 0 ? qspCopyToNewText(qspCurObjects[qspCurSelObject].Name) : qspNullString);
        break;
    case QSP_OP_SELACT:
        QSP_STR(tos) = (qspCurSelAction >= 0 ? qspCopyToNewText(qspCurActions[qspCurSelAction].Desc) : qspNullString);
        break;
    case QSP_OP_MAINTEXT:
        QSP_STR(tos) = (qspCurDesc.Len > 0 ? qspCopyToNewText(qspBufStringToString(qspCurDesc)) : qspNullString);
        break;
    case QSP_OP_STATTEXT:
        QSP_STR(tos) = (qspCurVars.Len > 0 ? qspCopyToNewText(qspBufStringToString(qspCurVars)) : qspNullString);
        break;
    case QSP_OP_CURACTS:
        QSP_STR(tos) = qspGetAllActionsAsCode();
        break;
    case QSP_OP_CUROBJS:
        QSP_STR(tos) = qspGetAllObjectsAsCode();
        break;
    /* External functions -------------------------------------------------------------- */
    default:
        qspOps[opCode].Func(args, argsCount, &tos);
        break;
    }
    if (argsCount)
    {
        qspFreeVariants(args, argsCount);
        qspReleaseMemory(args);
    }
    if (qspLocationState != oldLocationState) return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    return tos;
}

QSPVariant qspCalculateExprValue(QSPString expr)
{
    QSPVariant res;
    QSPMathExpression *expression = qspCompileMathExpression(expr);
    if (!expression) return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    res = qspCalculateValue(expression, expression->ItemsCount - 1);
    qspFreeMathExpression(expression);
    return res;
}

INLINE void qspFunctionLen(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSPVariant *res)
{
    switch (QSP_BASETYPE(args[0].Type))
    {
    case QSP_TYPE_UNDEF:
        QSP_PNUM(res) = 0;
        break;
    case QSP_TYPE_TUPLE:
        QSP_PNUM(res) = QSP_TUPLE(args[0]).ValsCount;
        break;
    case QSP_TYPE_NUM:
        {
            QSP_CHAR buf[QSP_MAX_BIGINT_LEN];
            QSP_PNUM(res) = qspStrLen(qspNumToStr(buf, QSP_NUM(args[0])));
            break;
        }
    case QSP_TYPE_STR:
        QSP_PNUM(res) = qspStrLen(QSP_STR(args[0]));
        break;
    }
}

INLINE void qspFunctionIsNum(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSPVariant *res)
{
    switch (QSP_BASETYPE(args[0].Type))
    {
    case QSP_TYPE_UNDEF:
        QSP_PNUM(res) = QSP_TOBOOL(QSP_FALSE);
        break;
    case QSP_TYPE_TUPLE:
        QSP_PNUM(res) = QSP_TOBOOL(qspIsTupleNumber(QSP_TUPLE(args[0])));
        break;
    case QSP_TYPE_NUM:
        QSP_PNUM(res) = QSP_TOBOOL(QSP_TRUE);
        break;
    case QSP_TYPE_STR:
        QSP_PNUM(res) = QSP_TOBOOL(qspIsStrNumber(QSP_STR(args[0])));
        break;
    }
}

INLINE void qspFunctionStrComp(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSPVariant *res)
{
    QSPRegExp *regExp = qspRegExpGetCompiled(QSP_STR(args[1]));
    if (!regExp) return;
    QSP_PNUM(res) = QSP_TOBOOL(qspRegExpStrMatch(regExp, QSP_STR(args[0])));
}

INLINE void qspFunctionStrFind(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int foundLen;
    QSP_CHAR *foundPos;
    QSPRegExp *regExp = qspRegExpGetCompiled(QSP_STR(args[1]));
    if (!regExp) return;
    if (count == 3)
    {
        int groupInd = QSP_TOINT(QSP_NUM(args[2]));
        foundPos = qspRegExpStrSearch(regExp, QSP_STR(args[0]), 0, groupInd, &foundLen);
    }
    else
        foundPos = qspRegExpStrSearch(regExp, QSP_STR(args[0]), 0, 0, &foundLen);
    if (foundPos && foundLen)
        QSP_PSTR(res) = qspCopyToNewText(qspStringFromLen(foundPos, foundLen));
    else
        QSP_PSTR(res) = qspNullString;
}

INLINE void qspFunctionStrPos(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    QSP_CHAR *foundPos;
    QSPRegExp *regExp = qspRegExpGetCompiled(QSP_STR(args[1]));
    if (!regExp) return;
    if (count == 3)
    {
        int groupInd = QSP_TOINT(QSP_NUM(args[2]));
        foundPos = qspRegExpStrSearch(regExp, QSP_STR(args[0]), 0, groupInd, 0);
    }
    else
        foundPos = qspRegExpStrSearch(regExp, QSP_STR(args[0]), 0, 0, 0);
    QSP_PNUM(res) = (foundPos ? (int)(foundPos - QSP_STR(args[0]).Str) + 1 : 0);
}

INLINE void qspFunctionInstr(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int beg;
    if (count == 2)
        beg = 0;
    else
    {
        beg = QSP_TOINT(QSP_NUM(args[2]) - 1);
        if (beg < 0) beg = 0;
    }
    if (beg < qspStrLen(QSP_STR(args[0])))
    {
        QSP_CHAR *foundPos;
        QSPString subString = QSP_STR(args[0]);
        subString.Str += beg;
        foundPos = qspStrStr(subString, QSP_STR(args[1]));
        QSP_PNUM(res) = (foundPos ? (int)(foundPos - QSP_STR(args[0]).Str) + 1 : 0);
    }
    else
        QSP_PNUM(res) = 0;
}

INLINE void qspFunctionMid(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int len = qspStrLen(QSP_STR(args[0]));
    if (len > 0)
    {
        int beg = QSP_TOINT(QSP_NUM(args[1]) - 1);
        if (beg < 0) beg = 0;
        len -= beg;
        if (len > 0)
        {
            if (count == 3)
            {
                int subLen = QSP_TOINT(QSP_NUM(args[2]));
                if (subLen < len)
                    len = (subLen > 0 ? subLen : 0);
            }
            QSP_PSTR(res) = qspCopyToNewText(qspStringFromLen(QSP_STR(args[0]).Str + beg, len));
            return;
        }
    }
    QSP_PSTR(res) = qspNullString;
}

INLINE void qspFunctionReplace(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    if (count >= 3)
    {
        int maxReplacements = (count == 4 ? QSP_TOINT(QSP_NUM(args[3])) : INT_MAX);
        QSP_PSTR(res) = qspReplaceText(QSP_STR(args[0]), QSP_STR(args[1]), QSP_STR(args[2]), maxReplacements, QSP_FALSE);
    }
    else
        QSP_PSTR(res) = qspReplaceText(QSP_STR(args[0]), QSP_STR(args[1]), qspNullString, INT_MAX, QSP_FALSE);
}

INLINE void qspFunctionArrType(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int arrIndex;
    QSP_TINYINT arrType;
    QSPString typePrefix;
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_VARREF_GET);
    if (!var) return;
    arrIndex = (count == 2 ? qspGetVarIndex(var, args[1], QSP_FALSE) : 0);
    arrType = ((arrIndex >= 0 && arrIndex < var->ValsCount) ? var->Values[arrIndex].Type : QSP_TYPE_UNDEF);
    switch (QSP_BASETYPE(arrType))
    {
    case QSP_TYPE_UNDEF:
        typePrefix = qspNullString;
        break;
    case QSP_TYPE_TUPLE:
        typePrefix = QSP_STATIC_STR(QSP_TUPLETYPE);
        break;
    case QSP_TYPE_NUM:
        typePrefix = QSP_STATIC_STR(QSP_NUMTYPE);
        break;
    case QSP_TYPE_STR:
        typePrefix = QSP_STATIC_STR(QSP_STRTYPE);
        break;
    }
    QSP_PSTR(res) = qspCopyToNewText(typePrefix);
}

INLINE void qspFunctionArrKey(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int arrIndex;
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_VARREF_GET);
    if (!var) return;
    arrIndex = (count == 2 ? QSP_TOINT(QSP_NUM(args[1])) : 0);
    if (arrIndex >= 0 && arrIndex < var->ValsCount)
    {
        int keyIndex = qspGetVarKeyIndex(var, arrIndex);
        if (keyIndex >= 0)
        {
            qspCopyToNewVariant(res, &var->Keys[keyIndex].Key);
            return;
        }
    }
    qspInitVariant(res, QSP_TYPE_UNDEF);
}

INLINE void qspFunctionArrPos(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    if (count == 2)
        QSP_PNUM(res) = qspArrayPos(QSP_STR(args[0]), args + 1, 0);
    else
    {
        int startInd = QSP_TOINT(QSP_NUM(args[2]));
        QSP_PNUM(res) = qspArrayPos(QSP_STR(args[0]), args + 1, startInd);
    }
}

INLINE void qspFunctionArrComp(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    if (count == 2)
        QSP_PNUM(res) = qspArrayPosRegExp(QSP_STR(args[0]), QSP_STR(args[1]), 0);
    else
    {
        int startInd = QSP_TOINT(QSP_NUM(args[2]));
        QSP_PNUM(res) = qspArrayPosRegExp(QSP_STR(args[0]), QSP_STR(args[1]), startInd);
    }
}

INLINE void qspFunctionArrPack(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int itemsCount;
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_VARREF_GET);
    if (!var) return;
    itemsCount = var->ValsCount;
    if (itemsCount > 0)
    {
        int startInd;
        if (count >= 2)
        {
            startInd = QSP_TOINT(QSP_NUM(args[1]));
            if (startInd < 0) startInd = 0;
        }
        else
            startInd = 0;
        itemsCount -= startInd;
        if (itemsCount > 0)
        {
            if (count == 3)
            {
                int itemsToCopy = QSP_TOINT(QSP_NUM(args[2]));
                if (itemsToCopy < itemsCount)
                    itemsCount = (itemsToCopy > 0 ? itemsToCopy : 0);
            }
            QSP_PTUPLE(res) = qspCopyToNewTuple(var->Values + startInd, itemsCount);
            return;
        }
    }
    QSP_PTUPLE(res) = qspNullTuple;
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
            if (qspVariantsCompare(args + i, args + minInd) < 0)
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
            if (qspVariantsCompare(args + i, args + maxInd) > 0)
                maxInd = i;
        }
        qspMoveToNewVariant(res, args + maxInd);
    }
}

INLINE void qspFunctionRand(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    QSP_BIGINT min, max;
    min = QSP_NUM(args[0]);
    max = (count >= 2 ? QSP_NUM(args[1]) : 1);
    if (count == 3)
    {
        QSP_BIGINT mean = QSP_NUM(args[2]);
        QSP_PNUM(res) = qspNormalRand(min, max, mean);
    }
    else
        QSP_PNUM(res) = qspUniformRand(min, max);
}

INLINE void qspFunctionRGB(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    int r, g, b, a = 0xFF;
    r = QSP_TOINT(QSP_NUM(args[0]));
    g = QSP_TOINT(QSP_NUM(args[1]));
    b = QSP_TOINT(QSP_NUM(args[2]));
    if (count == 4)
    {
        a = QSP_TOINT(QSP_NUM(args[3]));
        if (a < 0x00)
            a = 0x00;
        else if (a > 0xFF)
            a = 0xFF;
    }
    if (r < 0x00)
        r = 0x00;
    else if (r > 0xFF)
        r = 0xFF;
    if (g < 0x00)
        g = 0x00;
    else if (g > 0xFF)
        g = 0xFF;
    if (b < 0x00)
        b = 0x00;
    else if (b > 0xFF)
        b = 0xFF;
    QSP_PNUM(res) = (a << 24) | (b << 16) | (g << 8) | r;
}

INLINE void qspFunctionDesc(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSPVariant *res)
{
    QSPLocation *loc = qspLocByName(QSP_STR(args[0]));
    if (!loc)
    {
        qspSetError(QSP_ERR_LOCNOTFOUND);
        return;
    }
    QSP_PSTR(res) = qspFormatText(loc->Desc);
}

INLINE void qspFunctionGetObj(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSPVariant *res)
{
    int index = QSP_TOINT(QSP_NUM(args[0]) - 1);
    if (index >= 0 && index < qspCurObjsCount)
        QSP_PSTR(res) = qspCopyToNewText(qspCurObjects[index].Name);
    else
        QSP_PSTR(res) = qspNullString;
}

INLINE void qspFunctionIsPlay(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSPVariant *res)
{
    if (qspIsAnyString(QSP_STR(args[0])))
        QSP_PNUM(res) = QSP_TOBOOL(qspCallIsPlayingFile(QSP_STR(args[0])));
    else
        QSP_PNUM(res) = QSP_TOBOOL(QSP_FALSE);
}

INLINE void qspFunctionFunc(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    qspExecLocByNameWithArgs(QSP_STR(args[0]), args + 1, count - 1, QSP_TRUE, res);
}

INLINE void qspFunctionDynEval(QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    qspExecStringAsCodeWithArgs(QSP_STR(args[0]), args + 1, count - 1, res);
}
