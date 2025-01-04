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

#ifndef QSP_H
    #define QSP_H

    #include "qsp_export.h"

    enum
    {
        QSP_ERR_DIVBYZERO = 10,
        QSP_ERR_TYPEMISMATCH,
        QSP_ERR_STACKOVERFLOW,
        QSP_ERR_TOOMANYITEMS,
        QSP_ERR_CANTLOADFILE,
        QSP_ERR_GAMENOTLOADED,
        QSP_ERR_COLONNOTFOUND,
        QSP_ERR_CANTINCFILE,
        QSP_ERR_CANTADDACTION,
        QSP_ERR_EQNOTFOUND,
        QSP_ERR_LOCNOTFOUND,
        QSP_ERR_ENDNOTFOUND,
        QSP_ERR_LABELNOTFOUND,
        QSP_ERR_INCORRECTNAME,
        QSP_ERR_QUOTNOTFOUND,
        QSP_ERR_BRACKNOTFOUND,
        QSP_ERR_BRACKSNOTFOUND,
        QSP_ERR_SYNTAX,
        QSP_ERR_UNKNOWNACTION,
        QSP_ERR_ARGSCOUNT,
        QSP_ERR_CANTADDOBJECT,
        QSP_ERR_CANTADDMENUITEM,
        QSP_ERR_TOOMANYVARS,
        QSP_ERR_INCORRECTREGEXP,
        QSP_ERR_CODENOTFOUND,
        QSP_ERR_LOOPWHILENOTFOUND
    };

    enum
    {
        QSP_WIN_ACTS,
        QSP_WIN_OBJS,
        QSP_WIN_VARS,
        QSP_WIN_INPUT
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
        QSP_CALL_REFRESHINT, /* void func(QSP_BOOL isForced) */
        QSP_CALL_SETTIMER, /* void func(int msecs) */
        QSP_CALL_SETINPUTSTRTEXT, /* void func(QSPString text) */
        QSP_CALL_SYSTEM, /* void func(QSPString cmd) */
        QSP_CALL_OPENGAME, /* void func(QSPString file, QSP_BOOL isNewGame) */
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
        QSP_TYPE_TUPLE = 0,
        QSP_TYPE_NUM = 1,
        QSP_TYPE_STR = 2,
        QSP_TYPE_CODE = 3,
        QSP_TYPE_VARREF = 4,
        QSP_TYPE_UNDEF = 5, /* not used for values, it has to be a string-based type */
        QSP_TYPE_DEFINED_TYPES /* represents the number of defined values */
    };

    #define QSP_TRUE 1
    #define QSP_FALSE 0

    typedef char QSP_TINYINT;
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

    static const QSP_TINYINT qspBaseTypeTable[QSP_TYPE_DEFINED_TYPES] =
    {
        /* TUPLE */  QSP_TYPE_TUPLE,
        /* NUMBER */ QSP_TYPE_NUM,
        /* STRING */ QSP_TYPE_STR,
        /* CODE */   QSP_TYPE_STR,
        /* VARREF */ QSP_TYPE_STR,
        /* UNDEFINED */ QSP_TYPE_STR
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
        int Items;
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
        QSPString Image;
        QSPString Name;
    } QSPListItem;

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
