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

QSPMathOperation qspOps[qspOpLast_Operation];
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
    if (QSP_ISDEF(type))
    {
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
    qspAddOperation(qspOpStart, 127, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpEnd, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpOpenRoundBracket, 127, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpCloseRoundBracket, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpOpenSquareBracket, 127, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpCloseSquareBracket, 0, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpTuple, 127, 0, QSP_TYPE_TUPLE, 0, QSP_MAXMATHOPARGS, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddOperation(qspOpValue, 0, 0, QSP_TYPE_UNDEF, 0, 0);

    qspAddOperation(qspOpNegation, 18, 0, QSP_TYPE_UNDEF, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpAffirmation, 18, 0, QSP_TYPE_UNDEF, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpAppend, 12, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpAdd, 14, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpSub, 14, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpMul, 17, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpDiv, 17, 0, QSP_TYPE_UNDEF, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpMod, 16, 0, QSP_TYPE_NUM, 2, 2, QSP_TYPE_NUM, QSP_TYPE_NUM);

    qspAddOperation(qspOpAnd, 7, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_BOOL, QSP_TYPE_BOOL);
    qspAddOperation(qspOpOr, 6, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_BOOL, QSP_TYPE_BOOL);
    qspAddOperation(qspOpNot, 8, 0, QSP_TYPE_BOOL, 1, 1, QSP_TYPE_BOOL);
    qspAddOperation(qspOpNe, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpLeq, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpGeq, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpEq, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpLt, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpGt, 10, 0, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpIIf, 30, 0, QSP_TYPE_UNDEF, 3, 3, QSP_TYPE_BOOL, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);

    qspAddOperation(qspOpMin, 30, qspFunctionMin, QSP_TYPE_UNDEF, 1, QSP_MAXMATHOPARGS, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddOperation(qspOpMax, 30, qspFunctionMax, QSP_TYPE_UNDEF, 1, QSP_MAXMATHOPARGS, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddOperation(qspOpRand, 30, qspFunctionRand, QSP_TYPE_NUM, 1, 3, QSP_TYPE_NUM, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(qspOpRnd, 30, 0, QSP_TYPE_NUM, 0, 0);

    qspAddOperation(qspOpArrSize, 30, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_VARREF);
    qspAddOperation(qspOpArrType, 30, qspFunctionArrType, QSP_TYPE_STR, 1, 2, QSP_TYPE_VARREF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpArrItem, 30, 0, QSP_TYPE_UNDEF, 1, 2, QSP_TYPE_VARREF, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpInlineFirstArrItem, 30, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpInlineLastArrItem, 30, 0, QSP_TYPE_UNDEF, 0, 0);
    qspAddOperation(qspOpInlineIndexedArrItem, 30, 0, QSP_TYPE_UNDEF, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpArrPack, 30, qspFunctionArrPack, QSP_TYPE_TUPLE, 1, 3, QSP_TYPE_VARREF, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(qspOpArrPos, 30, qspFunctionArrPos, QSP_TYPE_NUM, 2, 3, QSP_TYPE_VARREF, QSP_TYPE_UNDEF, QSP_TYPE_NUM);
    qspAddOperation(qspOpArrComp, 30, qspFunctionArrComp, QSP_TYPE_NUM, 2, 3, QSP_TYPE_VARREF, QSP_TYPE_STR, QSP_TYPE_NUM);

    qspAddOperation(qspOpStr, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpFormat, 30, 0, QSP_TYPE_STR, 1, QSP_MAXMATHOPARGS, QSP_TYPE_STR, QSP_TYPE_TERM);
    qspAddOperation(qspOpVal, 30, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpIsNum, 30, qspFunctionIsNum, QSP_TYPE_BOOL, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpLen, 30, qspFunctionLen, QSP_TYPE_NUM, 1, 1, QSP_TYPE_UNDEF);
    qspAddOperation(qspOpLCase, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpUCase, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpTrim, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpMid, 30, qspFunctionMid, QSP_TYPE_STR, 2, 3, QSP_TYPE_STR, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(qspOpInstr, 30, qspFunctionInstr, QSP_TYPE_NUM, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddOperation(qspOpReplace, 30, qspFunctionReplace, QSP_TYPE_STR, 2, 4, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddOperation(qspOpStrComp, 30, qspFunctionStrComp, QSP_TYPE_BOOL, 2, 2, QSP_TYPE_STR, QSP_TYPE_STR);
    qspAddOperation(qspOpStrFind, 30, qspFunctionStrFind, QSP_TYPE_STR, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddOperation(qspOpStrPos, 30, qspFunctionStrPos, QSP_TYPE_NUM, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);

    qspAddOperation(qspOpFunc, 30, qspFunctionFunc, QSP_TYPE_UNDEF, 1, QSP_MAXMATHOPARGS, QSP_TYPE_STR, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddOperation(qspOpDynEval, 30, qspFunctionDynEval, QSP_TYPE_UNDEF, 1, QSP_MAXMATHOPARGS, QSP_TYPE_CODE, QSP_TYPE_UNDEF, QSP_TYPE_TERM);

    qspAddOperation(qspOpLoc, 11, 0, QSP_TYPE_BOOL, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpCurLoc, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpDesc, 30, qspFunctionDesc, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);

    qspAddOperation(qspOpMainText, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpStatText, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpUserText, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpInput, 30, 0, QSP_TYPE_STR, 1, 1, QSP_TYPE_STR);

    qspAddOperation(qspOpObj, 11, 0, QSP_TYPE_NUM, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpSelObj, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpGetObj, 30, qspFunctionGetObj, QSP_TYPE_STR, 1, 1, QSP_TYPE_NUM);
    qspAddOperation(qspOpCountObj, 30, 0, QSP_TYPE_NUM, 0, 0);
    qspAddOperation(qspOpCurObjs, 30, 0, QSP_TYPE_CODE, 0, 0);

    qspAddOperation(qspOpSelAct, 30, 0, QSP_TYPE_STR, 0, 0);
    qspAddOperation(qspOpCurActs, 30, 0, QSP_TYPE_CODE, 0, 0);

    qspAddOperation(qspOpRGB, 30, qspFunctionRGB, QSP_TYPE_NUM, 3, 4, QSP_TYPE_NUM, QSP_TYPE_NUM, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddOperation(qspOpIsPlay, 30, qspFunctionIsPlay, QSP_TYPE_BOOL, 1, 1, QSP_TYPE_STR);
    qspAddOperation(qspOpMsecsCount, 30, 0, QSP_TYPE_NUM, 0, 0);
    qspAddOperation(qspOpQSPVer, 30, 0, QSP_TYPE_STR, 0, 1, QSP_TYPE_STR);

    /* Names */
    qspAddOpName(qspOpCloseRoundBracket, QSP_RRBRACK, 1, QSP_FALSE);
    qspAddOpName(qspOpCloseSquareBracket, QSP_RSBRACK, 1, QSP_FALSE);
    qspAddOpName(qspOpComma, QSP_COMMA, 1, QSP_FALSE);

    qspAddOpName(qspOpAppend, QSP_APPEND, 1, QSP_FALSE);
    qspAddOpName(qspOpAdd, QSP_ADD, 1, QSP_FALSE);
    qspAddOpName(qspOpSub, QSP_SUB, 1, QSP_FALSE);
    qspAddOpName(qspOpMul, QSP_MUL, 1, QSP_FALSE);
    qspAddOpName(qspOpDiv, QSP_DIV, 1, QSP_FALSE);
    qspAddOpName(qspOpMod, QSP_FMT("MOD"), 1, QSP_FALSE);

    qspAddOpName(qspOpAnd, QSP_FMT("AND"), 1, QSP_FALSE);
    qspAddOpName(qspOpOr, QSP_FMT("OR"), 1, QSP_FALSE);
    qspAddOpName(qspOpNot, QSP_FMT("NO"), 1, QSP_FALSE);
    qspAddOpName(qspOpNe, QSP_NOTEQUAL1, 1, QSP_FALSE);
    qspAddOpName(qspOpNe, QSP_NOTEQUAL2, 0, QSP_FALSE);
    qspAddOpName(qspOpLeq, QSP_LESSEQ1, 0, QSP_FALSE);
    qspAddOpName(qspOpLeq, QSP_LESSEQ2, 0, QSP_FALSE);
    qspAddOpName(qspOpGeq, QSP_GREATEQ1, 0, QSP_FALSE);
    qspAddOpName(qspOpGeq, QSP_GREATEQ2, 0, QSP_FALSE);
    qspAddOpName(qspOpEq, QSP_EQUAL, 1, QSP_FALSE);
    qspAddOpName(qspOpLt, QSP_LESS, 1, QSP_FALSE);
    qspAddOpName(qspOpGt, QSP_GREAT, 1, QSP_FALSE);
    qspAddOpName(qspOpIIf, QSP_FMT("IIF"), 1, QSP_TRUE);

    qspAddOpName(qspOpMin, QSP_FMT("MIN"), 1, QSP_TRUE);
    qspAddOpName(qspOpMax, QSP_FMT("MAX"), 1, QSP_TRUE);
    qspAddOpName(qspOpRand, QSP_FMT("RAND"), 1, QSP_TRUE);
    qspAddOpName(qspOpRnd, QSP_FMT("RND"), 1, QSP_TRUE);

    qspAddOpName(qspOpArrSize, QSP_FMT("ARRSIZE"), 1, QSP_TRUE);
    qspAddOpName(qspOpArrType, QSP_FMT("ARRTYPE"), 1, QSP_TRUE);
    qspAddOpName(qspOpArrItem, QSP_FMT("ARRITEM"), 1, QSP_TRUE);
    qspAddOpName(qspOpArrPack, QSP_FMT("ARRPACK"), 1, QSP_TRUE);
    qspAddOpName(qspOpArrPos, QSP_FMT("ARRPOS"), 1, QSP_TRUE);
    qspAddOpName(qspOpArrComp, QSP_FMT("ARRCOMP"), 1, QSP_TRUE);

    qspAddOpName(qspOpStr, QSP_FMT("STR"), 1, QSP_TRUE);
    qspAddOpName(qspOpVal, QSP_FMT("VAL"), 1, QSP_TRUE);
    qspAddOpName(qspOpIsNum, QSP_FMT("ISNUM"), 1, QSP_TRUE);
    qspAddOpName(qspOpLen, QSP_FMT("LEN"), 1, QSP_TRUE);
    qspAddOpName(qspOpLCase, QSP_FMT("LCASE"), 1, QSP_TRUE);
    qspAddOpName(qspOpUCase, QSP_FMT("UCASE"), 1, QSP_TRUE);
    qspAddOpName(qspOpTrim, QSP_FMT("TRIM"), 1, QSP_TRUE);
    qspAddOpName(qspOpMid, QSP_FMT("MID"), 1, QSP_TRUE);
    qspAddOpName(qspOpInstr, QSP_FMT("INSTR"), 1, QSP_TRUE);
    qspAddOpName(qspOpReplace, QSP_FMT("REPLACE"), 1, QSP_TRUE);
    qspAddOpName(qspOpStrComp, QSP_FMT("STRCOMP"), 1, QSP_TRUE);
    qspAddOpName(qspOpStrFind, QSP_FMT("STRFIND"), 1, QSP_TRUE);
    qspAddOpName(qspOpStrPos, QSP_FMT("STRPOS"), 1, QSP_TRUE);

    qspAddOpName(qspOpFunc, QSP_FMT("FUNC"), 1, QSP_TRUE);
    qspAddOpName(qspOpDynEval, QSP_FMT("DYNEVAL"), 1, QSP_TRUE);

    qspAddOpName(qspOpLoc, QSP_FMT("LOC"), 1, QSP_TRUE);
    qspAddOpName(qspOpCurLoc, QSP_FMT("CURLOC"), 1, QSP_TRUE);
    qspAddOpName(qspOpDesc, QSP_FMT("DESC"), 1, QSP_TRUE);

    qspAddOpName(qspOpMainText, QSP_FMT("MAINTXT"), 1, QSP_TRUE);
    qspAddOpName(qspOpStatText, QSP_FMT("STATTXT"), 1, QSP_TRUE);
    qspAddOpName(qspOpUserText, QSP_FMT("USER_TEXT"), 1, QSP_TRUE);
    qspAddOpName(qspOpUserText, QSP_FMT("USRTXT"), 1, QSP_TRUE);
    qspAddOpName(qspOpInput, QSP_FMT("INPUT"), 1, QSP_TRUE);

    qspAddOpName(qspOpObj, QSP_FMT("OBJ"), 1, QSP_TRUE);
    qspAddOpName(qspOpSelObj, QSP_FMT("SELOBJ"), 1, QSP_TRUE);
    qspAddOpName(qspOpGetObj, QSP_FMT("GETOBJ"), 1, QSP_TRUE);
    qspAddOpName(qspOpCountObj, QSP_FMT("COUNTOBJ"), 1, QSP_TRUE);
    qspAddOpName(qspOpCurObjs, QSP_FMT("CUROBJS"), 1, QSP_TRUE);

    qspAddOpName(qspOpSelAct, QSP_FMT("SELACT"), 1, QSP_TRUE);
    qspAddOpName(qspOpCurActs, QSP_FMT("CURACTS"), 1, QSP_TRUE);

    qspAddOpName(qspOpRGB, QSP_FMT("RGB"), 1, QSP_TRUE);
    qspAddOpName(qspOpIsPlay, QSP_FMT("ISPLAY"), 1, QSP_TRUE);
    qspAddOpName(qspOpMsecsCount, QSP_FMT("MSECSCOUNT"), 1, QSP_TRUE);
    qspAddOpName(qspOpQSPVer, QSP_FMT("QSPVER"), 1, QSP_TRUE);

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
    return qspOpUnknown;
}

INLINE QSP_TINYINT qspOperatorOpCode(QSPString *expr)
{
    int i;
    QSPMathOpName *name;
    if (qspIsEmpty(*expr)) return qspOpEnd;
    for (i = 0; i < QSP_MATHOPSLEVELS; ++i)
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
    text = qspStringFromPair(expr->Str + QSP_CHAR_LEN, pos - QSP_CHAR_LEN);
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
    expr->Str = endPos + QSP_CHAR_LEN;
    return qspStringFromPair(startPos + QSP_CHAR_LEN, endPos);
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
    argStack[*opSp] = (opCode < qspOpFirst_Function ? qspOps[opCode].MinArgsCount : 0);
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
        if (!qspAppendOperationToCompiled(expression, qspOpFormat, QSP_MAXMATHOPARGS))
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
        if (!qspAppendValueToCompiled(expression, qspOpValue, v))
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
    if (!qspAppendOperationToCompiled(expression, qspOpFormat, partsCount)) return QSP_FALSE;
    return QSP_TRUE;
}

INLINE QSP_BOOL qspAppendExpressionToCompiled(QSPMathExpression *expression, QSPString s)
{
    QSPVariant v;
    QSPString name;
    QSPMathCompiledOp *compiledOp;
    QSP_TINYINT opCode, opStack[QSP_MATHSTACKSIZE], argStack[QSP_MATHSTACKSIZE];
    unsigned short nameStack[QSP_MATHSTACKSIZE]; /* var names of the pending qspOpInlineIndexedArrItem operations */
    QSP_BOOL waitForOperator = QSP_FALSE;
    int opSp = -1;
    if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpStart)) return QSP_FALSE;
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
                    if (!qspAppendOperationToCompiled(expression, qspOpTuple, argStack[opSp])) break;
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
                    qspSetError(QSP_ERR_BRACKETNOTFOUND);
                    break;
                }
                return QSP_TRUE;
            case qspOpCloseRoundBracket:
                if (opStack[opSp] != qspOpOpenRoundBracket)
                {
                    qspSetError(QSP_ERR_BRACKETNOTFOUND);
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
                    qspSetError(QSP_ERR_BRACKETNOTFOUND);
                    break;
                }
                --opSp; /* it's always positive */
                if (opStack[opSp] == qspOpInlineIndexedArrItem)
                {
                    /* The index is complete, the name waited on the stack */
                    compiledOp = qspAppendOperationToCompiled(expression, qspOpInlineIndexedArrItem, 1);
                    if (!compiledOp) break;
                    compiledOp->ValueIndex = nameStack[opSp];
                    --opSp;
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
                        if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpTuple)) break;
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
                if (opStack[opSp] >= qspOpFirst_Function)
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
                case qspOpNegation:
                    QSP_NUM(v) = -QSP_NUM(v);
                    --opSp;
                    break;
                case qspOpAffirmation:
                    --opSp;
                    break;
                }
                if (!qspAppendValueToCompiled(expression, qspOpValue, v)) break;
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
                    if (!qspAppendValueToCompiled(expression, qspOpValue, v))
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
                if (!qspAppendValueToCompiled(expression, qspOpValue, v))
                {
                    qspFreeVariant(&v);
                    break;
                }
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_NEGATION_CHAR)
            {
                if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpNegation)) break;
                s.Str += QSP_CHAR_LEN;
            }
            else if (*s.Str == QSP_AFFIRMATION_CHAR)
            {
                if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpAffirmation)) break;
                s.Str += QSP_CHAR_LEN;
            }
            else if (*s.Str == QSP_LRBRACK_CHAR) /* a subexpression OR a tuple */
            {
                if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpOpenRoundBracket)) break;
                s.Str += QSP_CHAR_LEN;
            }
            else if (*s.Str == QSP_RRBRACK_CHAR) /* happens when "(" gets closed with ")" without any values */
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
                    if (!qspAppendOperationToCompiled(expression, qspOpTuple, 0)) break;
                }
                s.Str += QSP_CHAR_LEN;
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_LSBRACK_CHAR) /* a tuple */
            {
                if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpOpenSquareBracket)) break;
                if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpTuple)) break;
                s.Str += QSP_CHAR_LEN;
            }
            else if (*s.Str == QSP_RSBRACK_CHAR) /* happens when "[" gets closed with "]" without any values */
            {
                if (opStack[opSp] != qspOpTuple)
                {
                    qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                if (!qspAppendOperationToCompiled(expression, qspOpTuple, 0)) break;
                --opSp; /* it's always positive */
                if (opStack[opSp] != qspOpOpenSquareBracket)
                {
                    qspSetError(QSP_ERR_BRACKETNOTFOUND);
                    break;
                }
                --opSp; /* it's always positive */
                s.Str += QSP_CHAR_LEN;
                waitForOperator = QSP_TRUE;
            }
            else if (*s.Str == QSP_USERFUNC_CHAR) /* a user function call */
            {
                s.Str += QSP_CHAR_LEN;
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
                if (!qspAppendValueToCompiled(expression, qspOpValue, v))
                {
                    qspFreeVariant(&v);
                    break;
                }
                /* Add a function call */
                if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpFunc)) break;
                ++argStack[opSp]; /* added the function name already */
                if (!qspIsEmpty(s) && *s.Str == QSP_LRBRACK_CHAR)
                {
                    if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpOpenRoundBracket)) break;
                    s.Str += QSP_CHAR_LEN;
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
                if (opCode >= qspOpFirst_Function)
                {
                    if (!qspIsEmpty(s) && *s.Str == QSP_LRBRACK_CHAR)
                    {
                        if (!qspPushOperationToStack(opStack, argStack, &opSp, opCode)) break;
                        if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpOpenRoundBracket)) break;
                        s.Str += QSP_CHAR_LEN;
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
                        s.Str += QSP_CHAR_LEN;
                        qspSkipSpaces(&s);
                        if (!qspIsEmpty(s) && *s.Str == QSP_RSBRACK_CHAR)
                        {
                            s.Str += QSP_CHAR_LEN;
                            if (!qspAppendValueToCompiled(expression, qspOpInlineLastArrItem, v))
                            {
                                qspFreeVariant(&v);
                                break;
                            }
                            waitForOperator = QSP_TRUE;
                        }
                        else
                        {
                            /* The operation gets appended after its index, so the name waits on the stack */
                            if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpInlineIndexedArrItem))
                            {
                                qspFreeVariant(&v);
                                break;
                            }
                            nameStack[opSp] = (unsigned short)qspStoreValue(expression, v);
                            if (!qspPushOperationToStack(opStack, argStack, &opSp, qspOpOpenSquareBracket)) break;
                        }
                    }
                    else
                    {
                        if (!qspAppendValueToCompiled(expression, qspOpInlineFirstArrItem, v))
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
    qspFreeVariants(expression->Values, expression->ValsCount);
    free(expression->Values);
    free(expression->CompItems);
    free(expression);
}

INLINE QSPVariant qspCalculateArgumentValue(QSPMathExpression *expression, int itemIndex, QSP_TINYINT type)
{
    QSPVariant res;
    QSPMathCompiledOp *item = expression->CompItems + itemIndex;
    if (item->OpCode == qspOpValue)
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
        if (item->OpCode == qspOpValue)
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
        case qspOpAnd: /* logical AND operator, we don't pre-evaluate arguments */
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
        case qspOpOr: /* logical OR operator, we don't pre-evaluate arguments */
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
        case qspOpNot: /* logical NOT operator, we don't pre-evaluate arguments */
            {
                QSPVariant arg = qspCalculateArgumentValue(expression, argIndices[0], QSP_TYPE_BOOL);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                QSP_NUM(tos) = QSP_TOBOOL(!QSP_NUM(arg));
                return tos;
            }
        case qspOpIIf: /* inline IF operator, we don't pre-evaluate arguments */
            {
                QSPVariant arg = qspCalculateArgumentValue(expression, argIndices[0], QSP_TYPE_BOOL);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                tos = qspCalculateArgumentValue(expression, (QSP_ISTRUE(QSP_NUM(arg)) ? argIndices[1] : argIndices[2]), QSP_TYPE_UNDEF);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                return tos;
            }
        case qspOpInlineIndexedArrItem: /* array item by index, we don't pre-evaluate arguments */
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
        case qspOpFormat: /* string with subexpressions, we don't pre-evaluate arguments */
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
    case qspOpValue:
        /* Copy the value instead of moving it because it has to be possible to reuse the compiled expression */
        qspCopyToNewVariant(&tos, expression->Values + item->ValueIndex);
        break;
    case qspOpArrItem:
        if (argsCount == 2)
            qspGetVarValueByIndex(QSP_STR(args[0]), args[1], &tos);
        else
            qspGetFirstVarValue(QSP_STR(args[0]), &tos);
        break;
    case qspOpInlineFirstArrItem:
        qspGetFirstVarValue(QSP_STR(expression->Values[item->ValueIndex]), &tos);
        break;
    case qspOpInlineLastArrItem:
        qspGetLastVarValue(QSP_STR(expression->Values[item->ValueIndex]), &tos);
        break;
    case qspOpAdd:
        qspAutoConvertCombine(args, args + 1, QSP_ADD_CHAR, &tos);
        break;
    case qspOpSub:
        qspAutoConvertCombine(args, args + 1, QSP_SUB_CHAR, &tos);
        break;
    case qspOpMul:
        qspAutoConvertCombine(args, args + 1, QSP_MUL_CHAR, &tos);
        break;
    case qspOpDiv:
        qspAutoConvertCombine(args, args + 1, QSP_DIV_CHAR, &tos);
        break;
    case qspOpMod:
        if (QSP_NUM(args[1]) == 0)
        {
            qspSetError(QSP_ERR_DIVBYZERO);
            break;
        }
        QSP_NUM(tos) = QSP_NUM(args[0]) % QSP_NUM(args[1]);
        break;
    case qspOpNegation:
        qspMultiplyVariantByNum(args, -1, &tos);
        break;
    case qspOpAffirmation:
        qspMultiplyVariantByNum(args, 1, &tos);
        break;
    case qspOpTuple:
        QSP_TUPLE(tos) = qspMoveToNewTuple(args, argsCount);
        break;
    case qspOpAppend:
        qspAutoConvertAppend(args, args + 1, &tos);
        break;
    case qspOpEq:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) == 0);
        break;
    case qspOpNe:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) != 0);
        break;
    case qspOpLt:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) < 0);
        break;
    case qspOpGt:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) > 0);
        break;
    case qspOpLeq:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) <= 0);
        break;
    case qspOpGeq:
        QSP_NUM(tos) = QSP_TOBOOL(qspVariantsCompare(args, args + 1) >= 0);
        break;
    /* Embedded functions -------------------------------------------------------------- */
    case qspOpLoc:
        QSP_NUM(tos) = QSP_TOBOOL(qspLocByName(QSP_STR(args[0])) != 0);
        break;
    case qspOpObj:
        QSP_NUM(tos) = qspObjsCountByName(QSP_STR(args[0]));
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
        QSP_NUM(tos) = qspUniformRand(1, 1000);
        break;
    case qspOpCountObj:
        QSP_NUM(tos) = qspCurObjsCount;
        break;
    case qspOpMsecsCount:
        QSP_NUM(tos) = qspGetTime();
        break;
    case qspOpQSPVer:
        QSP_STR(tos) = (argsCount ? qspCallVersion(QSP_STR(args[0])) : qspCallVersion(qspNullString));
        break;
    case qspOpUserText:
        QSP_STR(tos) = (qspCurInput.Str ? qspCopyToNewText(qspCurInput) : qspNullString);
        break;
    case qspOpCurLoc:
        QSP_STR(tos) = (qspCurLoc ? qspCopyToNewText(qspCurLoc->Name) : qspNullString);
        break;
    case qspOpSelObj:
        QSP_STR(tos) = (qspCurSelObject >= 0 ? qspCopyToNewText(qspCurObjects[qspCurSelObject].Name) : qspNullString);
        break;
    case qspOpSelAct:
        QSP_STR(tos) = (qspCurSelAction >= 0 ? qspCopyToNewText(qspCurActions[qspCurSelAction].Desc) : qspNullString);
        break;
    case qspOpMainText:
        QSP_STR(tos) = (qspCurDesc.Len > 0 ? qspCopyToNewText(qspBufStringToString(qspCurDesc)) : qspNullString);
        break;
    case qspOpStatText:
        QSP_STR(tos) = (qspCurVars.Len > 0 ? qspCopyToNewText(qspBufStringToString(qspCurVars)) : qspNullString);
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
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_FALSE);
    if (!var) return;
    arrIndex = (count == 2 ? qspGetVarIndex(var, args[1], QSP_FALSE) : 0);
    arrType = ((arrIndex >= 0 && arrIndex < var->ValsCount) ? var->Values[arrIndex].Type : QSP_TYPE_UNDEF);
    if (QSP_ISDEF(arrType))
    {
        QSPString typePrefix;
        switch (QSP_BASETYPE(arrType))
        {
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
        return;
    }
    QSP_PSTR(res) = qspNullString;
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
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_FALSE);
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
    int min, max;
    min = QSP_TOINT(QSP_NUM(args[0]));
    max = (count >= 2 ? QSP_TOINT(QSP_NUM(args[1])) : 1);
    if (count == 3)
    {
        int mean = QSP_TOINT(QSP_NUM(args[2]));
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
