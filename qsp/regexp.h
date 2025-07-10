/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
