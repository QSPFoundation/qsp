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

#include "common.h"
#include "actions.h"
#include "callbacks.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "mathops.h"
#include "objects.h"
#include "playlist.h"
#include "regexp.h"
#include "statements.h"
#include "text.h"
#include "time.h"
#include "variables.h"

static unsigned int qspRandX[55], qspRandY[256], qspRandZ;
static int qspRandI, qspRandJ;
QSP_BOOL qspIsDebug = QSP_FALSE;
QSPBufString qspCurDesc;
QSPBufString qspCurVars;
QSPString qspCurInput;
QSPString qspViewPath;
int qspTimerInterval = 0;
QSP_BOOL qspIsMainDescChanged = QSP_FALSE;
QSP_BOOL qspIsVarsDescChanged = QSP_FALSE;
QSP_BOOL qspCurToShowVars = QSP_TRUE;
QSP_BOOL qspCurToShowInput = QSP_TRUE;

INLINE unsigned int qspURand(void);

void qspInitRuntime(void)
{
    qspNullString = qspStringFromPair(0, 0);
    qspNullTuple = qspCopyToNewTuple(0, 0);

    qspIsDebug = QSP_FALSE;
    qspLocationState = 0;
    qspFullRefreshCount = 0;
    qspQstCRC = 0;
    qspMSCount = 0;
    qspLocs = 0;
    qspLocsNames = 0;
    qspLocsCount = 0;
    qspCurLoc = -1;
    qspTimerInterval = 0;
    qspCurToShowObjs = qspCurToShowActs = qspCurToShowVars = qspCurToShowInput = QSP_TRUE;

    setlocale(LC_ALL, QSP_LOCALE);
    qspSetSeed(0);
    qspInitVarTypes();
    qspInitSymbolClasses();
    qspPrepareExecution(QSP_TRUE);
    qspMemClear(QSP_TRUE);
    qspInitCallBacks();
    qspInitStats();
    qspInitMath();
}

void qspTerminateRuntime(void)
{
    qspMemClear(QSP_FALSE);
    qspCreateWorld(0, 0);
    qspTerminateMath();
    qspResetError(QSP_FALSE);
}

void qspPrepareExecution(QSP_BOOL toInit)
{
    qspResetError(toInit);

    /* Reset execution state */
    qspRealCurLoc = -1;
    qspRealActIndex = -1;
    qspRealLineNum = 0;
    qspRealLine = 0;

    /* Reset state of changes */
    qspIsMainDescChanged = qspIsVarsDescChanged = qspIsObjsListChanged = qspIsActsListChanged = QSP_FALSE;
}

void qspMemClear(QSP_BOOL toInit)
{
    qspClearAllIncludes(toInit);
    qspClearAllVars(toInit);
    qspClearAllObjects(toInit);
    qspClearAllActions(toInit);
    qspClearPlayList(toInit);
    qspClearAllRegExps(toInit);
    if (!toInit)
    {
        int i;
        if (qspCurDesc.Len > 0)
        {
            qspFreeBufString(&qspCurDesc);
            qspIsMainDescChanged = QSP_TRUE;
        }
        if (qspCurVars.Len > 0)
        {
            qspFreeBufString(&qspCurVars);
            qspIsVarsDescChanged = QSP_TRUE;
        }

        qspFreeString(&qspCurInput);
        qspFreeString(&qspViewPath);

        for (i = qspSavedVarGroupsCount - 1; i >= 0; --i)
            qspClearVars(qspSavedVarGroups[i].Vars, qspSavedVarGroups[i].VarsCount);
        if (qspSavedVarGroups) free(qspSavedVarGroups);
    }
    qspCurDesc = qspNewBufString(512);
    qspCurVars = qspNewBufString(512);
    qspCurInput = qspNullString;
    qspViewPath = qspNullString;
    qspSavedVarGroups = 0;
    qspSavedVarGroupsCount = 0;
    qspSavedVarGroupsBufSize = 0;
}

void qspSetSeed(unsigned int seed)
{
    int i;
    qspRandX[0] = 1;
    qspRandX[1] = seed;
    for (i = 2; i < 55; ++i)
        qspRandX[i] = qspRandX[i - 1] + qspRandX[i - 2];
    qspRandI = 23;
    qspRandJ = 54;
    for (i = 255; i >= 0; --i) qspURand();
    for (i = 255; i >= 0; --i) qspRandY[i] = qspURand();
    qspRandZ = qspURand();
}

INLINE unsigned int qspURand(void)
{
    if (--qspRandI < 0) qspRandI = 54;
    if (--qspRandJ < 0) qspRandJ = 54;
    return qspRandX[qspRandJ] += qspRandX[qspRandI];
}

int qspRand(void)
{
    int i = qspRandZ >> 24;
    qspRandZ = qspRandY[i];
    if (--qspRandI < 0) qspRandI = 54;
    if (--qspRandJ < 0) qspRandJ = 54;
    qspRandY[i] = qspRandX[qspRandJ] += qspRandX[qspRandI];
    return qspRandZ & QSP_RANDMASK;
}
