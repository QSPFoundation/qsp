/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "locations.h"
#include "actions.h"
#include "callbacks.h"
#include "codetools.h"
#include "common.h"
#include "errors.h"
#include "statements.h"
#include "text.h"
#include "variables.h"

QSPLocation **qspLocs = 0;
int qspLocsCount = 0;
int qspLocsCapacity = 0;
QSPLocName *qspLocsNames = 0;
int qspLocsNamesCapacity = 0;
QSPLocation *qspCurLoc = 0;
int qspLocationState = 0;
int qspFullRefreshCount = 0;
int qspCurLocCallDepth = 0;

INLINE QSPLocName *qspNewLocNames(int capacity);
INLINE QSPLocName *qspGetLocSlot(QSPString name, unsigned int nameHash);
INLINE QSPLocName *qspGetEmptyLocSlot(unsigned int nameHash);
INLINE void qspRebuildLocsNames(int newCapacity, int locsCount);
INLINE void qspAddBaseDescAndActions(QSPLocation *loc);
INLINE void qspExecLocation(QSPLocation *loc, QSP_BOOL toChangeDesc);

INLINE QSPLocName *qspNewLocNames(int capacity)
{
    int i;
    QSPLocName *slots = (QSPLocName *)malloc(capacity * sizeof(QSPLocName));
    for (i = 0; i < capacity; ++i)
        slots[i].Name = qspNullString;
    return slots;
}

INLINE QSPLocName *qspGetLocSlot(QSPString name, unsigned int nameHash)
{
    QSPLocName *slot;
    int mask = QSP_CAPACITYMASK(qspLocsNamesCapacity), ind = (int)(nameHash & mask);
    while ((slot = qspLocsNames + ind)->Name.Str && (slot->NameHash != nameHash || !qspStrsEqual(name, slot->Name)))
        ind = (ind + 1) & mask;
    return slot;
}

INLINE QSPLocName *qspGetEmptyLocSlot(unsigned int nameHash)
{
    QSPLocName *slot;
    int mask = QSP_CAPACITYMASK(qspLocsNamesCapacity), ind = (int)(nameHash & mask);
    while ((slot = qspLocsNames + ind)->Name.Str) ind = (ind + 1) & mask;
    return slot;
}

INLINE void qspRebuildLocsNames(int newCapacity, int locsCount)
{
    QSPLocName *slot, *oldSlot, *oldSlots = qspLocsNames;
    int i, oldCapacity = qspLocsNamesCapacity;
    qspLocsNamesCapacity = newCapacity;
    qspLocsNames = qspNewLocNames(newCapacity);
    /* Keep the names of the first locsCount locations */
    for (i = 0, oldSlot = oldSlots; i < oldCapacity; ++i, ++oldSlot)
    {
        if (oldSlot->Name.Str)
        {
            if (oldSlot->Index < locsCount)
            {
                /* This location is still in the world */
                slot = qspGetEmptyLocSlot(oldSlot->NameHash);
                *slot = *oldSlot;
            }
            else
                qspFreeString(&oldSlot->Name);
        }
    }
    free(oldSlots);
}

void qspInitWorld(void)
{
    qspLocsCount = 0;
    qspLocsCapacity = QSP_LOCSCAPACITY;
    qspLocs = (QSPLocation **)malloc(qspLocsCapacity * sizeof(QSPLocation *));
    qspLocsNamesCapacity = qspLocsCapacity * 2; /* the names table stays at most half full */
    qspLocsNames = qspNewLocNames(qspLocsNamesCapacity);
}

void qspTerminateWorld(void)
{
    qspTruncateWorld(0);
    free(qspLocs);
    free(qspLocsNames);
}

void qspTruncateWorld(int locsCount)
{
    int i;
    for (i = locsCount; i < qspLocsCount; ++i)
    {
        if (qspLocs[i] == qspCurLoc)
            qspUpdateLocation(&qspCurLoc, 0); /* the current location has to be in the world */
        qspReleaseLocation(qspLocs[i]);
    }
    qspLocsCount = locsCount;
    qspRebuildLocsNames(qspLocsNamesCapacity, locsCount); /* drops the names of the removed locations */
}

void qspFreeLocation(QSPLocation *loc)
{
    qspFreeString(&loc->Name);
    qspFreeString(&loc->Desc);
    if (loc->OnVisitCode) qspReleaseCodeBlock(loc->OnVisitCode);
    if (loc->Actions)
    {
        int i;
        QSPLocAct *curAct;
        for (i = 0, curAct = loc->Actions; i < loc->ActionsCount; ++i, ++curAct)
        {
            qspFreeString(&curAct->Image);
            qspFreeString(&curAct->Desc);
            if (curAct->OnPressCode) qspReleaseCodeBlock(curAct->OnPressCode);
        }
        free(loc->Actions);
    }
}

QSPLocation *qspAddLocation(QSPString name)
{
    QSPLocation *loc;
    QSPLocName *slot;
    unsigned int nameHash;
    QSPString upperName = qspCopyToNewText(name);
    qspUpperStr(&upperName);
    nameHash = qspGetTextHash(upperName);
    slot = qspGetLocSlot(upperName, nameHash);
    if (slot->Name.Str)
    {
        qspFreeString(&upperName);
        return 0;
    }
    if (qspLocsCount >= qspLocsCapacity)
    {
        qspLocsCapacity *= 2;
        qspLocs = (QSPLocation **)realloc(qspLocs, qspLocsCapacity * sizeof(QSPLocation *));
        qspRebuildLocsNames(qspLocsCapacity * 2, qspLocsCount); /* the names table stays at most half full */
    }
    /* Create a new location */
    loc = (QSPLocation *)malloc(sizeof(QSPLocation));
    loc->Name = qspCopyToNewText(name);
    loc->Desc = qspNullString;
    loc->OnVisitCode = 0;
    loc->Actions = 0;
    loc->ActionsCount = 0;
    loc->RefsCount = 1;
    qspLocs[qspLocsCount] = loc;
    /* Add the name to the index */
    slot = qspGetEmptyLocSlot(nameHash);
    slot->Name = upperName;
    slot->NameHash = nameHash;
    slot->Index = qspLocsCount;
    ++qspLocsCount;
    return loc;
}

QSPLocation *qspLocByName(QSPString name)
{
    if (qspLocsCount)
    {
        name = qspDelSpc(name);
        if (!qspIsEmpty(name))
        {
            QSPLocName *slot;
            name = qspCopyToNewText(name);
            qspUpperStr(&name);
            slot = qspGetLocSlot(name, qspGetTextHash(name));
            qspFreeString(&name);
            if (slot->Name.Str) return qspLocs[slot->Index];
        }
    }
    return 0;
}

INLINE void qspAddBaseDescAndActions(QSPLocation *loc)
{
    QSPString text;
    QSPLocAct *curAct;
    int i, endLine, oldLocationState = qspLocationState;
    /* Update base description */
    if (!qspIsEmpty(loc->Desc))
    {
        text = qspFormatText(loc->Desc);
        if (qspLocationState != oldLocationState)
        {
            qspFreeString(&text);
            return;
        }
        if (qspAddBufText(&qspCurDesc, text))
            qspCurWindowsChangedState |= QSP_WIN_MAIN;
        qspFreeString(&text);
    }
    /* Update base actions */
    for (i = 0, curAct = loc->Actions; i < loc->ActionsCount; ++i, ++curAct)
    {
        if (qspIsEmpty(curAct->Desc)) break;
        qspRealActIndex = i;
        text = qspFormatText(curAct->Desc);
        if (qspLocationState != oldLocationState)
        {
            qspFreeString(&text);
            return;
        }
        endLine = (curAct->OnPressCode ? curAct->OnPressCode->LinesCount : 0);
        qspAddAction(text, curAct->Image, curAct->OnPressCode, 0, endLine);
        qspFreeString(&text);
        if (qspLocationState != oldLocationState) return;
    }
}

INLINE void qspExecLocation(QSPLocation *loc, QSP_BOOL toChangeDesc)
{
    QSPLineOfCode *oldLine;
    QSPLocation *oldLoc;
    int oldActIndex, oldLineNum, oldLocationState;
    if (qspCurLocCallDepth >= QSP_MAXLOCCALLDEPTH)
    {
        qspSetError(QSP_ERR_LOCCALLDEPTH);
        return;
    }
    ++qspCurLocCallDepth;
    oldLocationState = qspLocationState;
    /* Remember the previous state to restore it after internal calls */
    oldLoc = qspRealCurLoc;
    oldActIndex = qspRealActIndex;
    oldLineNum = qspRealLineNum;
    oldLine = qspRealLine;
    /* Switch the current state */
    qspUpdateLocation(&qspRealCurLoc, loc);
    qspRealActIndex = -1;
    qspRealLineNum = 0;
    qspRealLine = 0;
    if (toChangeDesc && qspCurDesc.Len > 0)
    {
        qspClearBufString(&qspCurDesc);
        qspCurWindowsChangedState |= QSP_WIN_MAIN;
    }
    qspAcquireLocation(loc);
    if (qspIsDebug)
    {
        /* Trigger debugger only if we have base description or base actions */
        if (!qspIsEmpty(loc->Desc) || loc->ActionsCount)
        {
            qspCallDebug(qspNullString);
            if (qspLocationState != oldLocationState)
            {
                qspReleaseLocation(loc);
                return;
            }
        }
    }
    qspAddBaseDescAndActions(loc);
    if (qspLocationState != oldLocationState)
    {
        qspReleaseLocation(loc);
        return;
    }
    /* Execute the code */
    if (loc->OnVisitCode)
    {
        qspRealActIndex = -1;
        qspExecCode(loc->OnVisitCode, 0, loc->OnVisitCode->LinesCount, 1, 0);
    }
    qspRealLine = oldLine; /* the executed lines can be released */
    qspReleaseLocation(loc);
    if (qspLocationState != oldLocationState) return;
    /* Restore the old state */
    qspRealLineNum = oldLineNum;
    qspRealActIndex = oldActIndex;
    qspUpdateLocation(&qspRealCurLoc, oldLoc);
    --qspCurLocCallDepth;
}

void qspExecLocByNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount, QSP_BOOL toMoveArgs, QSPVariant *res)
{
    int oldLocationState;
    QSPLocation *loc = qspLocByName(name);
    if (!loc)
    {
        qspSetError(QSP_ERR_LOCNOTFOUND);
        return;
    }
    qspAllocateLocalScopeWithArgs(args, argsCount, toMoveArgs);

    oldLocationState = qspLocationState;
    qspExecLocation(loc, QSP_FALSE);
    if (qspLocationState != oldLocationState) return;

    if (res && !qspApplyResult(res)) return;
    qspReleaseLastLocalScope();
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

void qspNavigateToLocation(QSPLocation *loc, QSP_BOOL toChangeDesc, QSPVariant *args, QSP_TINYINT argsCount)
{
    int oldLocationState;
    qspUpdateLocation(&qspCurLoc, loc);
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
    qspExecLocation(loc, toChangeDesc);
    if (qspLocationState != oldLocationState) return;

    qspReleaseLastLocalScope();
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_NEWLOC), args, argsCount);
}
