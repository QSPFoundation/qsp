/* Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include "declarations.h"

volatile QSP_BOOL qspIsMustWait = QSP_FALSE;
QSP_BOOL qspIsMainDescChanged = QSP_FALSE;
QSP_BOOL qspIsVarsDescChanged = QSP_FALSE;
QSP_BOOL qspIsObjectsChanged = QSP_FALSE;
QSP_BOOL qspIsActionsChanged = QSP_FALSE;
long qspRefreshCount = 0;
long qspFullRefreshCount = 0;
long qspErrorLoc = -1;
long qspErrorWhere = 0;
long qspErrorLine = 0;
long qspRealCurLoc = -1;
long qspRealLine = 0;
long qspRealWhere = 0;
long qspErrorNum = 0;
QSPVar qspVars[QSP_VARSCOUNT];
QSPLocation *qspLocs = 0;
long qspLocsCount = 0;
long qspCurLoc = -1;
QSPObj qspCurObjects[QSP_MAXOBJECTS];
long qspCurObjectsCount = 0;
long qspCurSelObject = -1;
QSP_CHAR *qspCurMenuLocs[QSP_MAXMENUITEMS];
long qspCurMenuItems = 0;
QSP_CHAR *qspCurDesc = 0;
long qspCurDescLen = 0;
QSP_CHAR *qspCurVars = 0;
long qspCurVarsLen = 0;
QSP_CHAR *qspCurInput = 0;
long qspCurInputLen = 0;
QSP_CHAR *qspPlayList = 0;
long qspPlayListLen = 0;
QSPCurAct qspCurActions[QSP_MAXACTIONS];
long qspCurActionsCount = 0;
long qspCurSelAction = -1;
long qspMSCount = 0;
QSP_BOOL qspCurIsShowObjs = QSP_TRUE;
QSP_BOOL qspCurIsShowActs = QSP_TRUE;
QSP_BOOL qspCurIsShowVars = QSP_TRUE;
QSP_BOOL qspCurIsShowInput = QSP_TRUE;
QSP_CHAR *qspQstPath = 0;
long qspQstPathLen = 0;
QSP_CHAR *qspQstFullPath = 0;
long qspQstCRC = 0;

void qspSetError(long num)
{
	if (!qspErrorNum)
	{
		qspErrorNum = num;
		qspErrorLoc = qspRealCurLoc;
		qspErrorWhere = qspRealWhere;
		qspErrorLine = qspRealLine;
	}
}

void qspResetError()
{
	qspErrorNum = 0;
	qspErrorLoc = qspRealCurLoc = -1;
	qspErrorWhere = qspRealWhere = QSP_AREA_NONE;
	qspErrorLine = qspRealLine = 0;
}

void qspPrepareExecution()
{
	qspIsMainDescChanged = qspIsVarsDescChanged = qspIsObjectsChanged = qspIsActionsChanged = QSP_FALSE;
}

void qspClearMenu(QSP_BOOL isFirst)
{
	long i;
	if (!isFirst)
	{
		for (i = 0; i < qspCurMenuItems; ++i)
			free(qspCurMenuLocs[i]);
	}
	qspCurMenuItems = 0;
}

void qspMemClear(QSP_BOOL isFirst)
{
	qspClearVars(isFirst);
	qspClearObjects(isFirst);
	qspClearActions(isFirst);
	qspClearMenu(isFirst);
	qspCurLoc = -1;
	qspMSCount = 0;
	qspCurIsShowObjs = qspCurIsShowActs = qspCurIsShowVars = qspCurIsShowInput = QSP_TRUE;
	if (!isFirst)
	{
		if (qspCurDesc)
		{
			free(qspCurDesc);
			if (qspCurDescLen) qspIsMainDescChanged = QSP_TRUE;
		}
		if (qspCurVars)
		{
			free(qspCurVars);
			if (qspCurVarsLen) qspIsVarsDescChanged = QSP_TRUE;
		}
		if (qspCurInput) free(qspCurInput);
		if (qspPlayList) free(qspPlayList);
	}
	qspCurDesc = 0;
	qspCurDescLen = 0;
	qspCurVars = 0;
	qspCurVarsLen = 0;
	qspCurInput = 0;
	qspCurInputLen = 0;
	qspPlayList = 0;
	qspPlayListLen = 0;
}

void qspRefresh(QSP_BOOL isChangeDesc)
{
	long oldRefreshCount;
	qspClearActions(QSP_FALSE);
	++qspRefreshCount;
	if (isChangeDesc) ++qspFullRefreshCount;
	oldRefreshCount = qspRefreshCount;
	qspExecLoc(qspLocs[qspCurLoc].Name, isChangeDesc);
	if (qspErrorNum) return;
	if (qspRefreshCount == oldRefreshCount)
		qspExecLocByVarName(QSP_STRCHAR QSP_FMT("ONNEWLOC"));
}

QSP_CHAR *qspFormatText(QSP_CHAR *txt)
{
	QSP_CHAR *newTxt, *lPos, *rPos;
	long len, txtLen, oldTxtLen, bufSize;
	QSPVariant val = qspGetVarValueByName(QSP_FMT("DISABLESUBEX"));
	if (val.Num) return qspGetNewText(txt, -1);
	bufSize = 256;
	newTxt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	txtLen = oldTxtLen = 0;
	lPos = QSP_STRSTR(txt, QSP_LSUBEX);
	while (lPos)
	{
		len = (long)(lPos - txt);
		if ((txtLen += len) >= bufSize)
		{
			bufSize = txtLen + 128;
			newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
		}
		QSP_STRNCPY(newTxt + oldTxtLen, txt, len);
		oldTxtLen = txtLen;
		txt = lPos + QSP_LEN(QSP_LSUBEX);
		rPos = qspStrPos(txt, QSP_RSUBEX, QSP_FALSE);
		if (!rPos)
		{
			qspSetError(QSP_ERR_BRACKNOTFOUND);
			free(newTxt);
			return 0;
		}
		*rPos = 0;
		val = qspExprValue(txt);
		*rPos = QSP_RSUBEX[0];
		if (qspErrorNum)
		{
			free(newTxt);
			return 0;
		}
		val = qspConvertVariantTo(val, QSP_TRUE, QSP_TRUE, 0);
		if ((txtLen += (long)QSP_STRLEN(val.Str)) >= bufSize)
		{
			bufSize = txtLen + 128;
			newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
		}
		QSP_STRCPY(newTxt + oldTxtLen, val.Str);
		free(val.Str);
		oldTxtLen = txtLen;
		txt = rPos + QSP_LEN(QSP_RSUBEX);
		lPos = QSP_STRSTR(txt, QSP_LSUBEX);
	}
	return qspGetAddText(newTxt, txt, txtLen, -1);
}
