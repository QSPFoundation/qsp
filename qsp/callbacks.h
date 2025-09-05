/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "codetools.h"

#ifndef QSP_CALLSDEFINES
    #define QSP_CALLSDEFINES

    typedef struct
    {
        int LocationState;
        QSP_BOOL IsInCallback;
        QSP_TINYINT WindowsChangedState;
        int RealCurLoc;
        int RealActIndex;
        int RealLineNum;
        QSPLineOfCode *RealLine;
    } QSPCallState;

    extern QSP_CALLBACK qspCallbacks[QSP_CALL_DUMMY];
    extern QSP_BOOL qspIsInCallback;

    /* External functions */
    void qspPrepareCallback(QSPCallState *state, QSP_BOOL toRefreshUI);
    QSP_BOOL qspFinalizeCallback(QSPCallState *state, QSP_BOOL toResetLocationState);

    void qspInitCallbacks(void);
    void qspSetCallback(int type, QSP_CALLBACK func);

    void qspCallDebug(QSPString str);
    void qspCallSetTimer(int msecs);
    void qspCallRefreshInt(QSP_BOOL isForced);
    void qspCallSetInputStrText(QSPString text);
    void qspCallShowMessage(QSPString text);
    void qspCallShowPicture(QSPString file);
    void qspCallShowWindow(int type, QSP_BOOL toShow);
    void qspCallPlayFile(QSPString file, int volume);
    QSP_BOOL qspCallIsPlayingFile(QSPString file);
    void qspCallCloseFile(QSPString file);
    void qspCallSystem(QSPString cmd);
    void qspCallSleep(int msecs);
    int qspCallGetMSCount(void);
    void qspCallOpenGame(QSPString file, QSP_BOOL isNewGame);
    void qspCallOpenGameStatus(QSPString file);
    void qspCallSaveGameStatus(QSPString file);
    QSPString qspCallInputBox(QSPString text);
    int qspCallShowMenu(QSPListItem *items, int count);
    QSPString qspCallVersion(QSPString param);

#endif
