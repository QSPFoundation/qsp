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

#ifndef QSP_DEFAULTDEFINES
	#define QSP_DEFAULTDEFINES

	typedef wchar_t QSP_CHAR;

	#ifdef __cplusplus
		typedef int (*QSP_CALLBACK)(...);
	#else
		typedef int (*QSP_CALLBACK)();
	#endif

	#include "../../common/qsp.h"

	#ifdef __cplusplus
	extern "C"
	{
	#endif

	QSP_EXTERN QSP_BOOL QSPIsInCallBack();
	QSP_EXTERN void QSPEnableDebugMode(QSP_BOOL isDebug);
	QSP_EXTERN void QSPGetCurStateData(QSPString *loc, int *actIndex, int *line);
	QSP_EXTERN QSPString QSPGetVersion();
	QSP_EXTERN QSPString QSPGetCompiledDateTime();
	QSP_EXTERN int QSPGetFullRefreshCount();
	QSP_EXTERN QSPString QSPGetQstFullPath();
	QSP_EXTERN QSPString QSPGetCurLoc();
	QSP_EXTERN QSPString QSPGetMainDesc();
	QSP_EXTERN QSP_BOOL QSPIsMainDescChanged();
	QSP_EXTERN QSPString QSPGetVarsDesc();
	QSP_EXTERN QSP_BOOL QSPIsVarsDescChanged();
	QSP_EXTERN QSP_BOOL QSPGetExprValue(QSPString str, QSP_BOOL *isString, int *numVal, QSP_CHAR *strVal, int strValBufSize);
	QSP_EXTERN void QSPSetInputStrText(QSPString str);
	QSP_EXTERN int QSPGetActionsCount();
	QSP_EXTERN void QSPGetActionData(int ind, QSPString *imgPath, QSPString *desc);
	QSP_EXTERN QSP_BOOL QSPExecuteSelActionCode(QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPSetSelActionIndex(int ind, QSP_BOOL isRefresh);
	QSP_EXTERN int QSPGetSelActionIndex();
	QSP_EXTERN QSP_BOOL QSPIsActionsChanged();
	QSP_EXTERN int QSPGetObjectsCount();
	QSP_EXTERN void QSPGetObjectData(int ind, QSPString *imgPath, QSPString *desc);
	QSP_EXTERN QSP_BOOL QSPSetSelObjectIndex(int ind, QSP_BOOL isRefresh);
	QSP_EXTERN int QSPGetSelObjectIndex();
	QSP_EXTERN QSP_BOOL QSPIsObjectsChanged();
	QSP_EXTERN void QSPShowWindow(int type, QSP_BOOL isShow);
	QSP_EXTERN QSP_BOOL QSPGetVarValuesCount(QSPString name, int *count);
	QSP_EXTERN QSP_BOOL QSPGetVarValues(QSPString name, int ind, int *numVal, QSPString *strVal);
	QSP_EXTERN int QSPGetMaxVarsCount();
	QSP_EXTERN QSP_BOOL QSPGetVarNameByIndex(int ind, QSPString *name);
	QSP_EXTERN QSP_BOOL QSPExecString(QSPString str, QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPExecCounter(QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPExecUserInput(QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPExecLocationCode(QSPString name, QSP_BOOL isRefresh);
	QSP_EXTERN void QSPGetLastErrorData(int *errorNum, QSPString *errorLoc, int *errorActIndex, int *errorLine);
	QSP_EXTERN QSPString QSPGetErrorDesc(int errorNum);
	QSP_EXTERN QSP_BOOL QSPLoadGameWorld(QSPString file);
	QSP_EXTERN QSP_BOOL QSPSaveGame(QSPString file, QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPOpenSavedGame(QSPString file, QSP_BOOL isRefresh);
	/* Deprecated */
	QSP_EXTERN QSP_BOOL QSPLoadGameWorldFromData(const void *data, int dataSize, QSPString file);
	QSP_EXTERN QSP_BOOL QSPSaveGameAsData(void *buf, int bufSize, int *realSize, QSP_BOOL isRefresh);
	QSP_EXTERN QSP_BOOL QSPOpenSavedGameFromData(const void *data, int dataSize, QSP_BOOL isRefresh);
	/* ---------- */
	QSP_EXTERN QSP_BOOL QSPRestartGame(QSP_BOOL isRefresh);
	QSP_EXTERN void QSPSetCallBack(int type, QSP_CALLBACK func);
	QSP_EXTERN void QSPInit();
	QSP_EXTERN void QSPDeInit();

	#ifdef __cplusplus
	}
	#endif

#endif
