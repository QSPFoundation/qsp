/* Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include "callbacks.h"
#include "actions.h"
#include "common.h"
#include "errors.h"
#include "objects.h"
#include "text.h"

QSP_CALLBACK qspCallBacks[QSP_CALL_DUMMY];
QSP_BOOL qspIsInCallBack = QSP_FALSE;

static void qspSaveState(QSPExecState *);
static void qspRestoreState(QSPExecState *);

static void qspSaveState(QSPExecState *state)
{
	state->IsInCallBack = qspIsInCallBack;
	state->IsMustWait = qspIsMustWait;
	state->IsMainDescChanged = qspIsMainDescChanged;
	state->IsVarsDescChanged = qspIsVarsDescChanged;
	state->IsObjectsChanged = qspIsObjectsChanged;
	state->IsActionsChanged = qspIsActionsChanged;
	qspIsInCallBack = QSP_TRUE;
	qspIsMustWait = QSP_FALSE;
}

static void qspRestoreState(QSPExecState *state)
{
	qspResetError();
	qspIsActionsChanged = state->IsActionsChanged;
	qspIsObjectsChanged = state->IsObjectsChanged;
	qspIsVarsDescChanged = state->IsVarsDescChanged;
	qspIsMainDescChanged = state->IsMainDescChanged;
	qspIsMustWait = state->IsMustWait;
	qspIsInCallBack = state->IsInCallBack;
}

void qspInitCallBacks()
{
	long i;
	qspIsInCallBack = QSP_FALSE;
	for (i = 0; i < QSP_CALL_DUMMY; ++i)
		qspCallBacks[i] = 0;
}

void qspSetCallBack(long type, QSP_CALLBACK func)
{
	qspCallBacks[type] = func;
}

void qspCallDebug()
{
	/* Здесь передаем управление отладчику */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_DEBUG])
	{
		qspSaveState(&state);
		/* Специальный вызов */
		qspIsInCallBack = QSP_FALSE;
		qspCallBacks[QSP_CALL_DEBUG]();
		qspRestoreState(&state);
	}
}

void qspCallSetTimer(long msecs)
{
	/* Здесь устанавливаем интервал таймера */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_SETTIMER])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_SETTIMER](msecs);
		qspRestoreState(&state);
	}
}

void qspCallRefreshInt(QSP_BOOL isRedraw)
{
	/* Здесь выполняем обновление интерфейса */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_REFRESHINT])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_REFRESHINT](isRedraw);
		qspRestoreState(&state);
	}
}

void qspCallSetInputStrText(QSP_CHAR *text)
{
	/* Здесь устанавливаем текст строки ввода */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_SETINPUTSTRTEXT])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_SETINPUTSTRTEXT](text);
		qspRestoreState(&state);
	}
}

void qspCallAddMenuItem(QSP_CHAR *name, QSP_CHAR *imgPath)
{
	/* Здесь добавляем пункт меню */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_ADDMENUITEM])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_ADDMENUITEM](name, imgPath);
		qspRestoreState(&state);
	}
}

void qspCallSystem(QSP_CHAR *cmd)
{
	/* Здесь выполняем системный вызов */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_SYSTEM])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_SYSTEM](cmd);
		qspRestoreState(&state);
	}
}

void qspCallOpenGame()
{
	/* Здесь позволяем пользователю выбрать файл */
	/* состояния игры для загрузки и загружаем его */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_OPENGAMESTATUS])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_OPENGAMESTATUS]();
		qspRestoreState(&state);
	}
}

void qspCallSaveGame()
{
	/* Здесь позволяем пользователю выбрать файл */
	/* для сохранения состояния игры и сохраняем */
	/* в нем текущее состояние */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_SAVEGAMESTATUS])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_SAVEGAMESTATUS]();
		qspRestoreState(&state);
	}
}

void qspCallShowMessage(QSP_CHAR *text)
{
	/* Здесь показываем сообщение */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_SHOWMSGSTR])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_SHOWMSGSTR](text);
		qspRestoreState(&state);
	}
}

void qspCallShowMenu()
{
	/* Здесь показываем меню */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_SHOWMENU])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_SHOWMENU]();
		qspRestoreState(&state);
	}
}

void qspCallShowPicture(QSP_CHAR *file)
{
	/* Здесь показываем изображение */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_SHOWIMAGE])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_SHOWIMAGE](file);
		qspRestoreState(&state);
	}
}

void qspCallShowWindow(long type, QSP_BOOL isShow)
{
	/* Здесь показываем или скрываем окно */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_SHOWWINDOW])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_SHOWWINDOW](type, isShow);
		qspRestoreState(&state);
	}
}

void qspCallPlayFile(QSP_CHAR *file, long volume)
{
	/* Здесь начинаем воспроизведение файла с заданной громкостью */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_PLAYFILE])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_PLAYFILE](file, volume);
		qspRestoreState(&state);
	}
}

QSP_BOOL qspCallIsPlayingFile(QSP_CHAR *file)
{
	/* Здесь проверяем, проигрывается ли файл */
	QSPExecState state;
	QSP_BOOL isPlaying;
	if (qspCallBacks[QSP_CALL_ISPLAYINGFILE])
	{
		qspSaveState(&state);
		isPlaying = (QSP_BOOL)qspCallBacks[QSP_CALL_ISPLAYINGFILE](file);
		qspRestoreState(&state);
		return isPlaying;
	}
	return QSP_FALSE;
}

void qspCallSleep(long msecs)
{
	/* Здесь ожидаем заданное количество миллисекунд */
	/* Состояние не сохраняем */
	if (qspCallBacks[QSP_CALL_SLEEP]) qspCallBacks[QSP_CALL_SLEEP](msecs);
}

long qspCallGetMSCount()
{
	/* Здесь получаем количество миллисекунд, прошедших с момента последнего вызова функции */
	QSPExecState state;
	long count;
	if (qspCallBacks[QSP_CALL_GETMSCOUNT])
	{
		qspSaveState(&state);
		count = qspCallBacks[QSP_CALL_GETMSCOUNT]();
		qspRestoreState(&state);
		return count;
	}
	return 0;
}

void qspCallCloseFile(QSP_CHAR *file)
{
	/* Здесь выполняем закрытие файла */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_CLOSEFILE])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_CLOSEFILE](file);
		qspRestoreState(&state);
	}
}

void qspCallDeleteMenu()
{
	/* Здесь удаляем текущее меню */
	QSPExecState state;
	if (qspCallBacks[QSP_CALL_DELETEMENU])
	{
		qspSaveState(&state);
		qspCallBacks[QSP_CALL_DELETEMENU]();
		qspRestoreState(&state);
	}
}

QSP_CHAR *qspCallInputBox(QSP_CHAR *text)
{
	/* Здесь вводим текст */
	QSPExecState state;
	QSP_CHAR *buffer;
	long maxLen = 511;
	if (qspCallBacks[QSP_CALL_INPUTBOX])
	{
		qspSaveState(&state);
		buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
		*buffer = 0;
		qspCallBacks[QSP_CALL_INPUTBOX](text, buffer, maxLen);
		buffer[maxLen] = 0;
		qspRestoreState(&state);
	}
	else
		buffer = qspGetNewText(QSP_FMT(""), 0);
	return buffer;
}
