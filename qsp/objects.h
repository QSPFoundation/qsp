/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "variant.h"

#ifndef QSP_OBJSDEFINES
    #define QSP_OBJSDEFINES

    #define QSP_MAXOBJECTS 1000

    typedef struct
    {
        QSPString Image;
        QSPString Desc;
    } QSPObj;

    extern QSPObj qspCurObjects[QSP_MAXOBJECTS];
    extern int qspCurObjsCount;
    extern int qspCurSelObject;
    extern QSP_BOOL qspIsObjsListChanged;
    extern QSP_BOOL qspCurToShowObjs;

    /* External functions */
    void qspClearAllObjects(QSP_BOOL toInit);
    void qspClearAllObjectsWithEvents(void);
    int qspObjsCountByName(QSPString name);
    QSPString qspGetAllObjectsAsCode(void);
    /* Statements */
    void qspStatementAddObject(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementDelObj(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementUnSelect(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

#endif
