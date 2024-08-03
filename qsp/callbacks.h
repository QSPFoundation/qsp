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

#ifndef QSP_CALLSDEFINES
    #define QSP_CALLSDEFINES

    typedef struct
    {
        QSP_BOOL IsInCallBack;
        QSP_BOOL ToDisableCodeExec;
        QSP_BOOL IsMainDescChanged;
        QSP_BOOL IsVarsDescChanged;
        QSP_BOOL IsObjectsChanged;
        QSP_BOOL IsActionsChanged;
        int RealCurLoc;
        int RealLine;
        int RealActIndex;
    } QSPCallState;

    extern QSP_CALLBACK qspCallBacks[QSP_CALL_DUMMY];
    extern QSP_BOOL qspIsInCallBack;
    extern QSP_BOOL qspToDisableCodeExec;

    /* External functions */
    void qspPrepareCallBack(QSPCallState *state, QSP_BOOL toDisableCodeExec, QSP_BOOL toRefreshUI);
    void qspFinalizeCallBack(QSPCallState *state);

    void qspInitCallBacks();
    void qspSetCallBack(int type, QSP_CALLBACK func);

    void qspCallDebug(QSPString str);
    void qspCallSetTimer(int msecs);
    void qspCallRefreshInt(QSP_BOOL isForced);
    void qspCallSetInputStrText(QSPString text);
    void qspCallShowMessage(QSPString text);
    void qspCallShowPicture(QSPString file);
    int qspCallShowMenu(QSPListItem *items, int count);
    void qspCallPlayFile(QSPString file, int volume);
    QSP_BOOL qspCallIsPlayingFile(QSPString file);
    void qspCallCloseFile(QSPString file);
    void qspCallSystem(QSPString cmd);
    void qspCallSleep(int msecs);
    int qspCallGetMSCount();
    void qspCallOpenGame(QSPString file, QSP_BOOL isNewGame);
    void qspCallOpenGameStatus(QSPString file);
    void qspCallSaveGameStatus(QSPString file);
    QSPString qspCallInputBox(QSPString text);
    int qspCallShowMenu(QSPListItem *items, int count);
    QSPString qspCallVersion(QSPString param);

#endif
