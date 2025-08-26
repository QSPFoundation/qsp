/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "codetools.h"
#include "variant.h"

#ifndef QSP_ACTSDEFINES
    #define QSP_ACTSDEFINES

    #define QSP_MAXACTIONS 50

    typedef struct
    {
        QSPString Desc;
        QSPString Image;
        QSPLineOfCode *OnPressLines;
        int OnPressLinesCount;
        int Location;
        int ActIndex;
    } QSPCurAct;

    extern QSPCurAct qspCurActions[QSP_MAXACTIONS];
    extern int qspCurActsCount;
    extern int qspCurSelAction;
    extern QSP_BOOL qspIsActsListChanged;
    extern QSP_BOOL qspCurToShowActs;

    /* External functions */
    void qspClearAllActions(QSP_BOOL toInit);
    void qspAddAction(QSPString name, QSPString imgPath, QSPLineOfCode *code, int start, int end);
    void qspExecAction(int ind);
    QSPString qspGetAllActionsAsCode(void);
    /* Statements */
    void qspStatementSinglelineAddAct(QSPLineOfCode *line, int statPos, int endPos);
    void qspStatementMultilineAddAct(QSPLineOfCode *s, int lineInd, int endLine);
    void qspStatementDelAct(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

#endif
