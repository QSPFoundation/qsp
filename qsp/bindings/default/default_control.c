/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../../declarations.h"

#ifdef _DEFAULT_BINDING

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
#include "../../tuples.h"
#include "../../variables.h"
#include "../../variant.h"

QSPString QSPStringFromPair(QSP_CHAR *start, QSP_CHAR *end)
{
    return qspStringFromPair(start, end);
}

QSPString QSPStringFromLen(QSP_CHAR *start, int length)
{
    return qspStringFromLen(start, length);
}

QSPString QSPStringFromC(QSP_CHAR *s)
{
    return qspStringFromC(s);
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
    *loc = ((qspRealCurLoc >= 0 && qspRealCurLoc < qspLocsCount) ? qspLocs[qspRealCurLoc].Name : qspNullString);
    *actIndex = qspRealActIndex;
    *lineNum = qspRealLineNum;
}
/* Get names of all locations */
int QSPGetLocationNames(QSPString *locNames, int namesBufSize)
{
    int i;
    for (i = 0; i < qspLocsCount && i < namesBufSize; ++i)
        locNames[i] = qspLocs[i].Name;
    return qspLocsCount;
}
/* Get base description of the specified location */
QSPString QSPGetLocationDesc(QSPString locName)
{
    int locIndex = qspLocIndex(locName);
    if (locIndex >= 0) return qspLocs[locIndex].Desc;
    return qspNullString;
}
/* Get base actions of the specified location */
int QSPGetLocationActions(QSPString locName, QSPListItem *actions, int actionsBufSize)
{
    int locIndex = qspLocIndex(locName);
    if (locIndex >= 0)
    {
        int i;
        QSPLocation *loc = qspLocs + locIndex;
        for (i = 0; i < loc->ActionsCount && i < actionsBufSize; ++i)
        {
            actions[i].Name = loc->Actions[i].Desc;
            actions[i].Image = loc->Actions[i].Image;
        }
        return loc->ActionsCount;
    }
    return -1;
}
/* Get code of the base action of the specified location */
int QSPGetLocationActionCode(QSPString locName, int actionIndex, QSPLineInfo *lines, int linesBufSize)
{
    int locIndex = qspLocIndex(locName);
    if (locIndex >= 0 && actionIndex >= 0 && actionIndex < qspLocs[locIndex].ActionsCount)
    {
        int i;
        QSPLocAct *action = qspLocs[locIndex].Actions + actionIndex;
        for (i = 0; i < action->OnPressLinesCount && i < linesBufSize; ++i)
        {
            lines[i].Line = action->OnPressLines[i].Str;
            lines[i].LineNum = action->OnPressLines[i].LineNum;
        }
        return action->OnPressLinesCount;
    }
    return -1;
}
/* Get code of the specified location */
int QSPGetLocationCode(QSPString locName, QSPLineInfo *lines, int linesBufSize)
{
    int locIndex = qspLocIndex(locName);
    if (locIndex >= 0)
    {
        int i;
        QSPLocation *loc = qspLocs + locIndex;
        for (i = 0; i < loc->OnVisitLinesCount && i < linesBufSize; ++i)
        {
            lines[i].Line = loc->OnVisitLines[i].Str;
            lines[i].LineNum = loc->OnVisitLines[i].LineNum;
        }
        return loc->OnVisitLinesCount;
    }
    return -1;
}
/* Get code of the current action */
int QSPGetActionCode(int actionIndex, QSPLineInfo *lines, int linesBufSize)
{
    if (actionIndex >= 0 && actionIndex < qspCurActsCount)
    {
        int i;
        QSPCurAct *action = qspCurActions + actionIndex;
        for (i = 0; i < action->OnPressLinesCount && i < linesBufSize; ++i)
        {
            lines[i].Line = action->OnPressLines[i].Str;
            lines[i].LineNum = action->OnPressLines[i].LineNum;
        }
        return action->OnPressLinesCount;
    }
    return -1;
}
/* ------------------------------------------------------------ */
/* Version details */

/* Get version of the libqsp */
QSPString QSPGetVersion(void)
{
    return QSP_STATIC_STR(QSP_VER);
}
/* Get build datetime of the libqsp */
QSPString QSPGetCompiledDateTime(void)
{
    return QSP_STATIC_STR(QSP_FMT(__DATE__) QSP_FMT(", ") QSP_FMT(__TIME__));
}
/* ------------------------------------------------------------ */
/* Main description */

/* Get text of the main description */
QSPString QSPGetMainDesc(void)
{
    return qspBufTextToString(qspCurDesc);
}
/* Check whether the text has been updated */
QSP_BOOL QSPIsMainDescChanged(void)
{
    return qspIsMainDescChanged;
}
/* ------------------------------------------------------------ */
/* Additional description */

/* Get text of the additional description */
QSPString QSPGetVarsDesc(void)
{
    return qspBufTextToString(qspCurVars);
}
/* Check whether the text has been updated */
QSP_BOOL QSPIsVarsDescChanged(void)
{
    return qspIsVarsDescChanged;
}
/* ------------------------------------------------------------ */
/* Synchronize the value of the text input control */
void QSPSetInputStrText(QSPString val)
{
    qspUpdateText(&qspCurInput, val);
}
/* ------------------------------------------------------------ */
/* Actions */

/* Get current actions */
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
/* Check whether the actions have been updated */
QSP_BOOL QSPIsActionsChanged(void)
{
    return qspIsActsListChanged;
}
/* ------------------------------------------------------------ */
/* Objects */

/* Get current objects */
int QSPGetObjects(QSPListItem *items, int itemsBufSize)
{
    int i;
    for (i = 0; i < qspCurObjsCount && i < itemsBufSize; ++i)
    {
        items[i].Name = qspCurObjects[i].Desc;
        items[i].Image = qspCurObjects[i].Image;
    }
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
/* Check whether the objects have been updated */
QSP_BOOL QSPIsObjectsChanged(void)
{
    return qspIsObjsListChanged;
}
/* ------------------------------------------------------------ */
/* Synchronize visibility of a region of the UI */
void QSPShowWindow(int type, QSP_BOOL toShow)
{
    switch (type)
    {
    case QSP_WIN_ACTS:
        qspCurToShowActs = toShow;
        break;
    case QSP_WIN_OBJS:
        qspCurToShowObjs = toShow;
        break;
    case QSP_WIN_VARS:
        qspCurToShowVars = toShow;
        break;
    case QSP_WIN_INPUT:
        qspCurToShowInput = toShow;
        break;
    }
}
/* ------------------------------------------------------------ */
/* Variables */

/* Get the number of items in an array */
QSP_BOOL QSPGetVarValuesCount(QSPString name, int *count)
{
    QSPVar *var = qspVarReference(name, QSP_FALSE);
    if (var)
    {
        *count = var->ValsCount;
        return QSP_TRUE;
    }
    *count = 0;
    return QSP_FALSE;
}
/* Get index of an item by string */
QSP_BOOL QSPGetVarIndexByString(QSPString name, QSPString str, int *ind)
{
    QSPVar *var = qspVarReference(name, QSP_FALSE);
    if (var)
    {
        int arrIndex;
        QSPVariant index = qspRefStrVariant(str, QSP_TYPE_STR);
        arrIndex = qspGetVarIndex(var, index, QSP_FALSE);
        if (arrIndex >= 0)
        {
            *ind = arrIndex;
            return QSP_TRUE;
        }
    }
    *ind = -1;
    return QSP_FALSE;
}
/* Get value of the specified array item */
QSP_BOOL QSPGetVarValue(QSPString name, int ind, QSPVariant *res)
{
    QSPVar *var = qspVarReference(name, QSP_FALSE);
    if (var && ind >= 0 && ind < var->ValsCount)
    {
        *res = var->Values[ind];
        return QSP_TRUE;
    }

    *res = qspGetEmptyVariant(QSP_TYPE_UNDEF);
    return QSP_FALSE;
}
/* Get display string of the specified value */
QSP_BOOL QSPConvertValueToString(QSPVariant value, QSP_CHAR *buf, int bufSize)
{
    int resLen;
    QSPString res = qspGetVariantAsString(&value);
    resLen = qspStrLen(res);
    if (resLen >= bufSize) resLen = bufSize - 1;
    memcpy(buf, res.Str, resLen * sizeof(QSP_CHAR));
    buf[resLen] = 0;
    qspFreeString(&res);
    return QSP_TRUE;
}
/* Get numeric value of the specified array item */
QSP_BOOL QSPGetNumVarValue(QSPString name, int ind, QSP_BIGINT *res)
{
    QSPVar *var = qspVarReference(name, QSP_FALSE);
    if (var && ind >= 0 && ind < var->ValsCount)
    {
        QSPVariant *val = var->Values + ind;
        if (QSP_ISNUM(val->Type))
        {
            *res = QSP_PNUM(val);
            return QSP_TRUE;
        }
    }

    *res = 0;
    return QSP_FALSE;
}
/* Get string value of the specified array item */
QSP_BOOL QSPGetStrVarValue(QSPString name, int ind, QSPString *res)
{
    QSPVar *var = qspVarReference(name, QSP_FALSE);
    if (var && ind >= 0 && ind < var->ValsCount)
    {
        QSPVariant *val = var->Values + ind;
        if (QSP_ISSTR(val->Type))
        {
            *res = QSP_PSTR(val);
            return QSP_TRUE;
        }
    }

    *res = qspNullString;
    return QSP_FALSE;
}
/* ------------------------------------------------------------ */
/* Code execution */

/* Execute a line of code */
QSP_BOOL QSPExecString(QSPString s, QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    qspExecStringAsCode(s);
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* Calculate string value of an expression (causes execution of code) */
QSP_BOOL QSPCalculateStrExpression(QSPString s, QSP_CHAR *buf, int bufSize, QSP_BOOL toRefreshUI)
{
    int resLen;
    QSPVariant value;
    qspPrepareExecution(QSP_FALSE);
    qspPrepareStringToExecution(&s);
    value = qspCalculateExprValue(s);
    if (qspErrorNum) return QSP_FALSE;
    qspConvertVariantTo(&value, QSP_TYPE_STR);
    resLen = qspStrLen(QSP_STR(value));
    if (resLen >= bufSize) resLen = bufSize - 1;
    memcpy(buf, QSP_STR(value).Str, resLen * sizeof(QSP_CHAR));
    buf[resLen] = 0;
    qspFreeVariant(&value);
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* Calculate numeric value of an expression (causes execution of code) */
QSP_BOOL QSPCalculateNumExpression(QSPString s, QSP_BIGINT *res, QSP_BOOL toRefreshUI)
{
    QSPVariant value;
    qspPrepareExecution(QSP_FALSE);
    qspPrepareStringToExecution(&s);
    value = qspCalculateExprValue(s);
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
QSP_BOOL QSPExecLocationCode(QSPString name, QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    qspExecLocByNameWithArgs(name, 0, 0, QSP_TRUE, 0);
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
/* Execute code of the special "USERCOM" location */
QSP_BOOL QSPExecUserInput(QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_USERCOMMAND), 0, 0);
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Errors */

/* Get details of the last error */
QSPErrorInfo QSPGetLastErrorData(void)
{
    return qspLastError;
}
/* Get error description by numeric code */
QSPString QSPGetErrorDesc(int errorNum)
{
    return qspGetErrorDesc(errorNum);
}
/* ------------------------------------------------------------ */
/* Game controls */

/* Load game from data */
QSP_BOOL QSPLoadGameWorldFromData(const void *data, int dataSize, QSP_BOOL isNewGame)
{
    /* We don't execute any game code here */
    return qspOpenGame((void *)data, dataSize, isNewGame);
}
/* Save game state to a buffer */
QSP_BOOL QSPSaveGameAsData(void *buf, int *bufSize, QSP_BOOL toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    if (!qspSaveGameStatus(buf, bufSize, QSP_TRUE))
    {
        if (*bufSize)
        {
            /* Happens when we passed insufficient buffer, the new value contains required buffer size */
            /* We have to reserve some extra space to account for game updates during subsequent calls */
            *bufSize += QSP_SAVEDGAMEDATAEXTRASPACE;
        }
        return QSP_FALSE;
    }
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
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
    if (!qspNewGame(QSP_TRUE)) return QSP_FALSE;
    if (qspErrorNum) return QSP_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* Configure callbacks */
void QSPSetCallback(int type, QSP_CALLBACK func)
{
    qspSetCallback(type, func);
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

#endif
