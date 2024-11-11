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
    QSPString bufName;
    int i, actNameLen, bufSize;
    QSP_CHAR *buf;
    if (!qspCurActsCount) return -1;
    name = qspCopyToNewText(name);
    qspUpperStr(&name);
    bufSize = 64;
    buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
    for (i = 0; i < qspCurActsCount; ++i)
    {
        actNameLen = qspStrLen(qspCurActions[i].Desc);
        if (actNameLen)
        {
            if (actNameLen > bufSize)
            {
                bufSize = actNameLen + 16;
                buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
            }
            memcpy(buf, qspCurActions[i].Desc.Str, actNameLen * sizeof(QSP_CHAR));
        }
        bufName = qspStringFromLen(buf, actNameLen);
        qspUpperStr(&bufName);
        if (!qspStrsComp(bufName, name))
        {
            qspFreeString(&name);
            free(buf);
            return i;
        }
    }
    qspFreeString(&name);
    free(buf);
    return -1;
}

void qspAddAction(QSPVariant *args, QSP_TINYINT count, QSPLineOfCode *code, int start, int end)
{
    QSPCurAct *act;
    QSPString imgPath;
    if (qspActIndex(QSP_STR(args[0])) >= 0) return;
    if (qspCurActsCount == QSP_MAXACTIONS)
    {
        qspSetError(QSP_ERR_CANTADDACTION);
        return;
    }
    if (count == 2 && qspIsAnyString(QSP_STR(args[1])))
        imgPath = qspCopyToNewText(QSP_STR(args[1]));
    else
        imgPath = qspNullString;
    act = qspCurActions + qspCurActsCount++;
    act->Image = imgPath;
    act->Desc = qspCopyToNewText(QSP_STR(args[0]));
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
        int count;
        QSPLineOfCode *code;
        QSPCurAct *act = qspCurActions + ind;
        /* switch the current state */
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
    if (!qspIsCharAtPos(line->Str, firstPos, QSP_COLONDELIM[0]))
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
    if (qspLocationState != oldLocationState)
        return;
    firstPos += QSP_STATIC_LEN(QSP_COLONDELIM);
    lastPos = line->Str.Str + line->Stats[endPos - 1].EndPos;
    if (qspIsCharAtPos(line->Str, lastPos, QSP_COLONDELIM[0])) lastPos += QSP_STATIC_LEN(QSP_COLONDELIM);
    ++statPos; /* start with the internal code */
    code.Str = qspStringFromPair(firstPos, lastPos);
    code.Label = qspGetLineLabel(code.Str);
    code.LineNum = line->LineNum;
    code.LinesToElse = code.LinesToEnd = 0;
    code.IsMultiline = QSP_FALSE;
    code.StatsCount = endPos - statPos;
    qspCopyPrepStatements(&code.Stats, line->Stats, statPos, endPos, (int)(firstPos - line->Str.Str));
    qspAddAction(args, argsCount, &code, 0, 1);
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
    qspAddAction(args, argsCount, s, lineInd + 1, endLine);
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
