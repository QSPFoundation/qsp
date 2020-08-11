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

    #include "../qsp.h"

    #ifdef __cplusplus
    extern "C"
    {
    #endif

    QSP_EXTERN void QSPInit();
    QSP_EXTERN void QSPDeInit();
    QSP_EXTERN void QSPSetCallBack(int type, QSP_CALLBACK func);
    QSP_EXTERN void QSPEnableDebugMode(QSP_BOOL isDebug);
    QSP_EXTERN void QSPGetCurStateData(QSPString *loc, int *actIndex, int *line);
    QSP_EXTERN QSPString QSPGetVersion();
    QSP_EXTERN QSPString QSPGetCompiledDateTime();
    QSP_EXTERN int QSPGetFullRefreshCount();
    /* Main desc */
    QSP_EXTERN QSPString QSPGetMainDesc();
    QSP_EXTERN QSP_BOOL QSPIsMainDescChanged();
    /* Vars desc */
    QSP_EXTERN QSPString QSPGetVarsDesc();
    QSP_EXTERN QSP_BOOL QSPIsVarsDescChanged();
    /* Input string */
    QSP_EXTERN void QSPSetInputStrText(QSPString str);
    /* Actions */
    QSP_EXTERN int QSPGetActions(QSPListItem *items, int itemsBufSize);
    QSP_EXTERN QSP_BOOL QSPSetSelActionIndex(int ind, QSP_BOOL isRefresh);
    QSP_EXTERN int QSPGetSelActionIndex();
    QSP_EXTERN QSP_BOOL QSPIsActionsChanged();
    QSP_EXTERN QSP_BOOL QSPExecuteSelActionCode(QSP_BOOL isRefresh);
    /* Objects */
    QSP_EXTERN int QSPGetObjects(QSPListItem *items, int itemsBufSize);
    QSP_EXTERN QSP_BOOL QSPSetSelObjectIndex(int ind, QSP_BOOL isRefresh);
    QSP_EXTERN int QSPGetSelObjectIndex();
    QSP_EXTERN QSP_BOOL QSPIsObjectsChanged();
    /* Windows */
    QSP_EXTERN void QSPShowWindow(int type, QSP_BOOL isShow);
    /* Code execution */
    QSP_EXTERN QSP_BOOL QSPExecString(QSPString str, QSP_BOOL isRefresh);
    QSP_EXTERN QSP_BOOL QSPExecCounter(QSP_BOOL isRefresh);
    QSP_EXTERN QSP_BOOL QSPExecUserInput(QSP_BOOL isRefresh);
    QSP_EXTERN QSP_BOOL QSPExecLocationCode(QSPString name, QSP_BOOL isRefresh);
    /* Errors */
    QSP_EXTERN void QSPGetLastErrorData(int *errorNum, QSPString *errorLoc, int *errorActIndex, int *errorLine);
    QSP_EXTERN QSPString QSPGetErrorDesc(int errorNum);
    /* Game */
    QSP_EXTERN QSP_BOOL QSPLoadGameWorldFromData(const void *data, int dataSize, QSP_BOOL isNewGame);
    QSP_EXTERN QSP_BOOL QSPSaveGameAsData(void *buf, int bufSize, int *realSize, QSP_BOOL isRefresh);
    QSP_EXTERN QSP_BOOL QSPOpenSavedGameFromData(const void *data, int dataSize, QSP_BOOL isRefresh);

    QSP_EXTERN QSP_BOOL QSPRestartGame(QSP_BOOL isRefresh);
    /* Variables */
    QSP_EXTERN QSP_BOOL QSPGetVarValuesCount(QSPString name, int *count);
    QSP_EXTERN QSP_BOOL QSPGetVarValues(QSPString name, int ind, int *numVal, QSPString *strVal);
    QSP_EXTERN int QSPGetMaxVarsCount();
    QSP_EXTERN QSP_BOOL QSPGetVarNameByIndex(int ind, QSPString *name);

    #ifdef __cplusplus
    }
    #endif

#endif
