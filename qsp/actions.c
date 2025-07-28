/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "actions.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "statements.h"
#include "text.h"

QSPCurAct qspCurActions[QSP_MAXACTIONS];
int qspCurActsCount = 0;
int qspCurSelAction = -1;
QSP_BOOL qspIsActsListChanged = QSP_FALSE;
QSP_BOOL qspCurToShowActs = QSP_TRUE;

INLINE int qspActIndex(QSPString name);

void qspClearAllActions(QSP_BOOL toInit)
{
    if (!toInit && qspCurActsCount)
    {
        int i;
        for (i = 0; i < qspCurActsCount; ++i)
        {
            qspFreeString(&qspCurActions[i].Image);
            qspFreeString(&qspCurActions[i].Desc);
            qspFreePrepLines(qspCurActions[i].OnPressLines, qspCurActions[i].OnPressLinesCount);
        }
        qspIsActsListChanged = QSP_TRUE;
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
        buf = qspNewBufString(64);
        for (i = 0; i < qspCurActsCount; ++i)
        {
            qspUpdateBufString(&buf, qspCurActions[i].Desc);
            bufName = qspBufTextToString(buf);
            qspUpperStr(&bufName);
            if (!qspStrsCompare(bufName, name))
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

void qspAddAction(QSPString name, QSPString imgPath, QSPLineOfCode *code, int start, int end)
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
    qspCopyPrepLines(&act->OnPressLines, code, start, end);
    act->OnPressLinesCount = end - start;
    act->Location = qspRealCurLoc;
    act->ActIndex = qspRealActIndex;
    qspIsActsListChanged = QSP_TRUE;
}

void qspExecAction(int ind)
{
    if (ind >= 0 && ind < qspCurActsCount)
    {
        /* Keep the current location context here (don't reset special vars) */
        int count;
        QSPLineOfCode *code;
        QSPCurAct *act = qspCurActions + ind;
        /* Switch the current state */
        qspRealCurLoc = act->Location;
        qspRealActIndex = act->ActIndex;
        count = act->OnPressLinesCount;
        qspCopyPrepLines(&code, act->OnPressLines, 0, count);
        qspExecCodeBlockWithLocals(code, 0, count, 1, 0);
        qspFreePrepLines(code, count);
    }
}

QSPString qspGetAllActionsAsCode(void)
{
    int count, i;
    QSPString temp;
    QSPBufString res = qspNewBufString(256);
    for (i = 0; i < qspCurActsCount; ++i)
    {
        qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT("ACT ") QSP_DEFQUOT));
        temp = qspReplaceText(qspCurActions[i].Desc, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
        qspAddBufText(&res, temp);
        qspFreeNewString(&temp, &qspCurActions[i].Desc);
        if (qspCurActions[i].Image.Str)
        {
            qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(",") QSP_DEFQUOT));
            temp = qspReplaceText(qspCurActions[i].Image, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
            qspAddBufText(&res, temp);
            qspFreeNewString(&temp, &qspCurActions[i].Image);
        }
        qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(":")));
        count = qspCurActions[i].OnPressLinesCount;
        if (count == 1 && qspIsAnyString(qspCurActions[i].OnPressLines->Str))
            qspAddBufText(&res, qspCurActions[i].OnPressLines->Str);
        else
        {
            if (count >= 2)
            {
                qspAddBufText(&res, QSP_STATIC_STR(QSP_STRSDELIM));
                temp = qspJoinPrepLines(qspCurActions[i].OnPressLines, count, QSP_STATIC_STR(QSP_STRSDELIM));
                qspAddBufText(&res, temp);
                qspFreeString(&temp);
            }
            qspAddBufText(&res, QSP_STATIC_STR(QSP_STRSDELIM QSP_FMT("END")));
        }
        qspAddBufText(&res, QSP_STATIC_STR(QSP_STRSDELIM));
    }
    return qspBufTextToString(res);
}

void qspStatementSinglelineAddAct(QSPLineOfCode *line, int statPos, int endPos)
{
    QSPVariant args[2];
    QSP_TINYINT argsCount;
    QSPLineOfCode code;
    int oldLocationState;
    QSP_CHAR *lastPos, *firstPos = line->Str.Str + line->Stats[statPos].EndPos;
    if (!qspIsCharAtPos(line->Str, firstPos, QSP_COLONDELIM_CHAR))
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
    argsCount = qspGetStatArgs(line->Str, line->Stats + statPos, args);
    if (qspLocationState != oldLocationState) return;
    firstPos += QSP_CHAR_LEN;
    lastPos = line->Str.Str + line->Stats[endPos - 1].EndPos;
    if (qspIsCharAtPos(line->Str, lastPos, QSP_COLONDELIM_CHAR)) lastPos += QSP_CHAR_LEN;
    ++statPos; /* start with the internal code */
    code.Str = qspStringFromPair(firstPos, lastPos);
    code.Label = qspGetLineLabel(code.Str);
    code.LineNum = line->LineNum;
    code.LinesToElse = code.LinesToEnd = 0;
    code.IsMultiline = QSP_FALSE;
    code.StatsCount = endPos - statPos;
    qspCopyPrepStatements(&code.Stats, line->Stats, statPos, endPos, (int)(firstPos - line->Str.Str));
    if (argsCount == 2)
        qspAddAction(QSP_STR(args[0]), QSP_STR(args[1]), &code, 0, 1);
    else
        qspAddAction(QSP_STR(args[0]), qspNullString, &code, 0, 1);
    qspFreeVariants(args, argsCount);
    qspFreeLineOfCode(&code);
}

void qspStatementMultilineAddAct(QSPLineOfCode *s, int lineInd, int endLine)
{
    QSPVariant args[2];
    QSP_TINYINT argsCount;
    int oldLocationState = qspLocationState;
    QSPLineOfCode *line = s + lineInd;
    argsCount = qspGetStatArgs(line->Str, line->Stats, args);
    if (qspLocationState != oldLocationState) return;
    if (argsCount == 2)
        qspAddAction(QSP_STR(args[0]), QSP_STR(args[1]), s, lineInd + 1, endLine);
    else
        qspAddAction(QSP_STR(args[0]), qspNullString, s, lineInd + 1, endLine);
    qspFreeVariants(args, argsCount);
}

void qspStatementDelAct(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    int actInd = qspActIndex(QSP_STR(args[0]));
    if (actInd < 0) return;
    if (qspCurSelAction >= actInd) qspCurSelAction = -1;
    qspFreeString(&qspCurActions[actInd].Image);
    qspFreeString(&qspCurActions[actInd].Desc);
    qspFreePrepLines(qspCurActions[actInd].OnPressLines, qspCurActions[actInd].OnPressLinesCount);
    --qspCurActsCount;
    while (actInd < qspCurActsCount)
    {
        qspCurActions[actInd] = qspCurActions[actInd + 1];
        ++actInd;
    }
    qspIsActsListChanged = QSP_TRUE;
}
