/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "codetools.h"

#ifndef QSP_LOCSDEFINES
    #define QSP_LOCSDEFINES

    #define QSP_MAXLOCCALLDEPTH 2000
    #define QSP_LOCSCAPACITY 32

    typedef struct
    {
        QSPString Image;
        QSPString Desc;
        QSPCodeBlock *OnPressCode;
    } QSPLocAct;

    typedef struct
    {
        QSPString Name;
        QSPString Desc; /* base description */
        QSPCodeBlock *OnVisitCode; /* location code */
        QSPLocAct *Actions; /* base actions */
        int ActionsCount;
        int RefsCount;
    } QSPLocation;

    typedef struct
    {
        QSPString Name;
        unsigned int NameHash;
        int Index;
    } QSPLocName;

    extern QSPLocation **qspLocs;
    extern int qspLocsCount;
    extern int qspLocsCapacity;
    extern QSPLocName *qspLocsNames; /* hash table by names */
    extern int qspLocsNamesCapacity; /* has to be a power of 2 */
    extern QSPLocation *qspCurLoc;
    extern int qspLocationState; /* allows to check if we have to terminate execution of the code */
    extern int qspFullRefreshCount;
    extern int qspCurLocCallDepth;

    /* External functions */
    void qspInitWorld(void);
    void qspTerminateWorld(void);
    void qspTruncateWorld(int locsCount);
    void qspFreeLocation(QSPLocation *loc);
    QSPLocation *qspAddLocation(QSPString name);
    QSPLocation *qspLocByName(QSPString name);
    void qspExecLocByNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount, QSP_BOOL toMoveArgs, QSPVariant *res);
    void qspExecLocByVarNameWithArgs(QSPString name, QSPVariant *args, QSP_TINYINT argsCount);
    void qspNavigateToLocation(QSPLocation *loc, QSP_BOOL toChangeDesc, QSPVariant *args, QSP_TINYINT argsCount);

    INLINE void qspAcquireLocation(QSPLocation *loc)
    {
        ++loc->RefsCount;
    }

    INLINE void qspReleaseLocation(QSPLocation *loc)
    {
        if (--loc->RefsCount == 0)
        {
            qspFreeLocation(loc);
            free(loc);
        }
    }

    INLINE void qspUpdateLocation(QSPLocation **dest, QSPLocation *loc)
    {
        if (loc) qspAcquireLocation(loc);
        if (*dest) qspReleaseLocation(*dest);
        *dest = loc;
    }

#endif
