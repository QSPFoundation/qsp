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

static int qspActIndex(QSP_CHAR *);

void qspClearActions(QSP_BOOL isFirst)
{
	int i;
	if (!isFirst && qspCurActionsCount)
	{
		for (i = 0; i < qspCurActionsCount; ++i)
		{
			if (qspCurActions[i].Image) free(qspCurActions[i].Image);
			free(qspCurActions[i].Desc);
			qspFreePrepLines(qspCurActions[i].OnPressLines, qspCurActions[i].OnPressLinesCount);
		}
		qspIsActionsChanged = QSP_TRUE;
	}
	qspCurActionsCount = 0;
	qspCurSelAction = -1;
}

static int qspActIndex(QSP_CHAR *name)
{
	int i, actNameLen, bufSize;
	QSP_CHAR *uName, *buf;
	if (!qspCurActionsCount) return -1;
	qspUpperStr(uName = qspGetNewText(name, -1));
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
		qspStrCopy(buf, qspCurActions[i].Desc);
		qspUpperStr(buf);
		if (!qspStrsComp(buf, uName))
		{
			free(uName);
			free(buf);
			return i;
		}
	}
	free(uName);
	free(buf);
	return -1;
}

void qspAddAction(QSPVariant *args, int count, QSPLineOfCode *code, int start, int end, QSP_BOOL isManageLines)
{
	QSPCurAct *act;
	QSP_CHAR *imgPath;
	if (qspActIndex(QSP_STR(args[0])) >= 0) return;
	if (qspCurActionsCount == QSP_MAXACTIONS)
	{
		qspSetError(QSP_ERR_CANTADDACTION);
		return;
	}
	if (count == 2 && qspIsAnyString(QSP_STR(args[1])))
		imgPath = qspGetAbsFromRelPath(QSP_STR(args[1]));
	else
		imgPath = 0;
	act = qspCurActions + qspCurActionsCount++;
	act->Image = imgPath;
	act->Desc = qspGetNewText(QSP_STR(args[0]), -1);
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

QSP_CHAR *qspGetAllActionsAsCode()
{
	int len = 0, count, i;
	QSP_CHAR *res, *temp;
	res = qspGetNewText(QSP_FMT(""), 0);
	for (i = 0; i < qspCurActionsCount; ++i)
	{
		len = qspAddText(&res, QSP_FMT("ACT '"), len, 5, QSP_FALSE);
		temp = qspReplaceText(qspCurActions[i].Desc, QSP_FMT("'"), QSP_FMT("''"));
		len = qspAddText(&res, temp, len, -1, QSP_FALSE);
		free(temp);
		if (qspCurActions[i].Image)
		{
			len = qspAddText(&res, QSP_FMT("','"), len, 3, QSP_FALSE);
			temp = qspReplaceText(qspCurActions[i].Image + qspQstPathLen, QSP_FMT("'"), QSP_FMT("''"));
			len = qspAddText(&res, temp, len, -1, QSP_FALSE);
			free(temp);
		}
		len = qspAddText(&res, QSP_FMT("':"), len, 2, QSP_FALSE);
		count = qspCurActions[i].OnPressLinesCount;
		if (count == 1 && qspIsAnyString(qspCurActions[i].OnPressLines->Str))
			len = qspAddText(&res, qspCurActions[i].OnPressLines->Str, len, -1, QSP_FALSE);
		else
		{
			if (count >= 2)
			{
				len = qspAddText(&res, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
				temp = qspJoinPrepLines(qspCurActions[i].OnPressLines, count, QSP_STRSDELIM);
				len = qspAddText(&res, temp, len, -1, QSP_FALSE);
				free(temp);
			}
			len = qspAddText(&res, QSP_STRSDELIM QSP_FMT("END"), len, QSP_LEN(QSP_STRSDELIM) + 3, QSP_FALSE);
		}
		len = qspAddText(&res, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
	}
	return res;
}

void qspStatementSinglelineAddAct(QSPLineOfCode *s, int statPos, int endPos)
{
	QSPVariant args[2];
	QSPLineOfCode code;
	int i, oldRefreshCount, count, offset;
	QSP_CHAR ch, *pos = s->Str + s->Stats[statPos].EndPos;
	if (*pos != QSP_COLONDELIM[0])
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
	*pos = 0;
	count = qspGetStatArgs(s->Str + s->Stats[statPos].ParamPos, qspStatAct, args);
	*pos = QSP_COLONDELIM[0];
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return;
	++statPos;
	code.Str = pos + 1;
	code.Label = qspGetLineLabel(code.Str);
	code.LineNum = 0;
	code.IsMultiline = QSP_FALSE;
	pos = s->Str + s->Stats[endPos - 1].EndPos;
	if (*pos == QSP_COLONDELIM[0]) ++pos;
	ch = *pos;
	*pos = 0;
	code.StatsCount = endPos - statPos;
	code.Stats = (QSPCachedStat *)malloc(code.StatsCount * sizeof(QSPCachedStat));
	offset = (int)(code.Str - s->Str);
	for (i = 0; i < code.StatsCount; ++i)
	{
		code.Stats[i].Stat = s->Stats[statPos].Stat;
		code.Stats[i].EndPos = s->Stats[statPos].EndPos - offset;
		code.Stats[i].ParamPos = s->Stats[statPos].ParamPos - offset;
		++statPos;
	}
	qspAddAction(args, count, &code, 0, 1, QSP_FALSE);
	*pos = ch;
	qspFreeVariants(args, count);
	free(code.Stats);
	if (code.Label) free(code.Label);
}

void qspStatementMultilineAddAct(QSPLineOfCode *s, int endLine, int lineInd, QSP_BOOL isManageLines)
{
	QSPVariant args[2];
	int count, oldRefreshCount;
	QSP_CHAR *pos;
	QSPLineOfCode *line = s + lineInd;
	pos = line->Str + line->Stats->EndPos;
	oldRefreshCount = qspRefreshCount;
	*pos = 0;
	count = qspGetStatArgs(line->Str + line->Stats->ParamPos, qspStatAct, args);
	*pos = QSP_COLONDELIM[0];
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return;
	qspAddAction(args, count, s, lineInd + 1, endLine, isManageLines);
	qspFreeVariants(args, count);
}

QSP_BOOL qspStatementDelAct(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	int actInd = qspActIndex(QSP_STR(args[0]));
	if (actInd < 0) return QSP_FALSE;
	if (qspCurSelAction >= actInd) qspCurSelAction = -1;
	if (qspCurActions[actInd].Image) free(qspCurActions[actInd].Image);
	free(qspCurActions[actInd].Desc);
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
