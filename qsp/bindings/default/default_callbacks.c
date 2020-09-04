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
    qspIsExitOnError = QSP_FALSE;
    for (i = 0; i < QSP_CALL_DUMMY; ++i)
        qspCallBacks[i] = 0;
}

void qspSetCallBack(int type, QSP_CALLBACK func)
{
    qspCallBacks[type] = func;
}

void qspCallDebug(QSPString str)
{
    /* ����� �������� ���������� ��������� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_DEBUG])
    {
        qspSaveCallState(&state, QSP_FALSE, QSP_FALSE);
        qspCallBacks[QSP_CALL_DEBUG](str);
        qspRestoreCallState(&state);
    }
}

void qspCallSetTimer(int msecs)
{
    /* ����� ������������� �������� ������� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SETTIMER])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SETTIMER](msecs);
        qspRestoreCallState(&state);
    }
}

void qspCallRefreshInt(QSP_BOOL isRedraw)
{
    /* ����� ��������� ���������� ���������� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_REFRESHINT])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_REFRESHINT](isRedraw);
        qspRestoreCallState(&state);
    }
}

void qspCallSetInputStrText(QSPString text)
{
    /* ����� ������������� ����� ������ ����� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SETINPUTSTRTEXT])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SETINPUTSTRTEXT](text);
        qspRestoreCallState(&state);
    }
}

void qspCallSystem(QSPString cmd)
{
    /* ����� ��������� ��������� ����� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SYSTEM])
    {
        qspSaveCallState(&state, QSP_FALSE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SYSTEM](cmd);
        qspRestoreCallState(&state);
    }
}

void qspCallOpenGame(QSPString file, QSP_BOOL isNewGame)
{
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_OPENGAME])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_OPENGAME](file, isNewGame);
        qspRestoreCallState(&state);
    }
}

void qspCallOpenGameStatus(QSPString file)
{
    /* ����� ��������� ������������ ������� ���� */
    /* ��������� ���� ��� �������� � ��������� ��� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_OPENGAMESTATUS])
    {
        qspSaveCallState(&state, QSP_FALSE, QSP_TRUE);
        qspCallBacks[QSP_CALL_OPENGAMESTATUS](file);
        qspRestoreCallState(&state);
    }
}

void qspCallSaveGameStatus(QSPString file)
{
    /* ����� ��������� ������������ ������� ���� */
    /* ��� ���������� ��������� ���� � ��������� */
    /* � ��� ������� ��������� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SAVEGAMESTATUS])
    {
        qspSaveCallState(&state, QSP_FALSE, QSP_TRUE);
        qspCallBacks[QSP_CALL_SAVEGAMESTATUS](file);
        qspRestoreCallState(&state);
    }
}

void qspCallShowMessage(QSPString text)
{
    /* ����� ���������� ��������� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SHOWMSGSTR])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SHOWMSGSTR](text);
        qspRestoreCallState(&state);
    }
}

int qspCallShowMenu(QSPListItem *items, int count)
{
    /* ����� ���������� ���� */
    QSPCallState state;
    int index;
    if (qspCallBacks[QSP_CALL_SHOWMENU])
    {
        qspSaveCallState(&state, QSP_FALSE, QSP_TRUE);
        index = qspCallBacks[QSP_CALL_SHOWMENU](items, count);
        qspRestoreCallState(&state);
        return index;
    }
    return -1;
}

void qspCallShowPicture(QSPString file)
{
    /* ����� ���������� ����������� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SHOWIMAGE])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SHOWIMAGE](file);
        qspRestoreCallState(&state);
    }
}

void qspCallShowWindow(int type, QSP_BOOL isShow)
{
    /* ����� ���������� ��� �������� ���� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SHOWWINDOW])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SHOWWINDOW](type, isShow);
        qspRestoreCallState(&state);
    }
}

void qspCallPlayFile(QSPString file, int volume)
{
    /* ����� �������� ��������������� ����� � �������� ���������� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_PLAYFILE])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_PLAYFILE](file, volume);
        qspRestoreCallState(&state);
    }
}

QSP_BOOL qspCallIsPlayingFile(QSPString file)
{
    /* ����� ���������, ������������� �� ���� */
    QSPCallState state;
    QSP_BOOL isPlaying;
    if (qspCallBacks[QSP_CALL_ISPLAYINGFILE])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        isPlaying = (QSP_BOOL)qspCallBacks[QSP_CALL_ISPLAYINGFILE](file);
        qspRestoreCallState(&state);
        return isPlaying;
    }
    return QSP_FALSE;
}

void qspCallSleep(int msecs)
{
    /* ����� ������� �������� ���������� ����������� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_SLEEP])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_SLEEP](msecs);
        qspRestoreCallState(&state);
    }
}

int qspCallGetMSCount()
{
    /* ����� �������� ���������� �����������, ��������� � ������� ���������� ������ ������� */
    QSPCallState state;
    int count;
    if (qspCallBacks[QSP_CALL_GETMSCOUNT])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        count = qspCallBacks[QSP_CALL_GETMSCOUNT]();
        qspRestoreCallState(&state);
        return count;
    }
    return 0;
}

void qspCallCloseFile(QSPString file)
{
    /* ����� ��������� �������� ����� */
    QSPCallState state;
    if (qspCallBacks[QSP_CALL_CLOSEFILE])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
        qspCallBacks[QSP_CALL_CLOSEFILE](file);
        qspRestoreCallState(&state);
    }
}

QSPString qspCallInputBox(QSPString text)
{
    /* ����� ������ ����� */
    QSPCallState state;
    QSP_CHAR *buffer;
    int maxLen = 511;
    if (qspCallBacks[QSP_CALL_INPUTBOX])
    {
        qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
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
    return qspNewEmptyString();
}

#endif
