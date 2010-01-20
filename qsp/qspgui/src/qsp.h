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

#ifndef QSP_H
	#define QSP_H

	#ifdef _FLASH
		#include <AS3.h>
	#endif

	#ifdef EXPORT
		#ifdef _WIN
			#define QSP_EXTERN __declspec(dllexport)
		#else
			#define QSP_EXTERN extern
		#endif
	#else
		#define QSP_EXTERN
	#endif

	enum
	{
		QSP_ERR_DIVBYZERO = 100,
		QSP_ERR_TYPEMISMATCH,
		QSP_ERR_STACKOVERFLOW,
		QSP_ERR_TOOMANYITEMS,
		QSP_ERR_FILENOTFOUND,
		QSP_ERR_CANTLOADFILE,
		QSP_ERR_GAMENOTLOADED,
		QSP_ERR_COLONNOTFOUND,
		QSP_ERR_CANTINCFILE,
		QSP_ERR_CANTADDACTION,
		QSP_ERR_EQNOTFOUND,
		QSP_ERR_LOCNOTFOUND,
		QSP_ERR_ENDNOTFOUND,
		QSP_ERR_LABELNOTFOUND,
		QSP_ERR_NOTCORRECTNAME,
		QSP_ERR_QUOTNOTFOUND,
		QSP_ERR_BRACKNOTFOUND,
		QSP_ERR_BRACKSNOTFOUND,
		QSP_ERR_SYNTAX,
		QSP_ERR_UNKNOWNACTION,
		QSP_ERR_ARGSCOUNT,
		QSP_ERR_CANTADDOBJECT,
		QSP_ERR_CANTADDMENUITEM,
		QSP_ERR_TOOMANYVARS,
		QSP_ERR_INCORRECTREGEXP
	};

	enum
	{
		QSP_WIN_ACTS,
		QSP_WIN_OBJS,
		QSP_WIN_VARS,
		QSP_WIN_INPUT
	};

	enum
	{
		QSP_CALL_DEBUG, /* void func(const QSP_CHAR *str) */
		QSP_CALL_ISPLAYINGFILE, /* QSP_BOOL func(const QSP_CHAR *file) */
		QSP_CALL_PLAYFILE, /* void func(const QSP_CHAR *file, int volume) */
		QSP_CALL_CLOSEFILE, /* void func(const QSP_CHAR *file) */
		QSP_CALL_SHOWIMAGE, /* void func(const QSP_CHAR *file) */
		QSP_CALL_SHOWWINDOW, /* void func(int type, QSP_BOOL isShow) */
		QSP_CALL_DELETEMENU, /* void func() */
		QSP_CALL_ADDMENUITEM, /* void func(const QSP_CHAR *name, const QSP_CHAR *imgPath) */
		QSP_CALL_SHOWMENU, /* void func() */
		QSP_CALL_SHOWMSGSTR, /* void func(const QSP_CHAR *str) */
		QSP_CALL_REFRESHINT, /* void func(QSP_BOOL isRedraw) */
		QSP_CALL_SETTIMER, /* void func(int msecs) */
		QSP_CALL_SETINPUTSTRTEXT, /* void func(const QSP_CHAR *text) */
		QSP_CALL_SYSTEM, /* void func(const QSP_CHAR *str) */
		QSP_CALL_OPENGAMESTATUS, /* void func() */
		QSP_CALL_SAVEGAMESTATUS, /* void func() */
		QSP_CALL_SLEEP, /* void func(int msecs) */
		QSP_CALL_GETMSCOUNT, /* int func() */
		QSP_CALL_INPUTBOX, /* void func(const QSP_CHAR *text, QSP_CHAR *buffer, int maxLen) */
		QSP_CALL_DUMMY
	};

	#ifdef _UNICODE
		#ifndef _FLASH
			typedef wchar_t QSP_CHAR;
		#else
			typedef unsigned short QSP_CHAR;
		#endif
		#define QSP_FMT2(x) L##x
		#define QSP_FMT(x) QSP_FMT2(x)
	#else
		typedef char QSP_CHAR;
		#define QSP_FMT(x) x
	#endif

	typedef int QSP_BOOL;

	#define QSP_TRUE 1
	#define QSP_FALSE 0

	#ifndef _FLASH
		#ifdef __cplusplus
			typedef int (*QSP_CALLBACK)(...);
		#else
			typedef int (*QSP_CALLBACK)();
		#endif
	#else
		typedef struct
		{
			QSP_BOOL IsSet;
			AS3_Val ThisVal;
			AS3_Val FuncVal;
		} QSP_CALLBACK;
	#endif

	#ifdef __cplusplus
	extern "C"
	{
	#endif

	#ifndef _FLASH

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
		QSP_EXTERN QSP_BOOL QSPLoadGameWorldFromData(const char *data, int dataSize, const QSP_CHAR *file);
		QSP_EXTERN QSP_BOOL QSPSaveGame(const QSP_CHAR *file, QSP_BOOL isRefresh);
		QSP_EXTERN QSP_BOOL QSPSaveGameAsString(QSP_CHAR *strBuf, int strBufSize, int *realSize, QSP_BOOL isRefresh);
		QSP_EXTERN QSP_BOOL QSPOpenSavedGame(const QSP_CHAR *file, QSP_BOOL isRefresh);
		QSP_EXTERN QSP_BOOL QSPOpenSavedGameFromString(const QSP_CHAR *str, QSP_BOOL isRefresh);
		QSP_EXTERN QSP_BOOL QSPRestartGame(QSP_BOOL isRefresh);
		QSP_EXTERN void QSPSelectMenuItem(int ind);
		QSP_EXTERN void QSPSetCallBack(int type, QSP_CALLBACK func);
		QSP_EXTERN void QSPInit();
		QSP_EXTERN void QSPDeInit();

	#else

		QSP_EXTERN AS3_Val QSPEnableDebugMode(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetCurStateData(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetVersion(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetCompiledDateTime(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetFullRefreshCount(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetQstFullPath(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetCurLoc(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetMainDesc(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPIsMainDescChanged(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetVarsDesc(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPIsVarsDescChanged(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetExprValue(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPSetInputStrText(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetActionsCount(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetActionData(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPExecuteSelActionCode(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPSetSelActionIndex(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetSelActionIndex(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPIsActionsChanged(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetObjectsCount(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetObjectData(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPSetSelObjectIndex(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetSelObjectIndex(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPIsObjectsChanged(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPShowWindow(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetVarValuesCount(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetVarValues(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetMaxVarsCount(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetVarNameByIndex(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPExecString(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPExecCounter(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPExecUserInput(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPExecLocationCode(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetLastErrorData(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPGetErrorDesc(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPLoadGameWorld(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPLoadGameWorldFromData(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPSaveGame(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPSaveGameAsString(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPOpenSavedGame(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPOpenSavedGameFromString(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPRestartGame(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPSelectMenuItem(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPSetCallBack(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPInit(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPDeInit(void *param, AS3_Val args);
		QSP_EXTERN AS3_Val QSPReturnValue(void *param, AS3_Val args);

	#endif

	#ifdef __cplusplus
	}
	#endif

#endif
