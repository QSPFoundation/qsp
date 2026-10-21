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

/* Layouts read by qsp-wasm-memory.ts */
typedef char qspWasmLayoutCheck[(sizeof(QSP_CHAR) == 2 && sizeof(QSPVariant) == 12 && sizeof(QSP_BIGINT) == 4 &&
    sizeof(QSPListItem) == 16 && sizeof(QSPObjectItem) == 24 && sizeof(QSPLineInfo) == 12 &&
    sizeof(QSPErrorInfo) == 40) ? 1 : -1];

/* The JS side allocates & frees all the memory the functions write to */

INLINE int qspGetLinesInfo(QSPLineOfCode *line, int linesCount, QSPLineInfo *lines, int linesBufSize)
{
    int i;
    for (i = 0; i < linesCount && i < linesBufSize; ++i, ++line)
    {
        lines[i].Line = line->Str;
        lines[i].LineNum = line->LineNum;
    }
    return linesCount;
}

void QSPEnableDebugMode(QSP_BOOL isDebug)
{
    qspIsDebug = isDebug;
}

void QSPGetCurStateData(QSPString *loc, int *actIndex, int *lineNum)
{
    *loc = (qspRealCurLoc ? qspRealCurLoc->Name : qspNullString);
    *actIndex = qspRealActIndex;
    *lineNum = qspRealLineNum;
}

int QSPGetLocationNames(QSPString *locNames, int namesBufSize)
{
    int i;
    for (i = 0; i < qspLocsCount && i < namesBufSize; ++i)
        locNames[i] = qspLocs[i]->Name;
    return qspLocsCount;
}

void QSPGetLocationDesc(QSP_CHAR *locName, QSPString *desc)
{
    QSPLocation *loc = qspLocByName(qspStringFromC(locName));
    *desc = (loc ? loc->Desc : qspNullString);
}

/* The lists return the count (-1 if there is no such location or code), a bigger one than the
   buffer means the JS side has to repeat the call with a bigger buffer */
int QSPGetLocationActions(QSP_CHAR *locName, QSPListItem *actions, int actionsBufSize)
{
    QSPLocation *loc = qspLocByName(qspStringFromC(locName));
    if (loc)
    {
        int i;
        for (i = 0; i < loc->ActionsCount && i < actionsBufSize; ++i)
        {
            actions[i].Name = loc->Actions[i].Desc;
            actions[i].Image = loc->Actions[i].Image;
        }
        return loc->ActionsCount;
    }
    return -1;
}

int QSPGetLocationActionCode(QSP_CHAR *locName, int actionIndex, QSPLineInfo *lines, int linesBufSize)
{
    QSPLocation *loc = qspLocByName(qspStringFromC(locName));
    if (loc && actionIndex >= 0 && actionIndex < loc->ActionsCount)
    {
        QSPCodeBlock *code = loc->Actions[actionIndex].OnPressCode;
        if (code) return qspGetLinesInfo(code->Lines, code->LinesCount, lines, linesBufSize);
    }
    return -1;
}

int QSPGetLocationCode(QSP_CHAR *locName, QSPLineInfo *lines, int linesBufSize)
{
    QSPLocation *loc = qspLocByName(qspStringFromC(locName));
    if (loc)
    {
        QSPCodeBlock *code = loc->OnVisitCode;
        if (code) return qspGetLinesInfo(code->Lines, code->LinesCount, lines, linesBufSize);
    }
    return -1;
}

int QSPGetActionCode(int actionIndex, QSPLineInfo *lines, int linesBufSize)
{
    if (actionIndex >= 0 && actionIndex < qspCurActsCount)
    {
        QSPCurAct *action = qspCurActions + actionIndex;
        QSPCodeBlock *code = action->OnPressCode;
        if (code) return qspGetLinesInfo(code->Lines + action->OnPressStartLine, action->OnPressLinesCount, lines, linesBufSize);
    }
    return -1;
}

void QSPGetVersion(QSPString *version)
{
    *version = QSP_STATIC_STR(QSP_VER);
}

void QSPGetCompiledDateTime(QSPString *dateTime)
{
    *dateTime = QSP_STATIC_STR(QSP_FMT(__DATE__) QSP_FMT(", ") QSP_FMT(__TIME__));
}

void QSPGetCurLocation(QSPString *loc)
{
    *loc = (qspCurLoc ? qspCurLoc->Name : qspNullString);
}

void QSPGetMainDesc(QSPString *desc)
{
    *desc = qspBufStringToString(qspCurDesc);
}

void QSPGetVarsDesc(QSPString *desc)
{
    *desc = qspBufStringToString(qspCurVars);
}

int QSPGetActions(QSPListItem *items, int itemsBufSize)
{
    int i;
    for (i = 0; i < qspCurActsCount && i < itemsBufSize; ++i)
    {
        items[i].Name = qspCurActions[i].Desc;
        items[i].Image = qspCurActions[i].Image;
    }
    return qspCurActsCount;
}

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

int QSPGetSelActionIndex(void)
{
    return qspCurSelAction;
}

int QSPGetObjects(QSPObjectItem *items, int itemsBufSize)
{
    int i;
    for (i = 0; i < qspCurObjsCount && i < itemsBufSize; ++i)
        qspGetObjectInfoByIndex(i, items + i);
    return qspCurObjsCount;
}

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

int QSPGetSelObjectIndex(void)
{
    return qspCurSelObject;
}

int QSPGetWindowsChangedState(void)
{
    return qspCurWindowsChangedState;
}

void QSPShowWindow(int type, QSP_BOOL toShow)
{
    qspSetWindowState(type, toShow);
}

QSP_BOOL QSPGetVarValuesCount(QSP_CHAR *name, int *count)
{
    QSPVar *var = qspVarReference(qspStringFromC(name), QSP_VARREF_GETSILENTLY);
    if (var)
    {
        *count = var->ValsCount;
        return QSP_TRUE;
    }
    *count = 0;
    return QSP_FALSE;
}

QSP_BOOL QSPGetVarIndexByKey(QSP_CHAR *name, QSPVariant *key, int *ind)
{
    QSPVar *var = qspVarReference(qspStringFromC(name), QSP_VARREF_GETSILENTLY);
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

/* Both the value & the key are optional */
QSP_BOOL QSPGetVarValue(QSP_CHAR *name, int ind, QSPVariant *value, QSPVariant *key)
{
    QSPVar *var = qspVarReference(qspStringFromC(name), QSP_VARREF_GETSILENTLY);
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

QSP_BOOL QSPExecString(QSP_CHAR *s, QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    qspExecStringAsCode(qspStringFromC(s));
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}

/* A longer result is cut */
QSP_BOOL QSPCalculateStrExpression(QSP_CHAR *s, QSP_CHAR *buf, int bufSize, QSP_BOOL toRefreshUI)
{
    int resLen;
    QSPString codeStr;
    QSPVariant value;
    qspPrepareExecution(QSP_FALSE);
    codeStr = qspCopyToNewText(qspStringFromC(s));
    qspPrepareStringToExecution(&codeStr);
    value = qspCalculateExprValue(codeStr);
    qspFreeString(&codeStr);
    if (qspErrorNum) return QSP_FALSE;
    qspConvertVariantTo(&value, QSP_TYPE_STR);
    resLen = qspStrLen(QSP_STR(value));
    if (resLen >= bufSize) resLen = bufSize - 1;
    if (resLen) memcpy(buf, QSP_STR(value).Str, resLen * sizeof(QSP_CHAR));
    buf[resLen] = 0;
    qspFreeVariant(&value);
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}

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

QSP_BOOL QSPExecLocationCode(QSP_CHAR *name, QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    qspExecLocByNameWithArgs(qspStringFromC(name), 0, 0, QSP_TRUE, 0);
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}

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

QSP_BOOL QSPExecUserInput(QSP_CHAR *text, QSP_BOOL toRefreshUI)
{
    qspUpdateText(&qspCurInput, qspStringFromC(text));
    qspPrepareExecution(QSP_FALSE);
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_USERCOMMAND), 0, 0);
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}

void QSPGetLastErrorData(QSPErrorInfo *error)
{
    *error = qspLastError;
}

void QSPGetErrorDesc(int errorNum, QSPString *desc)
{
    *desc = qspGetErrorDesc(errorNum);
}

QSP_BOOL QSPLoadGameWorldFromData(const void *data, int dataSize, QSP_BOOL isNewGame)
{
    return qspOpenGame((void *)data, dataSize, isNewGame);
}

QSP_BOOL QSPSaveGameAsData(void *buf, int *bufSize, QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    if (!qspSaveGameStatus(buf, bufSize, QSP_TRUE))
    {
        if (*bufSize)
        {
            /* The buffer is too small: the new size is required, with extra space for game updates */
            *bufSize += QSP_SAVEDGAMEDATAEXTRASPACE;
        }
        return QSP_FALSE;
    }
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}

QSP_BOOL QSPOpenSavedGameFromData(const void *data, int dataSize, QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    if (!qspOpenGameStatus((void *)data, dataSize)) return QSP_FALSE;
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}

QSP_BOOL QSPRestartGame(QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    if (!qspRestartGame(QSP_TRUE)) return QSP_FALSE;
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}

void QSPInit(void)
{
#ifdef _DEBUG
    mwInit();
#endif
    qspInitRuntime();
}

void QSPTerminate(void)
{
    qspTerminateRuntime();
#ifdef _DEBUG
    mwTerm();
#endif
}

#ifdef QSP_WASM_ASAN
const char *__asan_default_options(void)
{
    return "detect_stack_use_after_return=1:print_stats=1";
}
#endif

/* Runs the leak check of the sanitizer builds */
void QSPRunChecks(void)
{
#ifdef QSP_WASM_ASAN
    __lsan_do_leak_check();
#endif
}

#endif
