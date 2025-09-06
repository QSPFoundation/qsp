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
QSPObjsGroup qspCurObjsGroups[QSP_MAXOBJECTS];
int qspCurObjsCount = 0;
int qspCurObjsGroupsCount = 0;
int qspCurSelObject = -1;

INLINE int qspObjsGroupCompare(const void *name, const void *compareTo);
INLINE int qspObjsGroupFloorCompare(const void *name, const void *compareTo);
INLINE int qspGetObjsGroupIndex(QSPString objName);
INLINE int qspAddObjsGroup(QSPString objName);
INLINE void qspRemoveObjsGroupByIndex(int index);
INLINE void qspRemoveObjectByIndex(int index);
INLINE void qspSendObjectsRemovalNotifications(QSPString *objNames, int count);
INLINE void qspRemoveObjectByIndexWithEvent(int index);
INLINE void qspClearObjectsByNameWithEvents(QSPString objName, int maxObjects);
INLINE void qspAddObjectWithEvent(QSPString objName, QSPString objImage, int objInd);
INLINE void qspUpdateObjsGroup(QSPString objName, QSPString objDesc, QSPString objImage, QSP_BOOL toUpdateImage);
INLINE void qspResetObjsGroupByIndex(int index);

INLINE int qspObjsGroupCompare(const void *name, const void *compareTo)
{
    return qspStrsCompare(*(QSPString *)name, ((QSPObjsGroup *)compareTo)->Name);
}

INLINE int qspObjsGroupFloorCompare(const void *name, const void *compareTo)
{
    QSPString key = *(QSPString *)name;
    QSPObjsGroup *objsGroup = (QSPObjsGroup *)compareTo;

    /* It's safe to check (item + 1) because the item never points to the last array item */
    if (qspStrsCompare(key, (objsGroup + 1)->Name) < 0 && qspStrsCompare(key, objsGroup->Name) >= 0)
        return 0;

    return qspStrsCompare(key, objsGroup->Name);
}

void qspClearAllObjects(QSP_BOOL toInit)
{
    if (!toInit && qspCurObjsCount)
    {
        int i;
        QSPObj *curObj;
        QSPObjsGroup *curObjsGroup;
        for (i = qspCurObjsCount, curObj = qspCurObjects; i > 0; --i, ++curObj)
        {
            qspFreeString(&curObj->Name);
            qspFreeString(&curObj->Image);
        }
        for (i = qspCurObjsGroupsCount, curObjsGroup = qspCurObjsGroups; i > 0; --i, ++curObjsGroup)
        {
            qspFreeString(&curObjsGroup->Name);
            qspFreeString(&curObjsGroup->Desc);
            qspFreeString(&curObjsGroup->Image);
        }
        qspCurWindowsChangedState |= QSP_WIN_OBJS;
    }
    qspCurObjsCount = 0;
    qspCurObjsGroupsCount = 0;
    qspCurSelObject = -1;
}

INLINE int qspGetObjsGroupIndex(QSPString objName)
{
    if (qspCurObjsGroupsCount)
    {
        QSPObjsGroup *objsGroup;
        objName = qspCopyToNewText(objName);
        qspUpperStr(&objName);
        objsGroup = (QSPObjsGroup *)bsearch(&objName, qspCurObjsGroups, qspCurObjsGroupsCount, sizeof(QSPObjsGroup), qspObjsGroupCompare);
        qspFreeString(&objName);
        if (objsGroup) return (int)(objsGroup - qspCurObjsGroups);
    }
    return -1;
}

INLINE int qspAddObjsGroup(QSPString objName)
{
    QSPObjsGroup *objsGroup;
    int floorItem, groupsCount = qspCurObjsGroupsCount;
    if (groupsCount == QSP_MAXOBJECTS)
    {
        qspSetError(QSP_ERR_CANTADDOBJECT);
        return -1;
    }
    objName = qspCopyToNewText(objName);
    qspUpperStr(&objName);
    floorItem = groupsCount - 1;
    if (groupsCount > 0)
    {
        /* Find the first item that's smaller than objName */
        objsGroup = qspCurObjsGroups + floorItem;
        if (qspStrsCompare(objName, objsGroup->Name) < 0)
        {
            objsGroup = (QSPObjsGroup *)bsearch(&objName, qspCurObjsGroups, floorItem, sizeof(QSPObjsGroup), qspObjsGroupFloorCompare);
            floorItem = (objsGroup ? (int)(objsGroup - qspCurObjsGroups) : -1);
        }
    }
    /* Shift existing items to allocate extra space */
    ++floorItem;
    while (groupsCount > floorItem)
    {
        qspCurObjsGroups[groupsCount] = qspCurObjsGroups[groupsCount - 1];
        --groupsCount;
    }
    /* Add new object group */
    objsGroup = qspCurObjsGroups + floorItem;
    objsGroup->Name = objName;
    objsGroup->Desc = qspNullString;
    objsGroup->Image = qspNullString;
    objsGroup->UpdatedFields = 0;
    objsGroup->ObjsCount = 0;
    ++qspCurObjsGroupsCount;
    return floorItem;
}

INLINE void qspRemoveObjsGroupByIndex(int index)
{
    if (index >= 0 && index < qspCurObjsGroupsCount)
    {
        QSPObjsGroup *objsGroup = qspCurObjsGroups + index;
        qspFreeString(&objsGroup->Name);
        qspFreeString(&objsGroup->Desc);
        qspFreeString(&objsGroup->Image);
        --qspCurObjsGroupsCount;
        while (index < qspCurObjsGroupsCount)
        {
            qspCurObjsGroups[index] = qspCurObjsGroups[index + 1];
            ++index;
        }
        qspCurWindowsChangedState |= QSP_WIN_OBJS;
    }
}

INLINE void qspRemoveObjectByIndex(int index)
{
    if (index >= 0 && index < qspCurObjsCount)
    {
        QSPObj *obj = qspCurObjects + index;
        int groupIndex = qspGetObjsGroupIndex(obj->Name);
        if (groupIndex >= 0)
        {
            QSPObjsGroup *objsGroup = qspCurObjsGroups + groupIndex;
            if (--objsGroup->ObjsCount <= 0)
                qspRemoveObjsGroupByIndex(groupIndex);
        }
        if (qspCurSelObject >= index) qspCurSelObject = -1;
        qspFreeString(&obj->Name);
        qspFreeString(&obj->Image);
        --qspCurObjsCount;
        while (index < qspCurObjsCount)
        {
            qspCurObjects[index] = qspCurObjects[index + 1];
            ++index;
        }
        qspCurWindowsChangedState |= QSP_WIN_OBJS;
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
    int groupIndex = qspGetObjsGroupIndex(objName);
    if (groupIndex >= 0)
    {
        QSPObjsGroup *objsGroup = qspCurObjsGroups + groupIndex;
        return objsGroup->ObjsCount;
    }
    return 0;
}

INLINE void qspAddObjectWithEvent(QSPString objName, QSPString objImage, int objInd)
{
    int i, groupIndex;
    QSPObj *obj;
    QSPVariant addedObjProps[2];
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
    /* Update object group */
    groupIndex = qspGetObjsGroupIndex(objName);
    if (groupIndex < 0)
    {
        /* Create object group for every object name */
        groupIndex = qspAddObjsGroup(objName);
        if (groupIndex < 0) return;
    }
    qspCurObjsGroups[groupIndex].ObjsCount++;
    /* Place the object at the specified position */
    for (i = qspCurObjsCount; i > objInd; --i)
        qspCurObjects[i] = qspCurObjects[i - 1];
    ++qspCurObjsCount;
    obj = qspCurObjects + objInd;
    obj->Name = qspCopyToNewText(objName);
    obj->Image = qspCopyToNewText(objImage);
    qspCurWindowsChangedState |= QSP_WIN_OBJS;
    /* Send notification */
    addedObjProps[0] = qspStrVariant(objName, QSP_TYPE_STR);
    addedObjProps[1] = qspStrVariant(objImage, QSP_TYPE_STR);
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_OBJADDED), addedObjProps, 2);
}

INLINE void qspUpdateObjsGroup(QSPString objName, QSPString objDesc, QSPString objImage, QSP_BOOL toUpdateImage)
{
    QSPObjsGroup *objsGroup;
    int groupIndex = qspGetObjsGroupIndex(objName);
    if (groupIndex < 0) return;
    objsGroup = qspCurObjsGroups + groupIndex;
    if (objsGroup->UpdatedFields & QSP_OBJUPDATED_DESC)
    {
        if (qspStrsCompare(objsGroup->Desc, objDesc))
        {
            qspUpdateText(&objsGroup->Desc, objDesc);
            qspCurWindowsChangedState |= QSP_WIN_OBJS;
        }
    }
    else
    {
        objsGroup->UpdatedFields |= QSP_OBJUPDATED_DESC;
        qspUpdateText(&objsGroup->Desc, objDesc);
        qspCurWindowsChangedState |= QSP_WIN_OBJS;
    }
    if (toUpdateImage)
    {
        if (objsGroup->UpdatedFields & QSP_OBJUPDATED_IMAGE)
        {
            if (qspStrsCompare(objsGroup->Image, objImage))
            {
                qspUpdateText(&objsGroup->Image, objImage);
                qspCurWindowsChangedState |= QSP_WIN_OBJS;
            }
        }
        else
        {
            objsGroup->UpdatedFields |= QSP_OBJUPDATED_IMAGE;
            qspUpdateText(&objsGroup->Image, objImage);
            qspCurWindowsChangedState |= QSP_WIN_OBJS;
        }
    }
}

INLINE void qspResetObjsGroupByIndex(int index)
{
    if (index >= 0 && index < qspCurObjsGroupsCount)
    {
        QSPObjsGroup *objsGroup = qspCurObjsGroups + index;
        if (objsGroup->UpdatedFields & QSP_OBJUPDATED_DESC)
        {
            qspClearText(&objsGroup->Desc);
            qspCurWindowsChangedState |= QSP_WIN_OBJS;
        }
        if (objsGroup->UpdatedFields & QSP_OBJUPDATED_IMAGE)
        {
            qspClearText(&objsGroup->Image);
            qspCurWindowsChangedState |= QSP_WIN_OBJS;
        }
        objsGroup->UpdatedFields = 0;
    }
}

QSPString qspGetAllObjectsAsCode(void)
{
    int i;
    QSPObj *curObj;
    QSPObjsGroup *curObjsGroup;
    QSPString temp;
    QSPBufString res = qspNewBufString(256);
    /* Add objects */
    curObj = qspCurObjects;
    for (i = qspCurObjsCount; i > 0; --i, ++curObj)
    {
        qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT("ADDOBJ ") QSP_DEFQUOT));
        temp = qspReplaceText(curObj->Name, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
        qspAddBufText(&res, temp);
        qspFreeNewString(&temp, &curObj->Name);
        if (curObj->Image.Str)
        {
            qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(", ") QSP_DEFQUOT));
            temp = qspReplaceText(curObj->Image, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
            qspAddBufText(&res, temp);
            qspFreeNewString(&temp, &curObj->Image);
        }
        qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_STRSDELIM));
    }
    /* Update object groups */
    curObjsGroup = qspCurObjsGroups;
    for (i = qspCurObjsGroupsCount; i > 0; --i, ++curObjsGroup)
    {
        /* Check for updates since object groups exist even for unmodified objects */
        if (curObjsGroup->UpdatedFields)
        {
            qspAddBufText(&res, QSP_STATIC_STR(QSP_FMT("MODOBJ ") QSP_DEFQUOT));
            temp = qspReplaceText(curObjsGroup->Name, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
            qspAddBufText(&res, temp);
            qspFreeNewString(&temp, &curObjsGroup->Name);
            qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(", ") QSP_DEFQUOT));
            temp = qspReplaceText(curObjsGroup->Desc, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
            qspAddBufText(&res, temp);
            qspFreeNewString(&temp, &curObjsGroup->Desc);
            if (curObjsGroup->UpdatedFields & QSP_OBJUPDATED_IMAGE)
            {
                qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_FMT(", ") QSP_DEFQUOT));
                temp = qspReplaceText(curObjsGroup->Image, QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
                qspAddBufText(&res, temp);
                qspFreeNewString(&temp, &curObjsGroup->Image);
            }
            qspAddBufText(&res, QSP_STATIC_STR(QSP_DEFQUOT QSP_STRSDELIM));
        }
    }
    return qspBufTextToString(res);
}

QSP_BOOL qspGetObjectInfoByIndex(int index, QSPObjectItem *info)
{
    if (index >= 0 && index < qspCurObjsCount)
    {
        QSPObj *obj = qspCurObjects + index;
        int groupIndex = qspGetObjsGroupIndex(obj->Name);
        if (groupIndex >= 0)
        {
            QSPObjsGroup *objsGroup = qspCurObjsGroups + groupIndex;
            info->Name = obj->Name;
            info->Title = ((objsGroup->UpdatedFields & QSP_OBJUPDATED_DESC) ? objsGroup->Desc : obj->Name);
            info->Image = ((objsGroup->UpdatedFields & QSP_OBJUPDATED_IMAGE) ? objsGroup->Image : obj->Image);
        }
        else
        {
            info->Name = obj->Name;
            info->Title = obj->Name;
            info->Image = obj->Image;
        }
        return QSP_TRUE;
    }
    return QSP_FALSE;
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
        qspUpdateObjsGroup(QSP_STR(args[0]), QSP_STR(args[1]), objImage, QSP_TRUE);
    }
    else
        qspUpdateObjsGroup(QSP_STR(args[0]), QSP_STR(args[1]), qspNullString, QSP_FALSE);
}

void qspStatementResetObj(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    int groupIndex;
    if (count)
    {
        groupIndex = qspGetObjsGroupIndex(QSP_STR(args[0]));
        if (groupIndex >= 0) qspResetObjsGroupByIndex(groupIndex);
    }
    else
    {
        for (groupIndex = 0; groupIndex < qspCurObjsGroupsCount; ++groupIndex)
            qspResetObjsGroupByIndex(groupIndex);
    }
}

void qspStatementUnSelect(QSPVariant *QSP_UNUSED(args), QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    qspCurSelObject = -1;
}
