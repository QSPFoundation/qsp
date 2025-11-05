/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "actions.h"

#ifndef QSP_LOCSDEFINES
    #define QSP_LOCSDEFINES

    #define QSP_MAXLOCCALLDEPTH 4000

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
        /* Base description */
        QSPString Desc;
        /* Location code */
        QSPLineOfCode *OnVisitLines;
        int OnVisitLinesCount;
        /* Base actions */
        QSPLocAct *Actions;
        int ActionsCount;
    } QSPLocation;

    typedef struct
    {
        int Index;
        QSPString Name;
    } QSPLocName;

    extern QSPLocation *qspLocs;
    extern int qspLocsCount;
    extern QSPLocName *qspLocsNames;
    extern int qspLocsNamesCount;
    extern int qspCurLoc;
    extern int qspLocationState; /* allows to check if we have to terminate execution of the code */
    extern int qspFullRefreshCount;
    extern int qspCurLocCallDepth;

    /* External functions */
    void qspResizeWorld(int newLocsCount);
    void qspUpdateLocsNames(void);
    int qspLocIndex(QSPString name);
    void qspExecLocByNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount, QSP_BOOL toMoveArgs, QSPVariant *res);
    void qspExecLocByVarNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount);
    void qspNavigateToLocation(int locInd, QSP_BOOL toChangeDesc, QSPVariant *args, QSP_TINYINT argsCount);

#endif
