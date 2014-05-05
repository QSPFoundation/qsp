/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
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

static int qspActIndex(QSPString name);

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

static int qspActIndex(QSPString name)
{
	QSPString uName, bufName;
	int i, actNameLen, bufSize;
	QSP_CHAR *buf;
	if (!qspCurActionsCount) return -1;
	uName = qspGetNewText(name);
	qspUpperStr(&uName);
	bufSize = 64;
	buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	for (i = 0; i < qspCurActionsCount; ++i)
	{
		actNameLen = qspStrLen(qspCurActions[i].Desc);
		if (actNameLen >= bufSize)
		{
			bufSize = actNameLen + 16;
			buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
		}
		memcpy(buf, qspCurActions[i].Desc.Str, actNameLen * sizeof(QSP_CHAR));
		bufName = qspStringFromLen(buf, actNameLen);
		qspUpperStr(&bufName);
		if (!qspStrsComp(bufName, uName))
		{
			qspFreeString(uName);
			free(buf);
			return i;
		}
	}
	qspFreeString(uName);
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
		imgPath = qspGetAbsFromRelPath(QSP_STR(args[1]));
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
		qspExecTopCodeWithLocals(code, count, 1, QSP_FALSE);
	else
	{
		qspRealLine = act->StartLine;
		qspExecTopCodeWithLocals(code, count, 0, QSP_FALSE);
	}
	qspFreePrepLines(code, count);
	qspRealLine = oldLine;
	qspRealActIndex = oldActIndex;
	qspRealCurLoc = oldLoc;
}

QSPString qspGetAllActionsAsCode()
{
	int count, i;
	QSPString res, temp;
	res = qspNewEmptyString();
	for (i = 0; i < qspCurActionsCount; ++i)
	{
		qspAddText(&res, QSP_STATIC_STR(QSP_FMT("ACT '")), QSP_FALSE);
		temp = qspReplaceText(qspCurActions[i].Desc, QSP_STATIC_STR(QSP_FMT("'")), QSP_STATIC_STR(QSP_FMT("''")));
		qspAddText(&res, temp, QSP_FALSE);
		qspFreeString(temp);
		if (qspCurActions[i].Image.Str)
		{
			qspAddText(&res, QSP_STATIC_STR(QSP_FMT("','")), QSP_FALSE);
			temp = qspReplaceText(
				qspStringFromPair(qspCurActions[i].Image.Str + qspStrLen(qspQstPath), qspCurActions[i].Image.End),
				QSP_STATIC_STR(QSP_FMT("'")), QSP_STATIC_STR(QSP_FMT("''")));
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
	int i, oldRefreshCount, count, offset;
	QSP_CHAR *lastPos, *firstPos = s->Str.Str + s->Stats[statPos].EndPos;
	if (*firstPos != QSP_COLONDELIM[0])
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
	count = qspGetStatArgs(qspStringFromPair(s->Str.Str + s->Stats[statPos].ParamPos, firstPos), qspStatAct, args);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return;
	++statPos;
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
		code.Stats[i].EndPos = s->Stats[statPos].EndPos - offset;
		code.Stats[i].ParamPos = s->Stats[statPos].ParamPos - offset;
		++statPos;
	}
	qspAddAction(args, count, &code, 0, 1, QSP_FALSE);
	qspFreeVariants(args, count);
	free(code.Stats);
	qspFreeString(code.Label);
}

void qspStatementMultilineAddAct(QSPLineOfCode *s, int endLine, int lineInd, QSP_BOOL isManageLines)
{
	QSPVariant args[2];
	int count, oldRefreshCount;
	QSPLineOfCode *line = s + lineInd;
	oldRefreshCount = qspRefreshCount;
	count = qspGetStatArgs(
		qspStringFromPair(line->Str.Str + line->Stats->ParamPos, line->Str.Str + line->Stats->EndPos),
		qspStatAct, args);
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
