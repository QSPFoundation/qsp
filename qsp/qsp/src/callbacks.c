/* Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru) */
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

QSP_CALLBACK qspCallBacks[QSP_CALL_DUMMY];

void qspInitCallBacks()
{
	long i;
	for (i = 0; i < QSP_CALL_DUMMY; ++i)
		qspCallBacks[i] = 0;
}

void qspSetCallBack(long type, QSP_CALLBACK func)
{
	qspCallBacks[type] = func;
}

void qspCallSetTimer(long msecs)
{
	/* Здесь устанавливаем интервал таймера */
	if (qspCallBacks[QSP_CALL_SETTIMER]) qspCallBacks[QSP_CALL_SETTIMER](msecs);
}

void qspCallRefreshInt(QSP_BOOL isRedraw)
{
	/* Здесь выполняем обновление интерфейса */
	if (qspCallBacks[QSP_CALL_REFRESHINT]) qspCallBacks[QSP_CALL_REFRESHINT](isRedraw);
}

void qspCallSetInputStrText(QSP_CHAR *text)
{
	/* Здесь устанавливаем текст строки ввода */
	if (qspCallBacks[QSP_CALL_SETINPUTSTRTEXT]) qspCallBacks[QSP_CALL_SETINPUTSTRTEXT](text);
}

void qspCallAddMenuItem(QSP_CHAR *name, QSP_CHAR *imgPath)
{
	/* Здесь добавляем пункт меню */
	if (qspCallBacks[QSP_CALL_ADDMENUITEM]) qspCallBacks[QSP_CALL_ADDMENUITEM](name, imgPath);
}

void qspCallSystem(QSP_CHAR *cmd)
{
	/* Здесь выполняем системный вызов */
	if (qspCallBacks[QSP_CALL_SYSTEM]) qspCallBacks[QSP_CALL_SYSTEM](cmd);
}

void qspCallOpenGame()
{
	/* Здесь позволяем пользователю выбрать файл */
	/* состояния игры для загрузки и загружаем его */
	if (qspCallBacks[QSP_CALL_OPENGAMESTATUS]) qspCallBacks[QSP_CALL_OPENGAMESTATUS]();
}

void qspCallSaveGame()
{
	/* Здесь позволяем пользователю выбрать файл */
	/* для сохранения состояния игры и сохраняем */
	/* в нем текущее состояние */
	if (qspCallBacks[QSP_CALL_SAVEGAMESTATUS]) qspCallBacks[QSP_CALL_SAVEGAMESTATUS]();
}

void qspCallShowMessage(QSP_CHAR *text)
{
	/* Здесь показываем сообщение */
	if (qspCallBacks[QSP_CALL_SHOWMSGSTR]) qspCallBacks[QSP_CALL_SHOWMSGSTR](text);
}

void qspCallShowMenu()
{
	/* Здесь показываем меню */
	if (qspCallBacks[QSP_CALL_SHOWMENU]) qspCallBacks[QSP_CALL_SHOWMENU]();
}

void qspCallShowPicture(QSP_CHAR *file)
{
	/* Здесь показываем изображение */
	if (qspCallBacks[QSP_CALL_SHOWIMAGE]) qspCallBacks[QSP_CALL_SHOWIMAGE](file);
}

void qspCallShowWindow(long type, QSP_BOOL isShow)
{
	/* Здесь показываем или скрываем окно */
	if (qspCallBacks[QSP_CALL_SHOWWINDOW]) qspCallBacks[QSP_CALL_SHOWWINDOW](type, isShow);
}

void qspCallPlayFile(QSP_CHAR *file, long volume)
{
	/* Здесь начинаем воспроизведение файла с заданной громкостью */
	if (qspCallBacks[QSP_CALL_PLAYFILE]) qspCallBacks[QSP_CALL_PLAYFILE](file, volume);
}

QSP_BOOL qspCallIsPlayingFile(QSP_CHAR *file)
{
	/* Здесь проверяем, проигрывается ли файл */
	return (qspCallBacks[QSP_CALL_ISPLAYINGFILE] && (QSP_BOOL)qspCallBacks[QSP_CALL_ISPLAYINGFILE](file));
}

void qspCallSleep(long msecs)
{
	/* Здесь ожидаем заданное количество миллисекунд */
	if (qspCallBacks[QSP_CALL_SLEEP]) qspCallBacks[QSP_CALL_SLEEP](msecs);
}

long qspCallGetMSCount()
{
	/* Здесь получаем количество миллисекунд, прошедших с момента последнего вызова функции */
	return (qspCallBacks[QSP_CALL_GETMSCOUNT] ? qspCallBacks[QSP_CALL_GETMSCOUNT]() : 0);
}

void qspCallCloseFile(QSP_CHAR *file)
{
	/* Здесь выполняем закрытие файла */
	if (qspCallBacks[QSP_CALL_CLOSEFILE]) qspCallBacks[QSP_CALL_CLOSEFILE](file);
}

void qspCallDeleteMenu()
{
	/* Здесь удаляем текущее меню */
	if (qspCallBacks[QSP_CALL_DELETEMENU]) qspCallBacks[QSP_CALL_DELETEMENU]();
}

QSP_CHAR *qspCallInputBox(QSP_CHAR *text)
{
	/* Здесь вводим текст */
	QSP_CHAR *buffer;
	long maxLen = 511;
	if (qspCallBacks[QSP_CALL_INPUTBOX])
	{
		buffer = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
		qspCallBacks[QSP_CALL_INPUTBOX](text, buffer, maxLen);
	}
	else
		buffer = qspGetNewText(QSP_FMT(""), 0);
	return buffer;
}
