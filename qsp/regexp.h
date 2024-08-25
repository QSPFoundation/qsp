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

#ifndef QSP_REGEXPDEFINES
    #define QSP_REGEXPDEFINES

    #define QSP_MAXCACHEDREGEXPS 10

    typedef struct
    {
        QSPString Text;
        regex_t *CompiledExp;
    } QSPRegExp;

    /* External functions */
    void qspClearAllRegExps(QSP_BOOL toInit);
    QSPRegExp *qspRegExpGetCompiled(QSPString exp);
    QSP_BOOL qspRegExpStrMatch(QSPRegExp *exp, QSPString str);
    QSP_CHAR *qspRegExpStrSearch(QSPRegExp *exp, QSPString str, QSP_CHAR *startPos, int groupInd, int *foundLen);

#endif
