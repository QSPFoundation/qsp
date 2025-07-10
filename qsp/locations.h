/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "actions.h"
#include "codetools.h"
#include "variant.h"

#ifndef QSP_LOCSDEFINES
    #define QSP_LOCSDEFINES

    typedef struct
    {
        QSPString Image;
        QSPString Desc;
        QSPLineOfCode *OnPressLines;
        int OnPressLinesCount;
    } QSPLocAct;
    typedef struct
    {
        QSPString Name;
        QSPString Desc;
        QSPLineOfCode *OnVisitLines;
        int OnVisitLinesCount;
        QSPLocAct Actions[QSP_MAXACTIONS];
        int ActionsCount; /* number of base actions, some of them can be empty */
    } QSPLocation;
    typedef struct
    {
        int Index;
        QSPString Name;
    } QSPLocName;

    extern QSPLocation *qspLocs;
    extern QSPLocName *qspLocsNames;
    extern int qspLocsCount;
    extern int qspCurLoc;
    extern int qspLocationState; /* allows to check if we have to terminate execution of the code */
    extern int qspFullRefreshCount;

    /* External functions */
    void qspCreateWorld(int start, int newLocsCount);
    void qspPrepareLocs(void);
    int qspLocIndex(QSPString name);
    void qspExecLocByNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount, QSP_BOOL toMoveArgs, QSPVariant *res);
    void qspExecLocByVarNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount);
    void qspNavigateToLocation(int locInd, QSP_BOOL toChangeDesc, QSPVariant *args, QSP_TINYINT argsCount);

#endif
