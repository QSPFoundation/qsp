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

#include "../../qsp.h"

#ifndef QSP_DEFAULTDEFINES
	#define QSP_DEFAULTDEFINES

	#ifdef _UNICODE
		typedef wchar_t QSP_CHAR;
	#endif

	#ifdef __cplusplus
		typedef int (*QSP_CALLBACK)(...);
	#else
		typedef int (*QSP_CALLBACK)();
	#endif

	#ifdef __cplusplus
	extern "C"
	{
	#endif

	QSP_EXTERN QSP_BOOL QSPIsInCallBack();
	QSP_EXTERN void QSPEnableDebugMode(QSP_BOOL isDebug);
	QSP_EXTERN void QSPGetCurStateData(QSP_CHAR **loc, int *actIndex, int *line);
	QSP_EXTERN const QSP_CHAR *QSPGetVersion();
	QSP_EXTERN const QSP_CHAR *QSPGetCompiledDateTime();
	QSP_EXTERN int QSPGetFullRefreshCount();
	QSP_EXTERN const QSP_CHAR *QSPGetQstFullPath();
	QSP_EXTERN const QSP_CHAR *QSPGetCurLoc();
	QSP_EXTERN const QSP_CHAR *QSPGetMainDesc();
	QSP_EXTERN QSP_BOOL QSPIsMainDescChanged();
	QSP_EXTERN const QSP_CHAR *QSPGetVarsDesc();
	QSP_EXTERN QSP_BOOL QSPIsVarsDescChanged();
	QSP_EXTERN QSP_BOOL QSPGetExprValue(const QSP_CHAR *str, QSP_BOOL *isString, int *numVal, QSP_CHAR *strVal, int strValBufSize);
	QSP_EXTERN void QSPSetInputStrText(const QSP_CHAR *str);
	QSP_EXTERN int QSPGetActionsCount();
	QSP_EXTERN void QSPGetActionData(int ind, QSP_CHAR **imgPath, QSP_CHAR **desc);
	QSP_EXTERN QSP_BOOL QSPExecuteSelActionCode(QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPSetSelActionIndex(int ind, QSP_BOOL isRefresh);
	QSP_EXTERN int QSPGetSelActionIndex();
	QSP_EXTERN QSP_BOOL QSPIsActionsChanged();
	QSP_EXTERN int QSPGetObjectsCount();
	QSP_EXTERN void QSPGetObjectData(int ind, QSP_CHAR **imgPath, QSP_CHAR **desc);
	QSP_EXTERN QSP_BOOL QSPSetSelObjectIndex(int ind, QSP_BOOL isRefresh);
	QSP_EXTERN int QSPGetSelObjectIndex();
	QSP_EXTERN QSP_BOOL QSPIsObjectsChanged();
	QSP_EXTERN void QSPShowWindow(int type, QSP_BOOL isShow);
	QSP_EXTERN QSP_BOOL QSPGetVarValuesCount(const QSP_CHAR *name, int *count);
	QSP_EXTERN QSP_BOOL QSPGetVarValues(const QSP_CHAR *name, int ind, int *numVal, QSP_CHAR **strVal);
	QSP_EXTERN int QSPGetMaxVarsCount();
	QSP_EXTERN QSP_BOOL QSPGetVarNameByIndex(int ind, QSP_CHAR **name);
	QSP_EXTERN QSP_BOOL QSPExecString(const QSP_CHAR *str, QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPExecCounter(QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPExecUserInput(QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPExecLocationCode(const QSP_CHAR *name, QSP_BOOL isRefresh);
	QSP_EXTERN void QSPGetLastErrorData(int *errorNum, QSP_CHAR **errorLoc, int *errorActIndex, int *errorLine);
	QSP_EXTERN const QSP_CHAR *QSPGetErrorDesc(int errorNum);
	QSP_EXTERN QSP_BOOL QSPLoadGameWorld(const QSP_CHAR *file);
	QSP_EXTERN QSP_BOOL QSPLoadGameWorldFromData(const void *data, int dataSize, const QSP_CHAR *file);
	QSP_EXTERN QSP_BOOL QSPSaveGame(const QSP_CHAR *file, QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPSaveGameAsData(void *buf, int bufSize, int *realSize, QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPOpenSavedGame(const QSP_CHAR *file, QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPOpenSavedGameFromData(const void *data, int dataSize, QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPRestartGame(QSP_BOOL isRefresh);
	QSP_EXTERN void QSPSetCallBack(int type, QSP_CALLBACK func);
	QSP_EXTERN void QSPInit();
	QSP_EXTERN void QSPDeInit();

	#ifdef __cplusplus
	}
	#endif

#endif
