/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stddef.h>
#include <emscripten.h>

#ifndef QSP_WASMDEFINES
    #define QSP_WASMDEFINES

    #ifdef _UNICODE
        typedef unsigned short QSP_CHAR; /* UTF-16 */
        #define QSP_FMT2(x) u##x
        #define QSP_FMT(x) QSP_FMT2(x)

        #define QSP_ONIG_ENC ONIG_ENCODING_UTF16_LE
        #define QSP_TO_GAME_SB(a) (unsigned char)qspReverseConvertUC((a), qspCP1251ToUnicodeTable)
        #define QSP_TO_GAME_UC(a) (unsigned short)(a)
        #define QSP_FROM_GAME_SB(a) qspDirectConvertUC((a), qspCP1251ToUnicodeTable)
        #define QSP_FROM_GAME_UC(a) (a)
        #define QSP_CHRLWR(a) qspToWLower(a)
        #define QSP_CHRUPR(a) qspToWUpper(a)
    #else
        #error "Non-Unicode build using WebAssembly binding is not supported"
    #endif

    #define QSP_TIME time

    /* The callbacks are not registered, the host is called directly */
    typedef void *QSP_CALLBACK;

    /* Functions called from JS (qsp-wasm-module.ts) */
    #define QSP_WASM_EXPORT EMSCRIPTEN_KEEPALIVE

    #include "../qsp.h"

    QSP_WASM_EXPORT void QSPInit(void);
    QSP_WASM_EXPORT void QSPTerminate(void);
    QSP_WASM_EXPORT void QSPGetVersion(QSPString *version);
    QSP_WASM_EXPORT void QSPGetCompiledDateTime(QSPString *dateTime);
    /* Debugging */
    QSP_WASM_EXPORT void QSPEnableDebugMode(QSP_BOOL isDebug);
    QSP_WASM_EXPORT void QSPGetCurStateData(QSPString *loc, int *actIndex, int *lineNum);
    QSP_WASM_EXPORT int QSPGetLocationNames(QSPString *locNames, int namesBufSize);
    QSP_WASM_EXPORT void QSPGetLocationDesc(QSP_CHAR *locName, QSPString *desc);
    QSP_WASM_EXPORT int QSPGetLocationActions(QSP_CHAR *locName, QSPListItem *actions, int actionsBufSize);
    QSP_WASM_EXPORT int QSPGetLocationActionCode(QSP_CHAR *locName, int actionIndex, QSPLineInfo *lines, int linesBufSize);
    QSP_WASM_EXPORT int QSPGetLocationCode(QSP_CHAR *locName, QSPLineInfo *lines, int linesBufSize);
    /* Main desc */
    QSP_WASM_EXPORT void QSPGetMainDesc(QSPString *desc);
    /* Vars desc */
    QSP_WASM_EXPORT void QSPGetVarsDesc(QSPString *desc);
    /* Actions */
    QSP_WASM_EXPORT int QSPGetActions(QSPListItem *items, int itemsBufSize);
    QSP_WASM_EXPORT QSP_BOOL QSPSetSelActionIndex(int ind, QSP_BOOL toRefreshUI);
    QSP_WASM_EXPORT int QSPGetSelActionIndex(void);
    QSP_WASM_EXPORT QSP_BOOL QSPExecuteSelActionCode(QSP_BOOL toRefreshUI);
    /* Objects */
    QSP_WASM_EXPORT int QSPGetObjects(QSPObjectItem *items, int itemsBufSize);
    QSP_WASM_EXPORT QSP_BOOL QSPSetSelObjectIndex(int ind, QSP_BOOL toRefreshUI);
    QSP_WASM_EXPORT int QSPGetSelObjectIndex(void);
    /* Windows */
    QSP_WASM_EXPORT int QSPGetWindowsChangedState(void);
    QSP_WASM_EXPORT void QSPShowWindow(int type, QSP_BOOL toShow);
    /* Code execution */
    QSP_WASM_EXPORT QSP_BOOL QSPExecString(QSP_CHAR *s, QSP_BOOL toRefreshUI);
    QSP_WASM_EXPORT QSP_BOOL QSPCalculateStrExpression(QSP_CHAR *s, QSP_CHAR *buf, int bufSize, QSP_BOOL toRefreshUI);
    QSP_WASM_EXPORT QSP_BOOL QSPCalculateNumExpression(QSP_CHAR *s, QSP_BIGINT *res, QSP_BOOL toRefreshUI);
    QSP_WASM_EXPORT QSP_BOOL QSPExecCounter(QSP_BOOL toRefreshUI);
    QSP_WASM_EXPORT QSP_BOOL QSPExecUserInput(QSP_CHAR *text, QSP_BOOL toRefreshUI);
    QSP_WASM_EXPORT QSP_BOOL QSPExecLocationCode(QSP_CHAR *name, QSP_BOOL toRefreshUI);
    /* Errors */
    QSP_WASM_EXPORT void QSPGetLastErrorData(QSPErrorInfo *error);
    QSP_WASM_EXPORT void QSPGetErrorDesc(int errorNum, QSPString *desc);
    /* Game */
    QSP_WASM_EXPORT QSP_BOOL QSPLoadGameWorldFromData(const void *data, int dataSize, QSP_BOOL isNewGame);
    QSP_WASM_EXPORT QSP_BOOL QSPSaveGameAsData(void *buf, int *bufSize, QSP_BOOL toRefreshUI);
    QSP_WASM_EXPORT QSP_BOOL QSPOpenSavedGameFromData(const void *data, int dataSize, QSP_BOOL toRefreshUI);

    QSP_WASM_EXPORT QSP_BOOL QSPRestartGame(QSP_BOOL toRefreshUI);
    /* Variables */
    QSP_WASM_EXPORT QSP_BOOL QSPGetVarValuesCount(QSP_CHAR *name, int *count);
    QSP_WASM_EXPORT QSP_BOOL QSPGetVarIndexByKey(QSP_CHAR *name, QSPVariant *key, int *ind);
    QSP_WASM_EXPORT QSP_BOOL QSPGetVarValue(QSP_CHAR *name, int ind, QSPVariant *value, QSPVariant *key);
    /* Sanitizer builds */
    QSP_WASM_EXPORT void QSPRunChecks(void);

#endif
