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
			qspFreeString(qspCurObjects[i].Image);
			qspFreeString(qspCurObjects[i].Desc);
		}
		qspIsObjectsChanged = QSP_TRUE;
	}
	qspCurObjectsCount = 0;
	qspCurSelObject = -1;
}

void qspClearObjectsWithNotify()
{
	QSPVariant v;
	QSPString *objs;
	int i, oldRefreshCount, oldCount = qspCurObjectsCount;
	if (oldCount)
	{
		objs = (QSPString *)malloc(oldCount * sizeof(QSPString));
		for (i = 0; i < oldCount; ++i)
			qspAddText(objs + i, qspCurObjects[i].Desc, QSP_TRUE);
		qspClearObjects(QSP_FALSE);
		v.IsStr = QSP_TRUE;
		oldRefreshCount = qspRefreshCount;
		for (i = 0; i < oldCount; ++i)
		{
			QSP_STR(v) = objs[i];
			qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_FMT("ONOBJDEL")), &v, 1);
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
	qspFreeString(qspCurObjects[index].Image);
	--qspCurObjectsCount;
	while (index < qspCurObjectsCount)
	{
		qspCurObjects[index] = qspCurObjects[index + 1];
		++index;
	}
	qspIsObjectsChanged = QSP_TRUE;
	qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_FMT("ONOBJDEL")), &name, 1);
	qspFreeString(QSP_STR(name));
}

int qspObjIndex(QSPString name)
{
	QSPString locName;
	int i, objNameLen, bufSize;
	QSP_CHAR *buf;
	if (!qspCurObjectsCount) return -1;
	name = qspGetNewText(name);
	qspUpperStr(&name);
	bufSize = 32;
	buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	for (i = 0; i < qspCurObjectsCount; ++i)
	{
		objNameLen = qspStrLen(qspCurObjects[i].Desc);
		if (objNameLen > bufSize)
		{
			bufSize = objNameLen + 8;
			buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
		}
		memcpy(buf, qspCurObjects[i].Desc.Str, objNameLen * sizeof(QSP_CHAR));
		locName = qspStringFromLen(buf, objNameLen);
		qspUpperStr(&locName);
		if (!qspStrsComp(locName, name))
		{
			free(name.Str);
			free(buf);
			return i;
		}
	}
	free(name.Str);
	free(buf);
	return -1;
}

QSP_BOOL qspStatementAddObject(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	QSPObj *obj;
	int i, objInd;
	QSPString imgPath;
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
		imgPath = qspNullString;
	for (i = qspCurObjectsCount; i > objInd; --i)
		qspCurObjects[i] = qspCurObjects[i - 1];
	++qspCurObjectsCount;
	obj = qspCurObjects + objInd;
	obj->Image = imgPath;
	obj->Desc = qspGetNewText(QSP_STR(args[0]));
	qspIsObjectsChanged = QSP_TRUE;
	if (count == 3) count = 2;
	qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_FMT("ONOBJADD")), args, count);
	return QSP_FALSE;
}

QSP_BOOL qspStatementDelObj(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
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

QSP_BOOL qspStatementUnSelect(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	qspCurSelObject = -1;
	return QSP_FALSE;
}
