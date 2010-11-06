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

#include "objects.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "text.h"
#include "variables.h"

QSPObj qspCurObjects[QSP_MAXOBJECTS];
int qspCurObjectsCount = 0;
int qspCurSelObject = -1;
QSP_BOOL qspIsObjectsChanged = QSP_FALSE;
QSP_BOOL qspCurIsShowObjs = QSP_TRUE;

static void qspRemoveObject(int);

void qspClearObjects(QSP_BOOL isFirst)
{
	int i;
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
	int i, oldRefreshCount, oldCount = qspCurObjectsCount;
	if (oldCount)
	{
		objs = (QSP_CHAR **)malloc(oldCount * sizeof(QSP_CHAR *));
		for (i = 0; i < oldCount; ++i)
			qspAddText(objs + i, qspCurObjects[i].Desc, 0, -1, QSP_TRUE);
		qspClearObjects(QSP_FALSE);
		v.IsStr = QSP_TRUE;
		oldRefreshCount = qspRefreshCount;
		for (i = 0; i < oldCount; ++i)
		{
			QSP_STR(v) = objs[i];
			qspExecLocByVarNameWithArgs(QSP_FMT("ONOBJDEL"), &v, 1);
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
		}
		qspFreeStrs(objs, oldCount);
	}
}

static void qspRemoveObject(int index)
{
	QSPVariant name;
	if (index < 0 || index >= qspCurObjectsCount) return;
	if (qspCurSelObject >= index) qspCurSelObject = -1;
	name.IsStr = QSP_TRUE;
	QSP_STR(name) = qspCurObjects[index].Desc;
	if (qspCurObjects[index].Image) free(qspCurObjects[index].Image);
	--qspCurObjectsCount;
	while (index < qspCurObjectsCount)
	{
		qspCurObjects[index] = qspCurObjects[index + 1];
		++index;
	}
	qspIsObjectsChanged = QSP_TRUE;
	qspExecLocByVarNameWithArgs(QSP_FMT("ONOBJDEL"), &name, 1);
	free(QSP_STR(name));
}

int qspObjIndex(QSP_CHAR *name)
{
	int i, objNameLen, bufSize;
	QSP_CHAR *uName, *buf;
	if (!qspCurObjectsCount) return -1;
	qspUpperStr(uName = qspGetNewText(name, -1));
	bufSize = 32;
	buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	for (i = 0; i < qspCurObjectsCount; ++i)
	{
		objNameLen = qspStrLen(qspCurObjects[i].Desc);
		if (objNameLen >= bufSize)
		{
			bufSize = objNameLen + 8;
			buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
		}
		qspStrCopy(buf, qspCurObjects[i].Desc);
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

QSP_BOOL qspStatementAddObject(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	QSPObj *obj;
	int i, objInd;
	QSP_CHAR *imgPath;
	if (count == 3)
	{
		objInd = QSP_NUM(args[2]) - 1;
		if (objInd < 0 || objInd > qspCurObjectsCount) return QSP_FALSE;
	}
	else
		objInd = qspCurObjectsCount;
	if (qspCurObjectsCount == QSP_MAXOBJECTS)
	{
		qspSetError(QSP_ERR_CANTADDOBJECT);
		return QSP_FALSE;
	}
	if (qspCurSelObject >= objInd) qspCurSelObject = -1;
	if (count >= 2 && qspIsAnyString(QSP_STR(args[1])))
		imgPath = qspGetAbsFromRelPath(QSP_STR(args[1]));
	else
		imgPath = 0;
	for (i = qspCurObjectsCount; i > objInd; --i)
		qspCurObjects[i] = qspCurObjects[i - 1];
	++qspCurObjectsCount;
	obj = qspCurObjects + objInd;
	obj->Image = imgPath;
	obj->Desc = qspGetNewText(QSP_STR(args[0]), -1);
	qspIsObjectsChanged = QSP_TRUE;
	if (count == 3) count = 2;
	qspExecLocByVarNameWithArgs(QSP_FMT("ONOBJADD"), args, count);
	return QSP_FALSE;
}

QSP_BOOL qspStatementDelObj(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	switch (extArg)
	{
	case 0:
		qspRemoveObject(qspObjIndex(QSP_STR(args[0])));
		break;
	case 1:
		if (count)
			qspRemoveObject(QSP_NUM(args[0]) - 1);
		else
			qspClearObjectsWithNotify();
		break;
	}
	return QSP_FALSE;
}

QSP_BOOL qspStatementUnSelect(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	qspCurSelObject = -1;
	return QSP_FALSE;
}
