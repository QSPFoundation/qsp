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
#include "variant.h"

#include "qsp_config.h"

#ifndef QSP_GAMEDEFINES
    #define QSP_GAMEDEFINES

    #define QSP_GAMEID QSP_FMT("QSPGAME")
    #define QSP_SAVEDGAMEID QSP_FMT("QSPSAVEDGAME")
    #define QSP_GAMEMINVER QSP_FMT(QSP_GAMEMINVER_VER_STR)
    #define QSP_MAXINCFILES 100
    #define QSP_DEFTIMERINTERVAL 500
    #define QSP_SAVEDGAMEDATAEXTRASPACE 8192

    extern int qspQstCRC;
    extern int qspCurIncLocsCount;

    /* External functions */
    void qspClearIncludes(QSP_BOOL);
    void qspNewGame(QSP_BOOL);
    QSP_BOOL qspOpenGame(void *data, int dataSize, QSP_BOOL isNewGame);
    QSP_BOOL qspSaveGameStatus(void *buf, int *bufSize);
    QSP_BOOL qspOpenGameStatus(void *data, int dataSize);
    /* Statements */
    QSP_BOOL qspStatementOpenQst(QSPVariant *args, QSP_TINYINT count, QSPString *jumpTo, QSP_TINYINT extArg);
    QSP_BOOL qspStatementOpenGame(QSPVariant *args, QSP_TINYINT count, QSPString *jumpTo, QSP_TINYINT extArg);
    QSP_BOOL qspStatementSaveGame(QSPVariant *args, QSP_TINYINT count, QSPString *jumpTo, QSP_TINYINT extArg);

#endif
