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

#ifndef QSP_H
    #define QSP_H

    #include "qsp_export.h"

    enum
    {
        QSP_ERR_DIVBYZERO = 100,
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
        QSP_CALL_SHOWWINDOW, /* void func(int type, QSP_BOOL isShow) */
        QSP_CALL_SHOWMENU, /* int func(QSPListItem *items, int count) */
        QSP_CALL_SHOWMSGSTR, /* void func(QSPString text) */
        QSP_CALL_REFRESHINT, /* void func(QSP_BOOL isRedraw) */
        QSP_CALL_SETTIMER, /* void func(int msecs) */
        QSP_CALL_SETINPUTSTRTEXT, /* void func(QSPString text) */
        QSP_CALL_SYSTEM, /* void func(QSPString cmd) */
        QSP_CALL_OPENGAME, /* void func(QSP_BOOL isNewGame) */
        QSP_CALL_OPENGAMESTATUS, /* void func(QSPString file) */
        QSP_CALL_SAVEGAMESTATUS, /* void func(QSPString file) */
        QSP_CALL_SLEEP, /* void func(int msecs) */
        QSP_CALL_GETMSCOUNT, /* int func() */
        QSP_CALL_INPUTBOX, /* void func(QSPString text, QSP_CHAR *buffer, int maxLen) */
        QSP_CALL_VERSION, /* void func(QSPString param, QSP_CHAR *buffer, int maxLen) */
        QSP_CALL_DUMMY
    };

    typedef struct
    {
        QSP_CHAR *Str;
        QSP_CHAR *End;
    } QSPString;

    typedef struct
    {
        QSPString Image;
        QSPString Name;
    } QSPListItem;

    typedef char QSP_TINYINT;
    typedef char QSP_BOOL;

    #define QSP_TRUE 1
    #define QSP_FALSE 0

#endif
