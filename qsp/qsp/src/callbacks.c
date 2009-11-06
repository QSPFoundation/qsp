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
#include "coding.h"
#include "common.h"
#include "errors.h"
#include "objects.h"
#include "text.h"

QSP_CALLBACK qspCallBacks[QSP_CALL_DUMMY];
volatile QSP_BOOL qspIsInCallBack = QSP_FALSE;

static void qspSaveCallState(QSPCallState *);
static void qspRestoreCallState(QSPCallState *);

static void qspSaveCallState(QSPCallState *state)
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

static void qspRestoreCallState(QSPCallState *state)
{
	qspResetError();
	if (state->IsActionsChanged) qspIsActionsChanged = QSP_TRUE;
	if (state->IsObjectsChanged) qspIsObjectsChanged = QSP_TRUE;
	if (state->IsVarsDescChanged) qspIsVarsDescChanged = QSP_TRUE;
	if (state->IsMainDescChanged) qspIsMainDescChanged = QSP_TRUE;
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

#ifndef _FLASH

void qspCallDebug(QSP_CHAR *str)
{
	/* Здесь передаем управление отладчику */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_DEBUG])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_DEBUG](str);
		qspRestoreCallState(&state);
	}
}

void qspCallSetTimer(long msecs)
{
	/* Здесь устанавливаем интервал таймера */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SETTIMER])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_SETTIMER](msecs);
		qspRestoreCallState(&state);
	}
}

void qspCallRefreshInt(QSP_BOOL isRedraw)
{
	/* Здесь выполняем обновление интерфейса */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_REFRESHINT])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_REFRESHINT](isRedraw);
		qspRestoreCallState(&state);
	}
}

void qspCallSetInputStrText(QSP_CHAR *text)
{
	/* Здесь устанавливаем текст строки ввода */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SETINPUTSTRTEXT])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_SETINPUTSTRTEXT](text);
		qspRestoreCallState(&state);
	}
}

void qspCallAddMenuItem(QSP_CHAR *name, QSP_CHAR *imgPath)
{
	/* Здесь добавляем пункт меню */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_ADDMENUITEM])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_ADDMENUITEM](name, imgPath);
		qspRestoreCallState(&state);
	}
}

void qspCallSystem(QSP_CHAR *cmd)
{
	/* Здесь выполняем системный вызов */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SYSTEM])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_SYSTEM](cmd);
		qspRestoreCallState(&state);
	}
}

void qspCallOpenGame()
{
	/* Здесь позволяем пользователю выбрать файл */
	/* состояния игры для загрузки и загружаем его */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_OPENGAMESTATUS])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_OPENGAMESTATUS]();
		qspRestoreCallState(&state);
	}
}

void qspCallSaveGame()
{
	/* Здесь позволяем пользователю выбрать файл */
	/* для сохранения состояния игры и сохраняем */
	/* в нем текущее состояние */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SAVEGAMESTATUS])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_SAVEGAMESTATUS]();
		qspRestoreCallState(&state);
	}
}

void qspCallShowMessage(QSP_CHAR *text)
{
	/* Здесь показываем сообщение */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SHOWMSGSTR])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_SHOWMSGSTR](text);
		qspRestoreCallState(&state);
	}
}

void qspCallShowMenu()
{
	/* Здесь показываем меню */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SHOWMENU])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_SHOWMENU]();
		qspRestoreCallState(&state);
	}
}

void qspCallShowPicture(QSP_CHAR *file)
{
	/* Здесь показываем изображение */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SHOWIMAGE])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_SHOWIMAGE](file);
		qspRestoreCallState(&state);
	}
}

void qspCallShowWindow(long type, QSP_BOOL isShow)
{
	/* Здесь показываем или скрываем окно */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SHOWWINDOW])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_SHOWWINDOW](type, isShow);
		qspRestoreCallState(&state);
	}
}

void qspCallPlayFile(QSP_CHAR *file, long volume)
{
	/* Здесь начинаем воспроизведение файла с заданной громкостью */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_PLAYFILE])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_PLAYFILE](file, volume);
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
		qspSaveCallState(&state);
		isPlaying = (QSP_BOOL)qspCallBacks[QSP_CALL_ISPLAYINGFILE](file);
		qspRestoreCallState(&state);
		return isPlaying;
	}
	return QSP_FALSE;
}

void qspCallSleep(long msecs)
{
	/* Здесь ожидаем заданное количество миллисекунд */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SLEEP])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_SLEEP](msecs);
		qspRestoreCallState(&state);
	}
}

long qspCallGetMSCount()
{
	/* Здесь получаем количество миллисекунд, прошедших с момента последнего вызова функции */
	QSPCallState state;
	long count;
	if (qspCallBacks[QSP_CALL_GETMSCOUNT])
	{
		qspSaveCallState(&state);
		count = qspCallBacks[QSP_CALL_GETMSCOUNT]();
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
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_CLOSEFILE](file);
		qspRestoreCallState(&state);
	}
}

void qspCallDeleteMenu()
{
	/* Здесь удаляем текущее меню */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_DELETEMENU])
	{
		qspSaveCallState(&state);
		qspCallBacks[QSP_CALL_DELETEMENU]();
		qspRestoreCallState(&state);
	}
}

QSP_CHAR *qspCallInputBox(QSP_CHAR *text)
{
	/* Здесь вводим текст */
	QSPCallState state;
	QSP_CHAR *buffer;
	long maxLen = 511;
	if (qspCallBacks[QSP_CALL_INPUTBOX])
	{
		qspSaveCallState(&state);
		buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
		*buffer = 0;
		qspCallBacks[QSP_CALL_INPUTBOX](text, buffer, maxLen);
		buffer[maxLen] = 0;
		qspRestoreCallState(&state);
	}
	else
		buffer = qspGetNewText(QSP_FMT(""), 0);
	return buffer;
}

#else

void qspCallDebug(QSP_CHAR *str)
{
	/* Здесь передаем управление отладчику */
	QSPCallState state;
	char *strUTF8;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_DEBUG])
	{
		qspSaveCallState(&state);
		if (str)
		{
			strUTF8 = qspW2C(str);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		qspCallBacks[QSP_CALL_DEBUG](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallSetTimer(long msecs)
{
	/* Здесь устанавливаем интервал таймера */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_SETTIMER])
	{
		qspSaveCallState(&state);
		args = AS3_Array("IntType", msecs);
		qspCallBacks[QSP_CALL_SETTIMER](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallRefreshInt(QSP_BOOL isRedraw)
{
	/* Здесь выполняем обновление интерфейса */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_REFRESHINT])
	{
		qspSaveCallState(&state);
		args = AS3_Array("IntType", isRedraw);
		qspCallBacks[QSP_CALL_REFRESHINT](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallSetInputStrText(QSP_CHAR *text)
{
	/* Здесь устанавливаем текст строки ввода */
	QSPCallState state;
	AS3_Val args;
	char *textUTF8;
	if (qspCallBacks[QSP_CALL_SETINPUTSTRTEXT])
	{
		qspSaveCallState(&state);
		if (text)
		{
			textUTF8 = qspW2C(text);
			args = AS3_Array("StrType", textUTF8);
			free(textUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		qspCallBacks[QSP_CALL_SETINPUTSTRTEXT](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallAddMenuItem(QSP_CHAR *name, QSP_CHAR *imgPath)
{
	/* Здесь добавляем пункт меню */
	QSPCallState state;
	AS3_Val args;
	char *nameUTF8;
	char *imgUTF8;
	if (qspCallBacks[QSP_CALL_ADDMENUITEM])
	{
		qspSaveCallState(&state);
		nameUTF8 = (name ? qspW2C(name) : 0);
		imgUTF8 = (imgPath ? qspW2C(imgPath) : 0);
		args = AS3_Array("StrType, StrType", nameUTF8, imgUTF8);
		if (nameUTF8) free(nameUTF8);
		if (imgUTF8) free(imgUTF8);
		qspCallBacks[QSP_CALL_ADDMENUITEM](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallSystem(QSP_CHAR *cmd)
{
	/* Здесь выполняем системный вызов */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_SYSTEM])
	{
		qspSaveCallState(&state);
		if (cmd)
		{
			strUTF8 = qspW2C(cmd);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		qspCallBacks[QSP_CALL_SYSTEM](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallOpenGame()
{
	/* Здесь позволяем пользователю выбрать файл */
	/* состояния игры для загрузки и загружаем его */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_OPENGAMESTATUS])
	{
		qspSaveCallState(&state);
		args = AS3_Array("");
		qspCallBacks[QSP_CALL_OPENGAMESTATUS](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallSaveGame()
{
	/* Здесь позволяем пользователю выбрать файл */
	/* для сохранения состояния игры и сохраняем */
	/* в нем текущее состояние */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_SAVEGAMESTATUS])
	{
		qspSaveCallState(&state);
		args = AS3_Array("");
		qspCallBacks[QSP_CALL_SAVEGAMESTATUS](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallShowMessage(QSP_CHAR *text)
{
	/* Здесь показываем сообщение */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_SHOWMSGSTR])
	{
		qspSaveCallState(&state);
		if (text)
		{
			strUTF8 = qspW2C(text);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		qspCallBacks[QSP_CALL_SHOWMSGSTR](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallShowMenu()
{
	/* Здесь показываем меню */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_SHOWMENU])
	{
		qspSaveCallState(&state);
		args = AS3_Array("");
		qspCallBacks[QSP_CALL_SHOWMENU](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallShowPicture(QSP_CHAR *file)
{
	/* Здесь показываем изображение */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_SHOWIMAGE])
	{
		qspSaveCallState(&state);
		if (file)
		{
			strUTF8 = qspW2C(file);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		qspCallBacks[QSP_CALL_SHOWIMAGE](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallShowWindow(long type, QSP_BOOL isShow)
{
	/* Здесь показываем или скрываем окно */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_SHOWWINDOW])
	{
		qspSaveCallState(&state);
		args = AS3_Array("IntType, IntType", type, isShow);
		qspCallBacks[QSP_CALL_SHOWWINDOW](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallPlayFile(QSP_CHAR *file, long volume)
{
	/* Здесь начинаем воспроизведение файла с заданной громкостью */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_PLAYFILE])
	{
		qspSaveCallState(&state);
		if (file)
		{
			strUTF8 = qspW2C(file);
			args = AS3_Array("StrType, IntType", strUTF8, volume);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType, IntType", 0, volume);
		qspCallBacks[QSP_CALL_PLAYFILE](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

QSP_BOOL qspCallIsPlayingFile(QSP_CHAR *file)
{
	/* Здесь проверяем, проигрывается ли файл */
	QSPCallState state;
	QSP_BOOL isPlaying;
	AS3_Val args;
	AS3_Val res;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_ISPLAYINGFILE])
	{
		qspSaveCallState(&state);
		if (file)
		{
			strUTF8 = qspW2C(file);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		res = qspCallBacks[QSP_CALL_ISPLAYINGFILE](args);
		AS3_Release(args);
		isPlaying = (QSP_BOOL)AS3_IntValue(res);
		AS3_Release(res);
		qspRestoreCallState(&state);
		return isPlaying;
	}
	return QSP_FALSE;
}

void qspCallSleep(long msecs)
{
	/* Здесь ожидаем заданное количество миллисекунд */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_SLEEP])
	{
		qspSaveCallState(&state);
		args = AS3_Array("IntType", msecs);
		qspCallBacks[QSP_CALL_SLEEP](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

long qspCallGetMSCount()
{
	/* Здесь получаем количество миллисекунд, прошедших с момента последнего вызова функции */
	QSPCallState state;
	long count;
	AS3_Val args;
	AS3_Val res;
	if (qspCallBacks[QSP_CALL_GETMSCOUNT])
	{
		qspSaveCallState(&state);
		args = AS3_Array("");
		res = qspCallBacks[QSP_CALL_GETMSCOUNT](args);
		AS3_Release(args);
		count = AS3_IntValue(res);
		AS3_Release(res);
		qspRestoreCallState(&state);
		return count;
	}
	return 0;
}

void qspCallCloseFile(QSP_CHAR *file)
{
	/* Здесь выполняем закрытие файла */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_CLOSEFILE])
	{
		qspSaveCallState(&state);
		if (file)
		{
			strUTF8 = qspW2C(file);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		qspCallBacks[QSP_CALL_CLOSEFILE](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

void qspCallDeleteMenu()
{
	/* Здесь удаляем текущее меню */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_DELETEMENU])
	{
		qspSaveCallState(&state);
		args = AS3_Array("");
		qspCallBacks[QSP_CALL_DELETEMENU](args);
		AS3_Release(args);
		qspRestoreCallState(&state);
	}
}

QSP_CHAR *qspCallInputBox(QSP_CHAR *text)
{
	/* Здесь вводим текст */
	QSPCallState state;
	QSP_CHAR *buffer;
	AS3_Val args;
	AS3_Val res;
	char *strUTF8;
	char *resText;
	long maxLen = 511;
	if (qspCallBacks[QSP_CALL_INPUTBOX])
	{
		qspSaveCallState(&state);
		if (text)
		{
			strUTF8 = qspW2C(text);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		res = qspCallBacks[QSP_CALL_INPUTBOX](args);
		AS3_Release(args);
		resText = AS3_StringValue(res);
		AS3_Release(res);
		buffer = qspC2W(resText);
		free(resText);
		qspRestoreCallState(&state);
	}
	else
		buffer = qspGetNewText(QSP_FMT(""), 0);
	return buffer;
}

#endif
