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

#include "../../declarations.h"

#ifdef _DEFAULT_BINDING

#include "../../callbacks.h"
#include "../../actions.h"
#include "../../coding.h"
#include "../../common.h"
#include "../../errors.h"
#include "../../objects.h"
#include "../../text.h"

void qspInitCallbacks(void)
{
    int i;
    qspIsInCallback = QSP_FALSE;
    qspToDisableCodeExec = QSP_FALSE;
    for (i = 0; i < QSP_CALL_DUMMY; ++i)
        qspCallbacks[i] = 0;
}

void qspSetCallback(int type, QSP_CALLBACK func)
{
    qspCallbacks[type] = func;
}

void qspCallDebug(QSPString str)
{
    /* Jump into the debugger */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_DEBUG])
    {
        qspPrepareCallback(&state, QSP_FALSE, QSP_FALSE);
        qspCallbacks[QSP_CALL_DEBUG](str);
        qspFinalizeCallback(&state);
        qspResetError(QSP_FALSE);
    }
}

void qspCallSetTimer(int msecs)
{
    /* Set timer interval */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SETTIMER])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        qspCallbacks[QSP_CALL_SETTIMER](msecs);
        qspFinalizeCallback(&state);
    }
}

void qspCallRefreshInt(QSP_BOOL isForced)
{
    /* Refresh UI to show the latest state */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_REFRESHINT])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        qspCallbacks[QSP_CALL_REFRESHINT](isForced);
        qspFinalizeCallback(&state);
    }
}

void qspCallSetInputStrText(QSPString text)
{
    /* Set value of the text input control */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SETINPUTSTRTEXT])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        qspCallbacks[QSP_CALL_SETINPUTSTRTEXT](text);
        qspFinalizeCallback(&state);
    }
}

void qspCallSystem(QSPString cmd)
{
    /* Execute system call */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SYSTEM])
    {
        qspPrepareCallback(&state, QSP_FALSE, QSP_FALSE);
        qspCallbacks[QSP_CALL_SYSTEM](cmd);
        qspFinalizeCallback(&state);
    }
}

void qspCallOpenGame(QSPString file, QSP_BOOL isNewGame)
{
    /* Open game file */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_OPENGAME])
    {
        qspPrepareCallback(&state, QSP_FALSE, QSP_FALSE);
        qspCallbacks[QSP_CALL_OPENGAME](file, isNewGame);
        qspFinalizeCallback(&state);
    }
}

void qspCallOpenGameStatus(QSPString file)
{
    /* Open game state (showing the dialog to choose a file) */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_OPENGAMESTATUS])
    {
        qspPrepareCallback(&state, QSP_FALSE, QSP_FALSE);
        qspCallbacks[QSP_CALL_OPENGAMESTATUS](file);
        qspFinalizeCallback(&state);
    }
}

void qspCallSaveGameStatus(QSPString file)
{
    /* Save game state (showing the dialog to choose a file) */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SAVEGAMESTATUS])
    {
        qspPrepareCallback(&state, QSP_FALSE, QSP_FALSE);
        qspCallbacks[QSP_CALL_SAVEGAMESTATUS](file);
        qspFinalizeCallback(&state);
    }
}

void qspCallShowMessage(QSPString text)
{
    /* Show a message */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SHOWMSGSTR])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_TRUE);
        qspCallbacks[QSP_CALL_SHOWMSGSTR](text);
        qspFinalizeCallback(&state);
    }
}

int qspCallShowMenu(QSPListItem *items, int count)
{
    /* Show a menu */
    QSPCallState state;
    int index;
    if (qspCallbacks[QSP_CALL_SHOWMENU])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_TRUE);
        index = qspCallbacks[QSP_CALL_SHOWMENU](items, count);
        qspFinalizeCallback(&state);
        return index;
    }
    return -1;
}

void qspCallShowPicture(QSPString file)
{
    /* Show an image */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SHOWIMAGE])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        qspCallbacks[QSP_CALL_SHOWIMAGE](file);
        qspFinalizeCallback(&state);
    }
}

void qspCallShowWindow(int type, QSP_BOOL toShow)
{
    /* Show (hide) a region of the UI */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SHOWWINDOW])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        qspCallbacks[QSP_CALL_SHOWWINDOW](type, toShow);
        qspFinalizeCallback(&state);
    }
}

void qspCallPlayFile(QSPString file, int volume)
{
    /* Start playing a music file */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_PLAYFILE])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        qspCallbacks[QSP_CALL_PLAYFILE](file, volume);
        qspFinalizeCallback(&state);
    }
}

QSP_BOOL qspCallIsPlayingFile(QSPString file)
{
    /* Check whether a file is still playing */
    QSPCallState state;
    QSP_BOOL isPlaying;
    if (qspCallbacks[QSP_CALL_ISPLAYINGFILE])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        isPlaying = (QSP_BOOL)qspCallbacks[QSP_CALL_ISPLAYINGFILE](file);
        qspFinalizeCallback(&state);
        return isPlaying;
    }
    return QSP_FALSE;
}

void qspCallCloseFile(QSPString file)
{
    /* Stop playing a file */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_CLOSEFILE])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        qspCallbacks[QSP_CALL_CLOSEFILE](file);
        qspFinalizeCallback(&state);
    }
}

void qspCallSleep(int msecs)
{
    /* Wait for the specified number of milliseconds */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SLEEP])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_TRUE);
        qspCallbacks[QSP_CALL_SLEEP](msecs);
        qspFinalizeCallback(&state);
    }
}

int qspCallGetMSCount(void)
{
    /* Get the number of milliseconds since the last call of this function */
    QSPCallState state;
    int count;
    if (qspCallbacks[QSP_CALL_GETMSCOUNT])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        count = qspCallbacks[QSP_CALL_GETMSCOUNT]();
        qspFinalizeCallback(&state);
        return count;
    }
    return 0;
}

QSPString qspCallInputBox(QSPString text)
{
    /* Get input from the user */
    QSPCallState state;
    QSP_CHAR *buffer;
    const int maxLen = 511;
    if (qspCallbacks[QSP_CALL_INPUTBOX])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_TRUE);
        /* Prepare input buffer */
        buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
        *buffer = 0;
        /* Process input */
        qspCallbacks[QSP_CALL_INPUTBOX](text, buffer, maxLen);
        buffer[maxLen] = 0;
        /* Clean up */
        qspFinalizeCallback(&state);
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
    if (qspCallbacks[QSP_CALL_VERSION])
    {
        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        /* Prepare buffer for the response */
        buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
        *buffer = 0;
        /* Process request */
        qspCallbacks[QSP_CALL_VERSION](param, buffer, maxLen);
        buffer[maxLen] = 0;
        /* Clean up */
        qspFinalizeCallback(&state);
        return qspStringFromC(buffer);
    }
    return qspCopyToNewText(QSP_STATIC_STR(QSP_VER));
}

#endif
