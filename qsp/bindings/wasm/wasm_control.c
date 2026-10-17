/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../../declarations.h"

#ifdef _WASM_BINDING

#include "../../actions.h"
#include "../../callbacks.h"
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
#include "../../tuples.h"
#include "../../variables.h"
#include "../../variant.h"

#ifdef __has_feature
    #if __has_feature(address_sanitizer)
        #define QSP_WASM_ASAN
        #include <sanitizer/asan_interface.h>
        #include <sanitizer/lsan_interface.h>
    #endif
#endif

/* Layouts assumed by qsp-wasm-memory.ts */
typedef char qspWasmLayoutCheck[(sizeof(QSP_CHAR) == 2 && sizeof(QSPVariant) == 12 && sizeof(QSP_BIGINT) == 4 &&
    sizeof(QSPListItem) == 16 && sizeof(QSPObjectItem) == 24 && sizeof(QSPErrorInfo) == 40) ? 1 : -1];

/* Strings passed by the JS side are UTF-16 buffers terminated with 0.
   The functions return the same results as the default binding (state, count, index); strings,
   variants, error data & lists are written through pointers instead of being returned or copied
   into buffers of the caller. Lists & buffers are allocated with malloc, the JS side frees them with _free. */

INLINE QSPString *qspWasmCopyLines(QSPLineOfCode *line, int count)
{
    int i;
    QSPString *lines = (QSPString *)malloc(count * sizeof(QSPString));
    for (i = 0; i < count; ++i, ++line)
        lines[i] = line->Str;
    return lines;
}
/* ------------------------------------------------------------ */
/* Debugger */

/* Enable the debugger */
void QSPEnableDebugMode(QSP_BOOL isDebug)
{
    qspIsDebug = isDebug;
}
/* Get current execution state */
void QSPGetCurStateData(QSPString *loc, int *actIndex, int *lineNum)
{
    *loc = (qspRealCurLoc ? qspRealCurLoc->Name : qspNullString);
    *actIndex = qspRealActIndex;
    *lineNum = qspRealLineNum;
}
/* Get names of all locations */
int QSPGetLocationNames(QSPString **locNames)
{
    int i;
    QSPString *names = (QSPString *)malloc(qspLocsCount * sizeof(QSPString));
    for (i = 0; i < qspLocsCount; ++i)
        names[i] = qspLocs[i]->Name;
    *locNames = names;
    return qspLocsCount;
}
/* Get base description of the specified location */
void QSPGetLocationDesc(QSP_CHAR *locName, QSPString *desc)
{
    QSPLocation *loc = qspLocByName(qspStringFromC(locName));
    *desc = (loc ? loc->Desc : qspNullString);
}
/* Get base actions of the specified location */
int QSPGetLocationActions(QSP_CHAR *locName, QSPListItem **actions)
{
    QSPLocation *loc = qspLocByName(qspStringFromC(locName));
    if (loc)
    {
        int i;
        QSPListItem *items = (QSPListItem *)malloc(loc->ActionsCount * sizeof(QSPListItem));
        for (i = 0; i < loc->ActionsCount; ++i)
        {
            items[i].Name = loc->Actions[i].Desc;
            items[i].Image = loc->Actions[i].Image;
        }
        *actions = items;
        return loc->ActionsCount;
    }
    *actions = 0;
    return -1;
}
/* Get code of the base action of the specified location */
int QSPGetLocationActionCode(QSP_CHAR *locName, int actionIndex, QSPString **lines)
{
    QSPLocation *loc = qspLocByName(qspStringFromC(locName));
    if (loc && actionIndex >= 0 && actionIndex < loc->ActionsCount)
    {
        QSPCodeBlock *code = loc->Actions[actionIndex].OnPressCode;
        if (code)
        {
            *lines = qspWasmCopyLines(code->Lines, code->LinesCount);
            return code->LinesCount;
        }
    }
    *lines = 0;
    return -1;
}
/* Get code of the specified location */
int QSPGetLocationCode(QSP_CHAR *locName, QSPString **lines)
{
    QSPLocation *loc = qspLocByName(qspStringFromC(locName));
    if (loc)
    {
        QSPCodeBlock *code = loc->OnVisitCode;
        if (code)
        {
            *lines = qspWasmCopyLines(code->Lines, code->LinesCount);
            return code->LinesCount;
        }
    }
    *lines = 0;
    return -1;
}
/* ------------------------------------------------------------ */
/* Version details */

/* Get version of the library */
void QSPGetVersion(QSPString *version)
{
    *version = QSP_STATIC_STR(QSP_VER);
}
/* Get build datetime of the library */
void QSPGetCompiledDateTime(QSPString *dateTime)
{
    *dateTime = QSP_STATIC_STR(QSP_FMT(__DATE__) QSP_FMT(", ") QSP_FMT(__TIME__));
}
/* ------------------------------------------------------------ */
/* Main description */

/* Get text of the main description */
void QSPGetMainDesc(QSPString *desc)
{
    *desc = qspBufStringToString(qspCurDesc);
}
/* ------------------------------------------------------------ */
/* Additional description */

/* Get text of the additional description */
void QSPGetVarsDesc(QSPString *desc)
{
    *desc = qspBufStringToString(qspCurVars);
}
/* ------------------------------------------------------------ */
/* Actions */

/* Get current actions */
int QSPGetActions(QSPListItem **items)
{
    int i;
    QSPListItem *actions = (QSPListItem *)malloc(qspCurActsCount * sizeof(QSPListItem));
    for (i = 0; i < qspCurActsCount; ++i)
    {
        actions[i].Name = qspCurActions[i].Desc;
        actions[i].Image = qspCurActions[i].Image;
    }
    *items = actions;
    return qspCurActsCount;
}
/* Set index of the selected action */
QSP_BOOL QSPSetSelActionIndex(int ind, QSP_BOOL toRefreshUI)
{
    if (ind >= 0 && ind < qspCurActsCount && ind != qspCurSelAction)
    {
        qspPrepareExecution(QSP_FALSE);
        qspCurSelAction = ind;
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_ACTSELECTED), 0, 0);
        if (qspErrorNum) return QSP_FALSE;
        if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    }
    return QSP_TRUE;
}
/* Execute the selected action */
QSP_BOOL QSPExecuteSelActionCode(QSP_BOOL toRefreshUI)
{
    if (qspCurSelAction >= 0)
    {
        qspPrepareExecution(QSP_FALSE);
        qspExecAction(qspCurSelAction);
        if (qspErrorNum) return QSP_FALSE;
        if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    }
    return QSP_TRUE;
}
/* Get index of the selected action */
int QSPGetSelActionIndex(void)
{
    return qspCurSelAction;
}
/* ------------------------------------------------------------ */
/* Objects */

/* Get current objects */
int QSPGetObjects(QSPObjectItem **items)
{
    int i;
    QSPObjectItem *objects = (QSPObjectItem *)malloc(qspCurObjsCount * sizeof(QSPObjectItem));
    for (i = 0; i < qspCurObjsCount; ++i)
        qspGetObjectInfoByIndex(i, objects + i);
    *items = objects;
    return qspCurObjsCount;
}
/* Set index of the selected object */
QSP_BOOL QSPSetSelObjectIndex(int ind, QSP_BOOL toRefreshUI)
{
    if (ind >= 0 && ind < qspCurObjsCount && ind != qspCurSelObject)
    {
        qspPrepareExecution(QSP_FALSE);
        qspCurSelObject = ind;
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_OBJSELECTED), 0, 0);
        if (qspErrorNum) return QSP_FALSE;
        if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    }
    return QSP_TRUE;
}
/* Get index of the selected object */
int QSPGetSelObjectIndex(void)
{
    return qspCurSelObject;
}
/* ------------------------------------------------------------ */
/* Get the state of changes per window */
int QSPGetWindowsChangedState(void)
{
    return qspCurWindowsChangedState;
}
/* Synchronize visibility of a region of the UI */
void QSPShowWindow(int type, QSP_BOOL toShow)
{
    qspSetWindowState(type, toShow);
}
/* ------------------------------------------------------------ */
/* Variables */

/* Get the number of items in an array */
QSP_BOOL QSPGetVarValuesCount(QSP_CHAR *name, int *count)
{
    QSPVar *var = qspVarReference(qspStringFromC(name), QSP_FALSE);
    if (var)
    {
        *count = var->ValsCount;
        return QSP_TRUE;
    }
    *count = 0;
    return QSP_FALSE;
}
/* Get index of an item by key */
QSP_BOOL QSPGetVarIndexByKey(QSP_CHAR *name, QSPVariant *key, int *ind)
{
    QSPVar *var = qspVarReference(qspStringFromC(name), QSP_FALSE);
    if (var)
    {
        int arrIndex = qspGetVarIndex(var, *key, QSP_FALSE);
        if (arrIndex >= 0 && arrIndex < var->ValsCount)
        {
            *ind = arrIndex;
            return QSP_TRUE;
        }
    }
    *ind = -1;
    return QSP_FALSE;
}
/* Get value & key of the specified array item, both are optional */
QSP_BOOL QSPGetVarValue(QSP_CHAR *name, int ind, QSPVariant *value, QSPVariant *key)
{
    QSPVar *var = qspVarReference(qspStringFromC(name), QSP_FALSE);
    if (var && ind >= 0 && ind < var->ValsCount)
    {
        if (value) *value = var->Values[ind];
        if (key)
        {
            int keyIndex = qspGetVarKeyIndex(var, ind);
            *key = (keyIndex >= 0 ? var->Keys[keyIndex].Key : qspGetEmptyVariant(QSP_TYPE_UNDEF));
        }
        return QSP_TRUE;
    }

    if (value) *value = qspGetEmptyVariant(QSP_TYPE_UNDEF);
    if (key) *key = qspGetEmptyVariant(QSP_TYPE_UNDEF);
    return QSP_FALSE;
}
/* ------------------------------------------------------------ */
/* Code execution */

/* Execute a line of code */
QSP_BOOL QSPExecString(QSP_CHAR *s, QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    qspExecStringAsCode(qspStringFromC(s));
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* Calculate string value of an expression (causes execution of code), the result is allocated */
QSP_BOOL QSPCalculateStrExpression(QSP_CHAR *s, QSPString *res, QSP_BOOL toRefreshUI)
{
    QSPString codeStr;
    QSPVariant value;
    *res = qspNullString;
    qspPrepareExecution(QSP_FALSE);
    codeStr = qspCopyToNewText(qspStringFromC(s));
    qspPrepareStringToExecution(&codeStr);
    value = qspCalculateExprValue(codeStr);
    qspFreeString(&codeStr);
    if (qspErrorNum) return QSP_FALSE;
    *res = qspGetVariantAsString(&value);
    qspFreeVariant(&value);
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* Calculate numeric value of an expression (causes execution of code) */
QSP_BOOL QSPCalculateNumExpression(QSP_CHAR *s, QSP_BIGINT *res, QSP_BOOL toRefreshUI)
{
    QSPString codeStr;
    QSPVariant value;
    qspPrepareExecution(QSP_FALSE);
    codeStr = qspCopyToNewText(qspStringFromC(s));
    qspPrepareStringToExecution(&codeStr);
    value = qspCalculateExprValue(codeStr);
    qspFreeString(&codeStr);
    if (qspErrorNum) return QSP_FALSE;
    if (!qspConvertVariantTo(&value, QSP_TYPE_NUM))
    {
        qspFreeVariant(&value);
        return QSP_FALSE;
    }
    *res = QSP_NUM(value);
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* Execute code of the specified location */
QSP_BOOL QSPExecLocationCode(QSP_CHAR *name, QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    qspExecLocByNameWithArgs(qspStringFromC(name), 0, 0, QSP_TRUE, 0);
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* Execute code of the special "COUNTER" location */
QSP_BOOL QSPExecCounter(QSP_BOOL toRefreshUI)
{
    if (!qspIsInCallback)
    {
        qspPrepareExecution(QSP_FALSE);
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_COUNTER), 0, 0);
        if (qspErrorNum) return QSP_FALSE;
        if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    }
    return QSP_TRUE;
}
/* Execute code of the special "USERCOM" location with the specified text of the input control */
QSP_BOOL QSPExecUserInput(QSP_CHAR *text, QSP_BOOL toRefreshUI)
{
    qspUpdateText(&qspCurInput, qspStringFromC(text));
    qspPrepareExecution(QSP_FALSE);
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_USERCOMMAND), 0, 0);
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Errors */

/* Get details of the last error */
void QSPGetLastErrorData(QSPErrorInfo *error)
{
    *error = qspLastError;
}
/* Get error description by numeric code */
void QSPGetErrorDesc(int errorNum, QSPString *desc)
{
    *desc = qspGetErrorDesc(errorNum);
}
/* ------------------------------------------------------------ */
/* Game controls */

/* Load game from data */
QSP_BOOL QSPLoadGameWorldFromData(const void *data, int dataSize, QSP_BOOL isNewGame)
{
    /* We don't execute any game code here */
    return qspOpenGame((void *)data, dataSize, isNewGame);
}
/* Save game state to an allocated buffer */
QSP_BOOL QSPSaveGameAsData(void **data, int *dataSize, QSP_BOOL toRefreshUI)
{
    void *buf, *newBuf;
    int bufSize = 64 * 1024;
    *data = 0;
    *dataSize = 0;
    buf = malloc(bufSize);
    if (!buf) return QSP_FALSE;
    qspPrepareExecution(QSP_FALSE);
    while (!qspSaveGameStatus(buf, &bufSize, QSP_TRUE))
    {
        if (!bufSize)
        {
            free(buf);
            return QSP_FALSE;
        }
        /* The buffer is too small, the new value contains required buffer size */
        /* We have to reserve some extra space to account for game updates during subsequent calls */
        bufSize += QSP_SAVEDGAMEDATAEXTRASPACE;
        if (!(newBuf = realloc(buf, bufSize)))
        {
            free(buf);
            return QSP_FALSE;
        }
        buf = newBuf;
    }
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    *data = buf;
    *dataSize = bufSize;
    return QSP_TRUE;
}
/* Load game state from data */
QSP_BOOL QSPOpenSavedGameFromData(const void *data, int dataSize, QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    if (!qspOpenGameStatus((void *)data, dataSize)) return QSP_FALSE;
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* Restart current game */
QSP_BOOL QSPRestartGame(QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    if (!qspRestartGame(QSP_TRUE)) return QSP_FALSE;
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Initialization of the engine */
void QSPInit(void)
{
#ifdef _DEBUG
    mwInit();
#endif
    qspInitRuntime();
}
/* Deallocate all resources */
void QSPTerminate(void)
{
    qspTerminateRuntime();
#ifdef _DEBUG
    mwTerm();
#endif
}
/* ------------------------------------------------------------ */
/* Sanitizer builds */
#ifdef QSP_WASM_ASAN
const char *__asan_default_options(void)
{
    return "detect_stack_use_after_return=1:print_stats=1";
}
#endif
/* Run the leak check */
void QSPRunChecks(void)
{
#ifdef QSP_WASM_ASAN
    __lsan_do_leak_check();
#endif
}

#endif
