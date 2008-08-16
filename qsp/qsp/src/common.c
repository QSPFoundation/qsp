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

#include "common.h"
#include "actions.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "menu.h"
#include "objects.h"
#include "playlist.h"
#include "variables.h"

QSP_CHAR *qspCurDesc = 0;
long qspCurDescLen = 0;
QSP_CHAR *qspCurVars = 0;
long qspCurVarsLen = 0;
QSP_CHAR *qspCurInput = 0;
long qspCurInputLen = 0;
QSP_BOOL qspIsMainDescChanged = QSP_FALSE;
QSP_BOOL qspIsVarsDescChanged = QSP_FALSE;
QSP_BOOL qspCurIsShowVars = QSP_TRUE;
QSP_BOOL qspCurIsShowInput = QSP_TRUE;

void qspPrepareExecution()
{
	qspResetError(QSP_TRUE);
	qspIsMainDescChanged = qspIsVarsDescChanged = qspIsObjectsChanged = qspIsActionsChanged = QSP_FALSE;
}

void qspMemClear(QSP_BOOL isFirst)
{
	qspClearIncludes(isFirst);
	qspClearVars(isFirst);
	qspClearObjects(isFirst);
	qspClearActions(isFirst);
	qspClearMenu(isFirst);
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
