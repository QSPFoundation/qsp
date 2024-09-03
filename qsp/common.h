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
#include "text.h"

#ifndef QSP_COMMONDEFINES
    #define QSP_COMMONDEFINES

    #define QSP_RANDMASK 0x7FFFFFFF

    extern QSP_BOOL qspIsDebug;
    extern int qspTimerInterval;

    extern QSPBufString qspCurDesc;
    extern QSPBufString qspCurVars;
    extern QSPString qspCurInput;
    extern QSPString qspViewPath;
    extern QSP_BOOL qspIsMainDescChanged;
    extern QSP_BOOL qspIsVarsDescChanged;
    extern QSP_BOOL qspCurToShowVars;
    extern QSP_BOOL qspCurToShowInput;

    /* External functions */
    void qspInitRuntime(void);
    void qspTerminateRuntime(void);
    void qspPrepareExecution(QSP_BOOL toInit);
    void qspMemClear(QSP_BOOL toInit);
    void qspSetSeed(unsigned int seed);
    int qspRand(void);

    INLINE int qspToInt(QSP_BIGINT val)
    {
        if (val > INT_MAX)
            return INT_MAX;
        if (val < INT_MIN)
            return INT_MIN;
        return val;
    }

#endif
