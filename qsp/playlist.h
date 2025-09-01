/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"

#ifndef QSP_PLAYLISTDEFINES
    #define QSP_PLAYLISTDEFINES

    #define QSP_PLVOLUMEDELIM QSP_FMT("*")
    QSP_DEFINE_SPECIAL_CHAR(QSP_PLVOLUMEDELIM, QSP_FMT('*'))

    #define QSP_MAXPLFILES 500

    extern QSPString qspPLFiles[QSP_MAXPLFILES];
    extern int qspPLFilesCount;

    /* External functions */
    void qspClearPlayList(QSP_BOOL toInit);
    void qspPlayPLFiles(void);
    void qspRefreshPlayList(void);
    /* Statements */
    void qspStatementPlayFile(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementCloseFile(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

#endif
