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

#include "../../declarations.h"

#ifdef _DEFAULT_BINDING

#include "../../actions.h"
#include "../../callbacks.h"
#include "../../coding.h"
#include "../../common.h"
#include "../../errors.h"
#include "../../game.h"
#include "../../locations.h"
#include "../../mathops.h"
#include "../../menu.h"
#include "../../objects.h"
#include "../../statements.h"
#include "../../text.h"
#include "../../time.h"
#include "../../variables.h"
#include "../../variant.h"

/* ------------------------------------------------------------ */
QSP_BOOL QSPIsInCallBack()
{
	return qspIsInCallBack;
}
/* ------------------------------------------------------------ */
/* Отладка */

/* Управление режимом отладки */
void QSPEnableDebugMode(QSP_BOOL isDebug)
{
	qspIsDebug = isDebug;
}
/* Получение данных текущего состояния */
void QSPGetCurStateData(QSP_CHAR **loc, int *actIndex, int *line)
{
	*loc = (qspRealCurLoc >= 0 && qspRealCurLoc < qspLocsCount ? qspLocs[qspRealCurLoc].Name : 0);
	*actIndex = qspRealActIndex;
	*line = qspRealLine;
}
/* ------------------------------------------------------------ */
/* Информация о версии */

/* Версия */
const QSP_CHAR *QSPGetVersion()
{
	return QSP_VER;
}
/* Дата и время компиляции */
const QSP_CHAR *QSPGetCompiledDateTime()
{
	return QSP_FMT(__DATE__) QSP_FMT(", ") QSP_FMT(__TIME__);
}
/* ------------------------------------------------------------ */
/* Количество полных обновлений локаций */
int QSPGetFullRefreshCount()
{
	return qspFullRefreshCount;
}
/* ------------------------------------------------------------ */
/* Полный путь к загруженному файлу игры */
const QSP_CHAR *QSPGetQstFullPath()
{
	return qspQstFullPath;
}
/* ------------------------------------------------------------ */
/* Название текущей локации */
const QSP_CHAR *QSPGetCurLoc()
{
	return (qspCurLoc >= 0 ? qspLocs[qspCurLoc].Name : 0);
}
/* ------------------------------------------------------------ */
/* Основное описание локации */

/* Текст основного окна описания локации */
const QSP_CHAR *QSPGetMainDesc()
{
	return qspCurDesc;
}
/* Возможность изменения текста основного описания */
QSP_BOOL QSPIsMainDescChanged()
{
	return qspIsMainDescChanged;
}
/* ------------------------------------------------------------ */
/* Дополнительное описание локации */

/* Текст дополнительного окна описания локации */
const QSP_CHAR *QSPGetVarsDesc()
{
	return qspCurVars;
}
/* Возможность изменения текста дополнительного описания */
QSP_BOOL QSPIsVarsDescChanged()
{
	return qspIsVarsDescChanged;
}
/* ------------------------------------------------------------ */
/* Получить значение указанного выражения */
QSP_BOOL QSPGetExprValue(const QSP_CHAR *expr, QSP_BOOL *isString, int *numVal, QSP_CHAR *strVal, int strValBufSize)
{
	QSPVariant v;
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspResetError();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	v = qspExprValue((QSP_CHAR *)expr);
	if (qspErrorNum) return QSP_FALSE;
	*isString = v.IsStr;
	if (v.IsStr)
	{
		qspStrNCopy(strVal, QSP_STR(v), strValBufSize - 1);
		free(QSP_STR(v));
		strVal[strValBufSize - 1] = 0;
	}
	else
		*numVal = QSP_NUM(v);
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Текст строки ввода */
void QSPSetInputStrText(const QSP_CHAR *val)
{
	qspCurInputLen = qspAddText(&qspCurInput, (QSP_CHAR *)val, 0, -1, QSP_FALSE);
}
/* ------------------------------------------------------------ */
/* Список действий */

/* Количество действий */
int QSPGetActionsCount()
{
	return qspCurActionsCount;
}
/* Данные действия с указанным индексом */
void QSPGetActionData(int ind, QSP_CHAR **image, QSP_CHAR **desc)
{
	if (ind >= 0 && ind < qspCurActionsCount)
	{
		*image = qspCurActions[ind].Image;
		*desc = qspCurActions[ind].Desc;
	}
	else
		*image = *desc = 0;
}
/* Выполнение кода выбранного действия */
QSP_BOOL QSPExecuteSelActionCode(QSP_BOOL isRefresh)
{
	if (qspCurSelAction >= 0)
	{
		if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
		qspPrepareExecution();
		if (qspIsDisableCodeExec) return QSP_FALSE;
		qspExecAction(qspCurSelAction);
		if (qspErrorNum) return QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	return QSP_TRUE;
}
/* Установить индекс выбранного действия */
QSP_BOOL QSPSetSelActionIndex(int ind, QSP_BOOL isRefresh)
{
	if (ind >= 0 && ind < qspCurActionsCount && ind != qspCurSelAction)
	{
		if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
		qspPrepareExecution();
		if (qspIsDisableCodeExec) return QSP_FALSE;
		qspCurSelAction = ind;
		qspExecLocByVarNameWithArgs(QSP_FMT("ONACTSEL"), 0, 0);
		if (qspErrorNum) return QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	return QSP_TRUE;
}
/* Получить индекс выбранного действия */
int QSPGetSelActionIndex()
{
	return qspCurSelAction;
}
/* Возможность изменения списка действий */
QSP_BOOL QSPIsActionsChanged()
{
	return qspIsActionsChanged;
}
/* ------------------------------------------------------------ */
/* Список объектов */

/* Количество объектов */
int QSPGetObjectsCount()
{
	return qspCurObjectsCount;
}
/* Данные объекта с указанным индексом */
void QSPGetObjectData(int ind, QSP_CHAR **image, QSP_CHAR **desc)
{
	if (ind >= 0 && ind < qspCurObjectsCount)
	{
		*image = qspCurObjects[ind].Image;
		*desc = qspCurObjects[ind].Desc;
	}
	else
		*image = *desc = 0;
}
/* Установить индекс выбранного объекта */
QSP_BOOL QSPSetSelObjectIndex(int ind, QSP_BOOL isRefresh)
{
	if (ind >= 0 && ind < qspCurObjectsCount && ind != qspCurSelObject)
	{
		if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
		qspPrepareExecution();
		if (qspIsDisableCodeExec) return QSP_FALSE;
		qspCurSelObject = ind;
		qspExecLocByVarNameWithArgs(QSP_FMT("ONOBJSEL"), 0, 0);
		if (qspErrorNum) return QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	return QSP_TRUE;
}
/* Получить индекс выбранного объекта */
int QSPGetSelObjectIndex()
{
	return qspCurSelObject;
}
/* Возможность изменения списка объектов */
QSP_BOOL QSPIsObjectsChanged()
{
	return qspIsObjectsChanged;
}
/* ------------------------------------------------------------ */
/* Показ / скрытие окон */
void QSPShowWindow(int type, QSP_BOOL isShow)
{
	switch (type)
	{
	case QSP_WIN_ACTS:
		qspCurIsShowActs = isShow;
		break;
	case QSP_WIN_OBJS:
		qspCurIsShowObjs = isShow;
		break;
	case QSP_WIN_VARS:
		qspCurIsShowVars = isShow;
		break;
	case QSP_WIN_INPUT:
		qspCurIsShowInput = isShow;
		break;
	}
}
/* ------------------------------------------------------------ */
/* Переменные */

/* Получить количество элементов массива */
QSP_BOOL QSPGetVarValuesCount(const QSP_CHAR *name, int *count)
{
	QSPVar *var;
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspResetError();
	var = qspVarReference((QSP_CHAR *)name, QSP_FALSE);
	if (qspErrorNum) return QSP_FALSE;
	*count = var->ValsCount;
	return QSP_TRUE;
}
/* Получить значения указанного элемента массива */
QSP_BOOL QSPGetVarValues(const QSP_CHAR *name, int ind, int *numVal, QSP_CHAR **strVal)
{
	QSPVar *var;
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspResetError();
	var = qspVarReference((QSP_CHAR *)name, QSP_FALSE);
	if (qspErrorNum || ind < 0 || ind >= var->ValsCount) return QSP_FALSE;
	*numVal = var->Values[ind].Num;
	*strVal = var->Values[ind].Str;
	return QSP_TRUE;
}
/* Получить максимальное количество переменных */
int QSPGetMaxVarsCount()
{
	return QSP_VARSCOUNT;
}
/* Получить имя переменной с указанным индексом */
QSP_BOOL QSPGetVarNameByIndex(int index, QSP_CHAR **name)
{
	if (index < 0 || index >= QSP_VARSCOUNT || !qspVars[index].Name) return QSP_FALSE;
	*name = qspVars[index].Name;
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Выполнение кода */

/* Выполнение строки кода */
QSP_BOOL QSPExecString(const QSP_CHAR *s, QSP_BOOL isRefresh)
{
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	qspExecStringAsCodeWithArgs((QSP_CHAR *)s, 0, 0, 0);
	if (qspErrorNum) return QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* Выполнение кода указанной локации */
QSP_BOOL QSPExecLocationCode(const QSP_CHAR *name, QSP_BOOL isRefresh)
{
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	qspExecLocByName((QSP_CHAR *)name, QSP_FALSE);
	if (qspErrorNum) return QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* Выполнение кода локации-счетчика */
QSP_BOOL QSPExecCounter(QSP_BOOL isRefresh)
{
	if (!qspIsInCallBack)
	{
		qspPrepareExecution();
		qspExecLocByVarNameWithArgs(QSP_FMT("COUNTER"), 0, 0);
		if (qspErrorNum) return QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	return QSP_TRUE;
}
/* Выполнение кода локации-обработчика строки ввода */
QSP_BOOL QSPExecUserInput(QSP_BOOL isRefresh)
{
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	qspExecLocByVarNameWithArgs(QSP_FMT("USERCOM"), 0, 0);
	if (qspErrorNum) return QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Ошибки */

/* Получить информацию о последней ошибке */
void QSPGetLastErrorData(int *errorNum, QSP_CHAR **errorLoc, int *errorActIndex, int *errorLine)
{
	*errorNum = qspErrorNum;
	*errorLoc = (qspErrorLoc >= 0 && qspErrorLoc < qspLocsCount ? qspLocs[qspErrorLoc].Name : 0);
	*errorActIndex = qspErrorActIndex;
	*errorLine = qspErrorLine;
}
/* Получить описание ошибки по ее номеру */
const QSP_CHAR *QSPGetErrorDesc(int errorNum)
{
	return qspGetErrorDesc(errorNum);
}
/* ------------------------------------------------------------ */
/* Управление игрой */

/* Загрузка новой игры из файла */
QSP_BOOL QSPLoadGameWorld(const QSP_CHAR *fileName)
{
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspResetError();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	qspOpenQuest((QSP_CHAR *)fileName, QSP_FALSE);
	if (qspErrorNum) return QSP_FALSE;
	return QSP_TRUE;
}
/* Загрузка новой игры из памяти */
QSP_BOOL QSPLoadGameWorldFromData(const void *data, int dataSize, const QSP_CHAR *fileName)
{
	char *ptr;
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspResetError();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	ptr = (char *)malloc(dataSize + 3);
	memcpy(ptr, data, dataSize);
	ptr[dataSize] = ptr[dataSize + 1] = ptr[dataSize + 2] = 0;
	qspOpenQuestFromData(ptr, dataSize + 3, (QSP_CHAR *)fileName, QSP_FALSE);
	free(ptr);
	if (qspErrorNum) return QSP_FALSE;
	return QSP_TRUE;
}
/* Сохранение состояния в файл */
QSP_BOOL QSPSaveGame(const QSP_CHAR *fileName, QSP_BOOL isRefresh)
{
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	qspSaveGameStatus((QSP_CHAR *)fileName);
	if (qspErrorNum) return QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* Сохранение состояния в память */
QSP_BOOL QSPSaveGameAsData(void *buf, int bufSize, int *realSize, QSP_BOOL isRefresh)
{
	int len, size;
	QSP_CHAR *data;
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	if (!(len = qspSaveGameStatusToString(&data)))
	{
		*realSize = 0;
		return QSP_FALSE;
	}
	size = len * sizeof(QSP_CHAR);
	*realSize = size;
	if (size > bufSize)
	{
		free(data);
		return QSP_FALSE;
	}
	memcpy(buf, data, size);
	free(data);
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* Загрузка состояния из файла */
QSP_BOOL QSPOpenSavedGame(const QSP_CHAR *fileName, QSP_BOOL isRefresh)
{
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	qspOpenGameStatus((QSP_CHAR *)fileName);
	if (qspErrorNum) return QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* Загрузка состояния из памяти */
QSP_BOOL QSPOpenSavedGameFromData(const void *data, int dataSize, QSP_BOOL isRefresh)
{
	int dataLen;
	QSP_CHAR *ptr;
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	dataLen = dataSize / sizeof(QSP_CHAR);
	ptr = (QSP_CHAR *)malloc((dataLen + 1) * sizeof(QSP_CHAR));
	memcpy(ptr, data, dataSize);
	ptr[dataLen] = 0;
	qspOpenGameStatusFromString(ptr);
	free(ptr);
	if (qspErrorNum) return QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* Перезапуск игры */
QSP_BOOL QSPRestartGame(QSP_BOOL isRefresh)
{
	if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return QSP_FALSE;
	qspNewGame(QSP_TRUE);
	if (qspErrorNum) return QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Установка CALLBACK'ов */
void QSPSetCallBack(int type, QSP_CALLBACK func)
{
	qspSetCallBack(type, func);
}
/* ------------------------------------------------------------ */
/* Инициализация */
void QSPInit()
{
	#ifdef _DEBUG
		mwInit();
	#endif
	qspIsDebug = QSP_FALSE;
	qspRefreshCount = qspFullRefreshCount = 0;
	qspQstPath = qspQstFullPath = 0;
	qspQstPathLen = 0;
	qspQstCRC = 0;
	qspRealCurLoc = -1;
	qspRealActIndex = -1;
	qspRealLine = 0;
	qspMSCount = 0;
	qspLocs = 0;
	qspLocsNames = 0;
	qspLocsCount = 0;
	qspCurLoc = -1;
	qspTimerInterval = 0;
	qspCurIsShowObjs = qspCurIsShowActs = qspCurIsShowVars = qspCurIsShowInput = QSP_TRUE;
	setlocale(LC_ALL, QSP_LOCALE);
	qspSetSeed(0);
	qspPrepareExecution();
	qspMemClear(QSP_TRUE);
	qspInitCallBacks();
	qspInitStats();
	qspInitMath();
}
/* Деинициализация */
void QSPDeInit()
{
	qspMemClear(QSP_FALSE);
	qspCreateWorld(0, 0);
	if (qspQstPath) free(qspQstPath);
	if (qspQstFullPath) free(qspQstFullPath);
	#ifdef _DEBUG
		mwTerm();
	#endif
}

#endif
