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

#ifdef _FLASH

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
AS3_Val QSPIsInCallBack(void *param, AS3_Val args)
{
	if (qspIsInCallBack)
		return AS3_True();
	else
		return AS3_False();
}
/* ------------------------------------------------------------ */
/* Управление режимом отладки */
AS3_Val QSPEnableDebugMode(void *param, AS3_Val args)
{
	QSP_BOOL isDebug;
	AS3_ArrayValue(args, "IntType", &isDebug);
	qspIsDebug = isDebug;
	return AS3_True();
}
/* Получение данных текущего состояния */
AS3_Val QSPGetCurStateData(void *param, AS3_Val args)
{
	char *locUTF8;
	AS3_Val res;
	if (qspRealCurLoc >= 0 && qspRealCurLoc < qspLocsCount)
	{
		locUTF8 = qspW2C(qspLocs[qspRealCurLoc].Name);
		res = AS3_Object("loc:StrType, actIndex:IntType, line:IntType", locUTF8, qspRealActIndex, qspRealLine);
		free(locUTF8);
	}
	else
		res = AS3_Object("loc:StrType, actIndex:IntType, line:IntType", 0, qspRealActIndex, qspRealLine);
	return res;
}
/* ------------------------------------------------------------ */
/* Информация о версии */

/* Версия */
AS3_Val QSPGetVersion(void *param, AS3_Val args)
{
	AS3_Val res;
	char *verUTF8 = qspW2C(QSP_VER);
	res = AS3_String(verUTF8);
	free(verUTF8);
	return res;
}
/* Дата и время компиляции */
AS3_Val QSPGetCompiledDateTime(void *param, AS3_Val args)
{
	AS3_Val res;
	char *infoUTF8 = qspW2C(QSP_FMT(__DATE__) QSP_FMT(", ") QSP_FMT(__TIME__));
	res = AS3_String(infoUTF8);
	free(infoUTF8);
	return res;
}
/* ------------------------------------------------------------ */
/* Количество полных обновлений локаций */
AS3_Val QSPGetFullRefreshCount(void *param, AS3_Val args)
{
	return AS3_Int(qspFullRefreshCount);
}
/* ------------------------------------------------------------ */
/* Полный путь к загруженному файлу игры */
AS3_Val QSPGetQstFullPath(void *param, AS3_Val args)
{
	AS3_Val res;
	char *pathUTF8;
	if (qspQstFullPath)
	{
		pathUTF8 = qspW2C(qspQstFullPath);
		res = AS3_String(pathUTF8);
		free(pathUTF8);
	}
	else
		res = AS3_String(0);
	return res;
}
/* ------------------------------------------------------------ */
/* Название текущей локации */
AS3_Val QSPGetCurLoc(void *param, AS3_Val args)
{
	AS3_Val res;
	char *locUTF8;
	if (qspCurLoc >= 0)
	{
		locUTF8 = qspW2C(qspLocs[qspCurLoc].Name);
		res = AS3_String(locUTF8);
		free(locUTF8);
	}
	else
		res = AS3_String(0);
	return res;
}
/* ------------------------------------------------------------ */
/* Основное описание локации */

/* Текст основного окна описания локации */
AS3_Val QSPGetMainDesc(void *param, AS3_Val args)
{
	AS3_Val res;
	char *descUTF8;
	if (qspCurDesc)
	{
		descUTF8 = qspW2C(qspCurDesc);
		res = AS3_String(descUTF8);
		free(descUTF8);
	}
	else
		res = AS3_String(0);
	return res;
}
/* Возможность изменения текста основного описания */
AS3_Val QSPIsMainDescChanged(void *param, AS3_Val args)
{
	if (qspIsMainDescChanged)
		return AS3_True();
	else
		return AS3_False();
}
/* ------------------------------------------------------------ */
/* Дополнительное описание локации */

/* Текст дополнительного окна описания локации */
AS3_Val QSPGetVarsDesc(void *param, AS3_Val args)
{
	AS3_Val res;
	char *descUTF8;
	if (qspCurVars)
	{
		descUTF8 = qspW2C(qspCurVars);
		res = AS3_String(descUTF8);
		free(descUTF8);
	}
	else
		res = AS3_String(0);
	return res;
}
/* Возможность изменения текста дополнительного описания */
AS3_Val QSPIsVarsDescChanged(void *param, AS3_Val args)
{
	if (qspIsVarsDescChanged)
		return AS3_True();
	else
		return AS3_False();
}
/* ------------------------------------------------------------ */
/* Получить значение указанного выражения */
AS3_Val QSPGetExprValue(void *param, AS3_Val args)
{
	char *expr;
	QSP_CHAR *exprWC;
	char *strUTF8;
	QSPVariant v;
	AS3_Val res;
	AS3_ArrayValue(args, "StrType", &expr);
	if (qspIsExitOnError && qspErrorNum) return AS3_Null();
	qspResetError();
	if (qspIsDisableCodeExec) return AS3_Null();
	exprWC = qspC2W(expr);
	v = qspExprValue(exprWC);
	free(exprWC);
	if (qspErrorNum) return AS3_Null();
	if (v.IsStr)
	{
		strUTF8 = qspW2C(QSP_STR(v));
		free(QSP_STR(v));
		res = AS3_Object("isString:IntType, numVal:IntType, strVal:StrType", QSP_TRUE, 0, strUTF8);
		free(strUTF8);
	}
	else
		res = AS3_Object("isString:IntType, numVal:IntType, strVal:StrType", QSP_FALSE, QSP_NUM(v), 0);
	return res;
}
/* ------------------------------------------------------------ */
/* Текст строки ввода */
AS3_Val QSPSetInputStrText(void *param, AS3_Val args)
{
	QSP_CHAR *valWC;
	char *val;
	AS3_ArrayValue(args, "StrType", &val);
	valWC = qspC2W(val);
	qspCurInputLen = qspAddText(&qspCurInput, valWC, 0, -1, QSP_FALSE);
	free(valWC);
	return AS3_True();
}
/* ------------------------------------------------------------ */
/* Список действий */

/* Количество действий */
AS3_Val QSPGetActionsCount(void *param, AS3_Val args)
{
	return AS3_Int(qspCurActionsCount);
}
/* Данные действия с указанным индексом */
AS3_Val QSPGetActionData(void *param, AS3_Val args)
{
	int ind;
	char *imageUTF8;
	char *descUTF8;
	AS3_Val res;
	AS3_ArrayValue(args, "IntType", &ind);
	if (ind >= 0 && ind < qspCurActionsCount)
	{
		imageUTF8 = (qspCurActions[ind].Image ? qspW2C(qspCurActions[ind].Image) : 0);
		descUTF8 = (qspCurActions[ind].Desc ? qspW2C(qspCurActions[ind].Desc) : 0);
		res = AS3_Object("image:StrType, desc:StrType", imageUTF8, descUTF8);
		if (imageUTF8) free(imageUTF8);
		if (descUTF8) free(descUTF8);
	}
	else
		res = AS3_Object("image:StrType, desc:StrType", 0, 0);
	return res;
}
/* Выполнение кода выбранного действия */
AS3_Val QSPExecuteSelActionCode(void *param, AS3_Val args)
{
	QSP_BOOL isRefresh;
	AS3_ArrayValue(args, "IntType", &isRefresh);
	if (qspCurSelAction >= 0)
	{
		if (qspIsExitOnError && qspErrorNum) return AS3_False();
		qspPrepareExecution();
		if (qspIsDisableCodeExec) return AS3_False();
		qspExecAction(qspCurSelAction);
		if (qspErrorNum) return AS3_False();
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	return AS3_True();
}
/* Установить индекс выбранного действия */
AS3_Val QSPSetSelActionIndex(void *param, AS3_Val args)
{
	int ind;
	QSP_BOOL isRefresh;
	AS3_ArrayValue(args, "IntType, IntType", &ind, &isRefresh);
	if (ind >= 0 && ind < qspCurActionsCount && ind != qspCurSelAction)
	{
		if (qspIsExitOnError && qspErrorNum) return AS3_False();
		qspPrepareExecution();
		if (qspIsDisableCodeExec) return AS3_False();
		qspCurSelAction = ind;
		qspExecLocByVarNameWithArgs(QSP_FMT("ONACTSEL"), 0, 0);
		if (qspErrorNum) return AS3_False();
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	return AS3_True();
}
/* Получить индекс выбранного действия */
AS3_Val QSPGetSelActionIndex(void *param, AS3_Val args)
{
	return AS3_Int(qspCurSelAction);
}
/* Возможность изменения списка действий */
AS3_Val QSPIsActionsChanged(void *param, AS3_Val args)
{
	if (qspIsActionsChanged)
		return AS3_True();
	else
		return AS3_False();
}
/* ------------------------------------------------------------ */
/* Список объектов */

/* Количество объектов */
AS3_Val QSPGetObjectsCount(void *param, AS3_Val args)
{
	return AS3_Int(qspCurObjectsCount);
}
/* Данные объекта с указанным индексом */
AS3_Val QSPGetObjectData(void *param, AS3_Val args)
{
	int ind;
	char *imageUTF8;
	char *descUTF8;
	AS3_Val res;
	AS3_ArrayValue(args, "IntType", &ind);
	if (ind >= 0 && ind < qspCurObjectsCount)
	{
		imageUTF8 = (qspCurObjects[ind].Image ? qspW2C(qspCurObjects[ind].Image) : 0);
		descUTF8 = (qspCurObjects[ind].Desc ? qspW2C(qspCurObjects[ind].Desc) : 0);
		res = AS3_Object("image:StrType, desc:StrType", imageUTF8, descUTF8);
		if (imageUTF8) free(imageUTF8);
		if (descUTF8) free(descUTF8);
	}
	else
		res = AS3_Object("image:StrType, desc:StrType", 0, 0);
	return res;
}
/* Установить индекс выбранного объекта */
AS3_Val QSPSetSelObjectIndex(void *param, AS3_Val args)
{
	int ind;
	QSP_BOOL isRefresh;
	AS3_ArrayValue(args, "IntType, IntType", &ind, &isRefresh);
	if (ind >= 0 && ind < qspCurObjectsCount && ind != qspCurSelObject)
	{
		if (qspIsExitOnError && qspErrorNum) return AS3_False();
		qspPrepareExecution();
		if (qspIsDisableCodeExec) return AS3_False();
		qspCurSelObject = ind;
		qspExecLocByVarNameWithArgs(QSP_FMT("ONOBJSEL"), 0, 0);
		if (qspErrorNum) return AS3_False();
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	return AS3_True();
}
/* Получить индекс выбранного объекта */
AS3_Val QSPGetSelObjectIndex(void *param, AS3_Val args)
{
	return AS3_Int(qspCurSelObject);
}
/* Возможность изменения списка объектов */
AS3_Val QSPIsObjectsChanged(void *param, AS3_Val args)
{
	if (qspIsObjectsChanged)
		return AS3_True();
	else
		return AS3_False();
}
/* ------------------------------------------------------------ */
/* Показ / скрытие окон */
AS3_Val QSPShowWindow(void *param, AS3_Val args)
{
	int type;
	QSP_BOOL isShow;
	AS3_ArrayValue(args, "IntType, IntType", &type, &isShow);
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
	return AS3_True();
}
/* ------------------------------------------------------------ */
/* Переменные */

/* Получить количество элементов массива */
AS3_Val QSPGetVarValuesCount(void *param, AS3_Val args)
{
	char *name;
	QSP_CHAR *nameWC;
	QSPVar *var;
	AS3_ArrayValue(args, "StrType", &name);
	if (qspIsExitOnError && qspErrorNum) return AS3_Null();
	qspResetError();
	nameWC = qspC2W(name);
	var = qspVarReference(nameWC, QSP_FALSE);
	free(nameWC);
	if (qspErrorNum) return AS3_Null();
	return AS3_Int(var->ValsCount);
}
/* Получить значения указанного элемента массива */
AS3_Val QSPGetVarValues(void *param, AS3_Val args)
{
	char *name;
	int ind;
	QSP_CHAR *nameWC;
	char *strUTF8;
	QSPVar *var;
	AS3_Val res;
	AS3_ArrayValue(args, "StrType, IntType", &name, &ind);
	if (qspIsExitOnError && qspErrorNum) return AS3_Null();
	qspResetError();
	nameWC = qspC2W(name);
	var = qspVarReference(nameWC, QSP_FALSE);
	free(nameWC);
	if (qspErrorNum || ind < 0 || ind >= var->ValsCount) return AS3_Null();
	if (var->Values[ind].Str)
	{
		strUTF8 = qspW2C(var->Values[ind].Str);
		res = AS3_Object("numVal:IntType, strVal:StrType", var->Values[ind].Num, strUTF8);
		free(strUTF8);
	}
	else
		res = AS3_Object("numVal:IntType, strVal:StrType", var->Values[ind].Num, 0);
	return res;
}
/* Получить максимальное количество переменных */
AS3_Val QSPGetMaxVarsCount(void *param, AS3_Val args)
{
	return AS3_Int(QSP_VARSCOUNT);
}
/* Получить имя переменной с указанным индексом */
AS3_Val QSPGetVarNameByIndex(void *param, AS3_Val args)
{
	int index;
	QSP_CHAR *name;
	char *nameUTF8;
	AS3_Val res;
	AS3_ArrayValue(args, "IntType", &index);
	if (index < 0 || index >= QSP_VARSCOUNT || !qspVars[index].Name) return AS3_Null();
	nameUTF8 = qspW2C(qspVars[index].Name);
	res = AS3_String(nameUTF8);
	free(nameUTF8);
	return res;
}
/* ------------------------------------------------------------ */
/* Выполнение кода */

/* Выполнение строки кода */
AS3_Val QSPExecString(void *param, AS3_Val args)
{
	char *s;
	QSP_CHAR *strWC;
	QSP_BOOL isRefresh;
	AS3_ArrayValue(args, "StrType, IntType", &s, &isRefresh);
	if (qspIsExitOnError && qspErrorNum) return AS3_False();
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return AS3_False();
	strWC = qspC2W(s);
	qspExecStringAsCodeWithArgs(strWC, 0, 0, 0);
	free(strWC);
	if (qspErrorNum) return AS3_False();
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return AS3_True();
}
/* Выполнение кода указанной локации */
AS3_Val QSPExecLocationCode(void *param, AS3_Val args)
{
	char *name;
	QSP_CHAR *nameWC;
	QSP_BOOL isRefresh;
	AS3_ArrayValue(args, "StrType, IntType", &name, &isRefresh);
	if (qspIsExitOnError && qspErrorNum) return AS3_False();
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return AS3_False();
	nameWC = qspC2W(name);
	qspExecLocByName(nameWC, QSP_FALSE);
	free(nameWC);
	if (qspErrorNum) return AS3_False();
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return AS3_True();
}
/* Выполнение кода локации-счетчика */
AS3_Val QSPExecCounter(void *param, AS3_Val args)
{
	QSP_BOOL isRefresh;
	AS3_ArrayValue(args, "IntType", &isRefresh);
	if (!qspIsInCallBack)
	{
		qspPrepareExecution();
		qspExecLocByVarNameWithArgs(QSP_FMT("COUNTER"), 0, 0);
		if (qspErrorNum) return AS3_False();
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	return AS3_True();
}
/* Выполнение кода локации-обработчика строки ввода */
AS3_Val QSPExecUserInput(void *param, AS3_Val args)
{
	QSP_BOOL isRefresh;
	AS3_ArrayValue(args, "IntType", &isRefresh);
	if (qspIsExitOnError && qspErrorNum) return AS3_False();
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return AS3_False();
	qspExecLocByVarNameWithArgs(QSP_FMT("USERCOM"), 0, 0);
	if (qspErrorNum) return AS3_False();
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return AS3_True();
}
/* ------------------------------------------------------------ */
/* Ошибки */

/* Получить информацию о последней ошибке */
AS3_Val QSPGetLastErrorData(void *param, AS3_Val args)
{
	AS3_Val res;
	char *locUTF8;
	if (qspErrorLoc >= 0 && qspErrorLoc < qspLocsCount)
	{
		locUTF8 = qspW2C(qspLocs[qspErrorLoc].Name);
		res = AS3_Object("errorNum:IntType, errorLoc:StrType, errorActIndex:IntType, errorLine:IntType",
			qspErrorNum, locUTF8, qspErrorActIndex, qspErrorLine);
		free(locUTF8);
	}
	else
	{
		res = AS3_Object("errorNum:IntType, errorLoc:StrType, errorActIndex:IntType, errorLine:IntType",
			qspErrorNum, 0, qspErrorActIndex, qspErrorLine);
	}
	return res;
}
/* Получить описание ошибки по ее номеру */
AS3_Val QSPGetErrorDesc(void *param, AS3_Val args)
{
	int errorNum;
	QSP_CHAR *str;
	char *strUTF8;
	AS3_Val res;
	AS3_ArrayValue(args, "IntType", &errorNum);
	str = qspGetErrorDesc(errorNum);
	strUTF8 = qspW2C(str);
	res = AS3_String(strUTF8);
	free(strUTF8);
	return res;
}
/* ------------------------------------------------------------ */
/* Управление игрой */

/* Загрузка новой игры из файла */
AS3_Val QSPLoadGameWorld(void *param, AS3_Val args)
{
	char *fileName;
	QSP_CHAR *fileNameWC;
	AS3_ArrayValue(args, "StrType", &fileName);
	if (qspIsExitOnError && qspErrorNum) return AS3_False();
	qspResetError();
	if (qspIsDisableCodeExec) return AS3_False();
	fileNameWC = qspC2W(fileName);
	qspOpenQuest(fileNameWC, QSP_FALSE);
	free(fileNameWC);
	if (qspErrorNum) return AS3_False();
	return AS3_True();
}
/* Загрузка новой игры из памяти */
AS3_Val QSPLoadGameWorldFromData(void *param, AS3_Val args)
{
	char *ptr;
	AS3_Val data;
	int dataSize;
	char *fileName;
	QSP_CHAR *fileNameWC;
	AS3_ArrayValue(args, "AS3ValType, IntType, StrType", &data, &dataSize, &fileName);
	if (qspIsExitOnError && qspErrorNum) return AS3_False();
	qspResetError();
	if (qspIsDisableCodeExec) return AS3_False();
	ptr = (char *)malloc(dataSize + 3);
	AS3_ByteArray_seek(data, 0, SEEK_SET);
	AS3_ByteArray_readBytes(ptr, data, dataSize);
	ptr[dataSize] = ptr[dataSize + 1] = ptr[dataSize + 2] = 0;
	fileNameWC = qspC2W(fileName);
	qspOpenQuestFromData(ptr, dataSize + 3, fileNameWC, QSP_FALSE);
	free(fileNameWC);
	free(ptr);
	if (qspErrorNum) return AS3_False();
	return AS3_True();
}
/* Сохранение состояния в файл */
AS3_Val QSPSaveGame(void *param, AS3_Val args)
{
	char *fileName;
	QSP_CHAR *fileNameWC;
	QSP_BOOL isRefresh;
	AS3_ArrayValue(args, "StrType, IntType", &fileName, &isRefresh);
	if (qspIsExitOnError && qspErrorNum) return AS3_False();
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return AS3_False();
	fileNameWC = qspC2W(fileName);
	qspSaveGameStatus(fileNameWC);
	free(fileNameWC);
	if (qspErrorNum) return AS3_False();
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return AS3_True();
}
/* Сохранение состояния в память */
AS3_Val QSPSaveGameAsData(void *param, AS3_Val args)
{
	int len;
	QSP_BOOL isRefresh;
	AS3_Val data;
	QSP_CHAR *buf;
	AS3_ArrayValue(args, "AS3ValType, IntType", &data, &isRefresh);
	if (qspIsExitOnError && qspErrorNum) return AS3_False();
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return AS3_False();
	if (!(len = qspSaveGameStatusToString(&buf))) return AS3_False();
	AS3_ByteArray_seek(data, 0, SEEK_SET);
	AS3_ByteArray_writeBytes(data, buf, len * sizeof(QSP_CHAR));
	free(buf);
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return AS3_True();
}
/* Загрузка состояния из файла */
AS3_Val QSPOpenSavedGame(void *param, AS3_Val args)
{
	char *fileName;
	QSP_CHAR *fileNameWC;
	QSP_BOOL isRefresh;
	AS3_ArrayValue(args, "StrType, IntType", &fileName, &isRefresh);
	if (qspIsExitOnError && qspErrorNum) return AS3_False();
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return AS3_False();
	fileNameWC = qspC2W(fileName);
	qspOpenGameStatus(fileNameWC);
	free(fileNameWC);
	if (qspErrorNum) return AS3_False();
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return AS3_True();
}
/* Загрузка состояния из памяти */
AS3_Val QSPOpenSavedGameFromData(void *param, AS3_Val args)
{
	AS3_Val data;
	int dataSize, dataLen;
	QSP_BOOL isRefresh;
	QSP_CHAR *ptr;
	AS3_ArrayValue(args, "AS3ValType, IntType, IntType", &data, &dataSize, &isRefresh);
	if (qspIsExitOnError && qspErrorNum) return AS3_False();
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return AS3_False();
	dataLen = dataSize / sizeof(QSP_CHAR);
	ptr = (QSP_CHAR *)malloc((dataLen + 1) * sizeof(QSP_CHAR));
	AS3_ByteArray_seek(data, 0, SEEK_SET);
	AS3_ByteArray_readBytes(ptr, data, dataSize);
	ptr[dataLen] = 0;
	qspOpenGameStatusFromString(ptr);
	free(ptr);
	if (qspErrorNum) return AS3_False();
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return AS3_True();
}
/* Перезапуск игры */
AS3_Val QSPRestartGame(void *param, AS3_Val args)
{
	QSP_BOOL isRefresh;
	AS3_ArrayValue(args, "IntType", &isRefresh);
	if (qspIsExitOnError && qspErrorNum) return AS3_False();
	qspPrepareExecution();
	if (qspIsDisableCodeExec) return AS3_False();
	qspNewGame(QSP_TRUE);
	if (qspErrorNum) return AS3_False();
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return AS3_True();
}
/* ------------------------------------------------------------ */
/* Установка CALLBACK'ов */
AS3_Val QSPSetCallBack(void *param, AS3_Val args)
{
	int type;
	AS3_Val thisVal;
	AS3_Val funcVal;
	QSP_CALLBACK callBack;
	AS3_ArrayValue(args, "IntType, AS3ValType, AS3ValType", &type, &thisVal, &funcVal);
	callBack.ThisVal = thisVal;
	callBack.FuncVal = funcVal;
	qspSetCallBack(type, callBack);
	return AS3_True();
}
/* ------------------------------------------------------------ */
/* Инициализация */
AS3_Val QSPInit(void *param, AS3_Val args)
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
	return AS3_True();
}
/* Деинициализация */
AS3_Val QSPDeInit(void *param, AS3_Val args)
{
	qspMemClear(QSP_FALSE);
	qspCreateWorld(0, 0);
	if (qspQstPath) free(qspQstPath);
	if (qspQstFullPath) free(qspQstFullPath);
#ifdef _DEBUG
	mwTerm();
#endif
	return AS3_True();
}

AS3_Val QSPReturnValue(void *param, AS3_Val args)
{
	AS3_Val resVal;
	AS3_ArrayValue(args, "AS3ValType", &resVal);
	qspSetReturnValue(resVal);
	return AS3_True();
}

int main()
{
	AS3_Val isInCallBack = AS3_Function(0, QSPIsInCallBack);
	AS3_Val enableDebugMode = AS3_Function(0, QSPEnableDebugMode);
	AS3_Val getCurStateData = AS3_Function(0, QSPGetCurStateData);
	AS3_Val getVersion = AS3_Function(0, QSPGetVersion);
	AS3_Val getCompiledDateTime = AS3_Function(0, QSPGetCompiledDateTime);
	AS3_Val getFullRefreshCount = AS3_Function(0, QSPGetFullRefreshCount);
	AS3_Val getQstFullPath = AS3_Function(0, QSPGetQstFullPath);
	AS3_Val getCurLoc = AS3_Function(0, QSPGetCurLoc);
	AS3_Val getMainDesc = AS3_Function(0, QSPGetMainDesc);
	AS3_Val isMainDescChanged = AS3_Function(0, QSPIsMainDescChanged);
	AS3_Val getVarsDesc = AS3_Function(0, QSPGetVarsDesc);
	AS3_Val isVarsDescChanged = AS3_Function(0, QSPIsVarsDescChanged);
	AS3_Val getExprValue = AS3_FunctionAsync(0, QSPGetExprValue);
	AS3_Val setInputStrText = AS3_Function(0, QSPSetInputStrText);
	AS3_Val getActionsCount = AS3_Function(0, QSPGetActionsCount);
	AS3_Val getActionData = AS3_Function(0, QSPGetActionData);
	AS3_Val executeSelActionCode = AS3_FunctionAsync(0, QSPExecuteSelActionCode);
	AS3_Val setSelActionIndex = AS3_FunctionAsync(0, QSPSetSelActionIndex);
	AS3_Val getSelActionIndex = AS3_Function(0, QSPGetSelActionIndex);
	AS3_Val isActionsChanged = AS3_Function(0, QSPIsActionsChanged);
	AS3_Val getObjectsCount = AS3_Function(0, QSPGetObjectsCount);
	AS3_Val getObjectData = AS3_Function(0, QSPGetObjectData);
	AS3_Val setSelObjectIndex = AS3_FunctionAsync(0, QSPSetSelObjectIndex);
	AS3_Val getSelObjectIndex = AS3_Function(0, QSPGetSelObjectIndex);
	AS3_Val isObjectsChanged = AS3_Function(0, QSPIsObjectsChanged);
	AS3_Val showWindow = AS3_Function(0, QSPShowWindow);
	AS3_Val getVarValuesCount = AS3_Function(0, QSPGetVarValuesCount);
	AS3_Val getVarValues = AS3_Function(0, QSPGetVarValues);
	AS3_Val getMaxVarsCount = AS3_Function(0, QSPGetMaxVarsCount);
	AS3_Val getVarNameByIndex = AS3_Function(0, QSPGetVarNameByIndex);
	AS3_Val execString = AS3_FunctionAsync(0, QSPExecString);
	AS3_Val execLocationCode = AS3_FunctionAsync(0, QSPExecLocationCode);
	AS3_Val execCounter = AS3_FunctionAsync(0, QSPExecCounter);
	AS3_Val execUserInput = AS3_FunctionAsync(0, QSPExecUserInput);
	AS3_Val getLastErrorData = AS3_Function(0, QSPGetLastErrorData);
	AS3_Val getErrorDesc = AS3_Function(0, QSPGetErrorDesc);
	AS3_Val loadGameWorld = AS3_Function(0, QSPLoadGameWorld);
	AS3_Val loadGameWorldFromData = AS3_Function(0, QSPLoadGameWorldFromData);
	AS3_Val saveGame = AS3_FunctionAsync(0, QSPSaveGame);
	AS3_Val saveGameAsData = AS3_FunctionAsync(0, QSPSaveGameAsData);
	AS3_Val openSavedGame = AS3_FunctionAsync(0, QSPOpenSavedGame);
	AS3_Val openSavedGameFromData = AS3_FunctionAsync(0, QSPOpenSavedGameFromData);
	AS3_Val restartGame = AS3_FunctionAsync(0, QSPRestartGame);
	AS3_Val setCallBack = AS3_Function(0, QSPSetCallBack);
	AS3_Val init = AS3_Function(0, QSPInit);
	AS3_Val deInit = AS3_Function(0, QSPDeInit);
	AS3_Val returnValue = AS3_Function(0, QSPReturnValue);

	AS3_Val result = AS3_Object(
		"QSPIsInCallBack:AS3ValType, QSPEnableDebugMode:AS3ValType, QSPGetCurStateData:AS3ValType, QSPGetVersion:AS3ValType, "
		"QSPGetCompiledDateTime:AS3ValType, QSPGetFullRefreshCount:AS3ValType, QSPGetQstFullPath:AS3ValType, "
		"QSPGetCurLoc:AS3ValType, QSPGetMainDesc:AS3ValType, QSPIsMainDescChanged:AS3ValType, "
		"QSPGetVarsDesc:AS3ValType, QSPIsVarsDescChanged:AS3ValType, QSPGetExprValue:AS3ValType, "
		"QSPSetInputStrText:AS3ValType, QSPGetActionsCount:AS3ValType, QSPGetActionData:AS3ValType, "
		"QSPExecuteSelActionCode:AS3ValType, QSPSetSelActionIndex:AS3ValType, QSPGetSelActionIndex:AS3ValType, "
		"QSPIsActionsChanged:AS3ValType, QSPGetObjectsCount:AS3ValType, QSPGetObjectData:AS3ValType, "
		"QSPSetSelObjectIndex:AS3ValType, QSPGetSelObjectIndex:AS3ValType, QSPIsObjectsChanged:AS3ValType, "
		"QSPShowWindow:AS3ValType, QSPGetVarValuesCount:AS3ValType, QSPGetVarValues:AS3ValType, "
		"QSPGetMaxVarsCount:AS3ValType, QSPGetVarNameByIndex:AS3ValType, QSPExecString:AS3ValType, "
		"QSPExecLocationCode:AS3ValType, QSPExecCounter:AS3ValType, QSPExecUserInput:AS3ValType, "
		"QSPGetLastErrorData:AS3ValType, QSPGetErrorDesc:AS3ValType, QSPLoadGameWorld:AS3ValType, "
		"QSPLoadGameWorldFromData:AS3ValType, QSPSaveGame:AS3ValType, QSPSaveGameAsData:AS3ValType, "
		"QSPOpenSavedGame:AS3ValType, QSPOpenSavedGameFromData:AS3ValType, QSPRestartGame:AS3ValType, "
		"QSPSetCallBack:AS3ValType, QSPInit:AS3ValType, QSPDeInit:AS3ValType, QSPReturnValue:AS3ValType",
		isInCallBack, enableDebugMode, getCurStateData, getVersion, getCompiledDateTime, getFullRefreshCount,
		getQstFullPath, getCurLoc, getMainDesc, isMainDescChanged, getVarsDesc, isVarsDescChanged,
		getExprValue, setInputStrText, getActionsCount, getActionData, executeSelActionCode, setSelActionIndex,
		getSelActionIndex, isActionsChanged, getObjectsCount, getObjectData, setSelObjectIndex,
		getSelObjectIndex, isObjectsChanged, showWindow, getVarValuesCount, getVarValues, getMaxVarsCount,
		getVarNameByIndex, execString, execLocationCode, execCounter, execUserInput, getLastErrorData,
		getErrorDesc, loadGameWorld, loadGameWorldFromData, saveGame, saveGameAsData, openSavedGame,
		openSavedGameFromData, restartGame, setCallBack, init, deInit, returnValue);

	// Release
	AS3_Release(isInCallBack);
	AS3_Release(enableDebugMode);
	AS3_Release(getCurStateData);
	AS3_Release(getVersion);
	AS3_Release(getCompiledDateTime);
	AS3_Release(getFullRefreshCount);
	AS3_Release(getQstFullPath);
	AS3_Release(getCurLoc);
	AS3_Release(getMainDesc);
	AS3_Release(isMainDescChanged);
	AS3_Release(getVarsDesc);
	AS3_Release(isVarsDescChanged);
	AS3_Release(getExprValue);
	AS3_Release(setInputStrText);
	AS3_Release(getActionsCount);
	AS3_Release(getActionData);
	AS3_Release(executeSelActionCode);
	AS3_Release(setSelActionIndex);
	AS3_Release(getSelActionIndex);
	AS3_Release(isActionsChanged);
	AS3_Release(getObjectsCount);
	AS3_Release(getObjectData);
	AS3_Release(setSelObjectIndex);
	AS3_Release(getSelObjectIndex);
	AS3_Release(isObjectsChanged);
	AS3_Release(showWindow);
	AS3_Release(getVarValuesCount);
	AS3_Release(getVarValues);
	AS3_Release(getMaxVarsCount);
	AS3_Release(getVarNameByIndex);
	AS3_Release(execString);
	AS3_Release(execLocationCode);
	AS3_Release(execCounter);
	AS3_Release(execUserInput);
	AS3_Release(getLastErrorData);
	AS3_Release(getErrorDesc);
	AS3_Release(loadGameWorld);
	AS3_Release(loadGameWorldFromData);
	AS3_Release(saveGame);
	AS3_Release(saveGameAsData);
	AS3_Release(openSavedGame);
	AS3_Release(openSavedGameFromData);
	AS3_Release(restartGame);
	AS3_Release(setCallBack);
	AS3_Release(init);
	AS3_Release(deInit);
	AS3_Release(returnValue);

	AS3_LibInit(result);

	return 0;
}

#endif
