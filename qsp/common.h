/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "text.h"

#ifndef QSP_COMMONDEFINES
    #define QSP_COMMONDEFINES

    #define QSP_RANDMASK 0x7FFFFFFF
    #define QSP_RANDMAX QSP_RANDMASK
    #define QSP_NORMAL_SCALE 3.0

    extern QSP_BOOL qspIsDebug;
    extern int qspTimerInterval;

    extern QSPBufString qspCurDesc;
    extern QSPBufString qspCurVars;
    extern QSPString qspCurInput;
    extern QSPString qspViewPath;
    extern QSP_BOOL qspIsMainDescChanged;
    extern QSP_BOOL qspIsVarsDescChanged;
    extern QSP_BOOL qspCurToShowVars;
    extern QSP_BOOL qspCurToShowInput;
    extern QSP_BOOL qspCurToShowView;

    /* External functions */
    void qspInitRuntime(void);
    void qspTerminateRuntime(void);
    void qspPrepareExecution(QSP_BOOL toInit);
    void qspMemClear(QSP_BOOL toInit);
    void qspSetSeed(unsigned int seed);
    int qspUniformRand(int min, int max);
    int qspNormalRand(int min, int max, int mean);

    INLINE int qspToInt(QSP_BIGINT val)
    {
        if (val > INT_MAX)
            return INT_MAX;
        if (val < INT_MIN)
            return INT_MIN;
        return val;
    }

#endif
