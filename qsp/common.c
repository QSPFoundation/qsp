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

#include "common.h"
#include "actions.h"
#include "errors.h"
#include "game.h"
#include "objects.h"
#include "playlist.h"
#include "regexp.h"
#include "text.h"
#include "variables.h"

static unsigned int qspRandX[55], qspRandY[256], qspRandZ;
static int qspRandI, qspRandJ;
QSP_BOOL qspIsDebug = QSP_FALSE;
QSPString qspCurDesc;
QSPString qspCurVars;
QSPString qspCurInput;
QSPString qspViewPath;
int qspTimerInterval = 0;
QSP_BOOL qspIsMainDescChanged = QSP_FALSE;
QSP_BOOL qspIsVarsDescChanged = QSP_FALSE;
QSP_BOOL qspCurIsShowVars = QSP_TRUE;
QSP_BOOL qspCurIsShowInput = QSP_TRUE;

static unsigned int qspURand();

void qspPrepareExecution()
{
	qspResetError();
	qspIsMainDescChanged = qspIsVarsDescChanged = qspIsObjectsChanged = qspIsActionsChanged = QSP_FALSE;
}

void qspMemClear(QSP_BOOL isFirst)
{
	int i;
	qspClearIncludes(isFirst);
	qspClearVars(isFirst);
	qspClearObjects(isFirst);
	qspClearActions(isFirst);
	qspClearPlayList(isFirst);
	qspClearRegExps(isFirst);
	if (!isFirst)
	{
		if (!qspIsEmpty(qspCurDesc)) qspIsMainDescChanged = QSP_TRUE;
		if (!qspIsEmpty(qspCurVars)) qspIsVarsDescChanged = QSP_TRUE;

		qspFreeString(qspCurDesc);
		qspFreeString(qspCurVars);
		qspFreeString(qspCurInput);
		qspFreeString(qspViewPath);

		for (i = qspSavedVarsGroupsCount - 1; i >= 0; --i)
			qspClearVarsList(qspSavedVarsGroups[i].Vars, qspSavedVarsGroups[i].VarsCount);
		if (qspSavedVarsGroups) free(qspSavedVarsGroups);
	}
	qspCurDesc = qspNullString;
	qspCurVars = qspNullString;
	qspCurInput = qspNullString;
	qspViewPath = qspNullString;
	qspSavedVarsGroups = 0;
	qspSavedVarsGroupsCount = 0;
}

void qspSetSeed(unsigned int seed)
{
	int i;
	qspRandX[0] = 1;
	qspRandX[1] = seed;
	for (i = 2; i < 55; ++i)
		qspRandX[i] = qspRandX[i - 1] + qspRandX[i - 2];
	qspRandI = 23;
	qspRandJ = 54;
	for (i = 255; i >= 0; --i) qspURand();
	for (i = 255; i >= 0; --i) qspRandY[i] = qspURand();
	qspRandZ = qspURand();
}

static unsigned int qspURand()
{
	if (--qspRandI < 0) qspRandI = 54;
	if (--qspRandJ < 0) qspRandJ = 54;
	return qspRandX[qspRandJ] += qspRandX[qspRandI];
}

int qspRand()
{
	int i = qspRandZ >> 24;
	qspRandZ = qspRandY[i];
	if (--qspRandI < 0) qspRandI = 54;
	if (--qspRandJ < 0) qspRandJ = 54;
	qspRandY[i] = qspRandX[qspRandJ] += qspRandX[qspRandI];
	return qspRandZ & QSP_RANDMASK;
}
