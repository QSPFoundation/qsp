/* Copyright (C) 2001-2020 Valeriy Argunov (val AT time DOT guru) */
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
#include "../../variables.h"
#include "../../variant.h"

/* ------------------------------------------------------------ */
/* ������� */

/* ���������� ������� ������� */
void QSPEnableDebugMode(QSP_BOOL isDebug)
{
    qspIsDebug = isDebug;
}
/* ��������� ������ �������� ��������� */
void QSPGetCurStateData(QSPString *loc, int *actIndex, int *line)
{
    *loc = (qspRealCurLoc >= 0 && qspRealCurLoc < qspLocsCount ? qspLocs[qspRealCurLoc].Name : qspNullString);
    *actIndex = qspRealActIndex;
    *line = qspRealLine;
}
/* ------------------------------------------------------------ */
/* ���������� � ������ */

/* ������ */
QSPString QSPGetVersion()
{
    return QSP_STATIC_STR(QSP_VER);
}
/* ���� � ����� ���������� */
QSPString QSPGetCompiledDateTime()
{
    return QSP_STATIC_STR(QSP_FMT(__DATE__) QSP_FMT(", ") QSP_FMT(__TIME__));
}
/* ------------------------------------------------------------ */
/* ���������� ������ ���������� ������� */
int QSPGetFullRefreshCount()
{
    return qspFullRefreshCount;
}
/* ------------------------------------------------------------ */
/* �������� �������� ������� */

/* ����� ��������� ���� �������� ������� */
QSPString QSPGetMainDesc()
{
    return qspCurDesc;
}
/* ����������� ��������� ������ ��������� �������� */
QSP_BOOL QSPIsMainDescChanged()
{
    return qspIsMainDescChanged;
}
/* ------------------------------------------------------------ */
/* �������������� �������� ������� */

/* ����� ��������������� ���� �������� ������� */
QSPString QSPGetVarsDesc()
{
    return qspCurVars;
}
/* ����������� ��������� ������ ��������������� �������� */
QSP_BOOL QSPIsVarsDescChanged()
{
    return qspIsVarsDescChanged;
}
/* ------------------------------------------------------------ */
/* ����� ������ ����� */
void QSPSetInputStrText(QSPString val)
{
    qspUpdateText(&qspCurInput, val);
}
/* ------------------------------------------------------------ */
/* ������ �������� */

int QSPGetActions(QSPListItem *items, int itemsBufSize)
{
    int i;
    for (i = 0; i < qspCurActionsCount && i < itemsBufSize; ++i)
    {
        items[i].Name = qspCurActions[i].Desc;
        items[i].Image = qspCurActions[i].Image;
    }
    return qspCurActionsCount;
}
/* ���������� ���� ���������� �������� */
QSP_BOOL QSPExecuteSelActionCode(QSP_BOOL isRefresh)
{
    if (qspCurSelAction >= 0)
    {
        if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
        qspPrepareExecution();
        if (qspIsDisableCodeExec) return QSP_FALSE;
        qspExecAction(qspCurSelAction);
        if (qspErrorNum) return QSP_FALSE;
        if (isRefresh) qspCallRefreshInt(QSP_FALSE);
    }
    return QSP_TRUE;
}
/* ���������� ������ ���������� �������� */
QSP_BOOL QSPSetSelActionIndex(int ind, QSP_BOOL isRefresh)
{
    if (ind >= 0 && ind < qspCurActionsCount && ind != qspCurSelAction)
    {
        if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
        qspPrepareExecution();
        if (qspIsDisableCodeExec) return QSP_FALSE;
        qspCurSelAction = ind;
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_FMT("ONACTSEL")), 0, 0);
        if (qspErrorNum) return QSP_FALSE;
        if (isRefresh) qspCallRefreshInt(QSP_FALSE);
    }
    return QSP_TRUE;
}
/* �������� ������ ���������� �������� */
int QSPGetSelActionIndex()
{
    return qspCurSelAction;
}
/* ����������� ��������� ������ �������� */
QSP_BOOL QSPIsActionsChanged()
{
    return qspIsActionsChanged;
}
/* ------------------------------------------------------------ */
/* ������ �������� */

int QSPGetObjects(QSPListItem *items, int itemsBufSize)
{
    int i;
    for (i = 0; i < qspCurObjectsCount && i < itemsBufSize; ++i)
    {
        items[i].Name = qspCurObjects[i].Desc;
        items[i].Image = qspCurObjects[i].Image;
    }
    return qspCurObjectsCount;
}
/* ���������� ������ ���������� ������� */
QSP_BOOL QSPSetSelObjectIndex(int ind, QSP_BOOL isRefresh)
{
    if (ind >= 0 && ind < qspCurObjectsCount && ind != qspCurSelObject)
    {
        if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
        qspPrepareExecution();
        if (qspIsDisableCodeExec) return QSP_FALSE;
        qspCurSelObject = ind;
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_FMT("ONOBJSEL")), 0, 0);
        if (qspErrorNum) return QSP_FALSE;
        if (isRefresh) qspCallRefreshInt(QSP_FALSE);
    }
    return QSP_TRUE;
}
/* �������� ������ ���������� ������� */
int QSPGetSelObjectIndex()
{
    return qspCurSelObject;
}
/* ����������� ��������� ������ �������� */
QSP_BOOL QSPIsObjectsChanged()
{
    return qspIsObjectsChanged;
}
/* ------------------------------------------------------------ */
/* ����� / ������� ���� */
void QSPShowWindow(int type, QSP_BOOL isShow)
{
    switch (type)
    {
    case QSP_WIN_ACTS:
        qspCurIsShowActs = isShow;
        break;
    case QSP_WIN_OBJS:
        qspCurIsShowObjs = isShow;
        break;
    case QSP_WIN_VARS:
        qspCurIsShowVars = isShow;
        break;
    case QSP_WIN_INPUT:
        qspCurIsShowInput = isShow;
        break;
    }
}
/* ------------------------------------------------------------ */
/* ���������� */

/* �������� ���������� ��������� ������� */
QSP_BOOL QSPGetVarValuesCount(QSPString name, int *count)
{
    QSPVar *var;
    if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
    qspResetError();
    var = qspVarReference(name, QSP_FALSE);
    if (qspErrorNum) return QSP_FALSE;
    *count = var->ValsCount;
    return QSP_TRUE;
}
/* �������� �������� ���������� �������� ������� */
QSP_BOOL QSPGetVarValues(QSPString name, int ind, int *numVal, QSPString *strVal)
{
    QSPVar *var;
    if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
    qspResetError();
    var = qspVarReference(name, QSP_FALSE);
    if (qspErrorNum || ind < 0 || ind >= var->ValsCount) return QSP_FALSE;
    *numVal = var->Values[ind].Num;
    *strVal = var->Values[ind].Str;
    return QSP_TRUE;
}
/* �������� ������������ ���������� ���������� */
int QSPGetMaxVarsCount()
{
    return QSP_VARSCOUNT;
}
/* �������� ��� ���������� � ��������� �������� */
QSP_BOOL QSPGetVarNameByIndex(int index, QSPString *name)
{
    if (index < 0 || index >= QSP_VARSCOUNT || !qspVars[index].Name.Str) return QSP_FALSE;
    *name = qspVars[index].Name;
    return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* ���������� ���� */

/* ���������� ������ ���� */
QSP_BOOL QSPExecString(QSPString s, QSP_BOOL isRefresh)
{
    if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
    qspPrepareExecution();
    if (qspIsDisableCodeExec) return QSP_FALSE;
    qspExecStringAsCodeWithArgs(s, 0, 0, 0);
    if (qspErrorNum) return QSP_FALSE;
    if (isRefresh) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* ���������� ���� ��������� ������� */
QSP_BOOL QSPExecLocationCode(QSPString name, QSP_BOOL isRefresh)
{
    if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
    qspPrepareExecution();
    if (qspIsDisableCodeExec) return QSP_FALSE;
    qspExecLocByName(name, QSP_FALSE);
    if (qspErrorNum) return QSP_FALSE;
    if (isRefresh) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* ���������� ���� �������-�������� */
QSP_BOOL QSPExecCounter(QSP_BOOL isRefresh)
{
    if (!qspIsInCallBack)
    {
        qspPrepareExecution();
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_FMT("COUNTER")), 0, 0);
        if (qspErrorNum) return QSP_FALSE;
        if (isRefresh) qspCallRefreshInt(QSP_FALSE);
    }
    return QSP_TRUE;
}
/* ���������� ���� �������-����������� ������ ����� */
QSP_BOOL QSPExecUserInput(QSP_BOOL isRefresh)
{
    if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
    qspPrepareExecution();
    if (qspIsDisableCodeExec) return QSP_FALSE;
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_FMT("USERCOM")), 0, 0);
    if (qspErrorNum) return QSP_FALSE;
    if (isRefresh) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* ������ */

/* �������� ���������� � ��������� ������ */
void QSPGetLastErrorData(int *errorNum, QSPString *errorLoc, int *errorActIndex, int *errorLine)
{
    *errorNum = qspErrorNum;
    *errorLoc = (qspErrorLoc >= 0 && qspErrorLoc < qspLocsCount ? qspLocs[qspErrorLoc].Name : qspNullString);
    *errorActIndex = qspErrorActIndex;
    *errorLine = qspErrorLine;
}
/* �������� �������� ������ �� �� ������ */
QSPString QSPGetErrorDesc(int errorNum)
{
    return qspGetErrorDesc(errorNum);
}
/* ------------------------------------------------------------ */
/* ���������� ����� */

/* �������� ����� ���� �� ������ */
QSP_BOOL QSPLoadGameWorldFromData(const void *data, int dataSize, QSP_BOOL isNewGame)
{
    if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
    qspResetError();
    if (qspIsDisableCodeExec) return QSP_FALSE;
    qspOpenQuestFromData((char *)data, dataSize, isNewGame);
    if (qspErrorNum) return QSP_FALSE;
    return QSP_TRUE;
}
/* ���������� ��������� � ������ */
QSP_BOOL QSPSaveGameAsData(void *buf, int bufSize, int *realSize, QSP_BOOL isRefresh)
{
    int size;
    QSPString data;
    if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
    qspPrepareExecution();
    if (qspIsDisableCodeExec) return QSP_FALSE;
    data = qspSaveGameStatusToString();
    if (!data.Str)
    {
        *realSize = 0;
        return QSP_FALSE;
    }
    size = qspStrLen(data) * sizeof(QSP_CHAR);
    *realSize = size;
    if (size > bufSize)
    {
        qspFreeString(data);
        return QSP_FALSE;
    }
    memcpy(buf, data.Str, size);
    qspFreeString(data);
    if (isRefresh) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* �������� ��������� �� ������ */
QSP_BOOL QSPOpenSavedGameFromData(const void *data, int dataSize, QSP_BOOL isRefresh)
{
    if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
    qspPrepareExecution();
    if (qspIsDisableCodeExec) return QSP_FALSE;
    qspOpenGameStatusFromString(qspStringFromLen((QSP_CHAR *)data, dataSize / sizeof(QSP_CHAR)));
    if (qspErrorNum) return QSP_FALSE;
    if (isRefresh) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* ���������� ���� */
QSP_BOOL QSPRestartGame(QSP_BOOL isRefresh)
{
    if (qspIsExitOnError && qspErrorNum) return QSP_FALSE;
    qspPrepareExecution();
    if (qspIsDisableCodeExec) return QSP_FALSE;
    qspNewGame(QSP_TRUE);
    if (qspErrorNum) return QSP_FALSE;
    if (isRefresh) qspCallRefreshInt(QSP_FALSE);
    return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* ��������� CALLBACK'�� */
void QSPSetCallBack(int type, QSP_CALLBACK func)
{
    qspSetCallBack(type, func);
}
/* ------------------------------------------------------------ */
/* ������������� */
void QSPInit()
{
    #ifdef _DEBUG
        mwInit();
    #endif
    qspNullString = qspStringFromPair(0, 0);
    qspEmptyString = QSP_STATIC_STR(QSP_FMT(""));
    qspIsDebug = QSP_FALSE;
    qspRefreshCount = qspFullRefreshCount = 0;
    qspQstCRC = 0;
    qspRealCurLoc = -1;
    qspRealActIndex = -1;
    qspRealLine = 0;
    qspMSCount = 0;
    qspLocs = 0;
    qspLocsNames = 0;
    qspLocsCount = 0;
    qspCurLoc = -1;
    qspTimerInterval = 0;
    qspCurIsShowObjs = qspCurIsShowActs = qspCurIsShowVars = qspCurIsShowInput = QSP_TRUE;
    setlocale(LC_ALL, QSP_LOCALE);
    qspSetSeed(0);
    qspPrepareExecution();
    qspMemClear(QSP_TRUE);
    qspInitCallBacks();
    qspInitStats();
    qspInitMath();
}
/* ��������������� */
void QSPDeInit()
{
    qspMemClear(QSP_FALSE);
    qspCreateWorld(0, 0);
    #ifdef _DEBUG
        mwTerm();
    #endif
}

#endif
