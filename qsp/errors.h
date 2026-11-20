/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "codetools.h"
#include "locations.h"

#ifndef QSP_ERRSDEFINES
    #define QSP_ERRSDEFINES

    typedef struct
    {
        QSPLocation *Loc;
        int ActIndex;
        int IntTopLineNum;
        QSPCodeBlock *Code;
        QSPLineOfCode *Line;
    } QSPRunState;

    extern int qspErrorNum;
    extern QSPErrorInfo qspLastError;

    extern QSPLocation *qspRunLoc;
    extern int qspRunActIndex; /* points to the base action */
    extern int qspIntRunTopLineNum; /* top-level line that executes the internal code */
    extern QSPCodeBlock *qspRunCode; /* the executing code */
    extern QSPLineOfCode *qspRunLine; /* the executing line */

    /* External functions */
    int qspGetTopLineNum(void);
    int qspGetIntLineNum(void);
    void qspSetError(int num);
    void qspResetError(QSP_BOOL toInit);
    QSPString qspGetErrorDesc(int errorNum);

    INLINE void qspSaveRunState(QSPRunState *state)
    {
        state->Loc = qspRunLoc; /* transfers ownership */
        state->ActIndex = qspRunActIndex;
        state->IntTopLineNum = qspIntRunTopLineNum;
        state->Code = qspRunCode;
        state->Line = qspRunLine;
        qspRunLoc = 0;
        qspRunActIndex = -1;
        qspIntRunTopLineNum = 0;
        qspRunCode = 0;
        qspRunLine = 0;
    }

    INLINE void qspRestoreRunState(QSPRunState *state)
    {
        qspRunLoc = state->Loc; /* transfers ownership */
        qspRunActIndex = state->ActIndex;
        qspIntRunTopLineNum = state->IntTopLineNum;
        qspRunCode = state->Code;
        qspRunLine = state->Line;
    }

#endif
