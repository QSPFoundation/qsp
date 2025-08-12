/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "objects.h"

#include "common.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "statements.h"
#include "text.h"
#include "variables.h"

QSPObj qspCurObjects[QSP_MAXOBJECTS];
int qspCurObjsCount = 0;
int qspCurSelObject = -1;
QSP_BOOL qspIsObjsListChanged = QSP_FALSE;
QSP_BOOL qspCurToShowObjs = QSP_TRUE;

INLINE void qspRemoveObjectByIndex(int index);
INLINE void qspSendRemovalNotifications(QSPString *objNames, int count);
INLINE void qspRemoveObjectByIndexWithEvent(int index);
INLINE void qspClearObjectsByNameWithEvents(QSPString name, int maxObjects);

void qspClearAllObjects(QSP_BOOL toInit)
{
    if (!toInit && qspCurObjsCount)
    {
        int i;
        for (i = 0; i < qspCurObjsCount; ++i)
        {
            qspFreeString(&qspCurObjects[i].Image);
            qspFreeString(&qspCurObjects[i].Desc);
        }
        qspIsObjsListChanged = QSP_TRUE;
    }
    qspCurObjsCount = 0;
    qspCurSelObject = -1;
}

INLINE void qspRemoveObjectByIndex(int index)
{
    if (index >= 0 && index < qspCurObjsCount)
    {
        if (qspCurSelObject >= index) qspCurSelObject = -1;
        qspFreeString(&qspCurObjects[index].Image);
        qspFreeString(&qspCurObjects[index].Desc);
        --qspCurObjsCount;
        while (index < qspCurObjsCount)
        {
            qspCurObjects[index] = qspCurObjects[index + 1];
            ++index;
        }
        qspIsObjsListChanged = QSP_TRUE;
    }
}

INLINE void qspSendRemovalNotifications(QSPString *objNames, int count)
{
    if (count >= 2)
    {
        QSPVariant objName;
        int i, oldLocationState;
        objName.Type = QSP_TYPE_STR;
        objName.IsRef = QSP_FALSE;
        oldLocationState = qspLocationState;
        for (i = 0; i < count; ++i)
        {
            QSP_STR(objName) = objNames[i];
            qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_OBJDELETED), &objName, 1);
            if (qspLocationState != oldLocationState) return;
        }
    }
    else if (count == 1)
    {
        QSPVariant objName = qspStrVariant(objNames[0], QSP_TYPE_STR);
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_OBJDELETED), &objName, 1);
    }
}

INLINE void qspRemoveObjectByIndexWithEvent(int index)
{
    if (index >= 0 && index < qspCurObjsCount)
    {
        QSPString objName = qspCopyToNewText(qspCurObjects[index].Desc);
        qspRemoveObjectByIndex(index);
        qspSendRemovalNotifications(&objName, 1);
        qspFreeString(&objName);
    }
}

void qspClearAllObjectsWithEvents(void)
{
    int objsCount = qspCurObjsCount;
    if (objsCount)
    {
        int i;
        QSPString *objNames = (QSPString *)malloc(objsCount * sizeof(QSPString));
        /* Add objects to the notification list */
        for (i = 0; i < objsCount; ++i)
            objNames[i] = qspCopyToNewText(qspCurObjects[i].Desc);
        /* Remove all objects */
        qspClearAllObjects(QSP_FALSE);
        qspSendRemovalNotifications(objNames, objsCount);
        qspFreeStrs(objNames, objsCount);
    }
}

INLINE void qspClearObjectsByNameWithEvents(QSPString name, int maxObjects)
{
    if (maxObjects > 0 && qspCurObjsCount)
    {
        QSPBufString buf;
        QSPString bufName;
        int i, objsCount = 0, objsBufSize = 4;
        QSPString *objNames = (QSPString *)malloc(objsBufSize * sizeof(QSPString));
        /* Prepare the name */
        name = qspCopyToNewText(name);
        qspUpperStr(&name);
        /* Fill the list with objects to remove */
        buf = qspNewBufString(32);
        i = 0;
        while (i < qspCurObjsCount && objsCount < maxObjects)
        {
            qspUpdateBufString(&buf, qspCurObjects[i].Desc);
            bufName = qspBufTextToString(buf);
            qspUpperStr(&bufName);
            if (!qspStrsCompare(bufName, name))
            {
                /* Add object to the notification list */
                if (objsCount >= objsBufSize)
                {
                    objsBufSize = objsCount + 8;
                    objNames = (QSPString *)realloc(objNames, objsBufSize * sizeof(QSPString));
                }
                objNames[objsCount++] = qspCopyToNewText(qspCurObjects[i].Desc);
                /* Remove the object & don't update the current index as the array shifts */
                qspRemoveObjectByIndex(i);
                continue;
            }
            ++i;
        }
        qspFreeString(&name);
        qspFreeBufString(&buf);
        qspSendRemovalNotifications(objNames, objsCount);
        qspFreeStrs(objNames, objsCount);
    }
}

int qspObjsCountByName(QSPString name)
{
    if (qspCurObjsCount)
    {
        int i, objsCount;
        QSPString bufName;
        QSPBufString buf;
        name = qspCopyToNewText(name);
        qspUpperStr(&name);
        buf = qspNewBufString(32);
        objsCount = 0;
        for (i = 0; i < qspCurObjsCount; ++i)
        {
            qspUpdateBufString(&buf, qspCurObjects[i].Desc);
            bufName = qspBufTextToString(buf);
            qspUpperStr(&bufName);
            if (!qspStrsCompare(bufName, name))
                ++objsCount;
        }
        qspFreeString(&name);
        qspFreeBufString(&buf);
        return objsCount;
    }
    return 0;
}

QSPString qspGetAllObjectsAsCode(void)
{
    int i;
    QSPString temp;
    QSPBufString res = qspNewBufString(256);
    for (i = 0; i < qspCurObjsCount; ++i)
    {
        qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT("ADDOBJ ") QSP_DEFQUOT));
        temp = qspReplaceText(qspCurObjects[i].Desc, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
        qspAddBufText(&res, temp);
        qspFreeNewString(&temp, &qspCurObjects[i].Desc);
        if (qspCurObjects[i].Image.Str)
        {
            qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(",") QSP_DEFQUOT));
            temp = qspReplaceText(qspCurObjects[i].Image, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
            qspAddBufText(&res, temp);
            qspFreeNewString(&temp, &qspCurObjects[i].Image);
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
        objInd = QSP_TOINT(QSP_NUM(args[2]) - 1);
        if (objInd < 0)
            objInd = 0;
        else if (objInd > qspCurObjsCount)
            objInd = qspCurObjsCount;
    }
    else
        objInd = qspCurObjsCount;
    if (qspCurObjsCount == QSP_MAXOBJECTS)
    {
        qspSetError(QSP_ERR_CANTADDOBJECT);
        return;
    }
    if (qspCurSelObject >= objInd) qspCurSelObject = -1;
    if (count >= 2 && qspIsAnyString(QSP_STR(args[1])))
        imgPath = qspCopyToNewText(QSP_STR(args[1]));
    else
        imgPath = qspNullString;
    /* Place the object at the specified position */
    for (i = qspCurObjsCount; i > objInd; --i)
        qspCurObjects[i] = qspCurObjects[i - 1];
    ++qspCurObjsCount;
    obj = qspCurObjects + objInd;
    obj->Image = imgPath;
    obj->Desc = qspCopyToNewText(QSP_STR(args[0]));
    qspIsObjsListChanged = QSP_TRUE;
    /* Send notification */
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_OBJADDED), args, count);
}

void qspStatementDelObj(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg)
{
    switch (extArg)
    {
    case qspStatDelObj:
        {
            int maxObjects = (count == 2 ? QSP_TOINT(QSP_NUM(args[1])) : 1);
            qspClearObjectsByNameWithEvents(QSP_STR(args[0]), maxObjects);
            break;
        }
    case qspStatKillObj:
        if (count)
        {
            int objInd = QSP_TOINT(QSP_NUM(args[0]) - 1);
            qspRemoveObjectByIndexWithEvent(objInd);
        }
        else
            qspClearAllObjectsWithEvents();
        break;
    }
}

void qspStatementUnSelect(QSPVariant *QSP_UNUSED(args), QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    qspCurSelObject = -1;
}
