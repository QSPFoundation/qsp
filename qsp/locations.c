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
#include "names.h"
#include "statements.h"
#include "text.h"
#include "variables.h"

QSPLocation **qspLocs = 0;
int qspLocsCount = 0;
int qspLocsCapacity = 0;

QSPLocation *qspCurLoc = 0;
int qspLocationState = 0;
int qspFullRefreshCount = 0;
int qspCurLocCallDepth = 0;

static QSPLocSlot *qspLocsSlots = 0; /* hash table by names, QSP_SLOTSCOUNT(qspLocsCapacity) slots */

INLINE QSPLocSlot *qspNewLocsSlots(int capacity);
INLINE QSPLocSlot *qspGetLocSlot(int nameIndex);
INLINE QSPLocSlot *qspGetEmptyLocSlot(int nameIndex);
INLINE void qspRebuildLocs(int newCapacity);
INLINE void qspAddBaseDescAndActions(QSPLocation *loc);
INLINE void qspExecLocation(QSPLocation *loc, QSP_BOOL toChangeDesc);

INLINE QSPLocSlot *qspNewLocsSlots(int capacity)
{
    QSPLocSlot *slots = (QSPLocSlot *)malloc(capacity * sizeof(QSPLocSlot));
    memset(slots, 0, capacity * sizeof(QSPLocSlot));
    return slots;
}

INLINE QSPLocSlot *qspGetLocSlot(int nameIndex)
{
    QSPLocSlot *slot;
    int mask = QSP_SLOTSMASK(qspLocsCapacity), ind = (int)(qspGetIndexHash(nameIndex) & mask);
    while ((slot = qspLocsSlots + ind)->NameIndex && slot->NameIndex != nameIndex)
        ind = (ind + 1) & mask;
    return slot;
}

INLINE QSPLocSlot *qspGetEmptyLocSlot(int nameIndex)
{
    int mask = QSP_SLOTSMASK(qspLocsCapacity), ind = (int)(qspGetIndexHash(nameIndex) & mask);
    while (qspLocsSlots[ind].NameIndex) ind = (ind + 1) & mask;
    return qspLocsSlots + ind;
}

INLINE void qspRebuildLocs(int newCapacity)
{
    QSPLocSlot *slot, *oldSlot, *oldSlots = qspLocsSlots;
    int i, oldSlotsCount = QSP_SLOTSCOUNT(qspLocsCapacity);
    qspLocsCapacity = newCapacity;
    qspLocs = (QSPLocation **)realloc(qspLocs, newCapacity * sizeof(QSPLocation *));
    qspLocsSlots = qspNewLocsSlots(QSP_SLOTSCOUNT(newCapacity));
    /* Keep the slots of the first qspLocsCount locations */
    for (i = 0, oldSlot = oldSlots; i < oldSlotsCount; ++i, ++oldSlot)
    {
        if (oldSlot->NameIndex && oldSlot->Index < qspLocsCount)
        {
            slot = qspGetEmptyLocSlot(oldSlot->NameIndex);
            *slot = *oldSlot;
        }
    }
    free(oldSlots);
}

void qspInitWorld(void)
{
    qspLocsCount = 0;
    qspLocsCapacity = QSP_LOCSCAPACITY;
    qspLocs = (QSPLocation **)malloc(qspLocsCapacity * sizeof(QSPLocation *));
    qspLocsSlots = qspNewLocsSlots(QSP_SLOTSCOUNT(qspLocsCapacity));
}

void qspTerminateWorld(void)
{
    qspTruncateWorld(0);
    free(qspLocs);
    free(qspLocsSlots);
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
    qspRebuildLocs(qspLocsCapacity); /* drops the slots of the removed locations */
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
    QSPLocSlot *slot;
    int nameIndex;
    name = qspDelSpc(name);
    nameIndex = qspGetNameIndex(name, QSP_TRUE);
    if (!nameIndex) return 0; /* empty names aren't allowed */
    slot = qspGetLocSlot(nameIndex);
    if (slot->NameIndex) return 0;
    if (qspLocsCount >= qspLocsCapacity)
    {
        qspRebuildLocs(qspLocsCapacity * 2);
        slot = qspGetEmptyLocSlot(nameIndex);
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
    slot->NameIndex = nameIndex;
    slot->Index = qspLocsCount;
    ++qspLocsCount;
    return loc;
}

int qspGetLocNameIndex(QSPString name, QSP_BOOL toIntern)
{
    name = qspDelSpc(name);
    return qspGetNameIndex(name, toIntern);
}

QSPLocation *qspGetLoc(int nameIndex)
{
    if (nameIndex > 0) /* empty names aren't allowed */
    {
        QSPLocSlot *slot = qspGetLocSlot(nameIndex);
        if (slot->NameIndex) return qspLocs[slot->Index];
    }
    return 0;
}

QSPLocation *qspGetLocByName(QSPString name)
{
    return qspGetLoc(qspGetLocNameIndex(name, QSP_FALSE));
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
        qspExecCode(loc->OnVisitCode, 0, loc->OnVisitCode->LinesCount, QSP_TRUE, 0);
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

void qspExecLocWithArgs(int nameIndex, QSPVariant *args, QSP_TINYINT argsCount, QSP_BOOL toMoveArgs, QSPVariant *res)
{
    int oldLocalScopeBase, oldLocationState;
    QSPLocation *loc = qspGetLoc(nameIndex);
    if (!loc)
    {
        qspSetError(QSP_ERR_LOCNOTFOUND);
        return;
    }
    oldLocalScopeBase = qspAllocateLocalScopeWithArgs(args, argsCount, toMoveArgs);

    oldLocationState = qspLocationState;
    qspExecLocation(loc, QSP_FALSE);
    if (qspLocationState != oldLocationState) return;

    if (res && !qspApplyResult(res)) return;
    qspReleaseLocalScope(oldLocalScopeBase);
}

void qspExecLocByNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount, QSP_BOOL toMoveArgs, QSPVariant *res)
{
    int nameIndex = qspGetLocNameIndex(name, QSP_FALSE);
    qspExecLocWithArgs(nameIndex, args, argsCount, toMoveArgs, res);
}

void qspExecLocByVarNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount)
{
    QSPVarData *varData;
    QSPString locName;
    int oldLocalVarsBase, ind, oldLocationState;
    /* Hide local variables */
    oldLocalVarsBase = qspHideLocalVars();
    /* We execute all locations specified in the array */
    oldLocationState = qspLocationState;
    ind = 0;
    while (1)
    {
        /* The variable might be updated during the previous code execution */
        if (!((varData = qspVarDataByName(name, QSP_REF_GET, 0))))
        {
            qspClearHiddenLocalVars(oldLocalVarsBase);
            return;
        }
        if (ind >= varData->ValsCount) break;
        if (!QSP_ISSTR(varData->Values[ind].Type)) break;
        locName = QSP_STR(varData->Values[ind]);
        if (!qspIsAnyString(locName)) break;
        qspExecLocByNameWithArgs(locName, args, argsCount, QSP_FALSE, 0);
        if (qspLocationState != oldLocationState)
        {
            qspClearHiddenLocalVars(oldLocalVarsBase);
            return;
        }
        ++ind;
    }
    qspRestoreHiddenLocalVars(oldLocalVarsBase);
}

void qspNavigateToLocation(QSPLocation *loc, QSP_BOOL toChangeDesc, QSPVariant *args, QSP_TINYINT argsCount)
{
    int oldLocalScopeBase, oldLocationState;
    qspUpdateLocation(&qspCurLoc, loc);
    /* Restore global variables */
    qspClearLocalVars();
    /* We assign global ARGS here */
    if (!qspSetArgs(args, argsCount, QSP_FALSE)) return;

    qspClearAllActions(QSP_FALSE);
    ++qspLocationState;
    if (toChangeDesc) ++qspFullRefreshCount;
    oldLocalScopeBase = qspAllocateLocalScope();

    oldLocationState = qspLocationState;
    qspExecLocation(loc, toChangeDesc);
    if (qspLocationState != oldLocationState) return;

    qspReleaseLocalScope(oldLocalScopeBase);
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_NEWLOC), args, argsCount);
}
