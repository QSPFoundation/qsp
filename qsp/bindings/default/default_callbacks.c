/* Copyright (C) 2001-2020 Valeriy Argunov (val AT time DOT guru) */
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

void qspInitCallBacks()
{
    int i;
    qspIsInCallBack = QSP_FALSE;
    qspIsDisableCodeExec = QSP_FALSE;
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
        qspSaveCallState(&state, QSP_FALSE);
        qspCallBacks[QSP_CALL_DEBUG](str);
        qspRestoreCallState(&state);
        qspResetError();
    }
}

void qspCallSetTimer(int msecs)
{
    /* Set timer interval */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SETTIMER])
    {
        qspSaveCallState(&state, QSP_TRUE);
        qspCallBacks[QSP_CALL_SETTIMER](msecs);
        qspRestoreCallState(&state);
    }
}

void qspCallRefreshInt(QSP_BOOL isRedraw)
{
    /* Refresh UI to show the latest state */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_REFRESHINT])
    {
        qspSaveCallState(&state, QSP_TRUE);
        qspCallBacks[QSP_CALL_REFRESHINT](isRedraw);
        qspRestoreCallState(&state);
    }
}

void qspCallSetInputStrText(QSPString text)
{
    /* Set value of the text input control */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SETINPUTSTRTEXT])
    {
        qspSaveCallState(&state, QSP_TRUE);
        qspCallBacks[QSP_CALL_SETINPUTSTRTEXT](text);
        qspRestoreCallState(&state);
    }
}

void qspCallSystem(QSPString cmd)
{
    /* Execute system call */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SYSTEM])
    {
        qspSaveCallState(&state, QSP_FALSE);
        qspCallBacks[QSP_CALL_SYSTEM](cmd);
        qspRestoreCallState(&state);
    }
}

void qspCallOpenGame(QSPString file, QSP_BOOL isNewGame)
{
    /* Open game file */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_OPENGAME])
    {
        qspSaveCallState(&state, QSP_FALSE);
        qspCallBacks[QSP_CALL_OPENGAME](file, isNewGame);
        qspRestoreCallState(&state);
    }
}

void qspCallOpenGameStatus(QSPString file)
{
    /* Open game state (showing a dialog to choose a file) */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_OPENGAMESTATUS])
    {
        qspSaveCallState(&state, QSP_FALSE);
        qspCallBacks[QSP_CALL_OPENGAMESTATUS](file);
        qspRestoreCallState(&state);
    }
}

void qspCallSaveGameStatus(QSPString file)
{
    /* Save game state (showing a dialog to choose a file) */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SAVEGAMESTATUS])
    {
        qspSaveCallState(&state, QSP_FALSE);
        qspCallBacks[QSP_CALL_SAVEGAMESTATUS](file);
        qspRestoreCallState(&state);
    }
}

void qspCallShowMessage(QSPString text)
{
    /* Show a message */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SHOWMSGSTR])
    {
        qspSaveCallState(&state, QSP_TRUE);
        qspCallBacks[QSP_CALL_SHOWMSGSTR](text);
        qspRestoreCallState(&state);
    }
}

int qspCallShowMenu(QSPListItem *items, int count)
{
    /* Show a menu */
    QSPCallState state;
    int index;
    if (qspCallBacks[QSP_CALL_SHOWMENU])
    {
        qspSaveCallState(&state, QSP_TRUE);
        index = qspCallBacks[QSP_CALL_SHOWMENU](items, count);
        qspRestoreCallState(&state);
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
        qspSaveCallState(&state, QSP_TRUE);
        qspCallBacks[QSP_CALL_SHOWIMAGE](file);
        qspRestoreCallState(&state);
    }
}

void qspCallShowWindow(int type, QSP_BOOL isShow)
{
    /* Show (hide) a region of the UI */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SHOWWINDOW])
    {
        qspSaveCallState(&state, QSP_TRUE);
        qspCallBacks[QSP_CALL_SHOWWINDOW](type, isShow);
        qspRestoreCallState(&state);
    }
}

void qspCallPlayFile(QSPString file, int volume)
{
    /* Start playing a music file */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_PLAYFILE])
    {
        qspSaveCallState(&state, QSP_TRUE);
        qspCallBacks[QSP_CALL_PLAYFILE](file, volume);
        qspRestoreCallState(&state);
    }
}

QSP_BOOL qspCallIsPlayingFile(QSPString file)
{
    /* Check whether a file is still playing */
    QSPCallState state;
    QSP_BOOL isPlaying;
    if (qspCallBacks[QSP_CALL_ISPLAYINGFILE])
    {
        qspSaveCallState(&state, QSP_TRUE);
        isPlaying = (QSP_BOOL)qspCallBacks[QSP_CALL_ISPLAYINGFILE](file);
        qspRestoreCallState(&state);
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
        qspSaveCallState(&state, QSP_TRUE);
        qspCallBacks[QSP_CALL_CLOSEFILE](file);
        qspRestoreCallState(&state);
    }
}

void qspCallSleep(int msecs)
{
    /* Wait for the specified number of milliseconds */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SLEEP])
    {
        qspSaveCallState(&state, QSP_TRUE);
        qspCallBacks[QSP_CALL_SLEEP](msecs);
        qspRestoreCallState(&state);
    }
}

int qspCallGetMSCount()
{
    /* Get the number of milliseconds since the last call of this function */
    QSPCallState state;
    int count;
    if (qspCallBacks[QSP_CALL_GETMSCOUNT])
    {
        qspSaveCallState(&state, QSP_TRUE);
        count = qspCallBacks[QSP_CALL_GETMSCOUNT]();
        qspRestoreCallState(&state);
        return count;
    }
    return 0;
}

QSPString qspCallInputBox(QSPString text)
{
    /* Get input from a user */
    QSPCallState state;
    QSP_CHAR *buffer;
    int maxLen = 511;
    if (qspCallBacks[QSP_CALL_INPUTBOX])
    {
        qspSaveCallState(&state, QSP_TRUE);
        /* Prepare input buffer */
        buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
        *buffer = 0;
        /* Process input */
        qspCallBacks[QSP_CALL_INPUTBOX](text, buffer, maxLen);
        buffer[maxLen] = 0;
        /* Clean up */
        qspRestoreCallState(&state);
        return qspStringFromC(buffer);
    }
    return qspNullString;
}

#endif
