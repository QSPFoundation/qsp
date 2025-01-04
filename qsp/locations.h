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
