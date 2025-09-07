/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "locations.h"
#include "codetools.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "statements.h"
#include "text.h"
#include "variables.h"

QSPLocation *qspLocs = 0;
QSPLocName *qspLocsNames = 0;
int qspLocsCount = 0;
int qspCurLoc = -1;
int qspLocationState = 0;
int qspFullRefreshCount = 0;

INLINE int qspLocsCompare(const void *locName1, const void *locName2);
INLINE int qspLocNameCompare(const void *name, const void *compareTo);
INLINE void qspExecLocByIndex(int locInd, QSP_BOOL toChangeDesc);

INLINE int qspLocsCompare(const void *locName1, const void *locName2)
{
    return qspStrsCompare(((QSPLocName *)locName1)->Name, ((QSPLocName *)locName2)->Name);
}

INLINE int qspLocNameCompare(const void *name, const void *compareTo)
{
    return qspStrsCompare(*(QSPString *)name, ((QSPLocName *)compareTo)->Name);
}

void qspCreateWorld(int start, int newLocsCount)
{
    int i, j;
    QSPLocation *curLoc;
    QSPLocAct *curAct;
    QSPLocName *curLocName;
    /* Release overlapping locations */
    curLoc = qspLocs + start;
    curLocName = qspLocsNames + start;
    for (i = start; i < qspLocsCount; ++i, ++curLoc, ++curLocName)
    {
        qspFreeString(&curLocName->Name);
        qspFreeString(&curLoc->Name);
        qspFreeString(&curLoc->Desc);
        qspFreePrepLines(curLoc->OnVisitLines, curLoc->OnVisitLinesCount);
        if (curLoc->Actions)
        {
            for (j = 0, curAct = curLoc->Actions; j < curLoc->ActionsCount; ++j, ++curAct)
            {
                qspFreeString(&curAct->Image);
                qspFreeString(&curAct->Desc);
                qspFreePrepLines(curAct->OnPressLines, curAct->OnPressLinesCount);
            }
            free(curLoc->Actions);
        }
    }
    /* Reallocate resources, we can either increase or decrease the number of locations here */
    if (qspLocsCount != newLocsCount)
    {
        qspLocsCount = newLocsCount;
        qspLocs = (QSPLocation *)realloc(qspLocs, qspLocsCount * sizeof(QSPLocation));
        qspLocsNames = (QSPLocName *)realloc(qspLocsNames, qspLocsCount * sizeof(QSPLocName));
    }
    /* Init locations */
    curLoc = qspLocs + start;
    curLocName = qspLocsNames + start;
    for (i = start; i < qspLocsCount; ++i, ++curLoc, ++curLocName)
    {
        curLocName->Name = qspNullString;
        curLoc->Name = qspNullString;
        curLoc->Desc = qspNullString;
        curLoc->OnVisitLines = 0;
        curLoc->OnVisitLinesCount = 0;
        curLoc->Actions = 0;
        curLoc->ActionsCount = 0;
    }
}

void qspPrepareLocs(void)
{
    int i;
    QSPLocation *curLoc = qspLocs;
    QSPLocName *curLocName = qspLocsNames;
    for (i = 0; i < qspLocsCount; ++i, ++curLoc, ++curLocName)
    {
        curLocName->Index = i;
        qspUpdateText(&curLocName->Name, curLoc->Name);
        qspUpperStr(&curLocName->Name);
    }
    qsort(qspLocsNames, qspLocsCount, sizeof(QSPLocName), qspLocsCompare);
}

int qspLocIndex(QSPString name)
{
    if (qspLocsCount)
    {
        name = qspDelSpc(name);
        if (!qspIsEmpty(name))
        {
            QSPLocName *loc;
            name = qspCopyToNewText(name);
            qspUpperStr(&name);
            loc = (QSPLocName *)bsearch(&name, qspLocsNames, qspLocsCount, sizeof(QSPLocName), qspLocNameCompare);
            qspFreeString(&name);
            if (loc) return loc->Index;
        }
    }
    return -1;
}

INLINE void qspExecLocByIndex(int locInd, QSP_BOOL toChangeDesc)
{
    QSPString locDesc, actionName;
    QSPLineOfCode *oldLine;
    QSPLocAct *curAct;
    int i, oldLoc, oldActIndex, oldLineNum, oldLocationState = qspLocationState;
    QSPLocation *loc = qspLocs + locInd;
    /* Remember the previous state to restore it after internal calls */
    oldLoc = qspRealCurLoc;
    oldActIndex = qspRealActIndex;
    oldLineNum = qspRealLineNum;
    oldLine = qspRealLine;
    /* Switch the current state */
    qspRealCurLoc = locInd;
    qspRealActIndex = -1;
    qspRealLineNum = 0;
    qspRealLine = 0;
    /* Update base description */
    locDesc = qspFormatText(loc->Desc, QSP_FALSE);
    if (qspLocationState != oldLocationState)
    {
        qspRealLine = oldLine;
        qspRealLineNum = oldLineNum;
        qspRealActIndex = oldActIndex;
        qspRealCurLoc = oldLoc;
        return;
    }
    if (toChangeDesc)
    {
        qspFreeBufString(&qspCurDesc);
        qspCurDesc = qspStringToBufString(locDesc, 512);
        qspCurWindowsChangedState |= QSP_WIN_MAIN;
    }
    else
    {
        if (qspAddBufText(&qspCurDesc, locDesc))
            qspCurWindowsChangedState |= QSP_WIN_MAIN;
        qspFreeString(&locDesc);
    }
    /* Update base actions */
    for (i = 0, curAct = loc->Actions; i < loc->ActionsCount; ++i, ++curAct)
    {
        if (qspIsEmpty(curAct->Desc)) break;
        actionName = qspFormatText(curAct->Desc, QSP_FALSE);
        if (qspLocationState != oldLocationState)
        {
            qspRealLine = oldLine;
            qspRealLineNum = oldLineNum;
            qspRealActIndex = oldActIndex;
            qspRealCurLoc = oldLoc;
            return;
        }
        qspRealActIndex = i;
        if (!qspIsEmpty(curAct->Image))
            qspAddAction(actionName, curAct->Image, curAct->OnPressLines, 0, curAct->OnPressLinesCount);
        else
            qspAddAction(actionName, qspNullString, curAct->OnPressLines, 0, curAct->OnPressLinesCount);
        qspFreeString(&actionName);
        if (qspLocationState != oldLocationState)
        {
            qspRealLine = oldLine;
            qspRealLineNum = oldLineNum;
            qspRealActIndex = oldActIndex;
            qspRealCurLoc = oldLoc;
            return;
        }
    }
    /* Execute the code */
    qspRealActIndex = -1;
    if (locInd < qspLocsCount - qspCurIncLocsCount)
        qspExecCode(loc->OnVisitLines, 0, loc->OnVisitLinesCount, 1, 0);
    else
    {
        /* Copy code for locations defined outside the base game file */
        QSPLineOfCode *code;
        int count = loc->OnVisitLinesCount;
        qspCopyPrepLines(&code, loc->OnVisitLines, 0, count);
        qspExecCode(code, 0, count, 1, 0);
        qspFreePrepLines(code, count);
    }
    /* Restore the old state */
    qspRealLine = oldLine;
    qspRealLineNum = oldLineNum;
    qspRealActIndex = oldActIndex;
    qspRealCurLoc = oldLoc;
}

void qspExecLocByNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount, QSP_BOOL toMoveArgs, QSPVariant *res)
{
    int oldLocationState, locInd = qspLocIndex(name);
    if (locInd < 0)
    {
        qspSetError(QSP_ERR_LOCNOTFOUND);
        return;
    }
    qspAllocateLocalScopeWithArgs(args, argsCount, toMoveArgs);

    oldLocationState = qspLocationState;
    qspExecLocByIndex(locInd, QSP_FALSE);
    if (qspLocationState != oldLocationState) return;

    if (res && !qspApplyResult(res)) return;
    qspRemoveLastLocalScope();
}

void qspExecLocByVarNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount)
{
    QSPVar *var;
    QSPString locName;
    QSPVarsScopeChunk *savedLocalVars;
    int ind, oldLocationState;
    /* Restore global variables */
    savedLocalVars = qspSaveLocalVarsAndRestoreGlobals();
    /* We execute all locations specified in the array */
    oldLocationState = qspLocationState;
    ind = 0;
    while (1)
    {
        /* The variable might be updated during the previous code execution */
        if (!((var = qspVarReference(name, QSP_FALSE))))
        {
            qspClearLocalVarsScopes(savedLocalVars);
            return;
        }
        if (ind >= var->ValsCount) break;
        if (!QSP_ISSTR(var->Values[ind].Type)) break;
        locName = QSP_STR(var->Values[ind]);
        if (!qspIsAnyString(locName)) break;
        qspExecLocByNameWithArgs(locName, args, argsCount, QSP_FALSE, 0);
        if (qspLocationState != oldLocationState)
        {
            qspClearLocalVarsScopes(savedLocalVars);
            return;
        }
        ++ind;
    }
    /* Restore the local scope */
    qspRestoreSavedLocalVars(savedLocalVars);
}

void qspNavigateToLocation(int locInd, QSP_BOOL toChangeDesc, QSPVariant *args, QSP_TINYINT argsCount)
{
    int oldLocationState;
    if (locInd < 0 || locInd >= qspLocsCount) return;
    qspCurLoc = locInd;
    /* Restore global variables */
    qspClearLocalVarsScopes(qspCurrentLocalVars);
    qspCurrentLocalVars = 0;
    /* We assign global ARGS here */
    if (!qspSetArgs(args, argsCount, QSP_FALSE)) return;

    qspClearAllActions(QSP_FALSE);
    ++qspLocationState;
    if (toChangeDesc) ++qspFullRefreshCount;
    qspAllocateLocalScope();

    oldLocationState = qspLocationState;
    qspExecLocByIndex(locInd, toChangeDesc);
    if (qspLocationState != oldLocationState) return;

    qspRemoveLastLocalScope();
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_NEWLOC), args, argsCount);
}
