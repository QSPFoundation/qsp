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

#include "locations.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "statements.h"
#include "text.h"
#include "variables.h"

QSPLocation *qspLocs = 0;
QSPLocName *qspLocsNames = 0;
int qspLocsCount = 0;
int qspCurLoc = -1;
int qspRefreshCount = 0;
int qspFullRefreshCount = 0;

static int qspLocsCompare(const void *, const void *);
static int qspLocStringCompare(const void *, const void *);

static int qspLocsCompare(const void *locName1, const void *locName2)
{
	return qspStrsComp(((QSPLocName *)locName1)->Name, ((QSPLocName *)locName2)->Name);
}

static int qspLocStringCompare(const void *name, const void *compareTo)
{
	return qspStrsComp((QSP_CHAR *)name, ((QSPLocName *)compareTo)->Name);
}

void qspCreateWorld(int start, int locsCount)
{
	int i, j;
	for (i = start; i < qspLocsCount; ++i)
	{
		free(qspLocsNames[i].Name);
		free(qspLocs[i].Name);
		free(qspLocs[i].Desc);
		qspFreePrepLines(qspLocs[i].OnVisitLines, qspLocs[i].OnVisitLinesCount);
		for (j = 0; j < QSP_MAXACTIONS; ++j)
		{
			if (qspLocs[i].Actions[j].Desc)
			{
				if (qspLocs[i].Actions[j].Image) free(qspLocs[i].Actions[j].Image);
				free(qspLocs[i].Actions[j].Desc);
				qspFreePrepLines(qspLocs[i].Actions[j].OnPressLines, qspLocs[i].Actions[j].OnPressLinesCount);
			}
		}
	}
	if (qspLocsCount != locsCount)
	{
		qspLocsCount = locsCount;
		qspLocs = (QSPLocation *)realloc(qspLocs, qspLocsCount * sizeof(QSPLocation));
		qspLocsNames = (QSPLocName *)realloc(qspLocsNames, qspLocsCount * sizeof(QSPLocName));
	}
	for (i = start; i < qspLocsCount; ++i)
	{
		qspLocsNames[i].Name = 0;
		for (j = 0; j < QSP_MAXACTIONS; ++j)
			qspLocs[i].Actions[j].Desc = 0;
	}
}

void qspPrepareLocs()
{
	int i;
	for (i = 0; i < qspLocsCount; ++i)
	{
		qspLocsNames[i].Index = i;
		qspUpperStr(qspLocsNames[i].Name = qspGetAddText(qspLocsNames[i].Name, qspLocs[i].Name, 0, -1));
	}
	qsort(qspLocsNames, qspLocsCount, sizeof(QSPLocName), qspLocsCompare);
}

int qspLocIndex(QSP_CHAR *name)
{
	QSPLocName *loc;
	QSP_CHAR *uName;
	if (!qspLocsCount) return -1;
	if (!(*name)) return -1;
	qspUpperStr(uName = qspDelSpc(name));
	loc = (QSPLocName *)bsearch(uName, qspLocsNames, qspLocsCount, sizeof(QSPLocName), qspLocStringCompare);
	free(uName);
	if (loc) return loc->Index;
	return -1;
}

void qspExecLocByIndex(int locInd, QSP_BOOL isChangeDesc, QSP_BOOL isNewLoc)
{
	QSPVariant args[2];
	QSP_CHAR *str;
	QSPLineOfCode *code;
	int i, count, oldLoc, oldActIndex, oldLine;
	QSPLocation *loc = qspLocs + locInd;
	oldLoc = qspRealCurLoc;
	oldActIndex = qspRealActIndex;
	oldLine = qspRealLine;
	qspRealCurLoc = locInd;
	qspRealActIndex = -1;
	qspRealLine = 0;
	if (!(str = qspFormatText(loc->Desc, QSP_FALSE)))
	{
		qspRealLine = oldLine;
		qspRealActIndex = oldActIndex;
		qspRealCurLoc = oldLoc;
		return;
	}
	if (isChangeDesc)
	{
		if (qspCurDesc) free(qspCurDesc);
		qspCurDescLen = qspStrLen(qspCurDesc = str);
		qspIsMainDescChanged = QSP_TRUE;
	}
	else
	{
		if (*str)
		{
			qspCurDescLen = qspAddText(&qspCurDesc, str, qspCurDescLen, -1, QSP_FALSE);
			qspIsMainDescChanged = QSP_TRUE;
		}
		free(str);
	}
	for (i = 0; i < QSP_MAXACTIONS; ++i)
	{
		str = loc->Actions[i].Desc;
		if (!(str && *str)) break;
		if (!(str = qspFormatText(str, QSP_FALSE)))
		{
			qspRealLine = oldLine;
			qspRealActIndex = oldActIndex;
			qspRealCurLoc = oldLoc;
			return;
		}
		qspRealActIndex = i;
		args[0].IsStr = QSP_TRUE;
		QSP_STR(args[0]) = str;
		str = loc->Actions[i].Image;
		if (str && *str)
		{
			args[1].IsStr = QSP_TRUE;
			QSP_STR(args[1]) = str;
			count = 2;
		}
		else
			count = 1;
		qspAddAction(args, count, loc->Actions[i].OnPressLines, 0, loc->Actions[i].OnPressLinesCount, QSP_TRUE);
		free(QSP_STR(args[0]));
		if (qspErrorNum)
		{
			qspRealLine = oldLine;
			qspRealActIndex = oldActIndex;
			qspRealCurLoc = oldLoc;
			return;
		}
	}
	qspRealActIndex = -1;
	if (locInd < qspLocsCount - qspCurIncLocsCount)
		qspExecTopCodeWithLocals(loc->OnVisitLines, loc->OnVisitLinesCount, 1, isNewLoc);
	else
	{
		count = loc->OnVisitLinesCount;
		qspCopyPrepLines(&code, loc->OnVisitLines, 0, count);
		qspExecTopCodeWithLocals(code, count, 1, isNewLoc);
		qspFreePrepLines(code, count);
	}
	qspRealLine = oldLine;
	qspRealActIndex = oldActIndex;
	qspRealCurLoc = oldLoc;
}

void qspExecLocByName(QSP_CHAR *name, QSP_BOOL isChangeDesc)
{
	int locInd = qspLocIndex(name);
	if (locInd < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return;
	}
	qspExecLocByIndex(locInd, isChangeDesc, QSP_FALSE);
}

void qspExecLocByNameWithArgs(QSP_CHAR *name, QSPVariant *args, int count, QSPVariant *res)
{
	QSPVar local, result, *var, *varRes;
	int oldRefreshCount;
	if (!(varRes = qspVarReference(QSP_VARRES, QSP_TRUE))) return;
	if (!(var = qspVarReference(QSP_VARARGS, QSP_TRUE))) return;
	qspMoveVar(&result, varRes);
	qspMoveVar(&local, var);
	qspSetArgs(var, args, count);
	oldRefreshCount = qspRefreshCount;
	qspExecLocByName(name, QSP_FALSE);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
	{
		qspEmptyVar(&local);
		qspEmptyVar(&result);
		return;
	}
	if (!(var = qspVarReference(QSP_VARARGS, QSP_TRUE)))
	{
		qspEmptyVar(&local);
		qspEmptyVar(&result);
		return;
	}
	qspEmptyVar(var);
	qspMoveVar(var, &local);
	if (!(varRes = qspVarReference(QSP_VARRES, QSP_TRUE)))
	{
		qspEmptyVar(&result);
		return;
	}
	if (res) qspApplyResult(varRes, res);
	qspEmptyVar(varRes);
	qspMoveVar(varRes, &result);
}

void qspExecLocByVarNameWithArgs(QSP_CHAR *name, QSPVariant *args, int count)
{
	QSPVar *var;
	QSP_CHAR *locName;
	int ind = 0, oldRefreshCount = qspRefreshCount;
	while (1)
	{
		if (!(var = qspVarReference(name, QSP_FALSE))) break;
		if (ind >= var->ValsCount) break;
		if (!((locName = var->Values[ind].Str) && qspIsAnyString(locName))) break;
		qspExecLocByNameWithArgs(locName, args, count, 0);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
		++ind;
	}
}

void qspRefreshCurLoc(QSP_BOOL isChangeDesc, QSPVariant *args, int count)
{
	QSPVar *var;
	int oldRefreshCount;
	if (!(var = qspVarReference(QSP_VARARGS, QSP_TRUE))) return;
	qspEmptyVar(var);
	qspSetArgs(var, args, count);
	qspClearActions(QSP_FALSE);
	++qspRefreshCount;
	if (isChangeDesc) ++qspFullRefreshCount;
	oldRefreshCount = qspRefreshCount;
	qspExecLocByIndex(qspCurLoc, isChangeDesc, QSP_TRUE);
	if (qspErrorNum) return;
	if (qspRefreshCount == oldRefreshCount)
		qspExecLocByVarNameWithArgs(QSP_FMT("ONNEWLOC"), args, count);
}
