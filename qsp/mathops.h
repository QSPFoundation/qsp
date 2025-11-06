/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"

#ifndef QSP_MATHDEFINES
    #define QSP_MATHDEFINES

    #define QSP_MATHOPSLEVELS 2
    #define QSP_MAXMATHOPSNAMES 150
    #define QSP_MAXMATHOPARGS 20
    #define QSP_MATHSTACKSIZE 30
    #define QSP_MAXMATHITEMS 200
    #define QSP_MAXCACHEDEXPSBUCKETSIZE 5
    #define QSP_CACHEDEXPSBUCKETS 512

    /* Helpers */
    #define QSP_TOBOOL(x) ((x) != 0) /* converts a number to a QSP boolean value */
    #define QSP_ISTRUE(x) ((x) != 0) /* checks whether a QSP numeric value represents boolean 'true' */
    #define QSP_ISFALSE(x) ((x) == 0) /* checks whether a QSP numeric value represents boolean 'false' */

    typedef void (*QSP_FUNCTION)(QSPVariant *args, QSP_TINYINT argsCount, QSPVariant *res);

    typedef struct
    {
        QSP_TINYINT Code;
        QSPString Name;
    } QSPMathOpName;

    typedef struct
    {
        QSP_TINYINT Priority;
        QSP_TINYINT ResType;
        QSP_TINYINT MinArgsCount;
        QSP_TINYINT MaxArgsCount;
        QSP_TINYINT ArgsTypes[QSP_MAXMATHOPARGS];
        QSP_FUNCTION Func;
    } QSPMathOperation;

    typedef struct
    {
        QSP_TINYINT OpCode;
        QSP_TINYINT ArgsCount;
        QSPVariant Value;
    } QSPMathCompiledOp;

    typedef struct
    {
        QSPMathCompiledOp *CompItems;
        int ItemsCount;
        int Capacity;
    } QSPMathExpression;

    typedef struct
    {
        QSPString Text;
        QSPMathExpression CompiledExp;
    } QSPCachedMathExp;

    typedef struct
    {
        QSPCachedMathExp Exps[QSP_MAXCACHEDEXPSBUCKETSIZE];
        int ExpsCount;
        int ExpToEvict;
    } QSPCachedMathExpsBucket;

    enum
    {
        qspOpUnknown,
        qspOpStart, /* sequence point */
        qspOpEnd, /* sequence point */
        qspOpOpenRoundBracket, /* sequence point */
        qspOpCloseRoundBracket, /* sequence point */
        qspOpOpenSquareBracket, /* sequence point */
        qspOpCloseSquareBracket, /* sequence point */
        qspOpComma, /* sequence point */
        qspOpTuple,
        qspOpValue,
        qspOpValueToFormat,
        qspOpNegation,
        qspOpAppend,
        qspOpAdd,
        qspOpSub,
        qspOpMul,
        qspOpDiv,
        qspOpMod,
        qspOpAnd,
        qspOpOr,
        qspOpNe,
        qspOpLeq,
        qspOpGeq,
        qspOpEq,
        qspOpLt,
        qspOpGt,

        qspOpFirst_Function,
        qspOpNot = qspOpFirst_Function,
        qspOpIIf,
        qspOpMin,
        qspOpMax,
        qspOpRand,
        qspOpRnd,
        qspOpArrSize,
        qspOpArrType,
        qspOpArrItem,
        qspOpFirstArrItem,
        qspOpLastArrItem,
        qspOpArrPack,
        qspOpArrPos,
        qspOpArrComp,
        qspOpStr,
        qspOpVal,
        qspOpIsNum,
        qspOpLen,
        qspOpLCase,
        qspOpUCase,
        qspOpTrim,
        qspOpMid,
        qspOpInstr,
        qspOpReplace,
        qspOpStrComp,
        qspOpStrFind,
        qspOpStrPos,
        qspOpFunc,
        qspOpDynEval,
        qspOpLoc,
        qspOpCurLoc,
        qspOpDesc,
        qspOpMainText,
        qspOpStatText,
        qspOpUserText,
        qspOpInput,
        qspOpObj,
        qspOpSelObj,
        qspOpGetObj,
        qspOpCountObj,
        qspOpCurObjs,
        qspOpSelAct,
        qspOpCurActs,
        qspOpRGB,
        qspOpIsPlay,
        qspOpMsecsCount,
        qspOpQSPVer,

        qspOpLast_Operation
    };

    /* External functions */
    void qspInitMath(void);
    void qspTerminateMath(void);
    void qspClearAllMathExps(QSP_BOOL toInit);
    QSP_BOOL qspCompileMathExpression(QSPString s, QSPMathExpression *expression);
    void qspFreeMathExpression(QSPMathExpression *expression);
    QSPVariant qspCalculateValue(QSPMathExpression *expression, int valueIndex);
    QSPVariant qspCalculateExprValue(QSPString expr);

#endif
