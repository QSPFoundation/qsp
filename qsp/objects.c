/* Copyright (C) 2001-2024 Val Argunov (byte AT qsp DOT org) */
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
#include "statements.h"
#include "text.h"
#include "variables.h"

QSPObj qspCurObjects[QSP_MAXOBJECTS];
int qspCurObjectsCount = 0;
int qspCurSelObject = -1;
QSP_BOOL qspIsObjectsChanged = QSP_FALSE;
QSP_BOOL qspCurToShowObjs = QSP_TRUE;

INLINE void qspRemoveObject(int);

void qspClearAllObjects(QSP_BOOL toInit)
{
    if (!toInit && qspCurObjectsCount)
    {
        int i;
        for (i = 0; i < qspCurObjectsCount; ++i)
        {
            qspFreeString(&qspCurObjects[i].Image);
            qspFreeString(&qspCurObjects[i].Desc);
        }
        qspIsObjectsChanged = QSP_TRUE;
    }
    qspCurObjectsCount = 0;
    qspCurSelObject = -1;
}

void qspClearAllObjectsWithNotify(void)
{
    QSPVariant v;
    QSPString *objs;
    int i, oldRefreshCount, oldCount = qspCurObjectsCount;
    if (oldCount)
    {
        objs = (QSPString *)malloc(oldCount * sizeof(QSPString));
        for (i = 0; i < oldCount; ++i)
            qspAddText(objs + i, qspCurObjects[i].Desc, QSP_TRUE);
        qspClearAllObjects(QSP_FALSE);
        v.Type = QSP_TYPE_STR;
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

INLINE void qspRemoveObject(int index)
{
    QSPVariant name;
    if (index < 0 || index >= qspCurObjectsCount) return;
    if (qspCurSelObject >= index) qspCurSelObject = -1;
    name.Type = QSP_TYPE_STR;
    QSP_STR(name) = qspCurObjects[index].Desc;
    qspFreeString(&qspCurObjects[index].Image);
    --qspCurObjectsCount;
    while (index < qspCurObjectsCount)
    {
        qspCurObjects[index] = qspCurObjects[index + 1];
        ++index;
    }
    qspIsObjectsChanged = QSP_TRUE;
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_FMT("ONOBJDEL")), &name, 1);
    qspFreeString(&QSP_STR(name));
}

int qspObjIndex(QSPString name)
{
    QSPString bufName;
    int i, objNameLen, bufSize;
    QSP_CHAR *buf;
    if (!qspCurObjectsCount) return -1;
    name = qspCopyToNewText(name);
    qspUpperStr(&name);
    bufSize = 32;
    buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
    for (i = 0; i < qspCurObjectsCount; ++i)
    {
        objNameLen = qspStrLen(qspCurObjects[i].Desc);
        if (objNameLen)
        {
            if (objNameLen > bufSize)
            {
                bufSize = objNameLen + 8;
                buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
            }
            memcpy(buf, qspCurObjects[i].Desc.Str, objNameLen * sizeof(QSP_CHAR));
        }
        bufName = qspStringFromLen(buf, objNameLen);
        qspUpperStr(&bufName);
        if (!qspStrsComp(bufName, name))
        {
            qspFreeString(&name);
            free(buf);
            return i;
        }
    }
    qspFreeString(&name);
    free(buf);
    return -1;
}

QSPString qspGetAllObjectsAsCode(void)
{
    int i;
    QSPString temp;
    QSPBufString res = qspNewBufString(256);
    for (i = 0; i < qspCurObjectsCount; ++i)
    {
        qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT("ADDOBJ ") QSP_DEFQUOT));
        temp = qspReplaceText(qspCurObjects[i].Desc, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT));
        qspAddBufText(&res, temp);
        qspFreeString(&temp);
        if (qspCurObjects[i].Image.Str)
        {
            qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(",") QSP_DEFQUOT));
            temp = qspReplaceText(qspCurObjects[i].Image, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT));
            qspAddBufText(&res, temp);
            qspFreeString(&temp);
        }
        qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_STRSDELIM));
    }
    return qspBufTextToString(res);
}

void qspStatementAddObject(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPObj *obj;
    int i, objInd;
    QSPString imgPath;
    if (count == 3)
    {
        objInd = QSP_NUM(args[2]) - 1;
        if (objInd < 0 || objInd > qspCurObjectsCount) return;
    }
    else
        objInd = qspCurObjectsCount;
    if (qspCurObjectsCount == QSP_MAXOBJECTS)
    {
        qspSetError(QSP_ERR_CANTADDOBJECT);
        return;
    }
    if (qspCurSelObject >= objInd) qspCurSelObject = -1;
    if (count >= 2 && qspIsAnyString(QSP_STR(args[1])))
        imgPath = qspCopyToNewText(QSP_STR(args[1]));
    else
        imgPath = qspNullString;
    for (i = qspCurObjectsCount; i > objInd; --i)
        qspCurObjects[i] = qspCurObjects[i - 1];
    ++qspCurObjectsCount;
    obj = qspCurObjects + objInd;
    obj->Image = imgPath;
    obj->Desc = qspCopyToNewText(QSP_STR(args[0]));
    qspIsObjectsChanged = QSP_TRUE;
    if (count == 3) count = 2;
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_FMT("ONOBJADD")), args, count);
}

void qspStatementDelObj(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg)
{
    switch (extArg)
    {
    case qspStatDelObj:
        qspRemoveObject(qspObjIndex(QSP_STR(args[0])));
        break;
    case qspStatKillObj:
        if (count)
            qspRemoveObject(QSP_NUM(args[0]) - 1);
        else
            qspClearAllObjectsWithNotify();
        break;
    }
}

void qspStatementUnSelect(QSPVariant *QSP_UNUSED(args), QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    qspCurSelObject = -1;
}
