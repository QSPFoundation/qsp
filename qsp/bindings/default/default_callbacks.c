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
    if (qspCallbacks[QSP_CALL_DEBUG])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_DEBUG](str);
        qspFinalizeCallback(&state, QSP_TRUE);
    }
}

void qspCallSetTimer(int msecs)
{
    /* Set timer interval */
    if (qspCallbacks[QSP_CALL_SETTIMER])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_SETTIMER](msecs);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

void qspCallRefreshInt(QSP_BOOL isForced)
{
    /* Refresh UI to show the latest state */
    if (qspCallbacks[QSP_CALL_REFRESHINT])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_REFRESHINT](isForced);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

void qspCallSetInputStrText(QSPString text)
{
    /* Set value of the text input control */
    if (qspCallbacks[QSP_CALL_SETINPUTSTRTEXT])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_SETINPUTSTRTEXT](text);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

void qspCallSystem(QSPString cmd)
{
    /* Execute system call */
    if (qspCallbacks[QSP_CALL_SYSTEM])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_SYSTEM](cmd);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

void qspCallOpenGame(QSPString file, QSP_BOOL isNewGame)
{
    /* Open game file */
    if (qspCallbacks[QSP_CALL_OPENGAME])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_OPENGAME](file, isNewGame);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

void qspCallOpenGameStatus(QSPString file)
{
    /* Open game state (showing the dialog to choose a file) */
    if (qspCallbacks[QSP_CALL_OPENGAMESTATUS])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_OPENGAMESTATUS](file);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

void qspCallSaveGameStatus(QSPString file)
{
    /* Save game state (showing the dialog to choose a file) */
    if (qspCallbacks[QSP_CALL_SAVEGAMESTATUS])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_SAVEGAMESTATUS](file);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

void qspCallShowMessage(QSPString text)
{
    /* Show a message */
    if (qspCallbacks[QSP_CALL_SHOWMSGSTR])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_TRUE);
        qspCallbacks[QSP_CALL_SHOWMSGSTR](text);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

int qspCallShowMenu(QSPListItem *items, int count)
{
    /* Show a menu */
    if (qspCallbacks[QSP_CALL_SHOWMENU])
    {
        QSPCallState state;
        int index;
        qspPrepareCallback(&state, QSP_TRUE);
        index = qspCallbacks[QSP_CALL_SHOWMENU](items, count);
        qspFinalizeCallback(&state, QSP_FALSE);
        return index;
    }
    return -1;
}

void qspCallShowPicture(QSPString file)
{
    /* Show an image */
    if (qspCallbacks[QSP_CALL_SHOWIMAGE])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_SHOWIMAGE](file);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

void qspCallShowWindow(int type, QSP_BOOL toShow)
{
    /* Show (hide) a region of the UI */
    if (qspCallbacks[QSP_CALL_SHOWWINDOW])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_SHOWWINDOW](type, toShow);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

void qspCallPlayFile(QSPString file, int volume)
{
    /* Start playing a music file */
    if (qspCallbacks[QSP_CALL_PLAYFILE])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_PLAYFILE](file, volume);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

QSP_BOOL qspCallIsPlayingFile(QSPString file)
{
    /* Check whether a file is still playing */
    if (qspCallbacks[QSP_CALL_ISPLAYINGFILE])
    {
        QSPCallState state;
        QSP_BOOL isPlaying;
        qspPrepareCallback(&state, QSP_FALSE);
        isPlaying = (QSP_BOOL)qspCallbacks[QSP_CALL_ISPLAYINGFILE](file);
        qspFinalizeCallback(&state, QSP_FALSE);
        return isPlaying;
    }
    return QSP_FALSE;
}

void qspCallCloseFile(QSPString file)
{
    /* Stop playing a file */
    if (qspCallbacks[QSP_CALL_CLOSEFILE])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_FALSE);
        qspCallbacks[QSP_CALL_CLOSEFILE](file);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

void qspCallSleep(int msecs)
{
    /* Wait for the specified number of milliseconds */
    if (qspCallbacks[QSP_CALL_SLEEP])
    {
        QSPCallState state;
        qspPrepareCallback(&state, QSP_TRUE);
        qspCallbacks[QSP_CALL_SLEEP](msecs);
        qspFinalizeCallback(&state, QSP_FALSE);
    }
}

int qspCallGetMSCount(void)
{
    /* Get the number of milliseconds since the last call of this function */
    if (qspCallbacks[QSP_CALL_GETMSCOUNT])
    {
        QSPCallState state;
        int count;
        qspPrepareCallback(&state, QSP_FALSE);
        count = qspCallbacks[QSP_CALL_GETMSCOUNT]();
        qspFinalizeCallback(&state, QSP_FALSE);
        return count;
    }
    return 0;
}

QSPString qspCallInputBox(QSPString text)
{
    /* Get input from the user */
    if (qspCallbacks[QSP_CALL_INPUTBOX])
    {
        const int maxLen = 511;
        QSPCallState state;
        QSPString res;
        QSP_CHAR *buffer;
        qspPrepareCallback(&state, QSP_TRUE);

        buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
        *buffer = 0;
        qspCallbacks[QSP_CALL_INPUTBOX](text, buffer, maxLen);
        buffer[maxLen] = 0;
        res = qspStringFromC(buffer);

        if (!qspFinalizeCallback(&state, QSP_FALSE))
        {
            qspFreeString(&res);
            return qspNullString;
        }
        return res;
    }
    return qspNullString;
}

QSPString qspCallVersion(QSPString param)
{
    /* Get info from the player */
    if (qspCallbacks[QSP_CALL_VERSION])
    {
        const int maxLen = 511;
        QSPCallState state;
        QSPString res;
        QSP_CHAR *buffer;
        qspPrepareCallback(&state, QSP_FALSE);

        buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
        *buffer = 0;
        qspCallbacks[QSP_CALL_VERSION](param, buffer, maxLen);
        buffer[maxLen] = 0;
        res = qspStringFromC(buffer);

        if (!qspFinalizeCallback(&state, QSP_FALSE))
        {
            qspFreeString(&res);
            return qspNullString;
        }

        if (!qspIsEmpty(res)) return res;
        qspFreeString(&res);
    }
    return qspCopyToNewText(QSP_STATIC_STR(QSP_VER));
}

#endif
