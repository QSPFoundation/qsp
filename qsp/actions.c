/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "actions.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "statements.h"
#include "text.h"

QSPCurAct qspCurActions[QSP_MAXACTIONS];
int qspCurActsCount = 0;
int qspCurSelAction = -1;

INLINE int qspActIndex(QSPString name);

void qspClearAllActions(QSP_BOOL toInit)
{
    if (!toInit && qspCurActsCount)
    {
        int i;
        QSPCurAct *curAct = qspCurActions;
        for (i = qspCurActsCount; i > 0; --i, ++curAct)
        {
            qspFreeString(&curAct->Image);
            qspFreeString(&curAct->Desc);
            if (curAct->OnPressCode) qspReleaseCodeBlock(curAct->OnPressCode);
            if (curAct->Location) qspReleaseLocation(curAct->Location);
        }
        qspCurWindowsChangedState |= QSP_WIN_ACTS;
    }
    qspCurActsCount = 0;
    qspCurSelAction = -1;
}

INLINE int qspActIndex(QSPString name)
{
    if (qspCurActsCount)
    {
        int i;
        QSPBufString buf;
        QSPString bufName;
        name = qspCopyToNewText(name);
        qspUpperStr(&name);
        buf = qspNewBufString(0, 64);
        for (i = 0; i < qspCurActsCount; ++i)
        {
            qspUpdateBufString(&buf, qspCurActions[i].Desc);
            bufName = qspBufStringToString(buf);
            qspUpperStr(&bufName);
            if (qspStrsEqual(bufName, name))
            {
                qspFreeString(&name);
                qspFreeBufString(&buf);
                return i;
            }
        }
        qspFreeString(&name);
        qspFreeBufString(&buf);
    }
    return -1;
}

void qspAddAction(QSPString name, QSPString imgPath, QSPCodeBlock *code, int start, int end)
{
    QSPCurAct *act;
    if (qspActIndex(name) >= 0) return;
    if (qspCurActsCount == QSP_MAXACTIONS)
    {
        qspSetError(QSP_ERR_CANTADDACTION);
        return;
    }
    act = qspCurActions + qspCurActsCount++;
    act->Image = (qspIsAnyString(imgPath) ? qspCopyToNewText(imgPath) : qspNullString);
    act->Desc = qspCopyToNewText(name);
    act->OnPressCode = code;
    act->OnPressStartLine = start;
    act->OnPressLinesCount = end - start;
    act->Location = qspRealCurLoc;
    act->ActIndex = qspRealActIndex;
    if (act->OnPressCode) qspAcquireCodeBlock(act->OnPressCode);
    if (act->Location) qspAcquireLocation(act->Location);
    qspCurWindowsChangedState |= QSP_WIN_ACTS;
}

void qspExecAction(int ind)
{
    if (ind >= 0 && ind < qspCurActsCount)
    {
        /* Keep the current location context here (don't reset special vars) */
        QSPLineOfCode *oldLine;
        QSPCurAct *act = qspCurActions + ind;
        QSPLocation *loc = act->Location;
        QSPCodeBlock *code = act->OnPressCode;
        /* Switch the current state */
        qspUpdateLocation(&qspRealCurLoc, loc);
        qspRealActIndex = act->ActIndex;
        if (code)
        {
            /* The code can release the action */
            if (loc) qspAcquireLocation(loc);
            qspAcquireCodeBlock(code);
            oldLine = qspRealLine;
            qspExecCodeBlockWithLocals(code, act->OnPressStartLine, act->OnPressStartLine + act->OnPressLinesCount, 1, 0);
            qspRealLine = oldLine;
            qspReleaseCodeBlock(code);
            if (loc) qspReleaseLocation(loc);
        }
    }
}

QSPString qspGetAllActionsAsCode(void)
{
    int count, i;
    QSPCurAct *curAct;
    QSPString temp;
    QSPBufString res = qspNewBufString(0, 256);
    curAct = qspCurActions;
    for (i = qspCurActsCount; i > 0; --i, ++curAct)
    {
        qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT("ACT ") QSP_DEFQUOT));
        temp = qspReplaceText(curAct->Desc, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
        qspAddBufText(&res, temp);
        qspFreeNewString(&temp, &curAct->Desc);
        if (curAct->Image.Str)
        {
            qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(",") QSP_DEFQUOT));
            temp = qspReplaceText(curAct->Image, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
            qspAddBufText(&res, temp);
            qspFreeNewString(&temp, &curAct->Image);
        }
        qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(":")));
        count = curAct->OnPressLinesCount;
        if (count == 1 && qspIsAnyString(curAct->OnPressCode->Lines[curAct->OnPressStartLine].Str))
            qspAddBufText(&res, curAct->OnPressCode->Lines[curAct->OnPressStartLine].Str);
        else
        {
            if (count >= 2)
            {
                qspAddBufText(&res, QSP_STATIC_STR(QSP_STRSDELIM));
                temp = qspJoinPrepLines(curAct->OnPressCode->Lines + curAct->OnPressStartLine, count, QSP_STATIC_STR(QSP_STRSDELIM));
                qspAddBufText(&res, temp);
                qspFreeString(&temp);
            }
            qspAddBufText(&res, QSP_STATIC_STR(QSP_STRSDELIM QSP_FMT("END")));
        }
        qspAddBufText(&res, QSP_STATIC_STR(QSP_STRSDELIM));
    }
    return qspBufStringToString(res);
}

void qspStatementSinglelineAddAct(QSPLineOfCode *line, int statPos, int endPos)
{
    QSPVariant args[2];
    QSP_TINYINT argsCount;
    QSPCodeBlock *code;
    int oldLocationState;
    QSPCachedStat *stat = line->Stats + statPos;
    if (!qspIsCharAtPos(line->Str, line->Str.Str + stat->EndPos, QSP_COLONDELIM_CHAR))
    {
        qspSetError(QSP_ERR_COLONNOTFOUND);
        return;
    }
    if (statPos == endPos - 1)
    {
        qspSetError(QSP_ERR_CODENOTFOUND);
        return;
    }
    oldLocationState = qspLocationState;
    argsCount = qspGetStatArgs(stat, stat->Data.Act->Args, args);
    if (qspLocationState != oldLocationState) return;
    code = qspGetSinglelineActCode(line, statPos, endPos);
    if (argsCount == 2)
        qspAddAction(QSP_STR(args[0]), QSP_STR(args[1]), code, 0, 1);
    else
        qspAddAction(QSP_STR(args[0]), qspNullString, code, 0, 1);
    qspFreeVariants(args, argsCount);
}

void qspStatementMultilineAddAct(QSPCodeBlock *code, int lineInd, int endLine)
{
    QSPVariant args[2];
    QSP_TINYINT argsCount;
    int oldLocationState = qspLocationState;
    QSPLineOfCode *line = code->Lines + lineInd;
    argsCount = qspGetStatArgs(line->Stats, line->Stats->Data.Act->Args, args);
    if (qspLocationState != oldLocationState) return;
    if (argsCount == 2)
        qspAddAction(QSP_STR(args[0]), QSP_STR(args[1]), code, lineInd + 1, endLine);
    else
        qspAddAction(QSP_STR(args[0]), qspNullString, code, lineInd + 1, endLine);
    qspFreeVariants(args, argsCount);
}

void qspStatementDelAct(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    int actInd = qspActIndex(QSP_STR(args[0]));
    if (actInd < 0) return;
    if (qspCurSelAction >= actInd) qspCurSelAction = -1;
    qspFreeString(&qspCurActions[actInd].Image);
    qspFreeString(&qspCurActions[actInd].Desc);
    if (qspCurActions[actInd].OnPressCode) qspReleaseCodeBlock(qspCurActions[actInd].OnPressCode);
    if (qspCurActions[actInd].Location) qspReleaseLocation(qspCurActions[actInd].Location);
    --qspCurActsCount;
    memmove(qspCurActions + actInd, qspCurActions + actInd + 1, (qspCurActsCount - actInd) * sizeof(QSPCurAct));
    qspCurWindowsChangedState |= QSP_WIN_ACTS;
}
