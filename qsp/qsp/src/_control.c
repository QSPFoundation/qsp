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

void qspWait(QSP_BOOL isBlock)
{
	while (qspIsMustWait) qspCallSleep(2);
	if (isBlock) qspIsMustWait = QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Информация о версии */

/* Версия */
QSP_CHAR *QSPGetVersion()
{
	return QSP_VER;
}
/* Дата и время компиляции */
QSP_CHAR *QSPGetCompiledDateTime()
{
	return QSP_FMT(__DATE__) QSP_FMT(", ") QSP_FMT(__TIME__);
}
/* ------------------------------------------------------------ */
/* Количество полных обновлений локаций */
long QSPGetFullRefreshCount()
{
	qspWait(QSP_FALSE);
	return qspFullRefreshCount;
}
/* ------------------------------------------------------------ */
/* Полный путь к загруженному файлу игры */
QSP_CHAR *QSPGetQstFullPath()
{
	qspWait(QSP_FALSE);
	return qspQstFullPath;
}
/* ------------------------------------------------------------ */
/* Название текущей локации */
QSP_CHAR *QSPGetCurLoc()
{
	QSP_CHAR *ret;
	qspWait(QSP_TRUE);
	ret = qspCurLoc >= 0 ? qspLocs[qspCurLoc].Name : 0;
	qspIsMustWait = QSP_FALSE;
	return ret;
}
/* ------------------------------------------------------------ */
/* Основное описание локации */

/* Текст основного окна описания локации */
QSP_CHAR *QSPGetMainDesc()
{
	qspWait(QSP_FALSE);
	return qspCurDesc;
}
/* Возможность изменения текста основного описания */
QSP_BOOL QSPIsMainDescChanged()
{
	qspWait(QSP_FALSE);
	return qspIsMainDescChanged;
}
/* ------------------------------------------------------------ */
/* Дополнительное описание локации */

/* Текст дополнительного окна описания локации */
QSP_CHAR *QSPGetVarsDesc()
{
	qspWait(QSP_FALSE);
	return qspCurVars;
}
/* Возможность изменения текста дополнительного описания */
QSP_BOOL QSPIsVarsDescChanged()
{
	qspWait(QSP_FALSE);
	return qspIsVarsDescChanged;
}
/* ------------------------------------------------------------ */
/* Получить значение указанного выражения */
QSP_BOOL QSPGetExprValue(QSP_CHAR *expr, QSP_BOOL *isString, long *numVal, QSP_CHAR *strVal, long strValBufSize)
{
	QSPVariant v;
	qspWait(QSP_TRUE);
	qspResetError();
	v = qspExprValue(expr);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	*isString = v.IsStr;
	if (v.IsStr)
	{
		QSP_STRNCPY(strVal, v.Str, strValBufSize - 1);
		free(v.Str);
		strVal[strValBufSize - 1] = 0;
	}
	else
		*numVal = v.Num;
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Текст строки ввода */
void QSPSetInputStrText(QSP_CHAR *val)
{
	qspWait(QSP_TRUE);
	qspCurInputLen = qspAddText(&qspCurInput, val, 0, -1, QSP_FALSE);
	qspIsMustWait = QSP_FALSE;
}
/* ------------------------------------------------------------ */
/* Список действий */

/* Количество действий */
long QSPGetActionsCount()
{
	qspWait(QSP_FALSE);
	return qspCurActionsCount;
}
/* Данные действия с указанным индексом */
void QSPGetActionData(long ind, QSP_CHAR **image, QSP_CHAR **desc)
{
	qspWait(QSP_TRUE);
	if (ind >= 0 && ind < qspCurActionsCount)
	{
		*image = qspCurActions[ind].Image;
		*desc = qspCurActions[ind].Desc;
	}
	else
		*image = *desc = 0;
	qspIsMustWait = QSP_FALSE;
}
/* Выполнение кода выбранного действия */
QSP_BOOL QSPExecuteSelActionCode(QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	if (qspCurSelAction >= 0)
	{
		qspResetError();
		qspPrepareExecution();
		qspExecAction(qspCurSelAction);
		if (qspErrorNum)
		{
			qspIsMustWait = QSP_FALSE;
			return QSP_FALSE;
		}
		qspIsMustWait = QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	else
		qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* Установить индекс выбранного действия */
QSP_BOOL QSPSetSelActionIndex(long ind, QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	if (ind >= 0 && ind < qspCurActionsCount && ind != qspCurSelAction)
	{
		qspCurSelAction = ind;
		qspResetError();
		qspPrepareExecution();
		qspExecLocByVarName(QSP_FMT("ONACTSEL"));
		if (qspErrorNum)
		{
			qspIsMustWait = QSP_FALSE;
			return QSP_FALSE;
		}
		qspIsMustWait = QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	else
		qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* Получить индекс выбранного действия */
long QSPGetSelActionIndex()
{
	qspWait(QSP_FALSE);
	return qspCurSelAction;
}
/* Возможность изменения списка действий */
QSP_BOOL QSPIsActionsChanged()
{
	qspWait(QSP_FALSE);
	return qspIsActionsChanged;
}
/* ------------------------------------------------------------ */
/* Список объектов */

/* Количество объектов */
long QSPGetObjectsCount()
{
	qspWait(QSP_FALSE);
	return qspCurObjectsCount;
}
/* Данные объекта с указанным индексом */
void QSPGetObjectData(long ind, QSP_CHAR **image, QSP_CHAR **desc)
{
	qspWait(QSP_TRUE);
	if (ind >= 0 && ind < qspCurObjectsCount)
	{
		*image = qspCurObjects[ind].Image;
		*desc = qspCurObjects[ind].Desc;
	}
	else
		*image = *desc = 0;
	qspIsMustWait = QSP_FALSE;
}
/* Установить индекс выбранного объекта */
QSP_BOOL QSPSetSelObjectIndex(long ind, QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	if (ind >= 0 && ind < qspCurObjectsCount && ind != qspCurSelObject)
	{
		qspCurSelObject = ind;
		qspResetError();
		qspPrepareExecution();
		qspExecLocByVarName(QSP_FMT("ONOBJSEL"));
		if (qspErrorNum)
		{
			qspIsMustWait = QSP_FALSE;
			return QSP_FALSE;
		}
		qspIsMustWait = QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	else
		qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* Получить индекс выбранного объекта */
long QSPGetSelObjectIndex()
{
	qspWait(QSP_FALSE);
	return qspCurSelObject;
}
/* Возможность изменения списка объектов */
QSP_BOOL QSPIsObjectsChanged()
{
	qspWait(QSP_FALSE);
	return qspIsObjectsChanged;
}
/* ------------------------------------------------------------ */
/* Показ / скрытие окон */
void QSPShowWindow(long type, QSP_BOOL isShow)
{
	qspWait(QSP_TRUE);
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
	qspIsMustWait = QSP_FALSE;
}
/* ------------------------------------------------------------ */
/* Переменные */

/* Получить количество элементов массива */
QSP_BOOL QSPGetVarValuesCount(QSP_CHAR *name, long *count)
{
	long varInd;
	qspWait(QSP_TRUE);
	qspResetError();
	varInd = qspVarIndex(name, QSP_FALSE);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	*count = qspVars[varInd].ValsCount;
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* Получить значения указанного элемента массива */
QSP_BOOL QSPGetVarValues(QSP_CHAR *name, long ind, long *numVal, QSP_CHAR **strVal)
{
	long varInd;
	qspWait(QSP_TRUE);
	qspResetError();
	varInd = qspVarIndex(name, QSP_FALSE);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	*numVal = qspVars[varInd].Value[ind];
	*strVal = qspVars[varInd].TextValue[ind];
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* Получить максимальное количество переменных */
long QSPGetMaxVarsCount()
{
	return QSP_VARSCOUNT;
}
/* Получить имя переменной с указанным индексом */
QSP_BOOL QSPGetVarNameByIndex(long index, QSP_CHAR **name)
{
	qspWait(QSP_TRUE);
	if (index < 0 || index >= QSP_VARSCOUNT || !qspVars[index].Name)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	*name = qspVars[index].Name;
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Выполнение кода */

/* Выполнение строки кода */
QSP_BOOL QSPExecString(QSP_CHAR *s, QSP_BOOL isRefresh)
{
	QSP_CHAR *jumpToFake;
	qspWait(QSP_TRUE);
	qspResetError();
	qspPrepareExecution();
	jumpToFake = qspGetNewText(QSP_FMT(""), 0);
	qspExecString(s, &jumpToFake);
	free(jumpToFake);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* Выполнение кода указанной локации */
QSP_BOOL QSPExecLocationCode(QSP_CHAR *name, QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	qspResetError();
	qspPrepareExecution();
	qspExecLocByName(name, QSP_FALSE);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* Выполнение кода локации-счетчика */
QSP_BOOL QSPExecCounter(QSP_BOOL isRefresh)
{
	if (!qspIsMustWait)
	{
		qspIsMustWait = QSP_TRUE;
		qspResetError();
		qspPrepareExecution();
		qspExecLocByVarName(QSP_FMT("COUNTER"));
		if (qspErrorNum)
		{
			qspIsMustWait = QSP_FALSE;
			return QSP_FALSE;
		}
		qspIsMustWait = QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	return QSP_TRUE;
}
/* Выполнение кода локации-обработчика строки ввода */
QSP_BOOL QSPExecUserInput(QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	qspResetError();
	qspPrepareExecution();
	qspExecLocByVarName(QSP_FMT("USERCOM"));
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Ошибки */

/* Получить информацию о последней ошибке */
void QSPGetLastErrorData(long *errorNum, QSP_CHAR **errorLoc, long *errorWhere, long *errorLine)
{
	qspWait(QSP_TRUE);
	*errorNum = qspErrorNum;
	*errorLoc = qspErrorLoc >= 0 ? qspLocs[qspErrorLoc].Name : 0;
	*errorWhere = qspErrorWhere;
	*errorLine = qspErrorLine;
	qspIsMustWait = QSP_FALSE;
}
/* Получить описание ошибки по ее номеру */
QSP_CHAR *QSPGetErrorDesc(long errorNum)
{
	QSP_CHAR *str;
	switch (errorNum)
	{
	case QSP_ERR_DIVBYZERO: str = QSP_FMT("Division by zero!"); break;
	case QSP_ERR_TYPEMISMATCH: str = QSP_FMT("Type mismatch!"); break;
	case QSP_ERR_STACKOVERFLOW: str = QSP_FMT("Stack overflow!"); break;
	case QSP_ERR_TOOMANYITEMS: str = QSP_FMT("Too many items in expression!"); break;
	case QSP_ERR_FILENOTFOUND: str = QSP_FMT("File not found!"); break;
	case QSP_ERR_CANTLOADFILE: str = QSP_FMT("Can't load file!"); break;
	case QSP_ERR_GAMENOTLOADED: str = QSP_FMT("Game not loaded!"); break;
	case QSP_ERR_COLONNOTFOUND: str = QSP_FMT("Sign [:] not found!"); break;
	case QSP_ERR_CANTINCFILE: str = QSP_FMT("Can't add file!"); break;
	case QSP_ERR_CANTADDACTION: str = QSP_FMT("Can't add action!"); break;
	case QSP_ERR_EQNOTFOUND: str = QSP_FMT("Sign [=] not found!"); break;
	case QSP_ERR_LOCNOTFOUND: str = QSP_FMT("Location not found!"); break;
	case QSP_ERR_ENDNOTFOUND: str = QSP_FMT("[end] not found!"); break;
	case QSP_ERR_LABELNOTFOUND: str = QSP_FMT("Label not found!"); break;
	case QSP_ERR_NOTCORRECTNAME: str = QSP_FMT("Incorrect variable's name!"); break;
	case QSP_ERR_QUOTNOTFOUND: str = QSP_FMT("Quote not found!"); break;
	case QSP_ERR_BRACKNOTFOUND: str = QSP_FMT("Bracket not found!"); break;
	case QSP_ERR_BRACKSNOTFOUND: str = QSP_FMT("Brackets not found!"); break;
	case QSP_ERR_SYNTAX: str = QSP_FMT("Syntax error!"); break;
	case QSP_ERR_UNKNOWNACTION: str = QSP_FMT("Unknown action!"); break;
	case QSP_ERR_ARGSCOUNT: str = QSP_FMT("Incorrect arguments' count!"); break;
	case QSP_ERR_CANTADDOBJECT: str = QSP_FMT("Can't add object!"); break;
	case QSP_ERR_CANTADDMENUITEM: str = QSP_FMT("Can't add menu's item!"); break;
	case QSP_ERR_TOOMANYVARS: str = QSP_FMT("Too many variables!"); break;
	case QSP_ERR_INCORRECTREGEXP: str = QSP_FMT("Regular expression's error!"); break;
	default: str = QSP_FMT("Unknown error!"); break;
	}
	return str;
}
/* ------------------------------------------------------------ */
/* Управление игрой */

/* Загрузка новой игры */
QSP_BOOL QSPLoadGameWorld(QSP_CHAR *fileName)
{
	qspWait(QSP_TRUE);
	qspResetError();
	qspOpenQuest(fileName, QSP_FALSE);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* Сохранение состояния */
QSP_BOOL QSPSaveGame(QSP_CHAR *fileName)
{
	qspWait(QSP_TRUE);
	qspResetError();
	qspPrepareExecution();
	qspSaveGameStatus(fileName);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* Загрузка состояния */
QSP_BOOL QSPOpenSavedGame(QSP_CHAR *fileName, QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	qspResetError();
	qspPrepareExecution();
	qspOpenGameStatus(fileName);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* Перезапуск игры */
QSP_BOOL QSPRestartGame(QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	qspResetError();
	qspPrepareExecution();
	qspNewGame(QSP_TRUE);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Меню */
/* Ф-я предназначена только для вызова из CallBack'а QSP_CALL_SHOWMENU */
void QSPSelectMenuItem(long index)
{
	if (index >= 0 && index < qspCurMenuItems) qspExecLocByName(qspCurMenuLocs[index], QSP_FALSE);
}
/* ------------------------------------------------------------ */
/* Установка CALLBACK'ов */
void QSPSetCallBack(long type, QSP_CALLBACK func)
{
	qspWait(QSP_TRUE);
	qspSetCallBack(type, func);
	qspIsMustWait = QSP_FALSE;
}
/* ------------------------------------------------------------ */
/* Инициализация */
void QSPInit()
{
	qspIsMustWait = QSP_FALSE;
	qspRefreshCount = qspFullRefreshCount = 0;
	qspQstCRC = 0;
	qspQstPath = qspQstFullPath = 0;
	qspQstPathLen = 0;
	qspLocs = 0;
	qspLocsCount = 0;
	setlocale(LC_ALL, QSP_LOCALE);
	qspResetError();
	qspPrepareExecution();
	qspMemClear(QSP_TRUE);
	qspClearIncludes(QSP_TRUE);
	qspInitCallBacks();
	qspInitStats();
	qspInitMath();
}
/* Деинициализация */
void QSPDeInit()
{
	qspWait(QSP_TRUE);
	qspMemClear(QSP_FALSE);
	qspClearIncludes(QSP_FALSE);
	qspCreateWorld(0, 0);
	if (qspQstPath) free(qspQstPath);
	if (qspQstFullPath) free(qspQstFullPath);
	qspCallCloseFile(0);
	qspIsMustWait = QSP_FALSE;
}
