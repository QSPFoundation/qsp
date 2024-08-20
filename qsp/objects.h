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
    extern int qspCurObjectsCount;
    extern int qspCurSelObject;
    extern QSP_BOOL qspIsObjectsChanged;
    extern QSP_BOOL qspCurToShowObjs;

    /* External functions */
    void qspClearAllObjects(QSP_BOOL toInit);
    void qspClearAllObjectsWithNotify(void);
    int qspObjIndex(QSPString name);
    QSPString qspGetAllObjectsAsCode(void);
    /* Statements */
    void qspStatementAddObject(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementDelObj(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementUnSelect(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

#endif
