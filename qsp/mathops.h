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
    #define QSP_MAXMATHITEMS 1000

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
        unsigned short ValueIndex; /* for operations with a value */
    } QSPMathCompiledOp;

    typedef struct
    {
        QSPMathCompiledOp *CompItems;
        int ItemsCount;
        int ItemsCapacity;
        QSPVariant *Values;
        int ValsCount;
        int ValsCapacity;
    } QSPMathExpression;

    enum
    {
        QSP_OP_UNKNOWN,
        QSP_OP_START, /* sequence point */
        QSP_OP_END, /* sequence point */
        QSP_OP_OPENROUNDBRACKET, /* sequence point */
        QSP_OP_CLOSEROUNDBRACKET, /* sequence point */
        QSP_OP_OPENSQUAREBRACKET, /* sequence point */
        QSP_OP_CLOSESQUAREBRACKET, /* sequence point */
        QSP_OP_COMMA, /* sequence point */
        QSP_OP_TUPLE,
        QSP_OP_VALUE,
        QSP_OP_NEGATION,
        QSP_OP_AFFIRMATION,
        QSP_OP_APPEND,
        QSP_OP_ADD,
        QSP_OP_SUB,
        QSP_OP_MUL,
        QSP_OP_DIV,
        QSP_OP_MOD,
        QSP_OP_AND,
        QSP_OP_OR,
        QSP_OP_NE,
        QSP_OP_LEQ,
        QSP_OP_GEQ,
        QSP_OP_EQ,
        QSP_OP_LT,
        QSP_OP_GT,

        QSP_OP_FIRSTFUNCTION,
        QSP_OP_NOT = QSP_OP_FIRSTFUNCTION,
        QSP_OP_IIF,
        QSP_OP_MIN,
        QSP_OP_MAX,
        QSP_OP_RAND,
        QSP_OP_RND,
        QSP_OP_ARRSIZE,
        QSP_OP_ARRTYPE,
        QSP_OP_ARRKEY,
        QSP_OP_ARRITEM,
        QSP_OP_INLINEFIRSTARRITEM,
        QSP_OP_INLINELASTARRITEM,
        QSP_OP_INLINEINDEXEDARRITEM,
        QSP_OP_ARRPACK,
        QSP_OP_ARRPOS,
        QSP_OP_ARRCOMP,
        QSP_OP_STR,
        QSP_OP_FORMAT,
        QSP_OP_VAL,
        QSP_OP_ISNUM,
        QSP_OP_LEN,
        QSP_OP_LCASE,
        QSP_OP_UCASE,
        QSP_OP_TRIM,
        QSP_OP_MID,
        QSP_OP_INSTR,
        QSP_OP_REPLACE,
        QSP_OP_STRCOMP,
        QSP_OP_STRFIND,
        QSP_OP_STRPOS,
        QSP_OP_FUNC,
        QSP_OP_DYNEVAL,
        QSP_OP_LOC,
        QSP_OP_CURLOC,
        QSP_OP_DESC,
        QSP_OP_MAINTEXT,
        QSP_OP_STATTEXT,
        QSP_OP_USERTEXT,
        QSP_OP_INPUT,
        QSP_OP_OBJ,
        QSP_OP_SELOBJ,
        QSP_OP_GETOBJ,
        QSP_OP_COUNTOBJ,
        QSP_OP_CUROBJS,
        QSP_OP_SELACT,
        QSP_OP_CURACTS,
        QSP_OP_RGB,
        QSP_OP_ISPLAY,
        QSP_OP_MSECSCOUNT,
        QSP_OP_QSPVER,

        QSP_OP_LASTOPERATION
    };

    /* External functions */
    void qspInitMath(void);
    void qspTerminateMath(void);
    QSPMathExpression *qspCompileMathExpression(QSPString s);
    void qspFreeMathExpression(QSPMathExpression *expression);
    QSPVariant qspCalculateValue(QSPMathExpression *expression, int itemIndex);
    QSPVariant qspCalculateExprValue(QSPString expr);

#endif
