/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef QSP_H
    #define QSP_H

    #include "qsp_export.h"

    enum
    {
        QSP_ERR_INTERNAL = 10,
        QSP_ERR_DIVBYZERO,
        QSP_ERR_TYPEMISMATCH,
        QSP_ERR_LOCCALLDEPTH,
        QSP_ERR_COMPLEXEXPRESSION,
        QSP_ERR_CANTLOADFILE,
        QSP_ERR_GAMENOTLOADED,
        QSP_ERR_COLONNOTFOUND,
        QSP_ERR_ENDNOTFOUND,
        QSP_ERR_CANTINCFILE,
        QSP_ERR_CANTADDACTION,
        QSP_ERR_CANTADDOBJECT,
        QSP_ERR_CANTADDMENUITEM,
        QSP_ERR_LOCNOTFOUND,
        QSP_ERR_LABELNOTFOUND,
        QSP_ERR_INCORRECTNAME,
        QSP_ERR_QUOTNOTFOUND,
        QSP_ERR_BRACKETNOTFOUND,
        QSP_ERR_SYNTAX,
        QSP_ERR_UNKNOWNACTION,
        QSP_ERR_ARGSCOUNT,
        QSP_ERR_TOOMANYVARS,
        QSP_ERR_INCORRECTREGEXP,
        QSP_ERR_CODENOTFOUND,
        QSP_ERR_LOOPWHILENOTFOUND
    };

    enum
    {
        QSP_WIN_MAIN = 1 << 0,
        QSP_WIN_VARS = 1 << 1,
        QSP_WIN_ACTS = 1 << 2,
        QSP_WIN_OBJS = 1 << 3,
        QSP_WIN_INPUT = 1 << 4,
        QSP_WIN_VIEW = 1 << 5,
        QSP_WIN_ALL = QSP_WIN_MAIN | QSP_WIN_VARS | QSP_WIN_ACTS | QSP_WIN_OBJS | QSP_WIN_INPUT | QSP_WIN_VIEW
    };

    enum
    {
        QSP_CALL_DEBUG, /* void func(QSPString str) */
        QSP_CALL_ISPLAYINGFILE, /* QSP_BOOL func(QSPString file) */
        QSP_CALL_PLAYFILE, /* void func(QSPString file, int volume) */
        QSP_CALL_CLOSEFILE, /* void func(QSPString file) */
        QSP_CALL_SHOWIMAGE, /* void func(QSPString file) */
        QSP_CALL_SHOWWINDOW, /* void func(int type, QSP_BOOL toShow) */
        QSP_CALL_SHOWMENU, /* int func(QSPListItem *items, int count) */
        QSP_CALL_SHOWMSGSTR, /* void func(QSPString text) */
        QSP_CALL_REFRESHINT, /* void func(QSP_BOOL isForced, QSP_BOOL isNewDesc) */
        QSP_CALL_SETTIMER, /* void func(int msecs) */
        QSP_CALL_SETINPUTSTRTEXT, /* void func(QSPString text) */
        QSP_CALL_SYSTEM, /* void func(QSPString cmd) */
        QSP_CALL_OPENGAME, /* void func(QSPString file, QSP_BOOL isNewGame) */
        QSP_CALL_INITGAME, /* void func(QSP_BOOL isNewGame) */
        QSP_CALL_OPENGAMESTATUS, /* void func(QSPString file) */
        QSP_CALL_SAVEGAMESTATUS, /* void func(QSPString file) */
        QSP_CALL_SLEEP, /* void func(int msecs) */
        QSP_CALL_GETMSCOUNT, /* int func() */
        QSP_CALL_INPUTBOX, /* void func(QSPString text, QSP_CHAR *buffer, int maxLen) */
        QSP_CALL_VERSION, /* void func(QSPString param, QSP_CHAR *buffer, int maxLen) */
        QSP_CALL_DUMMY
    };

    enum
    {
        QSP_TYPE_TERM = -64, /* not used for values */
        QSP_TYPE_INLINESTR = -1, /* not used for values */
        QSP_TYPE_TUPLE = 0,
        QSP_TYPE_NUM = 1,
        QSP_TYPE_BOOL = 2,
        QSP_TYPE_STR = 3,
        QSP_TYPE_CODE = 4,
        QSP_TYPE_VARREF = 5,
        QSP_TYPE_UNDEF = 6, /* not used for values, it has to be a string-based type */
        QSP_TYPE_DEFINED_TYPES /* represents the number of defined values */
    };

    #define QSP_TRUE 1
    #define QSP_FALSE 0

    typedef signed char QSP_TINYINT; /* char type can be unsigned by default */
    typedef char QSP_BOOL;

    #ifdef QSP_USE_BIGINT
        typedef long long QSP_BIGINT;
        #define QSP_MAX_BIGINT (((unsigned long long)-1) >> 1)
        #define QSP_MAX_BIGINT_LEN 20 /* don't forget about sign */
        #define QSP_TOINT(x) qspToInt(x)
    #else
        typedef int QSP_BIGINT;
        #define QSP_MAX_BIGINT (((unsigned int)-1) >> 1)
        #define QSP_MAX_BIGINT_LEN 12 /* don't forget about sign */
        #define QSP_TOINT(x) (x)
    #endif

    #define QSP_INC_POSITIVE(x) (++(x))
    #define QSP_DEC_POSITIVE(x) ((x) -= ((x) > 0))

    static const QSP_TINYINT qspBaseTypeTable[QSP_TYPE_DEFINED_TYPES] =
    {
        /* TUPLE */  QSP_TYPE_TUPLE,
        /* NUMBER */ QSP_TYPE_NUM,
        /* BOOL */   QSP_TYPE_NUM,
        /* STRING */ QSP_TYPE_STR,
        /* CODE */   QSP_TYPE_STR,
        /* VARREF */ QSP_TYPE_STR,
        /* UNDEF */  QSP_TYPE_STR
    };

    #define QSP_ISDEF(a) ((a) != QSP_TYPE_UNDEF)
    #define QSP_ISTUPLE(a) (qspBaseTypeTable[a] == QSP_TYPE_TUPLE)
    #define QSP_ISNUM(a) (qspBaseTypeTable[a] == QSP_TYPE_NUM)
    #define QSP_ISSTR(a) (qspBaseTypeTable[a] == QSP_TYPE_STR)
    #define QSP_BASETYPE(a) qspBaseTypeTable[a]

    #define QSP_STR(a) (a).Val.Str
    #define QSP_NUM(a) (a).Val.Num
    #define QSP_TUPLE(a) (a).Val.Tuple
    #define QSP_PSTR(a) (a)->Val.Str
    #define QSP_PNUM(a) (a)->Val.Num
    #define QSP_PTUPLE(a) (a)->Val.Tuple

    typedef struct QSPVariant_s QSPVariant;

    typedef struct
    {
        QSPVariant *Vals;
        int ValsCount;
    } QSPTuple;

    typedef struct
    {
        QSP_CHAR *Str;
        QSP_CHAR *End;
    } QSPString;

    typedef struct QSPVariant_s
    {
        union
        {
            QSPString Str;
            QSP_BIGINT Num;
            QSPTuple Tuple;
        } Val;
        QSP_TINYINT Type;
    } QSPVariant;

    typedef struct
    {
        QSPString Name;
        QSPString Image;
    } QSPListItem;

    typedef struct
    {
        QSPString Name;
        QSPString Title;
        QSPString Image;
    } QSPObjectItem;

    typedef struct
    {
        int ErrorNum;
        QSPString ErrorDesc;
        QSPString LocName; /* location name */
        int ActIndex; /* index of the base action */
        int TopLineNum; /* top-level line within the game code */
        int IntLineNum; /* line number of the actual code */
        QSPString IntLine; /* line of the actual code */
    } QSPErrorInfo;

    typedef struct
    {
        int LineNum;
        QSPString Line;
    } QSPLineInfo;

#endif
