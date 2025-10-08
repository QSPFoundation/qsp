/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "locations.h"
#include "callbacks.h"
#include "codetools.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "statements.h"
#include "text.h"
#include "variables.h"

QSPLocation *qspLocs = 0;
int qspLocsCount = 0;
QSPLocName *qspLocsNames = 0;
int qspLocsNamesCount = 0;
int qspCurLoc = -1;
int qspLocationState = 0;
int qspFullRefreshCount = 0;

INLINE int qspLocNamesCompare(const void *locName1, const void *locName2);
INLINE int qspLocNameCompare(const void *name, const void *compareTo);
INLINE void qspExecLocByIndex(int locInd, QSP_BOOL toChangeDesc);

INLINE int qspLocNamesCompare(const void *locName1, const void *locName2)
{
    return qspStrsCompare(((QSPLocName *)locName1)->Name, ((QSPLocName *)locName2)->Name);
}

INLINE int qspLocNameCompare(const void *name, const void *compareTo)
{
    return qspStrsCompare(*(QSPString *)name, ((QSPLocName *)compareTo)->Name);
}

void qspResizeWorld(int newLocsCount)
{
    int oldLocsCount = qspLocsCount;
    /* Clear existing locations only if the new world is smaller than the current one */
    if (qspLocs && newLocsCount < oldLocsCount)
    {
        int i, j;
        QSPLocation *curLoc;
        QSPLocAct *curAct;
        curLoc = qspLocs + newLocsCount;
        for (i = newLocsCount; i < oldLocsCount; ++i, ++curLoc)
        {
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
    }
    /* Adjust the location buffer */
    if (oldLocsCount != newLocsCount)
    {
        qspLocsCount = newLocsCount;
        if (newLocsCount)
            qspLocs = (QSPLocation *)realloc(qspLocs, newLocsCount * sizeof(QSPLocation));
        else
        {
            free(qspLocs);
            qspLocs = 0;
        }
    }
    /* Init new locations only if the new world is bigger than the current one */
    if (qspLocs && newLocsCount > oldLocsCount)
    {
        int i;
        QSPLocation *curLoc;
        curLoc = qspLocs + oldLocsCount;
        for (i = oldLocsCount; i < newLocsCount; ++i, ++curLoc)
        {
            curLoc->Name = qspNullString;
            curLoc->Desc = qspNullString;
            curLoc->OnVisitLines = 0;
            curLoc->OnVisitLinesCount = 0;
            curLoc->Actions = 0;
            curLoc->ActionsCount = 0;
        }
    }
}

void qspUpdateLocsNames(void)
{
    /* Clear old location index */
    if (qspLocsNames)
    {
        int i;
        QSPLocName *curLocName = qspLocsNames;
        for (i = 0; i < qspLocsNamesCount; ++i, ++curLocName)
            qspFreeString(&curLocName->Name);
    }
    /* Adjust the size of the location index */
    if (qspLocsNamesCount != qspLocsCount)
    {
        qspLocsNamesCount = qspLocsCount;
        if (qspLocsNamesCount)
            qspLocsNames = (QSPLocName *)realloc(qspLocsNames, qspLocsNamesCount * sizeof(QSPLocName));
        else
        {
            free(qspLocsNames);
            qspLocsNames = 0;
        }
    }
    /* Init new location index */
    if (qspLocsNames)
    {
        int i;
        QSPLocation *curLoc = qspLocs;
        QSPLocName *curLocName = qspLocsNames;
        for (i = 0; i < qspLocsCount; ++i, ++curLoc, ++curLocName)
        {
            curLocName->Index = i;
            curLocName->Name = qspCopyToNewText(curLoc->Name);
            qspUpperStr(&curLocName->Name);
        }
        qsort(qspLocsNames, qspLocsNamesCount, sizeof(QSPLocName), qspLocNamesCompare);
    }
}

int qspLocIndex(QSPString name)
{
    if (qspLocsNamesCount)
    {
        name = qspDelSpc(name);
        if (!qspIsEmpty(name))
        {
            QSPLocName *loc;
            name = qspCopyToNewText(name);
            qspUpperStr(&name);
            loc = (QSPLocName *)bsearch(&name, qspLocsNames, qspLocsNamesCount, sizeof(QSPLocName), qspLocNameCompare);
            qspFreeString(&name);
            if (loc) return loc->Index;
        }
    }
    return -1;
}

INLINE void qspExecLocByIndex(int locInd, QSP_BOOL toChangeDesc)
{
    QSPString actionName;
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
    if (toChangeDesc && qspCurDesc.Len > 0)
    {
        qspClearBufString(&qspCurDesc);
        qspCurWindowsChangedState |= QSP_WIN_MAIN;
    }
    if (qspIsDebug)
    {
        /* Trigger debugger only if we have base description or base actions */
        if (!qspIsEmpty(loc->Desc) || loc->ActionsCount)
        {
            qspCallDebug(qspNullString);
            if (qspLocationState != oldLocationState) return;
        }
    }
    /* Update base description */
    if (!qspIsEmpty(loc->Desc))
    {
        QSPString locDesc = qspFormatText(loc->Desc, QSP_TRUE);
        if (qspLocationState != oldLocationState) return;
        if (qspAddBufText(&qspCurDesc, locDesc))
            qspCurWindowsChangedState |= QSP_WIN_MAIN;
        qspFreeNewString(&locDesc, &loc->Desc);
    }
    /* Update base actions */
    for (i = 0, curAct = loc->Actions; i < loc->ActionsCount; ++i, ++curAct)
    {
        if (qspIsEmpty(curAct->Desc)) break;
        qspRealActIndex = i;
        actionName = qspFormatText(curAct->Desc, QSP_TRUE);
        if (qspLocationState != oldLocationState) return;
        if (!qspIsEmpty(curAct->Image))
            qspAddAction(actionName, curAct->Image, curAct->OnPressLines, 0, curAct->OnPressLinesCount);
        else
            qspAddAction(actionName, qspNullString, curAct->OnPressLines, 0, curAct->OnPressLinesCount);
        qspFreeNewString(&actionName, &curAct->Desc);
        if (qspLocationState != oldLocationState) return;
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
