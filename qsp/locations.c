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
	return qspStrsComp(*(QSPString *)name, ((QSPLocName *)compareTo)->Name);
}

void qspCreateWorld(int start, int locsCount)
{
	int i, j;
	for (i = start; i < qspLocsCount; ++i)
	{
		qspFreeString(qspLocsNames[i].Name);
		qspFreeString(qspLocs[i].Name);
		qspFreeString(qspLocs[i].Desc);
		qspFreePrepLines(qspLocs[i].OnVisitLines, qspLocs[i].OnVisitLinesCount);
		for (j = 0; j < QSP_MAXACTIONS; ++j)
		{
			if (qspLocs[i].Actions[j].Desc.Str)
			{
				qspFreeString(qspLocs[i].Actions[j].Image);
				qspFreeString(qspLocs[i].Actions[j].Desc);
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
		qspLocsNames[i].Name = qspNullString;
		for (j = 0; j < QSP_MAXACTIONS; ++j)
			qspLocs[i].Actions[j].Desc = qspNullString;
	}
}

void qspPrepareLocs()
{
	int i;
	for (i = 0; i < qspLocsCount; ++i)
	{
		qspLocsNames[i].Index = i;
		qspUpdateText(&qspLocsNames[i].Name, qspLocs[i].Name);
		qspUpperStr(&qspLocsNames[i].Name);
	}
	qsort(qspLocsNames, qspLocsCount, sizeof(QSPLocName), qspLocsCompare);
}

int qspLocIndex(QSPString name)
{
	QSPLocName *loc;
	if (!qspLocsCount) return -1;
	name = qspDelSpc(name);
	if (qspIsEmpty(name)) return -1;
	name = qspGetNewText(name);
	qspUpperStr(&name);
	loc = (QSPLocName *)bsearch(&name, qspLocsNames, qspLocsCount, sizeof(QSPLocName), qspLocStringCompare);
	qspFreeString(name);
	if (loc) return loc->Index;
	return -1;
}

void qspExecLocByIndex(int locInd, QSP_BOOL isChangeDesc, QSP_BOOL isNewLoc)
{
	QSPVariant args[2];
	QSPString str;
	QSPLineOfCode *code;
	int i, count, oldLoc, oldActIndex, oldLine;
	QSPLocation *loc = qspLocs + locInd;
	oldLoc = qspRealCurLoc;
	oldActIndex = qspRealActIndex;
	oldLine = qspRealLine;
	qspRealCurLoc = locInd;
	qspRealActIndex = -1;
	qspRealLine = 0;
	str = qspFormatText(loc->Desc, QSP_FALSE);
	if (!str.Str)
	{
		qspRealLine = oldLine;
		qspRealActIndex = oldActIndex;
		qspRealCurLoc = oldLoc;
		return;
	}
	if (isChangeDesc)
	{
		qspFreeString(qspCurDesc);
		qspCurDesc = str;
		qspIsMainDescChanged = QSP_TRUE;
	}
	else
	{
		if (!qspIsEmpty(str))
		{
			qspAddText(&qspCurDesc, str, QSP_FALSE);
			qspIsMainDescChanged = QSP_TRUE;
		}
		qspFreeString(str);
	}
	for (i = 0; i < QSP_MAXACTIONS; ++i)
	{
		str = loc->Actions[i].Desc;
		if (!str.Str || qspIsEmpty(str)) break; // use qspIsEmpty only
		str = qspFormatText(str, QSP_FALSE);
		if (!str.Str)
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
		if (str.Str && !qspIsEmpty(str)) // use qspIsEmpty only
		{
			args[1].IsStr = QSP_TRUE;
			QSP_STR(args[1]) = str;
			count = 2;
		}
		else
			count = 1;
		qspAddAction(args, count, loc->Actions[i].OnPressLines, 0, loc->Actions[i].OnPressLinesCount, QSP_TRUE);
		qspFreeString(QSP_STR(args[0]));
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

void qspExecLocByName(QSPString name, QSP_BOOL isChangeDesc)
{
	int locInd = qspLocIndex(name);
	if (locInd < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return;
	}
	qspExecLocByIndex(locInd, isChangeDesc, QSP_FALSE);
}

void qspExecLocByNameWithArgs(QSPString name, QSPVariant *args, int count, QSPVariant *res)
{
	int oldRefreshCount;
	QSPVar local, result, *varArgs, *varRes;
	QSPString resultName, argsName;
	resultName = QSP_STATIC_STR(QSP_VARRES);
	if (!(varRes = qspVarReference(resultName, QSP_TRUE))) return;
	argsName = QSP_STATIC_STR(QSP_VARARGS);
	if (!(varArgs = qspVarReference(argsName, QSP_TRUE))) return;
	qspMoveVar(&result, varRes);
	qspMoveVar(&local, varArgs);
	qspSetArgs(varArgs, args, count);
	oldRefreshCount = qspRefreshCount;
	qspExecLocByName(name, QSP_FALSE);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
	{
		qspEmptyVar(&local);
		qspEmptyVar(&result);
		return;
	}
	if (!(varArgs = qspVarReference(argsName, QSP_TRUE)))
	{
		qspEmptyVar(&local);
		qspEmptyVar(&result);
		return;
	}
	qspEmptyVar(varArgs);
	qspMoveVar(varArgs, &local);
	if (!(varRes = qspVarReference(resultName, QSP_TRUE)))
	{
		qspEmptyVar(&result);
		return;
	}
	if (res) qspApplyResult(varRes, res);
	qspEmptyVar(varRes);
	qspMoveVar(varRes, &result);
}

void qspExecLocByVarNameWithArgs(QSPString name, QSPVariant *args, int count)
{
	QSPVar *var;
	QSPString locName;
	int ind = 0, oldRefreshCount = qspRefreshCount;
	while (1)
	{
		if (!(var = qspVarReference(name, QSP_FALSE))) break;
		if (ind >= var->ValsCount) break;
		locName = var->Values[ind].Str;
		if (!(locName.Str && qspIsAnyString(locName))) break;
		qspExecLocByNameWithArgs(locName, args, count, 0);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
		++ind;
	}
}

void qspRefreshCurLoc(QSP_BOOL isChangeDesc, QSPVariant *args, int count)
{
	QSPVar *var;
	int oldRefreshCount;
	if (!(var = qspVarReference(QSP_STATIC_STR(QSP_VARARGS), QSP_TRUE))) return;
	qspEmptyVar(var);
	qspSetArgs(var, args, count);
	qspClearActions(QSP_FALSE);
	++qspRefreshCount;
	if (isChangeDesc) ++qspFullRefreshCount;
	oldRefreshCount = qspRefreshCount;
	qspExecLocByIndex(qspCurLoc, isChangeDesc, QSP_TRUE);
	if (qspErrorNum) return;
	if (qspRefreshCount == oldRefreshCount)
		qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_FMT("ONNEWLOC")), args, count);
}
