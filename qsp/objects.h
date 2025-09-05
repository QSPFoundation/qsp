/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"

#ifndef QSP_OBJSDEFINES
    #define QSP_OBJSDEFINES

    #define QSP_MAXOBJECTS 1000

    typedef struct
    {
        QSPString Name;
        QSPString Image;
    } QSPObj;

    enum
    {
        QSP_OBJUPDATED_DESC = 1 << 0,
        QSP_OBJUPDATED_IMAGE = 1 << 1
    };

    typedef struct
    {
        QSPString Name;
        QSPString Desc;
        QSPString Image;
        QSP_TINYINT UpdatedFields;
        int ObjsCount;
    } QSPObjsGroup;

    extern QSPObj qspCurObjects[QSP_MAXOBJECTS];
    extern QSPObjsGroup qspCurObjsGroups[QSP_MAXOBJECTS];
    extern int qspCurObjsCount;
    extern int qspCurObjsGroupsCount;
    extern int qspCurSelObject;
    extern QSP_BOOL qspIsObjsListChanged;

    /* External functions */
    void qspClearAllObjects(QSP_BOOL toInit);
    void qspClearAllObjectsWithEvents(void);
    int qspObjsCountByName(QSPString objName);
    QSPString qspGetAllObjectsAsCode(void);
    QSP_BOOL qspGetObjectInfoByIndex(int index, QSPObjectItem *info);
    /* Statements */
    void qspStatementAddObject(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementDelObj(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementModObj(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementUnSelect(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

#endif
