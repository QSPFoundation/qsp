/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "codetools.h"

#ifndef QSP_ERRSDEFINES
    #define QSP_ERRSDEFINES

    extern int qspErrorNum;
    extern QSPErrorInfo qspLastError;

    extern int qspRealCurLoc;
    extern int qspRealActIndex; /* points to the base action */
    extern int qspRealLineNum; /* points to the top-level line within the game code */
    extern QSPLineOfCode *qspRealLine; /* contains the internal details */

    /* External functions */
    void qspSetError(int num);
    void qspResetError(QSP_BOOL toInit);
    QSPString qspGetErrorDesc(int errorNum);

#endif
