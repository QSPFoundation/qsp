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

#include "../../qsp.h"

#ifndef QSP_JAVADEFINES
	#define QSP_JAVADEFINES

	#ifdef _UNICODE
		typedef wchar_t QSP_CHAR;
	#endif

	typedef int QSP_CALLBACK;

	void QSPIsInCallBack(QSP_BOOL *res);
	void QSPEnableDebugMode(QSP_BOOL isDebug);
	void QSPGetCurStateData(QSP_CHAR **loc, int *actIndex, int *line);
	void QSPGetVersion(const QSP_CHAR* *res);
	void QSPGetCompiledDateTime(const QSP_CHAR* *res);
	void QSPGetFullRefreshCount(int *res);
	void QSPGetQstFullPath(const QSP_CHAR* *res);
	void QSPGetCurLoc(const QSP_CHAR* *res);
	void QSPGetMainDesc(const QSP_CHAR* *res);
	void QSPIsMainDescChanged(QSP_BOOL *res);
	void QSPGetVarsDesc(const QSP_CHAR* *res);
	void QSPIsVarsDescChanged(QSP_BOOL *res);
	void QSPGetExprValue(QSP_BOOL *res, const QSP_CHAR *expr, QSP_BOOL *isString, int *numVal, QSP_CHAR *strVal, int strValBufSize);
	void QSPSetInputStrText(const QSP_CHAR *val);
	void QSPGetActionsCount(int* res);
	void QSPGetActionData(int ind, QSP_CHAR **image, QSP_CHAR **desc);
	void QSPExecuteSelActionCode(QSP_BOOL *res, QSP_BOOL isRefresh);
	void QSPSetSelActionIndex(QSP_BOOL *res, int ind, QSP_BOOL isRefresh);
	void QSPGetSelActionIndex(int *res);
	void QSPIsActionsChanged(QSP_BOOL *res);
	void QSPGetObjectsCount(int *res);
	void QSPGetObjectData(int ind, QSP_CHAR **image, QSP_CHAR **desc);
	void QSPSetSelObjectIndex(QSP_BOOL *res, int ind, QSP_BOOL isRefresh);
	void QSPGetSelObjectIndex(int* res);
	void QSPIsObjectsChanged(QSP_BOOL *res);
	void QSPShowWindow(int type, QSP_BOOL isShow);
	void QSPGetVarValuesCount(QSP_BOOL *res, const QSP_CHAR *name, int *count);
	void QSPGetVarValues(QSP_BOOL *res, const QSP_CHAR *name, int ind, int *numVal, QSP_CHAR **strVal);
	void QSPGetMaxVarsCount(int *res);
	void QSPGetVarNameByIndex(QSP_BOOL *res, int index, QSP_CHAR **name);
	void QSPExecString(QSP_BOOL *res, const QSP_CHAR *s, QSP_BOOL isRefresh);
	void QSPExecLocationCode(QSP_BOOL *res, const QSP_CHAR *name, QSP_BOOL isRefresh);
	void QSPExecCounter(QSP_BOOL *res, QSP_BOOL isRefresh);
	void QSPExecUserInput(QSP_BOOL *res, QSP_BOOL isRefresh);
	void QSPGetLastErrorData(int *errorNum, QSP_CHAR **errorLoc, int *errorActIndex, int *errorLine);
	void QSPGetErrorDesc(const QSP_CHAR* *res, int errorNum);
	void QSPLoadGameWorld(QSP_BOOL *res, const QSP_CHAR *fileName);
	void QSPLoadGameWorldFromData(QSP_BOOL *res, const void *data, int dataSize, const QSP_CHAR *fileName);
	void QSPSaveGame(QSP_BOOL *res, const QSP_CHAR *fileName, QSP_BOOL isRefresh);
	void QSPSaveGameAsData(QSP_BOOL *res, void *buf, int bufSize, int *realSize, QSP_BOOL isRefresh);
	void QSPOpenSavedGame(QSP_BOOL *res, const QSP_CHAR *fileName, QSP_BOOL isRefresh);
	void QSPOpenSavedGameFromData(QSP_BOOL *res, const void *data, int dataSize, QSP_BOOL isRefresh);
	void QSPRestartGame(QSP_BOOL *res, QSP_BOOL isRefresh);
	void QSPSetCallBack(int type, QSP_CALLBACK func);
	void QSPInit();
	void QSPDeInit();

#endif
