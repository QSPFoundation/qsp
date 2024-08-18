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

#include "../../declarations.h"

#ifdef _DEFAULT_BINDING

#include "../../callbacks.h"
#include "../../actions.h"
#include "../../coding.h"
#include "../../common.h"
#include "../../errors.h"
#include "../../objects.h"
#include "../../text.h"

void qspInitCallBacks(void)
{
    int i;
    qspIsInCallBack = QSP_FALSE;
    qspToDisableCodeExec = QSP_FALSE;
    for (i = 0; i < QSP_CALL_DUMMY; ++i)
        qspCallBacks[i] = 0;
}

void qspSetCallBack(int type, QSP_CALLBACK func)
{
    qspCallBacks[type] = func;
}

void qspCallDebug(QSPString str)
{
    /* Jump into the debugger */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_DEBUG])
    {
        qspPrepareCallBack(&state, QSP_FALSE, QSP_FALSE);
        qspCallBacks[QSP_CALL_DEBUG](str);
        qspFinalizeCallBack(&state);
        qspResetError(QSP_FALSE);
    }
}

void qspCallSetTimer(int msecs)
{
    /* Set timer interval */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SETTIMER])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SETTIMER](msecs);
        qspFinalizeCallBack(&state);
    }
}

void qspCallRefreshInt(QSP_BOOL isForced)
{
    /* Refresh UI to show the latest state */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_REFRESHINT])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_REFRESHINT](isForced);
        qspFinalizeCallBack(&state);
    }
}

void qspCallSetInputStrText(QSPString text)
{
    /* Set value of the text input control */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SETINPUTSTRTEXT])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SETINPUTSTRTEXT](text);
        qspFinalizeCallBack(&state);
    }
}

void qspCallSystem(QSPString cmd)
{
    /* Execute system call */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SYSTEM])
    {
        qspPrepareCallBack(&state, QSP_FALSE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SYSTEM](cmd);
        qspFinalizeCallBack(&state);
    }
}

void qspCallOpenGame(QSPString file, QSP_BOOL isNewGame)
{
    /* Open game file */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_OPENGAME])
    {
        qspPrepareCallBack(&state, QSP_FALSE, QSP_FALSE);
        qspCallBacks[QSP_CALL_OPENGAME](file, isNewGame);
        qspFinalizeCallBack(&state);
    }
}

void qspCallOpenGameStatus(QSPString file)
{
    /* Open game state (showing a dialog to choose a file) */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_OPENGAMESTATUS])
    {
        qspPrepareCallBack(&state, QSP_FALSE, QSP_FALSE);
        qspCallBacks[QSP_CALL_OPENGAMESTATUS](file);
        qspFinalizeCallBack(&state);
    }
}

void qspCallSaveGameStatus(QSPString file)
{
    /* Save game state (showing a dialog to choose a file) */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SAVEGAMESTATUS])
    {
        qspPrepareCallBack(&state, QSP_FALSE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SAVEGAMESTATUS](file);
        qspFinalizeCallBack(&state);
    }
}

void qspCallShowMessage(QSPString text)
{
    /* Show a message */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SHOWMSGSTR])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_TRUE);
        qspCallBacks[QSP_CALL_SHOWMSGSTR](text);
        qspFinalizeCallBack(&state);
    }
}

int qspCallShowMenu(QSPListItem *items, int count)
{
    /* Show a menu */
    QSPCallState state;
    int index;
    if (qspCallBacks[QSP_CALL_SHOWMENU])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_TRUE);
        index = qspCallBacks[QSP_CALL_SHOWMENU](items, count);
        qspFinalizeCallBack(&state);
        return index;
    }
    return -1;
}

void qspCallShowPicture(QSPString file)
{
    /* Show an image */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SHOWIMAGE])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SHOWIMAGE](file);
        qspFinalizeCallBack(&state);
    }
}

void qspCallShowWindow(int type, QSP_BOOL toShow)
{
    /* Show (hide) a region of the UI */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SHOWWINDOW])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SHOWWINDOW](type, toShow);
        qspFinalizeCallBack(&state);
    }
}

void qspCallPlayFile(QSPString file, int volume)
{
    /* Start playing a music file */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_PLAYFILE])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_PLAYFILE](file, volume);
        qspFinalizeCallBack(&state);
    }
}

QSP_BOOL qspCallIsPlayingFile(QSPString file)
{
    /* Check whether a file is still playing */
    QSPCallState state;
    QSP_BOOL isPlaying;
    if (qspCallBacks[QSP_CALL_ISPLAYINGFILE])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_FALSE);
        isPlaying = (QSP_BOOL)qspCallBacks[QSP_CALL_ISPLAYINGFILE](file);
        qspFinalizeCallBack(&state);
        return isPlaying;
    }
    return QSP_FALSE;
}

void qspCallCloseFile(QSPString file)
{
    /* Stop playing a file */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_CLOSEFILE])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_CLOSEFILE](file);
        qspFinalizeCallBack(&state);
    }
}

void qspCallSleep(int msecs)
{
    /* Wait for the specified number of milliseconds */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SLEEP])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_TRUE);
        qspCallBacks[QSP_CALL_SLEEP](msecs);
        qspFinalizeCallBack(&state);
    }
}

int qspCallGetMSCount(void)
{
    /* Get the number of milliseconds since the last call of this function */
    QSPCallState state;
    int count;
    if (qspCallBacks[QSP_CALL_GETMSCOUNT])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_FALSE);
        count = qspCallBacks[QSP_CALL_GETMSCOUNT]();
        qspFinalizeCallBack(&state);
        return count;
    }
    return 0;
}

QSPString qspCallInputBox(QSPString text)
{
    /* Get input from a user */
    QSPCallState state;
    QSP_CHAR *buffer;
    const int maxLen = 511;
    if (qspCallBacks[QSP_CALL_INPUTBOX])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_TRUE);
        /* Prepare input buffer */
        buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
        *buffer = 0;
        /* Process input */
        qspCallBacks[QSP_CALL_INPUTBOX](text, buffer, maxLen);
        buffer[maxLen] = 0;
        /* Clean up */
        qspFinalizeCallBack(&state);
        return qspStringFromC(buffer);
    }
    return qspNullString;
}

QSPString qspCallVersion(QSPString param)
{
    /* Get info from the player */
    QSPCallState state;
    QSP_CHAR *buffer;
    const int maxLen = 511;
    if (qspCallBacks[QSP_CALL_VERSION])
    {
        qspPrepareCallBack(&state, QSP_TRUE, QSP_FALSE);
        /* Prepare buffer for the response */
        buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
        *buffer = 0;
        /* Process request */
        qspCallBacks[QSP_CALL_VERSION](param, buffer, maxLen);
        buffer[maxLen] = 0;
        /* Clean up */
        qspFinalizeCallBack(&state);
        return qspStringFromC(buffer);
    }
    return qspCopyToNewText(QSP_STATIC_STR(QSP_VER));
}

#endif
