/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "qsp_config.h"

#ifndef QSP_GAMEDEFINES
    #define QSP_GAMEDEFINES

    #define QSP_GAMEID QSP_FMT("QSPGAME")
    #define QSP_SAVEDGAMEID QSP_FMT("QSPSAVEDGAME")
    #define QSP_GAMEMINVER QSP_FMT(QSP_GAMEMINVER_STR)
    #define QSP_MAXINCFILES 100
    #define QSP_DEFTIMERINTERVAL 500
    #define QSP_SAVEDGAMEDATAEXTRASPACE 8192

    extern int qspQstCRC;
    extern int qspCurIncLocsCount;

    /* External functions */
    void qspClearAllIncludes(QSP_BOOL toInit);
    QSP_BOOL qspNewGame(QSP_BOOL toReset);
    QSP_BOOL qspOpenGame(void *data, int dataSize, QSP_BOOL isNewGame);
    QSP_BOOL qspSaveGameStatus(void *buf, int *bufSize, QSP_BOOL isUCS);
    QSP_BOOL qspOpenGameStatus(void *data, int dataSize);
    /* Statements */
    void qspStatementOpenQst(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementOpenGame(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementSaveGame(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

#endif
