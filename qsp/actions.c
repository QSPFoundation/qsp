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
        QSPString bufName, upperName = qspUpperText(name);
        buf = qspNewBufString(0, 64);
        for (i = 0; i < qspCurActsCount; ++i)
        {
            qspUpdateBufString(&buf, qspCurActions[i].Desc);
            bufName = qspBufStringToString(buf);
            qspUpperStr(&bufName);
            if (qspStrsEqual(bufName, upperName))
            {
                qspFreeNewString(&upperName, &name);
                qspFreeBufString(&buf);
                return i;
            }
        }
        qspFreeNewString(&upperName, &name);
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
    if (end > start)
    {
        act->OnPressCode = code;
        act->OnPressStartLine = start;
        act->OnPressLinesCount = end - start;
    }
    else
    {
        act->OnPressCode = 0;
        act->OnPressStartLine = 0;
        act->OnPressLinesCount = 0;
    }
    act->Location = qspRunLoc;
    act->ActIndex = qspRunActIndex;
    act->TopLineNum = qspGetTopLineNum();
    if (act->OnPressCode) qspAcquireCodeBlock(act->OnPressCode);
    if (act->Location) qspAcquireLocation(act->Location);
    qspCurWindowsChangedState |= QSP_WIN_ACTS;
}

void qspExecAction(int ind)
{
    if (ind >= 0 && ind < qspCurActsCount)
    {
        /* Keep the current location context here (don't reset special vars) */
        QSPCurAct *act = qspCurActions + ind;
        QSPCodeBlock *code = act->OnPressCode;
        if (code)
        {
            QSPRunState oldState;
            /* The code can release the action */
            qspAcquireCodeBlock(code);
            /* Switch the current state */
            qspSaveRunState(&oldState);
            qspUpdateLocation(&qspRunLoc, act->Location);
            qspRunActIndex = act->ActIndex;
            qspIntRunTopLineNum = act->TopLineNum;
            qspRunCode = code;
            qspExecCodeBlockWithLocals(code, act->OnPressStartLine, act->OnPressStartLine + act->OnPressLinesCount, 0);
            qspUpdateLocation(&qspRunLoc, 0);
            qspRestoreRunState(&oldState);
            qspReleaseCodeBlock(code);
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
        qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT("ACT ")));
        qspAddBufQuotedText(&res, curAct->Desc, QSP_STATIC_STR(QSP_DEFQUOT));
        if (curAct->Image.Str)
        {
            qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT(",")));
            qspAddBufQuotedText(&res, curAct->Image, QSP_STATIC_STR(QSP_DEFQUOT));
        }
        qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT(":")));
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
    QSPCachedAct *act;
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
    act = stat->Data.Act;
    oldLocationState = qspLocationState;
    argsCount = qspGetStatArgs(stat, act->Args, args);
    if (qspLocationState != oldLocationState) return;
    if (!act->OnPressCode)
    {
        QSP_CHAR *firstPos, *lastPos;
        firstPos = line->Str.Str + stat->EndPos + QSP_STATIC_LEN(QSP_COLONDELIM); /* skip the colon */
        lastPos = line->Str.Str + line->Stats[endPos - 1].EndPos;
        if (qspIsCharAtPos(line->Str, lastPos, QSP_COLONDELIM_CHAR))
            lastPos += QSP_STATIC_LEN(QSP_COLONDELIM); /* include the last colon */
        act->OnPressCode = qspNewSinglelineCodeBlock(qspStringFromPair(firstPos, lastPos), line->LineNum, qspRunCode->IsTopLevel);
    }
    if (argsCount == 2)
        qspAddAction(QSP_STR(args[0]), QSP_STR(args[1]), act->OnPressCode, 0, 1);
    else
        qspAddAction(QSP_STR(args[0]), qspNullString, act->OnPressCode, 0, 1);
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
