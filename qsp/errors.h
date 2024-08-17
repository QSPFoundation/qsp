/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
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
#include "codetools.h"

#ifndef QSP_ERRSDEFINES
    #define QSP_ERRSDEFINES

    extern int qspErrorNum;
    extern int qspErrorLoc;
    extern int qspErrorActIndex; /* points to the base action */
    extern int qspErrorLineNum; /* points to the top-level line within the game code */
    extern int qspRealCurLoc;
    extern int qspRealActIndex; /* points to the base action */
    extern int qspRealLineNum; /* points to the top-level line within the game code */
    extern QSPLineOfCode *qspRealLine = 0; /* contains the internal details */

    /* External functions */
    void qspSetError(int num);
    void qspResetError();
    QSPString qspGetErrorDesc(int errorNum);

#endif
