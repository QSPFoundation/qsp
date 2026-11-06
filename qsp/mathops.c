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
INLINE QSPString qspGetCodeBlock(QSPString *expr);
INLINE QSP_BOOL qspParseRecordName(QSPString *s, QSPField *field, QSPString *code);
INLINE QSPMathLevel *qspPushOperationToStack(QSPMathLevel *levels, int *opSp, QSP_TINYINT opCode);
INLINE int qspStoreValue(QSPMathExpression *expression, QSPVariant v);
INLINE QSPMathCompiledOp *qspAppendOperationToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, QSP_TINYINT argsCount);
INLINE QSPMathCompiledOp *qspAppendValueToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, QSPVariant v);
INLINE QSPMathCompiledOp *qspAppendVarItemToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, int nameIndex, QSP_TINYINT varType);
INLINE QSP_BOOL qspStartFormatPart(QSPMathExpression *expression, QSP_TINYINT *partsCount);
INLINE QSP_BOOL qspAppendFormatText(QSPMathExpression *expression, QSPString text, QSP_TINYINT *partsCount);
INLINE QSP_BOOL qspAppendFormatToCompiled(QSPMathExpression *expression, QSPString text);
INLINE QSP_BOOL qspAppendRecordFieldToCompiled(QSPMathExpression *expression, QSPMathLevel *level, QSPField *field, QSPString code);
INLINE QSP_BOOL qspAppendTupleToCompiled(QSPMathExpression *expression, QSPMathLevel *level);
INLINE QSP_TINYINT qspGetFieldsOpCode(QSPMathExpression *expression, int groupEnd, QSP_TINYINT *varType);
INLINE QSP_BOOL qspAppendFieldsToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, QSPField *fields, QSPString *dynamicNames, int count);
INLINE QSP_BOOL qspAppendExpressionToCompiled(QSPMathExpression *expression, QSPString s);
INLINE int qspSkipMathValue(QSPMathExpression *expression, int itemIndex);
INLINE QSPVariant qspCalculateArgumentValue(QSPMathExpression *expression, int itemIndex, QSP_TINYINT type);
INLINE QSPVariant qspCalculateFormatValue(QSPMathExpression *expression, int *argIndices, QSP_TINYINT argsCount);
INLINE QSP_BOOL qspCalculateDynamicNames(QSPMathExpression *expression, QSPTuple names, int *argIndices, QSPField *fields);
INLINE QSPVariant qspCalculateDynamicFieldValue(QSPMathExpression *expression, QSPMathCompiledOp *item, int *argIndices);
INLINE QSPVariant qspCalculateDynamicInlineFieldValue(QSPMathExpression *expression, QSPMathCompiledOp *item, int *argIndices);
INLINE QSPVariant qspCalculateFieldValue(QSPMathExpression *expression, QSPMathCompiledOp *item, int baseIndex);
INLINE QSPVariant qspCalculateInlineFieldValue(QSPMathExpression *expression, QSPMathCompiledOp *item, int baseIndex);
INLINE QSP_BOOL qspConvertRecordValues(QSPVariant *values, QSPField *fields, int count);
INLINE QSPTuple qspMoveToNewRecord(QSPVariant *values, int count, QSPTuple names);
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
    qspAddOperation(QSP_OP_RECORD, 127, 0, QSP_TYPE_TUPLE, 0, QSP_MAXMATHOPARGS, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddOperation(QSP_OP_FIELD, 127, 0, QSP_TYPE_UNDEF, 1, QSP_MAXMATHOPARGS, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddOperation(QSP_OP_INLINEFIELD, 127, 0, QSP_TYPE_UNDEF, 1, QSP_MAXMATHOPARGS, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
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
    qspAddOperation(QSP_OP_ARRTYPE, 30, qspFunctionArrType, QSP_TYPE_STR, 1, 3, QSP_TYPE_VARREF, QSP_TYPE_UNDEF, QSP_TYPE_TUPLE);
    qspAddOperation(QSP_OP_ARRKEY, 30, qspFunctionArrKey, QSP_TYPE_UNDEF, 1, 2, QSP_TYPE_VARREF, QSP_TYPE_NUM);
    qspAddOperation(QSP_OP_ARRITEM, 30, 0, QSP_TYPE_UNDEF, 1, 3, QSP_TYPE_VARREF, QSP_TYPE_UNDEF, QSP_TYPE_TUPLE);
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
    qspAddOpName(QSP_OP_FIELD, QSP_FIELDDELIM, 1, QSP_FALSE);

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

INLINE QSPString qspGetCodeBlock(QSPString *expr)
{
    QSP_CHAR *endPos, *startPos = expr->Str;
    endPos = qspDelimPos(*expr, QSP_RCODE_CHAR);
    if (!endPos)
    {
        qspSetError(QSP_ERR_QUOTNOTFOUND);
        return qspNullString;
    }
    expr->Str = endPos + QSP_STATIC_LEN(QSP_RCODE);
    return qspStringFromPair(startPos + QSP_STATIC_LEN(QSP_LCODE), endPos);
}

INLINE QSP_BOOL qspParseRecordName(QSPString *s, QSPField *field, QSPString *code)
{
    if (!qspParseFieldName(s, field, code)) return QSP_FALSE;
    qspSkipSpaces(s);
    if (!qspParseChar(s, QSP_FIELDVALUEDELIM_CHAR))
    {
        qspSetError(QSP_ERR_SYNTAX);
        return QSP_FALSE;
    }
    return QSP_TRUE;
}

INLINE QSPMathLevel *qspPushOperationToStack(QSPMathLevel *levels, int *opSp, QSP_TINYINT opCode)
{
    QSPMathLevel *level;
    if (*opSp >= QSP_MATHSTACKSIZE - 1)
    {
        qspSetError(QSP_ERR_COMPLEXEXPRESSION);
        return 0;
    }
    ++(*opSp);
    level = levels + *opSp;
    level->OpCode = opCode;
    level->ArgsCount = (opCode < QSP_OP_FIRSTFUNCTION ? qspOps[opCode].MinArgsCount : 0);
    return level;
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

INLINE QSPMathCompiledOp *qspAppendValueToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, QSPVariant v)
{
    QSPMathCompiledOp *compiledOp = qspAppendOperationToCompiled(expression, opCode, 0);
    if (!compiledOp) return 0;
    compiledOp->ValueIndex = (unsigned short)qspStoreValue(expression, v);
    return compiledOp;
}

INLINE QSPMathCompiledOp *qspAppendVarItemToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, int nameIndex, QSP_TINYINT varType)
{
    QSPMathCompiledOp *compiledOp = qspAppendOperationToCompiled(expression, opCode, qspOps[opCode].MinArgsCount);
    if (!compiledOp) return 0;
    compiledOp->NameIndex = nameIndex;
    compiledOp->VarType = varType;
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

INLINE QSP_BOOL qspAppendRecordFieldToCompiled(QSPMathExpression *expression, QSPMathLevel *level, QSPField *field, QSPString code)
{
    QSPField *fieldItem;
    QSPTuple *fieldNames = &level->Fields;
    if (fieldNames->FieldsCount > level->ArgsCount)
    {
        qspSetError(QSP_ERR_SYNTAX); /* some names don't have values */
        return QSP_FALSE;
    }
    if (qspGetTupleFieldIndex(*fieldNames, field->NameIndex) >= 0)
    {
        qspSetError(QSP_ERR_DUPLICATEFIELD);
        return QSP_FALSE;
    }
    if (field->NameIndex > 0)
    {
        qspReserveTupleItems(fieldNames, level->ArgsCount + 1); /* values before the name have no names */
        fieldItem = fieldNames->Fields + level->ArgsCount;
        fieldItem->NameIndex = field->NameIndex;
    }
    else
    {
        /* Code of the dynamic field name gets stored before its value (type prefix) */
        if (!qspAppendExpressionToCompiled(expression, code)) return QSP_FALSE;
        qspReserveTupleItems(fieldNames, level->ArgsCount + 2);
        fieldItem = fieldNames->Fields + level->ArgsCount;
        fieldItem->NameIndex = field->NameIndex;
        qspInitVariant(&fieldItem->Value, QSP_TYPE_STR); /* the name is a string */
        ++level->ArgsCount;
        ++fieldItem;
    }
    /* Store the specified value (type prefix) */
    qspMoveToNewVariant(&fieldItem->Value, &field->Value);
    return QSP_TRUE;
}

INLINE QSP_BOOL qspAppendTupleToCompiled(QSPMathExpression *expression, QSPMathLevel *level)
{
    if (++level->ArgsCount > qspOps[level->OpCode].MaxArgsCount)
    {
        qspSetError(QSP_ERR_ARGSCOUNT);
        return QSP_FALSE;
    }
    if (level->OpCode == QSP_OP_RECORD && level->Fields.FieldsCount) /* a record without names is a tuple */
    {
        QSPMathCompiledOp *compiledOp = qspAppendOperationToCompiled(expression, QSP_OP_RECORD, level->ArgsCount);
        if (!compiledOp) return QSP_FALSE;
        /* Transfer field names to the expression */
        compiledOp->ValueIndex = (unsigned short)qspStoreValue(expression, qspTupleVariant(level->Fields));
        level->Fields = qspNullTuple;
        return QSP_TRUE;
    }
    return (qspAppendOperationToCompiled(expression, QSP_OP_TUPLE, level->ArgsCount) != 0);
}

INLINE QSP_TINYINT qspGetFieldsOpCode(QSPMathExpression *expression, int groupEnd, QSP_TINYINT *varType)
{
    *varType = QSP_TYPE_UNDEF;
    if (expression->ItemsCount != groupEnd) /* parenthesized value is read as a whole */
    {
        QSPMathCompiledOp *lastOp = expression->CompItems + expression->ItemsCount - 1;
        switch (lastOp->OpCode)
        {
        case QSP_OP_INLINEFIRSTARRITEM:
        case QSP_OP_INLINELASTARRITEM:
        case QSP_OP_INLINEINDEXEDARRITEM:
            /* Fields of array item are read in place */
            *varType = lastOp->VarType;
            return QSP_OP_INLINEFIELD;
        }
    }
    return QSP_OP_FIELD;
}

INLINE QSP_BOOL qspAppendFieldsToCompiled(QSPMathExpression *expression, QSP_TINYINT opCode, QSPField *fields, QSPString *dynamicNames, int count)
{
    int i;
    QSPVariant v;
    QSPMathCompiledOp *lastOp;
    QSP_TINYINT argsCount = 1; /* the value before the fields is the first argument */
    for (i = 0; i < count; ++i)
    {
        if (fields[i].NameIndex < 0)
        {
            if (argsCount == QSP_MAXMATHOPARGS)
            {
                qspSetError(QSP_ERR_COMPLEXEXPRESSION);
                return QSP_FALSE;
            }
            if (!qspAppendExpressionToCompiled(expression, dynamicNames[i])) return QSP_FALSE;
            ++argsCount;
        }
    }
    lastOp = qspAppendOperationToCompiled(expression, opCode, argsCount);
    if (!lastOp) return QSP_FALSE;
    v = qspTupleVariant(qspMoveFieldsToNewTuple(fields, count));
    lastOp->ValueIndex = (unsigned short)qspStoreValue(expression, v);
    return QSP_TRUE;
}

INLINE QSP_BOOL qspAppendExpressionToCompiled(QSPMathExpression *expression, QSPString s)
{
    QSPVariant v;
    QSPMathLevel *level, levels[QSP_MATHSTACKSIZE]; /* pending operations */
    QSPField field, fields[QSP_MAXMATHOPARGS];
    QSPString name, plainName, code, dynamicNames[QSP_MAXMATHOPARGS];
    QSP_TINYINT opCode, varType, fieldsOp;
    unsigned int nameHash;
    int nameIndex, fieldsCount, opSp = -1, groupEnd = -1; /* specifies where the last parenthesized value ends */
    QSP_BOOL waitForOperator = QSP_FALSE;
    if (!qspPushOperationToStack(levels, &opSp, QSP_OP_START)) return QSP_FALSE;
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
            while (qspOps[opCode].Priority <= qspOps[levels[opSp].OpCode].Priority && qspOps[levels[opSp].OpCode].Priority != 127)
            {
                if (!qspAppendOperationToCompiled(expression, levels[opSp].OpCode, levels[opSp].ArgsCount)) break;
                --opSp; /* it's always positive */
            }
            if (qspErrorNum) break;
            switch (opCode)
            {
            case QSP_OP_END:
            case QSP_OP_CLOSEROUNDBRACKET:
            case QSP_OP_CLOSESQUAREBRACKET:
                if (levels[opSp].OpCode == QSP_OP_TUPLE || levels[opSp].OpCode == QSP_OP_RECORD)
                {
                    if (!qspAppendTupleToCompiled(expression, levels + opSp)) break;
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
                if (levels[opSp].OpCode != QSP_OP_OPENROUNDBRACKET)
                {
                    qspSetError(QSP_ERR_BRACKETNOTFOUND);
                    break;
                }
                opCode = levels[--opSp].OpCode;
                if (opCode >= QSP_OP_FIRSTFUNCTION)
                {
                    ++levels[opSp].ArgsCount;
                    if (levels[opSp].ArgsCount < qspOps[opCode].MinArgsCount || levels[opSp].ArgsCount > qspOps[opCode].MaxArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                    /* Brackets complete the call, a field after them reads its result */
                    if (!qspAppendOperationToCompiled(expression, opCode, levels[opSp].ArgsCount)) break;
                    --opSp;
                }
                else
                    groupEnd = expression->ItemsCount;
                break;
            case QSP_OP_CLOSESQUAREBRACKET:
                if (levels[opSp].OpCode != QSP_OP_OPENSQUAREBRACKET)
                {
                    qspSetError(QSP_ERR_BRACKETNOTFOUND);
                    break;
                }
                --opSp; /* it's always positive */
                if (levels[opSp].OpCode == QSP_OP_INLINEINDEXEDARRITEM)
                {
                    /* The index is complete, the name waited on the stack */
                    if (!qspAppendVarItemToCompiled(expression, QSP_OP_INLINEINDEXEDARRITEM, levels[opSp].NameIndex, levels[opSp].VarType)) break;
                    --opSp;
                }
                break;
            case QSP_OP_COMMA:
                if (levels[opSp].OpCode == QSP_OP_OPENROUNDBRACKET && levels[opSp - 1].OpCode >= QSP_OP_FIRSTFUNCTION)
                {
                    if (++levels[opSp - 1].ArgsCount > qspOps[levels[opSp - 1].OpCode].MaxArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                }
                else
                {
                    if (levels[opSp].OpCode != QSP_OP_TUPLE && levels[opSp].OpCode != QSP_OP_RECORD)
                    {
                        if (!qspPushOperationToStack(levels, &opSp, QSP_OP_TUPLE)) break;
                    }
                    if (++levels[opSp].ArgsCount > qspOps[levels[opSp].OpCode].MaxArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                }
                waitForOperator = QSP_FALSE;
                break;
            case QSP_OP_FIELD:
                fieldsOp = qspGetFieldsOpCode(expression, groupEnd, &varType);
                fieldsCount = qspParseFieldsPath(&s, varType, fields, dynamicNames);
                if (fieldsCount >= 0) qspAppendFieldsToCompiled(expression, fieldsOp, fields, dynamicNames, fieldsCount);
                break;
            default:
                if (!qspPushOperationToStack(levels, &opSp, opCode)) break;
                waitForOperator = QSP_FALSE;
                break;
            }
            if (qspErrorNum) break;
        }
        else
        {
            if (qspIsEmpty(s))
            {
                if (levels[opSp].OpCode >= QSP_OP_FIRSTFUNCTION)
                    qspSetError(QSP_ERR_ARGSCOUNT);
                else
                    qspSetError(QSP_ERR_SYNTAX);
                break;
            }
            else if (qspIsInClass(*s.Str, QSP_CHAR_DIGIT))
            {
                v = qspNumVariant(qspParseUnsignedNumber(&s));
                switch (levels[opSp].OpCode)
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
            else if (qspParseChar(&s, QSP_UNDEFTYPE_CHAR))
            {
                qspInitVariant(&v, QSP_TYPE_UNDEF);
                if (!qspAppendValueToCompiled(expression, QSP_OP_VALUE, v)) break;
                waitForOperator = QSP_TRUE;
            }
            else if (qspParseChar(&s, QSP_NEGATION_CHAR))
            {
                if (!qspPushOperationToStack(levels, &opSp, QSP_OP_NEGATION)) break;
            }
            else if (qspParseChar(&s, QSP_AFFIRMATION_CHAR))
            {
                if (!qspPushOperationToStack(levels, &opSp, QSP_OP_AFFIRMATION)) break;
            }
            else if (qspParseChar(&s, QSP_LRBRACK_CHAR)) /* a subexpression OR a tuple */
            {
                if (!qspPushOperationToStack(levels, &opSp, QSP_OP_OPENROUNDBRACKET)) break;
            }
            else if (qspParseChar(&s, QSP_RRBRACK_CHAR)) /* happens when "(" gets closed with ")" without any values */
            {
                opCode = levels[opSp].OpCode;
                if (opCode != QSP_OP_OPENROUNDBRACKET)
                {
                    if (opCode >= QSP_OP_FIRSTFUNCTION)
                        qspSetError(QSP_ERR_ARGSCOUNT);
                    else
                        qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                opCode = levels[--opSp].OpCode;
                if (opCode >= QSP_OP_FIRSTFUNCTION)
                {
                    if (levels[opSp].ArgsCount < qspOps[opCode].MinArgsCount)
                    {
                        qspSetError(QSP_ERR_ARGSCOUNT);
                        break;
                    }
                    /* Brackets complete the call */
                    if (!qspAppendOperationToCompiled(expression, opCode, levels[opSp].ArgsCount)) break;
                    --opSp;
                }
                else
                {
                    if (!qspAppendOperationToCompiled(expression, QSP_OP_TUPLE, 0)) break;
                }
                waitForOperator = QSP_TRUE;
            }
            else if (qspParseChar(&s, QSP_LSBRACK_CHAR)) /* a tuple */
            {
                qspSkipSpaces(&s);
                if (qspParseChar(&s, QSP_RSBRACK_CHAR))
                {
                    if (!qspAppendOperationToCompiled(expression, QSP_OP_TUPLE, 0)) break;
                    waitForOperator = QSP_TRUE;
                }
                else
                {
                    if (!qspPushOperationToStack(levels, &opSp, QSP_OP_OPENSQUAREBRACKET)) break;
                    level = qspPushOperationToStack(levels, &opSp, QSP_OP_RECORD); /* brackets might hold field names */
                    if (!level) break;
                    level->Fields = qspNullTuple;
                }
            }
            else if (qspParseChar(&s, QSP_USERFUNC_CHAR)) /* a user function call */
            {
                qspSkipSpaces(&s);
                name = qspParseToDelim(&s, QSP_CHAR_DELIM);
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
                if (qspParseChar(&s, QSP_LRBRACK_CHAR))
                {
                    level = qspPushOperationToStack(levels, &opSp, QSP_OP_FUNC);
                    if (!level) break;
                    ++level->ArgsCount; /* added the function name already */
                    if (!qspPushOperationToStack(levels, &opSp, QSP_OP_OPENROUNDBRACKET)) break;
                }
                else
                {
                    /* The call without arguments is complete */
                    if (!qspAppendOperationToCompiled(expression, QSP_OP_FUNC, 1)) break;
                    waitForOperator = QSP_TRUE;
                }
            }
            else if (qspIsInClass(*s.Str, QSP_CHAR_QUOT))
            {
                code = s;
                if (!qspUnquoteToNewText(&s, &name)) break;
                code.End = s.Str; /* code of a computed name */
                qspSkipSpaces(&s);
                if (levels[opSp].OpCode == QSP_OP_RECORD && qspParseChar(&s, QSP_FIELDVALUEDELIM_CHAR)) /* field name in quotes */
                {
                    if (!qspInitQuotedField(&field, name, QSP_TYPE_UNDEF))
                    {
                        qspFreeString(&name);
                        break;
                    }
                    qspFreeString(&name);
                    if (!qspAppendRecordFieldToCompiled(expression, levels + opSp, &field, code)) break;
                }
                else
                {
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
            }
            else if (!qspIsInClass(*s.Str, QSP_CHAR_DELIM))
            {
                name = qspParseToDelim(&s, QSP_CHAR_DELIM | QSP_CHAR_FIELDREF);
                if (qspIsEmpty(name))
                {
                    qspSetError(QSP_ERR_SYNTAX);
                    break;
                }
                qspSkipSpaces(&s);
                if (levels[opSp].OpCode == QSP_OP_RECORD && !qspIsEmpty(s) &&
                    (qspIsInClass(*s.Str, QSP_CHAR_QUOT) || *s.Str == QSP_FIELDVALUEDELIM_CHAR)) /* a field name */
                {
                    s.Str = name.Str; /* field parser reads the name with prefix/quotes */
                    if (!qspParseRecordName(&s, &field, &code)) break;
                    if (!qspAppendRecordFieldToCompiled(expression, levels + opSp, &field, code)) break;
                }
                else
                {
                    opCode = qspFunctionOpCode(name);
                    if (opCode >= QSP_OP_FIRSTFUNCTION)
                    {
                        if (qspParseChar(&s, QSP_LRBRACK_CHAR))
                        {
                            if (!qspPushOperationToStack(levels, &opSp, opCode)) break;
                            if (!qspPushOperationToStack(levels, &opSp, QSP_OP_OPENROUNDBRACKET)) break;
                        }
                        else if (qspOps[opCode].MinArgsCount < 2)
                        {
                            if (qspOps[opCode].MinArgsCount)
                            {
                                /* The function has single argument */
                                level = qspPushOperationToStack(levels, &opSp, opCode);
                                if (!level) break;
                                ++level->ArgsCount;
                            }
                            else
                            {
                                /* The function has no arguments */
                                if (!qspAppendOperationToCompiled(expression, opCode, 0)) break;
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
                        plainName = qspPrepareVarName(name, &nameHash); /* validates the name */
                        if (qspIsEmpty(plainName))
                        {
                            qspSetError(QSP_ERR_INCORRECTVARNAME);
                            break;
                        }
                        nameIndex = qspGetExactNameIndex(plainName, nameHash, QSP_TRUE);
                        varType = qspGetVarType(name);
                        if (qspParseChar(&s, QSP_LSBRACK_CHAR))
                        {
                            qspSkipSpaces(&s);
                            if (qspParseChar(&s, QSP_RSBRACK_CHAR))
                            {
                                if (!qspAppendVarItemToCompiled(expression, QSP_OP_INLINELASTARRITEM, nameIndex, varType)) break;
                                waitForOperator = QSP_TRUE;
                            }
                            else
                            {
                                /* The operation gets appended after its index, so the variable waits on the stack */
                                level = qspPushOperationToStack(levels, &opSp, QSP_OP_INLINEINDEXEDARRITEM);
                                if (!level) break;
                                level->NameIndex = nameIndex;
                                level->VarType = varType;
                                if (!qspPushOperationToStack(levels, &opSp, QSP_OP_OPENSQUAREBRACKET)) break;
                            }
                        }
                        else
                        {
                            if (!qspAppendVarItemToCompiled(expression, QSP_OP_INLINEFIRSTARRITEM, nameIndex, varType)) break;
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
    /* Release field names of open records */
    while (opSp >= 0)
    {
        if (levels[opSp].OpCode == QSP_OP_RECORD)
            qspFreeTuple(&levels[opSp].Fields);
        --opSp;
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

INLINE QSP_BOOL qspCalculateDynamicNames(QSPMathExpression *expression, QSPTuple names, int *argIndices, QSPField *fields)
{
    /* Read dynamic field names from arguments */
    QSPVariant name;
    int oldLocationState = qspLocationState;
    QSPField *fieldsEnd = fields + names.FieldsCount;
    memcpy(fields, names.Fields, names.FieldsCount * sizeof(QSPField));
    while (fields < fieldsEnd)
    {
        if (fields->NameIndex < 0)
        {
            name = qspCalculateArgumentValue(expression, *argIndices, QSP_TYPE_STR);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            if (!qspGetFieldNameIndex(QSP_STR(name), QSP_REF_GET, &fields->NameIndex))
            {
                qspFreeVariant(&name);
                return QSP_FALSE;
            }
            qspFreeVariant(&name);
            ++argIndices;
        }
        ++fields;
    }
    return QSP_TRUE;
}

INLINE QSPVariant qspCalculateDynamicFieldValue(QSPMathExpression *expression, QSPMathCompiledOp *item, int *argIndices)
{
    QSPVariant base, res, *value;
    QSPField *fields;
    int oldLocationState = qspLocationState;
    QSPTuple names = QSP_TUPLE(expression->Values[item->ValueIndex]);
    base = qspCalculateArgumentValue(expression, argIndices[0], QSP_TYPE_UNDEF);
    if (qspLocationState != oldLocationState) return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    fields = (QSPField *)qspAllocateMemory(names.FieldsCount * sizeof(QSPField));
    if (!qspCalculateDynamicNames(expression, names, argIndices + 1, fields))
    {
        qspReleaseMemory(fields);
        qspFreeVariant(&base);
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    }
    value = qspGetTupleFieldValueByPath(&base, fields, names.FieldsCount);
    qspInitVariantFrom(&res, value, names.Fields[names.FieldsCount - 1].Value.Type); /* value has no name, the last field types it */
    qspReleaseMemory(fields);
    qspFreeVariant(&base);
    return res;
}

INLINE QSPVariant qspCalculateDynamicInlineFieldValue(QSPMathExpression *expression, QSPMathCompiledOp *item, int *argIndices)
{
    QSPField *fields;
    QSPVariant res = qspGetEmptyVariant(QSP_TYPE_UNDEF);
    QSPMathCompiledOp *baseItem = expression->CompItems + argIndices[0];
    QSPTuple names = QSP_TUPLE(expression->Values[item->ValueIndex]);
    fields = (QSPField *)qspAllocateMemory(names.FieldsCount * sizeof(QSPField));
    switch (baseItem->OpCode)
    {
    case QSP_OP_INLINEFIRSTARRITEM:
        if (qspCalculateDynamicNames(expression, names, argIndices + 1, fields))
            qspGetFirstVarFieldValue(baseItem->NameIndex, baseItem->VarType, fields, names.FieldsCount, &res);
        break;
    case QSP_OP_INLINELASTARRITEM:
        if (qspCalculateDynamicNames(expression, names, argIndices + 1, fields))
            qspGetLastVarFieldValue(baseItem->NameIndex, baseItem->VarType, fields, names.FieldsCount, &res);
        break;
    default: /* QSP_OP_INLINEINDEXEDARRITEM */
        {
            int oldLocationState = qspLocationState;
            QSPVariant index = qspCalculateArgumentValue(expression, argIndices[0] - 1, QSP_TYPE_UNDEF);
            if (qspLocationState != oldLocationState) break;
            if (qspCalculateDynamicNames(expression, names, argIndices + 1, fields))
                qspGetVarFieldValueByIndex(baseItem->NameIndex, baseItem->VarType, index, fields, names.FieldsCount, &res);
            qspFreeVariant(&index);
            break;
        }
    }
    qspReleaseMemory(fields);
    return res;
}

INLINE QSPVariant qspCalculateFieldValue(QSPMathExpression *expression, QSPMathCompiledOp *item, int baseIndex)
{
    QSPVariant base, res, *value;
    int oldLocationState = qspLocationState;
    QSPTuple names = QSP_TUPLE(expression->Values[item->ValueIndex]);
    base = qspCalculateArgumentValue(expression, baseIndex, QSP_TYPE_UNDEF);
    if (qspLocationState != oldLocationState) return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    value = qspGetTupleFieldValueByPath(&base, names.Fields, names.FieldsCount);
    qspInitVariantFrom(&res, value, names.Fields[names.FieldsCount - 1].Value.Type); /* value has no name, the last field types it */
    qspFreeVariant(&base);
    return res;
}

INLINE QSPVariant qspCalculateInlineFieldValue(QSPMathExpression *expression, QSPMathCompiledOp *item, int baseIndex)
{
    /* Fields of the array item get read in place */
    QSPVariant res = qspGetEmptyVariant(QSP_TYPE_UNDEF);
    QSPMathCompiledOp *baseItem = expression->CompItems + baseIndex;
    QSPTuple names = QSP_TUPLE(expression->Values[item->ValueIndex]);
    switch (baseItem->OpCode)
    {
    case QSP_OP_INLINEFIRSTARRITEM:
        qspGetFirstVarFieldValue(baseItem->NameIndex, baseItem->VarType, names.Fields, names.FieldsCount, &res);
        break;
    case QSP_OP_INLINELASTARRITEM:
        qspGetLastVarFieldValue(baseItem->NameIndex, baseItem->VarType, names.Fields, names.FieldsCount, &res);
        break;
    default: /* QSP_OP_INLINEINDEXEDARRITEM */
        {
            int oldLocationState = qspLocationState;
            QSPVariant index = qspCalculateArgumentValue(expression, baseIndex - 1, QSP_TYPE_UNDEF);
            if (qspLocationState != oldLocationState) return qspGetEmptyVariant(QSP_TYPE_UNDEF);
            qspGetVarFieldValueByIndex(baseItem->NameIndex, baseItem->VarType, index, names.Fields, names.FieldsCount, &res);
            qspFreeVariant(&index);
            break;
        }
    }
    return res;
}

INLINE QSP_BOOL qspConvertRecordValues(QSPVariant *values, QSPField *fields, int count)
{
    QSP_TINYINT type;
    while (--count >= 0)
    {
        type = fields->Value.Type;
        if (QSP_ISDEF(type) && !qspConvertVariantTo(values, type))
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            return QSP_FALSE;
        }
        ++values, ++fields;
    }
    return QSP_TRUE;
}

INLINE QSPTuple qspMoveToNewRecord(QSPVariant *values, int count, QSPTuple names)
{
    /* Passed values contain both dynamic field names and field values */
    QSPTuple tuple;
    QSPVariant *valuesEnd = values + count;
    QSPField *item, *name, *namesEnd = names.Fields + names.FieldsCount;
    /* Calculate the actual number of fields, exclude unknown names */
    for (name = names.Fields; name < namesEnd; ++name)
        if (name->NameIndex < 0) --count;
    /* Compose result with field names and values */
    tuple = qspNewTuple(count);
    tuple.FieldsCount = 0;
    for (item = tuple.Fields, name = names.Fields; name < namesEnd; ++item, ++name, ++values)
    {
        if (name->NameIndex < 0)
        {
            /* The value is the name of the field */
            if (!qspGetFieldNameIndex(QSP_PSTR(values), QSP_REF_GETORCREATE, &item->NameIndex))
            {
                qspFreeTuple(&tuple);
                return qspNullTuple;
            }
            ++name;
            ++values;
        }
        else
            item->NameIndex = name->NameIndex;
        if (qspGetTupleFieldIndex(tuple, item->NameIndex) >= 0)
        {
            qspFreeTuple(&tuple);
            qspSetError(QSP_ERR_DUPLICATEFIELD);
            return qspNullTuple;
        }
        qspMoveToNewVariant(&item->Value, values);
        ++tuple.FieldsCount;
    }
    tuple.FieldsCount = count;
    /* Move the rest of the unnamed values */
    while (values < valuesEnd)
    {
        item->NameIndex = 0;
        qspMoveToNewVariant(&item->Value, values);
        ++item;
        ++values;
    }
    return tuple;
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
                qspGetVarValueByIndex(item->NameIndex, item->VarType, index, &tos);
                qspFreeVariant(&index);
                if (qspLocationState != oldLocationState)
                    return qspGetEmptyVariant(QSP_TYPE_UNDEF);
                return tos;
            }
        case QSP_OP_FORMAT: /* string with subexpressions, we don't pre-evaluate arguments */
            return qspCalculateFormatValue(expression, argIndices, argsCount);
        case QSP_OP_FIELD: /* field of a value */
            if (argsCount > 1)
                return qspCalculateDynamicFieldValue(expression, item, argIndices);
            return qspCalculateFieldValue(expression, item, argIndices[0]);
        case QSP_OP_INLINEFIELD: /* field of array item, we don't pre-evaluate arguments */
            if (argsCount > 1)
                return qspCalculateDynamicInlineFieldValue(expression, item, argIndices);
            return qspCalculateInlineFieldValue(expression, item, argIndices[0]);
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
        switch (argsCount)
        {
        case 1:
            qspGetVarValueByName(QSP_STR(args[0]), qspNumVariant(0), &tos);
            break;
        case 2:
            qspGetVarValueByName(QSP_STR(args[0]), args[1], &tos);
            break;
        default:
            qspGetVarFieldValueByPath(QSP_STR(args[0]), args[1], QSP_TUPLE(args[2]), &tos);
            break;
        }
        break;
    case QSP_OP_INLINEFIRSTARRITEM:
        qspGetFirstVarValue(item->NameIndex, item->VarType, &tos);
        break;
    case QSP_OP_INLINELASTARRITEM:
        qspGetLastVarValue(item->NameIndex, item->VarType, &tos);
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
    case QSP_OP_RECORD:
        {
            QSPTuple names = QSP_TUPLE(expression->Values[item->ValueIndex]);
            if (!qspConvertRecordValues(args, names.Fields, names.FieldsCount)) break;
            QSP_TUPLE(tos) = qspMoveToNewRecord(args, argsCount, names);
            break;
        }
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
        QSP_PNUM(res) = QSP_TUPLE(args[0]).FieldsCount;
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
    QSPVariant *value;
    QSPString typePrefix;
    QSPVar *var = qspVarReferenceByName(QSP_STR(args[0]), QSP_REF_GET);
    if (!var) return;
    arrIndex = (count >= 2 ? qspGetVarItemIndex(var, args[1], QSP_FALSE) : 0);
    value = ((arrIndex >= 0 && arrIndex < var->ValsCount) ? var->Values + arrIndex : 0);
    if (count == 3)
    {
        /* Extract the specified fields anyway to raise an error if they are wrong */
        QSPField fields[QSP_MAXMATHOPARGS];
        int fieldsCount = qspGetFieldsFromPath(QSP_TUPLE(args[2]), fields, QSP_REF_GET);
        if (fieldsCount < 0) return;
        value = qspGetTupleFieldValueByPath(value, fields, fieldsCount);
    }
    if (!value)
    {
        QSP_PSTR(res) = qspNullString; /* no such item or field */
        return;
    }
    switch (QSP_BASETYPE(value->Type))
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
    QSPVar *var = qspVarReferenceByName(QSP_STR(args[0]), QSP_REF_GET);
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
    int startInd, maxCount;
    QSPVar *var = qspVarReferenceByName(QSP_STR(args[0]), QSP_REF_GET);
    if (!var) return;
    startInd = (count >= 2 ? QSP_TOINT(QSP_NUM(args[1])) : 0);
    maxCount = (count == 3 ? QSP_TOINT(QSP_NUM(args[2])) : var->ValsCount);
    QSP_PTUPLE(res) = qspCopyArrayToNewTuple(var, startInd, maxCount);
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
