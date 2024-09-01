/* Copyright (C) 2001-2024 Val Argunov (byte AT qsp DOT org) */
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

#include "declarations.h"
#include "variant.h"

#ifndef QSP_MATHDEFINES
    #define QSP_MATHDEFINES

    #define QSP_OPSLEVELS 2
    #define QSP_MAXOPSNAMES 150
    #define QSP_OPMAXARGS 20
    #define QSP_STACKSIZE 30
    #define QSP_MAXITEMS 200

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
        QSP_TINYINT ArgsTypes[QSP_OPMAXARGS];
        QSP_FUNCTION Func;
    } QSPMathOperation;

    typedef struct
    {
        QSPVariant CompValues[QSP_MAXITEMS];
        QSP_TINYINT CompOpCodes[QSP_MAXITEMS];
        QSP_TINYINT CompArgsCounts[QSP_MAXITEMS];
        int ItemsCount;
        QSP_BOOL IsReusable;
    } QSPMathExpression;

    enum
    {
        qspOpUnknown,
        qspOpStart, /* sequence point */
        qspOpEnd, /* sequence point */
        qspOpComma, /* sequence point */
        qspOpOpenSquareBracket, /* sequence point */
        qspOpCloseSquareBracket, /* sequence point */
        qspOpOpenRoundBracket, /* sequence point */
        qspOpCloseRoundBracket, /* sequence point */
        qspOpTuple,
        qspOpValue,
        qspOpValueToFormat,
        qspOpNegation,
        qspOpMul,
        qspOpDiv,
        qspOpAdd,
        qspOpSub,
        qspOpMod,
        qspOpNe,
        qspOpLeq,
        qspOpGeq,
        qspOpEq,
        qspOpLt,
        qspOpGt,
        qspOpAnd,
        qspOpOr,
        qspOpAppend,

        qspOpFirst_Function,
        qspOpArrItem = qspOpFirst_Function,
        qspOpLastArrItem,
        qspOpNot,
        qspOpLoc,
        qspOpObj,
        qspOpMin,
        qspOpMax,
        qspOpRand,
        qspOpIIf,
        qspOpRGB,
        qspOpLen,
        qspOpIsNum,
        qspOpLCase,
        qspOpUCase,
        qspOpInput,
        qspOpStr,
        qspOpVal,
        qspOpArrSize,
        qspOpIsPlay,
        qspOpDesc,
        qspOpTrim,
        qspOpGetObj,
        qspOpStrComp,
        qspOpStrFind,
        qspOpStrPos,
        qspOpMid,
        qspOpArrPos,
        qspOpArrComp,
        qspOpInstr,
        qspOpReplace,
        qspOpFunc,
        qspOpDynEval,
        qspOpRnd,
        qspOpCountObj,
        qspOpMsecsCount,
        qspOpQSPVer,
        qspOpUserText,
        qspOpCurLoc,
        qspOpSelObj,
        qspOpSelAct,
        qspOpMainText,
        qspOpStatText,
        qspOpCurActs,
        qspOpCurObjs,

        qspOpLast_Operation
    };

    /* External functions */
    void qspInitMath(void);
    void qspTerminateMath(void);
    QSP_BOOL qspCompileMathExpression(QSPString s, QSP_BOOL isReusable, QSPMathExpression *expression);
    int qspFreeMathValue(QSPMathExpression *expression, int valueIndex);
    QSPVariant qspCalculateValue(QSPMathExpression *expression, int valueIndex);
    QSPVariant qspCalculateExprValue(QSPString expr);

#endif
