/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "codetools.h"
#include "locations.h"
#include "variant.h"

#ifndef QSP_ACTSDEFINES
    #define QSP_ACTSDEFINES

    #define QSP_MAXACTIONS 50

    typedef struct
    {
        QSPString Desc;
        QSPString Image;
        QSPCodeBlock *OnPressCode;
        int OnPressStartLine;
        int OnPressLinesCount;
        QSPLocation *Location;
        int ActIndex;
    } QSPCurAct;

    extern QSPCurAct qspCurActions[QSP_MAXACTIONS];
    extern int qspCurActsCount;
    extern int qspCurSelAction;

    /* External functions */
    void qspClearAllActions(QSP_BOOL toInit);
    void qspAddAction(QSPString name, QSPString imgPath, QSPCodeBlock *code, int start, int end);
    void qspExecAction(int ind);
    QSPString qspGetAllActionsAsCode(void);
    /* Statements */
    void qspStatementSinglelineAddAct(QSPLineOfCode *line, int statPos, int endPos);
    void qspStatementMultilineAddAct(QSPCodeBlock *code, int lineInd, int endLine);
    void qspStatementDelAct(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

#endif
