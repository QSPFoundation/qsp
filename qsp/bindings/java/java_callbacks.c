/* Copyright (C) 2010 Ntropy (ntropy AT qsp DOT su) */
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

#ifdef _JAVA

#include "../../callbacks.h"
#include "../../actions.h"
#include "../../coding.h"
#include "../../common.h"
#include "../../errors.h"
#include "../../objects.h"
#include "../../text.h"

extern int _call_java(int a, int b, int c, int d);

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

void qspCallDebug(QSP_CHAR *str)
{
	/* Здесь передаем управление отладчику */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_DEBUG])
	{
		qspSaveCallState(&state, QSP_FALSE, QSP_FALSE);
		_call_java(QSP_CALL_DEBUG, (int)qspCallBacks[QSP_CALL_DEBUG], (int)str, 0);
		qspRestoreCallState(&state);
	}
}

void qspCallSetTimer(int msecs)
{
	/* Здесь устанавливаем интервал таймера */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SETTIMER])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_SETTIMER, (int)qspCallBacks[QSP_CALL_SETTIMER], (int)msecs, 0);
		qspRestoreCallState(&state);
	}
}

void qspCallRefreshInt(QSP_BOOL isRedraw)
{
	/* Здесь выполняем обновление интерфейса */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_REFRESHINT])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_REFRESHINT, (int)qspCallBacks[QSP_CALL_REFRESHINT], (int)isRedraw, 0);
		qspRestoreCallState(&state);
	}
}

void qspCallSetInputStrText(QSP_CHAR *text)
{
	/* Здесь устанавливаем текст строки ввода */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SETINPUTSTRTEXT])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_SETINPUTSTRTEXT, (int)qspCallBacks[QSP_CALL_SETINPUTSTRTEXT], (int)text, 0);
		qspRestoreCallState(&state);
	}
}

void qspCallAddMenuItem(QSP_CHAR *name, QSP_CHAR *imgPath)
{
	/* Здесь добавляем пункт меню */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_ADDMENUITEM])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_ADDMENUITEM, (int)qspCallBacks[QSP_CALL_ADDMENUITEM], (int)name, (int)imgPath);
		qspRestoreCallState(&state);
	}
}

void qspCallSystem(QSP_CHAR *cmd)
{
	/* Здесь выполняем системный вызов */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SYSTEM])
	{
		qspSaveCallState(&state, QSP_FALSE, QSP_FALSE);
		_call_java(QSP_CALL_SYSTEM, (int)qspCallBacks[QSP_CALL_SYSTEM], (int)cmd, 0);
		qspRestoreCallState(&state);
	}
}

void qspCallOpenGame(QSP_CHAR *file)
{
	/* Здесь позволяем пользователю выбрать файл */
	/* состояния игры для загрузки и загружаем его */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_OPENGAMESTATUS])
	{
		qspSaveCallState(&state, QSP_FALSE, QSP_TRUE);
		_call_java(QSP_CALL_OPENGAMESTATUS, (int)qspCallBacks[QSP_CALL_OPENGAMESTATUS], (int)file, 0);
		qspRestoreCallState(&state);
	}
}

void qspCallSaveGame(QSP_CHAR *file)
{
	/* Здесь позволяем пользователю выбрать файл */
	/* для сохранения состояния игры и сохраняем */
	/* в нем текущее состояние */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SAVEGAMESTATUS])
	{
		qspSaveCallState(&state, QSP_FALSE, QSP_TRUE);
		_call_java(QSP_CALL_SAVEGAMESTATUS, (int)qspCallBacks[QSP_CALL_SAVEGAMESTATUS], (int)file, 0);
		qspRestoreCallState(&state);
	}
}

void qspCallShowMessage(QSP_CHAR *text)
{
	/* Здесь показываем сообщение */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SHOWMSGSTR])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_SHOWMSGSTR, (int)qspCallBacks[QSP_CALL_SHOWMSGSTR], (int)text, 0);
		qspRestoreCallState(&state);
	}
}

int qspCallShowMenu()
{
	/* Здесь показываем меню */
	QSPCallState state;
	int index;
	if (qspCallBacks[QSP_CALL_SHOWMENU])
	{
		qspSaveCallState(&state, QSP_FALSE, QSP_TRUE);
		index = _call_java(QSP_CALL_SHOWMENU, (int)qspCallBacks[QSP_CALL_SHOWMENU], 0, 0);
		qspRestoreCallState(&state);
		return index;
	}
	return -1;
}

void qspCallShowPicture(QSP_CHAR *file)
{
	/* Здесь показываем изображение */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SHOWIMAGE])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_SHOWIMAGE, (int)qspCallBacks[QSP_CALL_SHOWIMAGE], (int)file, 0);
		qspRestoreCallState(&state);
	}
}

void qspCallShowWindow(int type, QSP_BOOL isShow)
{
	/* Здесь показываем или скрываем окно */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SHOWWINDOW])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_SHOWWINDOW, (int)qspCallBacks[QSP_CALL_SHOWWINDOW], (int)type, (int)isShow);
		qspRestoreCallState(&state);
	}
}

void qspCallPlayFile(QSP_CHAR *file, int volume)
{
	/* Здесь начинаем воспроизведение файла с заданной громкостью */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_PLAYFILE])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_PLAYFILE, (int)qspCallBacks[QSP_CALL_PLAYFILE], (int)file, (int)volume);
		qspRestoreCallState(&state);
	}
}

QSP_BOOL qspCallIsPlayingFile(QSP_CHAR *file)
{
	/* Здесь проверяем, проигрывается ли файл */
	QSPCallState state;
	QSP_BOOL isPlaying;
	if (qspCallBacks[QSP_CALL_ISPLAYINGFILE])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		isPlaying = (QSP_BOOL)_call_java(QSP_CALL_ISPLAYINGFILE, (int)qspCallBacks[QSP_CALL_ISPLAYINGFILE], (int)file, 0);
		qspRestoreCallState(&state);
		return isPlaying;
	}
	return QSP_FALSE;
}

void qspCallSleep(int msecs)
{
	/* Здесь ожидаем заданное количество миллисекунд */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SLEEP])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_SLEEP, (int)qspCallBacks[QSP_CALL_SLEEP], (int)msecs, 0);
		qspRestoreCallState(&state);
	}
}

int qspCallGetMSCount()
{
	/* Здесь получаем количество миллисекунд, прошедших с момента последнего вызова функции */
	QSPCallState state;
	int count;
	if (qspCallBacks[QSP_CALL_GETMSCOUNT])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		count = _call_java(QSP_CALL_GETMSCOUNT, (int)qspCallBacks[QSP_CALL_GETMSCOUNT], 0, 0);
		qspRestoreCallState(&state);
		return count;
	}
	return 0;
}

void qspCallCloseFile(QSP_CHAR *file)
{
	/* Здесь выполняем закрытие файла */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_CLOSEFILE])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_CLOSEFILE, (int)qspCallBacks[QSP_CALL_CLOSEFILE], (int)file, 0);
		qspRestoreCallState(&state);
	}
}

void qspCallDeleteMenu()
{
	/* Здесь удаляем текущее меню */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_DELETEMENU])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		_call_java(QSP_CALL_DELETEMENU, (int)qspCallBacks[QSP_CALL_DELETEMENU], 0, 0);
		qspRestoreCallState(&state);
	}
}

QSP_CHAR *qspCallInputBox(QSP_CHAR *text)
{
	/* Здесь вводим текст */
	QSPCallState state;
	QSP_CHAR *buffer;
	int maxLen = 511;
	if (qspCallBacks[QSP_CALL_INPUTBOX])
	{
		qspSaveCallState(&state, QSP_TRUE, QSP_FALSE);
		buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
		*buffer = 0;
		//_call_java(QSP_CALL_INPUTBOX, (int)qspCallBacks[QSP_CALL_INPUTBOX], text, buffer, maxLen);
		_call_java(QSP_CALL_INPUTBOX, (int)qspCallBacks[QSP_CALL_INPUTBOX], (int)text, (int)buffer);
		buffer[maxLen] = 0;
		qspRestoreCallState(&state);
	}
	else
		buffer = qspGetNewText(QSP_FMT(""), 0);
	return buffer;
}

#endif
