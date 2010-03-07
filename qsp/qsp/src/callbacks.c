/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
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
volatile QSP_BOOL qspIsDisableCodeExec = QSP_FALSE;

#ifdef _FLASH
	AS3_Val result;
#endif

static void qspSaveCallState(QSPCallState *, QSP_BOOL);
static void qspRestoreCallState(QSPCallState *);

static void qspSaveCallState(QSPCallState *state, QSP_BOOL isDisableCodeExec)
{
	state->IsInCallBack = qspIsInCallBack;
	state->IsDisableCodeExec = qspIsDisableCodeExec;
	state->IsMustWait = qspIsMustWait;
	state->IsMainDescChanged = qspIsMainDescChanged;
	state->IsVarsDescChanged = qspIsVarsDescChanged;
	state->IsObjectsChanged = qspIsObjectsChanged;
	state->IsActionsChanged = qspIsActionsChanged;
	qspIsInCallBack = QSP_TRUE;
	qspIsDisableCodeExec = isDisableCodeExec;
	qspIsMustWait = QSP_FALSE;
}

static void qspRestoreCallState(QSPCallState *state)
{
	qspIsMustWait = state->IsMustWait;
	qspIsDisableCodeExec = state->IsDisableCodeExec;
	qspIsInCallBack = state->IsInCallBack;
	if (state->IsActionsChanged) qspIsActionsChanged = QSP_TRUE;
	if (state->IsObjectsChanged) qspIsObjectsChanged = QSP_TRUE;
	if (state->IsVarsDescChanged) qspIsVarsDescChanged = QSP_TRUE;
	if (state->IsMainDescChanged) qspIsMainDescChanged = QSP_TRUE;
	qspResetError();
}

#ifndef _FLASH

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

void qspCallDebug(QSP_CHAR *str)
{
	/* Здесь передаем управление отладчику */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_DEBUG])
	{
		qspSaveCallState(&state, QSP_FALSE);
		qspCallBacks[QSP_CALL_DEBUG](str);
		qspRestoreCallState(&state);
	}
}

void qspCallSetTimer(int msecs)
{
	/* Здесь устанавливаем интервал таймера */
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
	/* Здесь выполняем обновление интерфейса */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_REFRESHINT])
	{
		qspSaveCallState(&state, QSP_TRUE);
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
		qspSaveCallState(&state, QSP_TRUE);
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
		qspSaveCallState(&state, QSP_TRUE);
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
		qspSaveCallState(&state, QSP_FALSE);
		qspCallBacks[QSP_CALL_SYSTEM](cmd);
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
		qspSaveCallState(&state, QSP_FALSE);
		qspCallBacks[QSP_CALL_OPENGAMESTATUS](file);
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
		qspSaveCallState(&state, QSP_FALSE);
		qspCallBacks[QSP_CALL_SAVEGAMESTATUS](file);
		qspRestoreCallState(&state);
	}
}

void qspCallShowMessage(QSP_CHAR *text)
{
	/* Здесь показываем сообщение */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SHOWMSGSTR])
	{
		qspSaveCallState(&state, QSP_TRUE);
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
		qspSaveCallState(&state, QSP_FALSE);
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
		qspSaveCallState(&state, QSP_TRUE);
		qspCallBacks[QSP_CALL_SHOWIMAGE](file);
		qspRestoreCallState(&state);
	}
}

void qspCallShowWindow(int type, QSP_BOOL isShow)
{
	/* Здесь показываем или скрываем окно */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_SHOWWINDOW])
	{
		qspSaveCallState(&state, QSP_TRUE);
		qspCallBacks[QSP_CALL_SHOWWINDOW](type, isShow);
		qspRestoreCallState(&state);
	}
}

void qspCallPlayFile(QSP_CHAR *file, int volume)
{
	/* Здесь начинаем воспроизведение файла с заданной громкостью */
	QSPCallState state;
	if (qspCallBacks[QSP_CALL_PLAYFILE])
	{
		qspSaveCallState(&state, QSP_TRUE);
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
		qspSaveCallState(&state, QSP_TRUE);
		isPlaying = (QSP_BOOL)qspCallBacks[QSP_CALL_ISPLAYINGFILE](file);
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
		qspSaveCallState(&state, QSP_TRUE);
		qspCallBacks[QSP_CALL_SLEEP](msecs);
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
		qspSaveCallState(&state, QSP_TRUE);
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
		qspSaveCallState(&state, QSP_TRUE);
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
		qspSaveCallState(&state, QSP_TRUE);
		qspCallBacks[QSP_CALL_DELETEMENU]();
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
		qspSaveCallState(&state, QSP_TRUE);
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

void qspInitCallBacks()
{
	int i;
	qspIsInCallBack = QSP_FALSE;
	qspIsDisableCodeExec = QSP_FALSE;
	for (i = 0; i < QSP_CALL_DUMMY; ++i)
		qspCallBacks[i].IsSet = QSP_FALSE;
}

void qspSetCallBack(int type, QSP_CALLBACK func)
{
	qspCallBacks[type] = func;
	AS3_Acquire(qspCallBacks[type].ThisVal);
	AS3_Acquire(qspCallBacks[type].FuncVal);
	qspCallBacks[type].IsSet = QSP_TRUE;
}

void qspCallDebug(QSP_CHAR *str)
{
	/* Здесь передаем управление отладчику */
	QSPCallState state;
	char *strUTF8;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_DEBUG].IsSet)
	{
		qspSaveCallState(&state, QSP_FALSE);
		if (str)
		{
			strUTF8 = qspW2C(str);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		AS3_Call(qspCallBacks[QSP_CALL_DEBUG].FuncVal, qspCallBacks[QSP_CALL_DEBUG].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallSetTimer(int msecs)
{
	/* Здесь устанавливаем интервал таймера */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_SETTIMER].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		args = AS3_Array("IntType", msecs);
		AS3_Call(qspCallBacks[QSP_CALL_SETTIMER].FuncVal, qspCallBacks[QSP_CALL_SETTIMER].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallRefreshInt(QSP_BOOL isRedraw)
{
	/* Здесь выполняем обновление интерфейса */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_REFRESHINT].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		args = AS3_Array("IntType", isRedraw);
		AS3_Call(qspCallBacks[QSP_CALL_REFRESHINT].FuncVal, qspCallBacks[QSP_CALL_REFRESHINT].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallSetInputStrText(QSP_CHAR *text)
{
	/* Здесь устанавливаем текст строки ввода */
	QSPCallState state;
	AS3_Val args;
	char *textUTF8;
	if (qspCallBacks[QSP_CALL_SETINPUTSTRTEXT].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		if (text)
		{
			textUTF8 = qspW2C(text);
			args = AS3_Array("StrType", textUTF8);
			free(textUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		AS3_Call(qspCallBacks[QSP_CALL_SETINPUTSTRTEXT].FuncVal, qspCallBacks[QSP_CALL_SETINPUTSTRTEXT].ThisVal, args);
		AS3_Release(args);
		flyield();
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
	if (qspCallBacks[QSP_CALL_ADDMENUITEM].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		nameUTF8 = (name ? qspW2C(name) : 0);
		imgUTF8 = (imgPath ? qspW2C(imgPath) : 0);
		args = AS3_Array("StrType, StrType", nameUTF8, imgUTF8);
		if (nameUTF8) free(nameUTF8);
		if (imgUTF8) free(imgUTF8);
		AS3_Call(qspCallBacks[QSP_CALL_ADDMENUITEM].FuncVal, qspCallBacks[QSP_CALL_ADDMENUITEM].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallSystem(QSP_CHAR *cmd)
{
	/* Здесь выполняем системный вызов */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_SYSTEM].IsSet)
	{
		qspSaveCallState(&state, QSP_FALSE);
		if (cmd)
		{
			strUTF8 = qspW2C(cmd);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		AS3_Call(qspCallBacks[QSP_CALL_SYSTEM].FuncVal, qspCallBacks[QSP_CALL_SYSTEM].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallOpenGame(QSP_CHAR *file)
{
	/* Здесь позволяем пользователю выбрать файл */
	/* состояния игры для загрузки и загружаем его */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_OPENGAMESTATUS].IsSet)
	{
		qspSaveCallState(&state, QSP_FALSE);
		if (file)
		{
			strUTF8 = qspW2C(file);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		AS3_Call(qspCallBacks[QSP_CALL_OPENGAMESTATUS].FuncVal, qspCallBacks[QSP_CALL_OPENGAMESTATUS].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallSaveGame(QSP_CHAR *file)
{
	/* Здесь позволяем пользователю выбрать файл */
	/* для сохранения состояния игры и сохраняем */
	/* в нем текущее состояние */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_SAVEGAMESTATUS].IsSet)
	{
		qspSaveCallState(&state, QSP_FALSE);
		if (file)
		{
			strUTF8 = qspW2C(file);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		AS3_Call(qspCallBacks[QSP_CALL_SAVEGAMESTATUS].FuncVal, qspCallBacks[QSP_CALL_SAVEGAMESTATUS].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallShowMessage(QSP_CHAR *text)
{
	/* Здесь показываем сообщение */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_SHOWMSGSTR].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		if (text)
		{
			strUTF8 = qspW2C(text);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		AS3_Call(qspCallBacks[QSP_CALL_SHOWMSGSTR].FuncVal, qspCallBacks[QSP_CALL_SHOWMSGSTR].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallShowMenu()
{
	/* Здесь показываем меню */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_SHOWMENU].IsSet)
	{
		qspSaveCallState(&state, QSP_FALSE);
		args = AS3_Array("");
		AS3_Call(qspCallBacks[QSP_CALL_SHOWMENU].FuncVal, qspCallBacks[QSP_CALL_SHOWMENU].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallShowPicture(QSP_CHAR *file)
{
	/* Здесь показываем изображение */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_SHOWIMAGE].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		if (file)
		{
			strUTF8 = qspW2C(file);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		AS3_Call(qspCallBacks[QSP_CALL_SHOWIMAGE].FuncVal, qspCallBacks[QSP_CALL_SHOWIMAGE].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallShowWindow(int type, QSP_BOOL isShow)
{
	/* Здесь показываем или скрываем окно */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_SHOWWINDOW].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		args = AS3_Array("IntType, IntType", type, isShow);
		AS3_Call(qspCallBacks[QSP_CALL_SHOWWINDOW].FuncVal, qspCallBacks[QSP_CALL_SHOWWINDOW].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallPlayFile(QSP_CHAR *file, int volume)
{
	/* Здесь начинаем воспроизведение файла с заданной громкостью */
	QSPCallState state;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_PLAYFILE].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		if (file)
		{
			strUTF8 = qspW2C(file);
			args = AS3_Array("StrType, IntType", strUTF8, volume);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType, IntType", 0, volume);
		AS3_Call(qspCallBacks[QSP_CALL_PLAYFILE].FuncVal, qspCallBacks[QSP_CALL_PLAYFILE].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

QSP_BOOL qspCallIsPlayingFile(QSP_CHAR *file)
{
	/* Здесь проверяем, проигрывается ли файл */
	QSPCallState state;
	QSP_BOOL isPlaying;
	AS3_Val args;
	char *strUTF8;
	if (qspCallBacks[QSP_CALL_ISPLAYINGFILE].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		if (file)
		{
			strUTF8 = qspW2C(file);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		AS3_Call(qspCallBacks[QSP_CALL_ISPLAYINGFILE].FuncVal, qspCallBacks[QSP_CALL_ISPLAYINGFILE].ThisVal, args);
		AS3_Release(args);
		flyield();
		isPlaying = (QSP_BOOL)AS3_IntValue(result);
		AS3_Release(result);
		qspRestoreCallState(&state);
		return isPlaying;
	}
	return QSP_FALSE;
}

void qspCallSleep(int msecs)
{
	/* Здесь ожидаем заданное количество миллисекунд */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_SLEEP].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		args = AS3_Array("IntType", msecs);
		AS3_Call(qspCallBacks[QSP_CALL_SLEEP].FuncVal, qspCallBacks[QSP_CALL_SLEEP].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

int qspCallGetMSCount()
{
	/* Здесь получаем количество миллисекунд, прошедших с момента последнего вызова функции */
	QSPCallState state;
	int count;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_GETMSCOUNT].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		args = AS3_Array("");
		AS3_Call(qspCallBacks[QSP_CALL_GETMSCOUNT].FuncVal, qspCallBacks[QSP_CALL_GETMSCOUNT].ThisVal, args);
		AS3_Release(args);
		flyield();
		count = AS3_IntValue(result);
		AS3_Release(result);
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
	if (qspCallBacks[QSP_CALL_CLOSEFILE].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		if (file)
		{
			strUTF8 = qspW2C(file);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		AS3_Call(qspCallBacks[QSP_CALL_CLOSEFILE].FuncVal, qspCallBacks[QSP_CALL_CLOSEFILE].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

void qspCallDeleteMenu()
{
	/* Здесь удаляем текущее меню */
	QSPCallState state;
	AS3_Val args;
	if (qspCallBacks[QSP_CALL_DELETEMENU].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		args = AS3_Array("");
		AS3_Call(qspCallBacks[QSP_CALL_DELETEMENU].FuncVal, qspCallBacks[QSP_CALL_DELETEMENU].ThisVal, args);
		AS3_Release(args);
		flyield();
		qspRestoreCallState(&state);
	}
}

QSP_CHAR *qspCallInputBox(QSP_CHAR *text)
{
	/* Здесь вводим текст */
	QSPCallState state;
	QSP_CHAR *buffer;
	AS3_Val args;
	char *strUTF8;
	char *resText;
	int maxLen = 511;
	if (qspCallBacks[QSP_CALL_INPUTBOX].IsSet)
	{
		qspSaveCallState(&state, QSP_TRUE);
		if (text)
		{
			strUTF8 = qspW2C(text);
			args = AS3_Array("StrType", strUTF8);
			free(strUTF8);
		}
		else
			args = AS3_Array("StrType", 0);
		AS3_Call(qspCallBacks[QSP_CALL_INPUTBOX].FuncVal, qspCallBacks[QSP_CALL_INPUTBOX].ThisVal, args);
		AS3_Release(args);
		flyield();
		resText = AS3_StringValue(result);
		AS3_Release(result);
		buffer = qspC2W(resText);
		free(resText);
		qspRestoreCallState(&state);
	}
	else
		buffer = qspGetNewText(QSP_FMT(""), 0);
	return buffer;
}

void qspSetReturnValue(AS3_Val res)
{
	result = res;
	AS3_Acquire(result);
}

#endif
