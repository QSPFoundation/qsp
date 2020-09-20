/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
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
int qspCurActionsCount = 0;
int qspCurSelAction = -1;
QSP_BOOL qspIsActionsChanged = QSP_FALSE;
QSP_BOOL qspCurIsShowActs = QSP_TRUE;

INLINE int qspActIndex(QSPString name);

void qspClearActions(QSP_BOOL isFirst)
{
    int i;
    if (!isFirst && qspCurActionsCount)
    {
        for (i = 0; i < qspCurActionsCount; ++i)
        {
            qspFreeString(qspCurActions[i].Image);
            qspFreeString(qspCurActions[i].Desc);
            qspFreePrepLines(qspCurActions[i].OnPressLines, qspCurActions[i].OnPressLinesCount);
        }
        qspIsActionsChanged = QSP_TRUE;
    }
    qspCurActionsCount = 0;
    qspCurSelAction = -1;
}

INLINE int qspActIndex(QSPString name)
{
    QSPString bufName;
    int i, actNameLen, bufSize;
    QSP_CHAR *buf;
    if (!qspCurActionsCount) return -1;
    name = qspGetNewText(name);
    qspUpperStr(&name);
    bufSize = 64;
    buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
    for (i = 0; i < qspCurActionsCount; ++i)
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
            qspFreeString(name);
            free(buf);
            return i;
        }
    }
    qspFreeString(name);
    free(buf);
    return -1;
}

void qspAddAction(QSPVariant *args, int count, QSPLineOfCode *code, int start, int end, QSP_BOOL isManageLines)
{
    QSPCurAct *act;
    QSPString imgPath;
    if (qspActIndex(QSP_STR(args[0])) >= 0) return;
    if (qspCurActionsCount == QSP_MAXACTIONS)
    {
        qspSetError(QSP_ERR_CANTADDACTION);
        return;
    }
    if (count == 2 && qspIsAnyString(QSP_STR(args[1])))
        imgPath = qspGetNewText(QSP_STR(args[1]));
    else
        imgPath = qspNullString;
    act = qspCurActions + qspCurActionsCount++;
    act->Image = imgPath;
    act->Desc = qspGetNewText(QSP_STR(args[0]));
    qspCopyPrepLines(&act->OnPressLines, code, start, end);
    act->OnPressLinesCount = end - start;
    act->Location = qspRealCurLoc;
    act->ActIndex = qspRealActIndex;
    act->StartLine = qspRealLine;
    act->IsManageLines = isManageLines;
    qspIsActionsChanged = QSP_TRUE;
}

void qspExecAction(int ind)
{
    QSPCurAct *act;
    QSPLineOfCode *code;
    int count, oldLoc, oldActIndex, oldLine;
    oldLoc = qspRealCurLoc;
    oldActIndex = qspRealActIndex;
    oldLine = qspRealLine;
    act = qspCurActions + ind;
    qspRealCurLoc = act->Location;
    qspRealActIndex = act->ActIndex;
    count = act->OnPressLinesCount;
    qspCopyPrepLines(&code, act->OnPressLines, 0, count);
    if (act->IsManageLines)
        qspExecCodeBlockWithLocals(code, 0, count, 1, 0);
    else
    {
        qspRealLine = act->StartLine;
        qspExecCodeBlockWithLocals(code, 0, count, 0, 0);
    }
    qspFreePrepLines(code, count);
    qspRealLine = oldLine;
    qspRealActIndex = oldActIndex;
    qspRealCurLoc = oldLoc;
}

QSPString qspGetAllActionsAsCode()
{
    int count, i;
    QSPString temp, res;
    res = qspNullString;
    for (i = 0; i < qspCurActionsCount; ++i)
    {
        qspAddText(&res, QSP_STATIC_STR(QSP_FMT("ACT '")), QSP_FALSE);
        temp = qspReplaceText(qspCurActions[i].Desc, QSP_STATIC_STR(QSP_FMT("'")), QSP_STATIC_STR(QSP_FMT("''")));
        qspAddText(&res, temp, QSP_FALSE);
        qspFreeString(temp);
        if (qspCurActions[i].Image.Str)
        {
            qspAddText(&res, QSP_STATIC_STR(QSP_FMT("','")), QSP_FALSE);
            temp = qspReplaceText(qspCurActions[i].Image, QSP_STATIC_STR(QSP_FMT("'")), QSP_STATIC_STR(QSP_FMT("''")));
            qspAddText(&res, temp, QSP_FALSE);
            qspFreeString(temp);
        }
        qspAddText(&res, QSP_STATIC_STR(QSP_FMT("':")), QSP_FALSE);
        count = qspCurActions[i].OnPressLinesCount;
        if (count == 1 && qspIsAnyString(qspCurActions[i].OnPressLines->Str))
            qspAddText(&res, qspCurActions[i].OnPressLines->Str, QSP_FALSE);
        else
        {
            if (count >= 2)
            {
                qspAddText(&res, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
                temp = qspJoinPrepLines(qspCurActions[i].OnPressLines, count, QSP_STATIC_STR(QSP_STRSDELIM));
                qspAddText(&res, temp, QSP_FALSE);
                qspFreeString(temp);
            }
            qspAddText(&res, QSP_STATIC_STR(QSP_STRSDELIM QSP_FMT("END")), QSP_FALSE);
        }
        qspAddText(&res, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
    }
    return res;
}

void qspStatementSinglelineAddAct(QSPLineOfCode *s, int statPos, int endPos)
{
    QSPVariant args[2];
    QSPLineOfCode code;
    int i, j, oldRefreshCount, count, offset;
    QSP_CHAR *lastPos, *firstPos = s->Str.Str + s->Stats[statPos].EndPos;
    if (firstPos == s->Str.End || *firstPos != QSP_COLONDELIM[0])
    {
        qspSetError(QSP_ERR_COLONNOTFOUND);
        return;
    }
    if (statPos == endPos - 1)
    {
        qspSetError(QSP_ERR_CODENOTFOUND);
        return;
    }
    oldRefreshCount = qspRefreshCount;
    count = qspGetStatArgs(s->Str, s->Stats + statPos, args);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum)
        return;
    ++statPos; /* start with the internal code */
    firstPos += QSP_STATIC_LEN(QSP_COLONDELIM);
    lastPos = s->Str.Str + s->Stats[endPos - 1].EndPos;
    if (lastPos != s->Str.End && *lastPos == QSP_COLONDELIM[0]) lastPos += QSP_STATIC_LEN(QSP_COLONDELIM);
    code.Str = qspStringFromPair(firstPos, lastPos);
    code.Label = qspGetLineLabel(code.Str);
    code.LineNum = 0;
    code.IsMultiline = QSP_FALSE;
    code.StatsCount = endPos - statPos;
    code.Stats = (QSPCachedStat *)malloc(code.StatsCount * sizeof(QSPCachedStat));
    offset = (int)(firstPos - s->Str.Str);
    for (i = 0; i < code.StatsCount; ++i)
    {
        code.Stats[i].Stat = s->Stats[statPos].Stat;
        code.Stats[i].ParamPos = s->Stats[statPos].ParamPos - offset;
        code.Stats[i].EndPos = s->Stats[statPos].EndPos - offset;
        code.Stats[i].ErrorCode = s->Stats[statPos].ErrorCode;
        code.Stats[i].ArgsCount = s->Stats[statPos].ArgsCount;
        code.Stats[i].Args = (QSPCachedArg *)malloc(code.Stats[i].ArgsCount * sizeof(QSPCachedArg));
        for (j = 0; j < code.Stats[i].ArgsCount; ++j)
        {
            code.Stats[i].Args[j].StartPos = s->Stats[statPos].Args[j].StartPos - offset;
            code.Stats[i].Args[j].EndPos = s->Stats[statPos].Args[j].EndPos - offset;
        }
        ++statPos;
    }
    qspAddAction(args, count, &code, 0, 1, QSP_FALSE);
    qspFreeVariants(args, count);
    qspFreeLineOfCode(&code);
}

void qspStatementMultilineAddAct(QSPLineOfCode *s, int endLine, int lineInd, QSP_BOOL isManageLines)
{
    QSPVariant args[2];
    int count, oldRefreshCount;
    QSPLineOfCode *line = s + lineInd;
    oldRefreshCount = qspRefreshCount;
    count = qspGetStatArgs(line->Str, line->Stats, args);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum) return;
    qspAddAction(args, count, s, lineInd + 1, endLine, isManageLines);
    qspFreeVariants(args, count);
}

QSP_BOOL qspStatementDelAct(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    int actInd = qspActIndex(QSP_STR(args[0]));
    if (actInd < 0) return QSP_FALSE;
    if (qspCurSelAction >= actInd) qspCurSelAction = -1;
    qspFreeString(qspCurActions[actInd].Image);
    qspFreeString(qspCurActions[actInd].Desc);
    qspFreePrepLines(qspCurActions[actInd].OnPressLines, qspCurActions[actInd].OnPressLinesCount);
    --qspCurActionsCount;
    while (actInd < qspCurActionsCount)
    {
        qspCurActions[actInd] = qspCurActions[actInd + 1];
        ++actInd;
    }
    qspIsActionsChanged = QSP_TRUE;
    return QSP_FALSE;
}
