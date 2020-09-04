/* Copyright (C) 2001-2020 Valeriy Argunov (val AT time DOT guru) */
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "declarations.h"

#ifndef QSP_TEXTDEFINES
    #define QSP_TEXTDEFINES

    #define QSP_STRSDELIM QSP_FMT("\r\n")
    #define QSP_SPACES QSP_FMT(" \t")
    #define QSP_QUOTS QSP_FMT("'\"")
    #define QSP_LQUOT QSP_FMT("{")
    #define QSP_RQUOT QSP_FMT("}")

    /* Helpers */
    #define QSP_LEN(x) (sizeof(x) / sizeof(QSP_CHAR) - 1)

    /* External functions */
    int qspAddText(QSP_CHAR **, QSP_CHAR *, int, int, QSP_BOOL);
    QSP_BOOL qspIsInList(QSP_CHAR *, QSP_CHAR);
    QSP_CHAR *qspSkipSpaces(QSP_CHAR *);
    QSP_CHAR *qspDelSpc(QSP_CHAR *);
    int qspStrLen(QSP_CHAR *);
    QSP_CHAR *qspStrStr(QSP_CHAR *, QSP_CHAR *);
    QSP_CHAR *qspStrCopy(QSP_CHAR *, QSP_CHAR *);
    QSP_CHAR *qspStrNCopy(QSP_CHAR *, QSP_CHAR *, int);
    QSP_CHAR *qspReplaceText(QSP_CHAR *, QSP_CHAR *, QSP_CHAR *);
    int qspStrsComp(QSP_CHAR *, QSP_CHAR *);
    QSP_BOOL qspIsEqual(QSP_CHAR *, QSP_CHAR *, int);
    int qspSplitStr(QSP_CHAR *, QSP_CHAR *, QSP_CHAR ***);
    void qspFreeStrs(void **, int);
    QSP_BOOL qspIsDigit(QSP_CHAR);
    int qspStrToNum(QSP_CHAR *, QSP_BOOL *);
    QSP_CHAR *qspNumToStr(QSP_CHAR *, int);

#endif
