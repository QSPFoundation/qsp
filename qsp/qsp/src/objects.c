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

void qspClearObjects(QSP_BOOL isFirst)
{
	long i;
	if (!isFirst && qspCurObjectsCount)
	{
		for (i = 0; i < qspCurObjectsCount; ++i)
		{
			if (qspCurObjects[i].Image) free(qspCurObjects[i].Image);
			free(qspCurObjects[i].Desc);
		}
		qspIsObjectsChanged = QSP_TRUE;
	}
	qspCurObjectsCount = 0;
	qspCurSelObject = -1;
}

void qspClearObjectsWithNotify()
{
	QSPVariant v;
	QSP_CHAR **objs;
	long i, varInd, locInd, oldRefreshCount, oldCount = qspCurObjectsCount;
	if (oldCount)
	{
		locInd = qspLocIndexByVarName(QSP_STRCHAR QSP_FMT("ONOBJDEL"));
		if (locInd < 0)
		{
			qspClearObjects(QSP_FALSE);
			return;
		}
		objs = (QSP_CHAR **)malloc(oldCount * sizeof(QSP_CHAR *));
		for (i = 0; i < oldCount; ++i)
			qspAddText(objs + i, qspCurObjects[i].Desc, 0, -1, QSP_TRUE);
		qspClearObjects(QSP_FALSE);
		varInd = qspVarIndex(QSP_FMT("LASTOBJ"), QSP_TRUE);
		if (varInd < 0)
		{
			qspFreeStrs(objs, oldCount, QSP_FALSE);
			return;
		}
		v.IsStr = QSP_TRUE;
		oldRefreshCount = qspRefreshCount;
		for (i = 0; i < oldCount; ++i)
		{
			v.Str = objs[i];
			qspSetVarValueByIndex(varInd, 0, v, QSP_TRUE);
			if (qspErrorNum) break;
			qspExecLocByIndex(locInd, QSP_FALSE);
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
		}
		qspFreeStrs(objs, oldCount, QSP_FALSE);
	}
}

long qspObjIndex(QSP_CHAR *name)
{
	long i, objNameLen, bufSize;
	QSP_CHAR *uName, *buf;
	if (!qspCurObjectsCount) return -1;
	qspUpperStr(uName = qspGetNewText(name, -1));
	bufSize = 16;
	buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	for (i = 0; i < qspCurObjectsCount; ++i)
	{
		objNameLen = (long)QSP_STRLEN(qspCurObjects[i].Desc);
		if (objNameLen >= bufSize)
		{
			bufSize = objNameLen + 8;
			buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
		}
		QSP_STRCPY(buf, qspCurObjects[i].Desc);
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

QSP_BOOL qspStatementAddObject(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long varInd;
	QSPObj *obj;
	QSP_CHAR *imgPath;
	if (qspCurObjectsCount == QSP_MAXOBJECTS)
	{
		qspSetError(QSP_ERR_CANTADDOBJECT);
		return QSP_FALSE;
	}
	if (count == 2 && qspIsAnyString(args[1].Str))
	{
		imgPath = qspGetNewText(qspQstPath, qspQstPathLen);
		imgPath = qspGetAddText(imgPath, args[1].Str, qspQstPathLen, -1);
	}
	else
		imgPath = 0;
	obj = qspCurObjects + qspCurObjectsCount++;
	obj->Image = imgPath;
	obj->Desc = qspGetNewText(args[0].Str, -1);
	qspIsObjectsChanged = QSP_TRUE;
	varInd = qspVarIndex(QSP_FMT("LASTOBJ"), QSP_TRUE);
	if (varInd < 0) return QSP_FALSE;
	qspSetVarValueByIndex(varInd, 0, args[0], QSP_TRUE);
	if (qspErrorNum) return QSP_FALSE;
	qspExecLocByVarName(QSP_STRCHAR QSP_FMT("ONOBJADD"));
	return QSP_FALSE;
}

QSP_BOOL qspStatementDelObj(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long varInd, objInd = qspObjIndex(args[0].Str);
	if (objInd < 0) return QSP_FALSE;
	if (qspCurSelObject >= objInd) qspCurSelObject = -1;
	if (qspCurObjects[objInd].Image) free(qspCurObjects[objInd].Image);
	free(qspCurObjects[objInd].Desc);
	--qspCurObjectsCount;
	while (objInd < qspCurObjectsCount)
	{
		qspCurObjects[objInd] = qspCurObjects[objInd + 1];
		++objInd;
	}
	qspIsObjectsChanged = QSP_TRUE;
	varInd = qspVarIndex(QSP_FMT("LASTOBJ"), QSP_TRUE);
	if (varInd < 0) return QSP_FALSE;
	qspSetVarValueByIndex(varInd, 0, args[0], QSP_TRUE);
	if (qspErrorNum) return QSP_FALSE;
	qspExecLocByVarName(QSP_STRCHAR QSP_FMT("ONOBJDEL"));
	return QSP_FALSE;
}

QSP_BOOL qspStatementUnSelect(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	qspCurSelObject = -1;
	return QSP_FALSE;
}
