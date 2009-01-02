/* Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru) */
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
#include "statements.h"
#include "text.h"

QSPCurAct qspCurActions[QSP_MAXACTIONS];
long qspCurActionsCount = 0;
long qspCurSelAction = -1;
QSP_BOOL qspIsActionsChanged = QSP_FALSE;
QSP_BOOL qspCurIsShowActs = QSP_TRUE;

static long qspActIndex(QSP_CHAR *);

void qspClearActions(QSP_BOOL isFirst)
{
	long i;
	if (!isFirst && qspCurActionsCount)
	{
		for (i = 0; i < qspCurActionsCount; ++i)
		{
			if (qspCurActions[i].Image) free(qspCurActions[i].Image);
			free(qspCurActions[i].Desc);
			qspFreeStrs(qspCurActions[i].OnPressLines, qspCurActions[i].OnPressLinesCount, QSP_FALSE);
		}
		qspIsActionsChanged = QSP_TRUE;
	}
	qspCurActionsCount = 0;
	qspCurSelAction = -1;
}

static long qspActIndex(QSP_CHAR *name)
{
	long i, actNameLen, bufSize;
	QSP_CHAR *uName, *buf;
	if (!qspCurActionsCount) return -1;
	qspUpperStr(uName = qspGetNewText(name, -1));
	bufSize = 32;
	buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	for (i = 0; i < qspCurActionsCount; ++i)
	{
		actNameLen = (long)QSP_STRLEN(qspCurActions[i].Desc);
		if (actNameLen >= bufSize)
		{
			bufSize = actNameLen + 16;
			buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
		}
		QSP_STRCPY(buf, qspCurActions[i].Desc);
		qspUpperStr(buf);
		if (!QSP_STRCMP(buf, uName))
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

void qspAddAction(QSPVariant *args, long count, QSP_CHAR **code, long start, long end, QSP_BOOL isMultiline)
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
	{
		imgPath = qspGetNewText(qspQstPath, qspQstPathLen);
		imgPath = qspGetAddText(imgPath, QSP_STR(args[1]), qspQstPathLen, -1);
	}
	else
		imgPath = 0;
	act = qspCurActions + qspCurActionsCount++;
	act->Image = imgPath;
	act->Desc = qspGetNewText(QSP_STR(args[0]), -1);
	qspCopyStrs(&act->OnPressLines, code, start, end);
	act->OnPressLinesCount = end - start;
	act->Location = qspRealCurLoc;
	act->Where = qspRealWhere;
	act->StartLine = (isMultiline ? qspRealLine + 1 : qspRealLine);
	qspIsActionsChanged = QSP_TRUE;
}

void qspExecAction(long ind)
{
	QSPCurAct *act;
	QSP_CHAR **code;
	long count, oldLoc, oldWhere, oldLine;
	oldLoc = qspRealCurLoc;
	oldWhere = qspRealWhere;
	oldLine = qspRealLine;
	act = qspCurActions + ind;
	qspRealCurLoc = act->Location;
	qspRealWhere = act->Where;
	count = act->OnPressLinesCount;
	qspCopyStrs(&code, act->OnPressLines, 0, count);
	qspExecCode(code, 0, count, act->StartLine, 0, QSP_TRUE);
	qspFreeStrs(code, count, QSP_FALSE);
	qspRealLine = oldLine;
	qspRealWhere = oldWhere;
	qspRealCurLoc = oldLoc;
}

void qspStatementAddAct(QSP_CHAR *s)
{
	long count;
	QSPVariant args[QSP_STATMAXARGS];
	QSP_CHAR *code, *pos = qspStrPos(s, QSP_COLONDELIM, QSP_FALSE);
	if (!pos)
	{
		qspSetError(QSP_ERR_COLONNOTFOUND);
		return;
	}
	*pos = 0;
	count = qspGetStatArgs(s, qspStatAct, args);
	*pos = QSP_COLONDELIM[0];
	if (qspErrorNum) return;
	code = pos + 1;
	qspAddAction(args, count, &code, 0, 1, QSP_FALSE);
	qspFreeVariants(args, count);
}

QSP_BOOL qspStatementDelAct(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long actInd = qspActIndex(QSP_STR(args[0]));
	if (actInd < 0) return QSP_FALSE;
	if (qspCurSelAction >= actInd) qspCurSelAction = -1;
	if (qspCurActions[actInd].Image) free(qspCurActions[actInd].Image);
	free(qspCurActions[actInd].Desc);
	qspFreeStrs(qspCurActions[actInd].OnPressLines, qspCurActions[actInd].OnPressLinesCount, QSP_FALSE);
	--qspCurActionsCount;
	while (actInd < qspCurActionsCount)
	{
		qspCurActions[actInd] = qspCurActions[actInd + 1];
		++actInd;
	}
	qspIsActionsChanged = QSP_TRUE;
	return QSP_FALSE;
}
