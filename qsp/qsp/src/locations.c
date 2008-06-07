/* Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include "declarations.h"
#include "statements.h"

void qspCreateWorld(long start, long locsCount)
{
	long i, j;
	for (i = start; i < qspLocsCount; ++i)
	{
		free(qspLocs[i].Name);
		free(qspLocs[i].Desc);
		qspFreeStrs(qspLocs[i].OnVisitLines, qspLocs[i].OnVisitLinesCount, QSP_FALSE);
		for (j = 0; j < QSP_MAXACTIONS; ++j)
			if (qspLocs[i].Actions[j].Desc)
			{
				if (qspLocs[i].Actions[j].Image) free(qspLocs[i].Actions[j].Image);
				free(qspLocs[i].Actions[j].Desc);
				qspFreeStrs(qspLocs[i].Actions[j].OnPressLines, qspLocs[i].Actions[j].OnPressLinesCount, QSP_FALSE);
			}
	}
	if (qspLocsCount != locsCount)
	{
		qspLocsCount = locsCount;
		qspLocs = (QSPLocation *)realloc(qspLocs, qspLocsCount * sizeof(QSPLocation));
	}
	for (i = start; i < qspLocsCount; ++i)
		for (j = 0; j < QSP_MAXACTIONS; ++j)
			qspLocs[i].Actions[j].Desc = 0;
}

long qspLocIndex(QSP_CHAR *name)
{
	long i, locNameLen, bufSize;
	QSP_CHAR *uName, *buf;
	if (!qspLocsCount) return -1;
	uName = qspDelSpc(name);
	if (!(*uName))
	{
		free(uName);
		return -1;
	}
	qspUpperStr(uName);
	bufSize = 16;
	buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	for (i = 0; i < qspLocsCount; ++i)
	{
		locNameLen = (long)QSP_STRLEN(qspLocs[i].Name);
		if (locNameLen >= bufSize)
		{
			bufSize = locNameLen + 8;
			buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
		}
		QSP_STRCPY(buf, qspLocs[i].Name);
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

void qspExecLocByName(QSP_CHAR *name, QSP_BOOL isChangeDesc)
{
	QSPVariant args[QSP_STATMAXARGS];
	QSP_CHAR *str, **code;
	long i, count, oldLoc, oldWhere, locInd = qspLocIndex(name);
	if (locInd < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return;
	}
	oldLoc = qspRealCurLoc;
	oldWhere = qspRealWhere;
	qspRealCurLoc = locInd;
	qspRealWhere = QSP_AREA_ONLOCVISIT;
	qspRealLine = 0;
	str = qspFormatText(qspLocs[locInd].Desc);
	if (qspErrorNum)
	{
		qspRealWhere = oldWhere;
		qspRealCurLoc = oldLoc;
		return;
	}
	if (isChangeDesc)
	{
		if (qspCurDesc) free(qspCurDesc);
		qspCurDescLen = (long)QSP_STRLEN(qspCurDesc = str);
		qspIsMainDescChanged = QSP_TRUE;
	}
	else
	{
		qspCurDescLen = qspAddText(&qspCurDesc, str, qspCurDescLen, -1, QSP_FALSE);
		if (*str) qspIsMainDescChanged = QSP_TRUE;
		free(str);
	}
	qspRealWhere = QSP_AREA_ONLOCACTION;
	for (i = 0; i < QSP_MAXACTIONS; ++i)
	{
		str = qspLocs[locInd].Actions[i].Desc;
		if (!(str && *str)) break;
		str = qspFormatText(str);
		if (qspErrorNum)
		{
			qspRealWhere = oldWhere;
			qspRealCurLoc = oldLoc;
			return;
		}
		args[0].IsStr = QSP_TRUE;
		args[0].Str = str;
		str = qspLocs[locInd].Actions[i].Image;
		if (str && *str)
		{
			args[1].IsStr = QSP_TRUE;
			args[1].Str = str;
			count = 2;
		}
		else
			count = 1;
		qspAddAction(args, count, qspLocs[locInd].Actions[i].OnPressLines, 0, qspLocs[locInd].Actions[i].OnPressLinesCount, QSP_TRUE);
		free(args[0].Str);
		if (qspErrorNum)
		{
			qspRealWhere = oldWhere;
			qspRealCurLoc = oldLoc;
			return;
		}
	}
	qspRealWhere = QSP_AREA_ONLOCVISIT;
	count = qspLocs[locInd].OnVisitLinesCount;
	qspCopyStrs(&code, qspLocs[locInd].OnVisitLines, 0, count);
	qspExecCode(code, 0, count, 1, 0, QSP_TRUE);
	qspFreeStrs(code, count, QSP_FALSE);
	qspRealWhere = oldWhere;
	qspRealCurLoc = oldLoc;
}

void qspExecLocByVarName(QSP_CHAR *name)
{
	QSPVariant v = qspGetVarValueByName(name);
	if (qspIsAnyString(v.Str)) qspExecLocByName(v.Str, QSP_FALSE);
	free(v.Str);
}
