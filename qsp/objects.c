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
INLINE void qspSendObjectsRemovalNotifications(QSPString *objNames, int count);
INLINE void qspRemoveObjectByIndexWithEvent(int index);
INLINE void qspClearObjectsByNameWithEvents(QSPString objName, int maxObjects);
INLINE void qspAddObjectWithEvent(QSPString objName, QSPString objImage, int objInd);
INLINE void qspUpdateObjectsByName(QSPString objName, QSPString objDesc, QSPString objImage, QSP_BOOL toUpdateImage);

void qspClearAllObjects(QSP_BOOL toInit)
{
    if (!toInit && qspCurObjsCount)
    {
        int i;
        QSPObj *curObj = qspCurObjects;
        for (i = qspCurObjsCount; i > 0; --i, ++curObj)
        {
            qspFreeString(&curObj->Name);
            qspFreeString(&curObj->Desc);
            qspFreeString(&curObj->Image);
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
        qspFreeString(&qspCurObjects[index].Name);
        qspFreeString(&qspCurObjects[index].Desc);
        qspFreeString(&qspCurObjects[index].Image);
        --qspCurObjsCount;
        while (index < qspCurObjsCount)
        {
            qspCurObjects[index] = qspCurObjects[index + 1];
            ++index;
        }
        qspIsObjsListChanged = QSP_TRUE;
    }
}

INLINE void qspSendObjectsRemovalNotifications(QSPString *objNames, int count)
{
    if (count >= 2)
    {
        QSPVariant removedObjName;
        int i, oldLocationState;
        removedObjName.Type = QSP_TYPE_STR;
        oldLocationState = qspLocationState;
        for (i = 0; i < count; ++i)
        {
            QSP_STR(removedObjName) = objNames[i];
            qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_OBJDELETED), &removedObjName, 1);
            if (qspLocationState != oldLocationState) return;
        }
    }
    else if (count == 1)
    {
        QSPVariant removedObjName = qspStrVariant(objNames[0], QSP_TYPE_STR);
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_OBJDELETED), &removedObjName, 1);
    }
}

INLINE void qspRemoveObjectByIndexWithEvent(int index)
{
    if (index >= 0 && index < qspCurObjsCount)
    {
        QSPString objName = qspCopyToNewText(qspCurObjects[index].Name);
        qspRemoveObjectByIndex(index);
        qspSendObjectsRemovalNotifications(&objName, 1);
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
            objNames[i] = qspCopyToNewText(qspCurObjects[i].Name);
        /* Remove all objects */
        qspClearAllObjects(QSP_FALSE);
        qspSendObjectsRemovalNotifications(objNames, objsCount);
        qspFreeStrs(objNames, objsCount);
    }
}

INLINE void qspClearObjectsByNameWithEvents(QSPString objName, int maxObjects)
{
    if (maxObjects > 0 && qspCurObjsCount)
    {
        QSPBufString buf;
        QSPString bufName;
        int i, objsCount = 0, objsBufSize = 4;
        QSPString *objNames = (QSPString *)malloc(objsBufSize * sizeof(QSPString));
        /* Prepare the name */
        objName = qspCopyToNewText(objName);
        qspUpperStr(&objName);
        /* Fill the list with objects to remove */
        buf = qspNewBufString(32);
        i = 0;
        while (i < qspCurObjsCount && objsCount < maxObjects)
        {
            qspUpdateBufString(&buf, qspCurObjects[i].Name);
            bufName = qspBufTextToString(buf);
            qspUpperStr(&bufName);
            if (!qspStrsCompare(bufName, objName))
            {
                /* Add object to the notification list */
                if (objsCount >= objsBufSize)
                {
                    objsBufSize = objsCount + 8;
                    objNames = (QSPString *)realloc(objNames, objsBufSize * sizeof(QSPString));
                }
                objNames[objsCount++] = qspCopyToNewText(qspCurObjects[i].Name);
                /* Remove the object & don't update the current index as the array shifts */
                qspRemoveObjectByIndex(i);
                continue;
            }
            ++i;
        }
        qspFreeString(&objName);
        qspFreeBufString(&buf);
        qspSendObjectsRemovalNotifications(objNames, objsCount);
        qspFreeStrs(objNames, objsCount);
    }
}

int qspObjsCountByName(QSPString objName)
{
    if (qspCurObjsCount)
    {
        int i, objsCount;
        QSPString bufName;
        QSPBufString buf;
        objName = qspCopyToNewText(objName);
        qspUpperStr(&objName);
        buf = qspNewBufString(32);
        objsCount = 0;
        for (i = 0; i < qspCurObjsCount; ++i)
        {
            qspUpdateBufString(&buf, qspCurObjects[i].Name);
            bufName = qspBufTextToString(buf);
            qspUpperStr(&bufName);
            if (!qspStrsCompare(bufName, objName))
                ++objsCount;
        }
        qspFreeString(&objName);
        qspFreeBufString(&buf);
        return objsCount;
    }
    return 0;
}

INLINE void qspAddObjectWithEvent(QSPString objName, QSPString objImage, int objInd)
{
    int i;
    QSPObj *obj;
    QSPVariant addedObjName;
    if (qspCurObjsCount == QSP_MAXOBJECTS)
    {
        qspSetError(QSP_ERR_CANTADDOBJECT);
        return;
    }
    if (objInd < 0)
        objInd = 0;
    else if (objInd > qspCurObjsCount)
        objInd = qspCurObjsCount;
    if (qspCurSelObject >= objInd) qspCurSelObject = -1;
    /* Place the object at the specified position */
    for (i = qspCurObjsCount; i > objInd; --i)
        qspCurObjects[i] = qspCurObjects[i - 1];
    ++qspCurObjsCount;
    obj = qspCurObjects + objInd;
    obj->Name = qspCopyToNewText(objName);
    obj->Desc = qspCopyToNewText(objName);
    obj->Image = qspCopyToNewText(objImage);
    qspIsObjsListChanged = QSP_TRUE;
    /* Send notification */
    addedObjName = qspStrVariant(objName, QSP_TYPE_STR);
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_OBJADDED), &addedObjName, 1);
}

INLINE void qspUpdateObjectsByName(QSPString objName, QSPString objDesc, QSPString objImage, QSP_BOOL toUpdateImage)
{
    if (qspCurObjsCount)
    {
        int i;
        QSPObj *curObj;
        QSPString bufName;
        QSPBufString buf;
        objName = qspCopyToNewText(objName);
        qspUpperStr(&objName);
        buf = qspNewBufString(32);
        curObj = qspCurObjects;
        for (i = qspCurObjsCount; i > 0; --i, ++curObj)
        {
            qspUpdateBufString(&buf, curObj->Name);
            bufName = qspBufTextToString(buf);
            qspUpperStr(&bufName);
            if (!qspStrsCompare(bufName, objName))
            {
                if (qspStrsCompare(curObj->Desc, objDesc))
                {
                    qspUpdateText(&curObj->Desc, objDesc);
                    qspIsObjsListChanged = QSP_TRUE;
                }
                if (toUpdateImage && qspStrsCompare(curObj->Image, objImage))
                {
                    qspUpdateText(&curObj->Image, objImage);
                    qspIsObjsListChanged = QSP_TRUE;
                }
            }
        }
        qspFreeString(&objName);
        qspFreeBufString(&buf);
    }
}

QSPString qspGetAllObjectsAsCode(void)
{
    int i;
    QSPObj *curObj;
    QSPString objName, temp;
    QSPBufString res = qspNewBufString(256);
    curObj = qspCurObjects;
    for (i = qspCurObjsCount; i > 0; --i, ++curObj)
    {
        /* Add the object */
        objName = qspReplaceText(curObj->Name, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
        qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT("ADDOBJ ") QSP_DEFQUOT));
        qspAddBufText(&res, objName);
        if (curObj->Image.Str)
        {
            qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(", ") QSP_DEFQUOT));
            temp = qspReplaceText(curObj->Image, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
            qspAddBufText(&res, temp);
            qspFreeNewString(&temp, &curObj->Image);
        }
        qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_STRSDELIM));
        /* Assign the description if it's not the same as the name */
        if (qspStrsCompare(curObj->Name, curObj->Desc))
        {
            qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT("MODOBJ ") QSP_DEFQUOT));
            qspAddBufText(&res, objName);
            qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(", ") QSP_DEFQUOT));
            temp = qspReplaceText(curObj->Desc, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
            qspAddBufText(&res, temp);
            qspFreeNewString(&temp, &curObj->Desc);
            qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_STRSDELIM));
        }
        qspFreeNewString(&objName, &curObj->Name);
    }
    return qspBufTextToString(res);
}

void qspStatementAddObject(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (count >= 2)
    {
        QSPString objImage = (qspIsAnyString(QSP_STR(args[1])) ? QSP_STR(args[1]) : qspNullString);
        int objInd = (count == 3 ? QSP_TOINT(QSP_NUM(args[2]) - 1) : qspCurObjsCount);
        qspAddObjectWithEvent(QSP_STR(args[0]), objImage, objInd);
    }
    else
        qspAddObjectWithEvent(QSP_STR(args[0]), qspNullString, qspCurObjsCount);
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

void qspStatementModObj(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (count == 3)
    {
        QSPString objImage = (qspIsAnyString(QSP_STR(args[2])) ? QSP_STR(args[2]) : qspNullString);
        qspUpdateObjectsByName(QSP_STR(args[0]), QSP_STR(args[1]), objImage, QSP_TRUE);
    }
    else
        qspUpdateObjectsByName(QSP_STR(args[0]), QSP_STR(args[1]), qspNullString, QSP_FALSE);
}

void qspStatementUnSelect(QSPVariant *QSP_UNUSED(args), QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    qspCurSelObject = -1;
}
