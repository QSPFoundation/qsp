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

#include <AS3.h>
#include "../../qsp.h"

#ifndef QSP_FLASHDEFINES
	#define QSP_FLASHDEFINES

	#ifdef _UNICODE
		typedef unsigned short QSP_CHAR;
	#endif

	typedef struct
	{
		QSP_BOOL IsSet;
		AS3_Val ThisVal;
		AS3_Val FuncVal;
	} QSP_CALLBACK;

	char *qspW2C(QSP_CHAR *);
	QSP_CHAR *qspC2W(char *);

	void qspSetReturnValue(AS3_Val res);

	QSP_EXTERN AS3_Val QSPIsInCallBack(void *param, AS3_Val args);
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
	QSP_EXTERN AS3_Val QSPSaveGameAsData(void *param, AS3_Val args);
	QSP_EXTERN AS3_Val QSPOpenSavedGame(void *param, AS3_Val args);
	QSP_EXTERN AS3_Val QSPOpenSavedGameFromData(void *param, AS3_Val args);
	QSP_EXTERN AS3_Val QSPRestartGame(void *param, AS3_Val args);
	QSP_EXTERN AS3_Val QSPSetCallBack(void *param, AS3_Val args);
	QSP_EXTERN AS3_Val QSPInit(void *param, AS3_Val args);
	QSP_EXTERN AS3_Val QSPDeInit(void *param, AS3_Val args);
	QSP_EXTERN AS3_Val QSPReturnValue(void *param, AS3_Val args);

#endif
