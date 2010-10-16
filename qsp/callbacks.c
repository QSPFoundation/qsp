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

#include "declarations.h"
#include "callbacks.h"
#include "actions.h"
#include "coding.h"
#include "common.h"
#include "errors.h"
#include "objects.h"
#include "text.h"

QSP_CALLBACK qspCallBacks[QSP_CALL_DUMMY];
QSP_BOOL qspIsInCallBack = QSP_FALSE;
QSP_BOOL qspIsDisableCodeExec = QSP_FALSE;
QSP_BOOL qspIsExitOnError = QSP_FALSE;

void qspSaveCallState(QSPCallState *state, QSP_BOOL isDisableCodeExec, QSP_BOOL isExitOnError)
{
	state->IsInCallBack = qspIsInCallBack;
	state->IsDisableCodeExec = qspIsDisableCodeExec;
	state->IsExitOnError = qspIsExitOnError;
	state->IsMainDescChanged = qspIsMainDescChanged;
	state->IsVarsDescChanged = qspIsVarsDescChanged;
	state->IsObjectsChanged = qspIsObjectsChanged;
	state->IsActionsChanged = qspIsActionsChanged;
	qspIsInCallBack = QSP_TRUE;
	qspIsDisableCodeExec = isDisableCodeExec;
	qspIsExitOnError = isExitOnError;
}

void qspRestoreCallState(QSPCallState *state)
{
	if (!qspIsExitOnError) qspResetError();
	qspIsExitOnError = state->IsExitOnError;
	qspIsDisableCodeExec = state->IsDisableCodeExec;
	qspIsInCallBack = state->IsInCallBack;
	if (state->IsActionsChanged) qspIsActionsChanged = QSP_TRUE;
	if (state->IsObjectsChanged) qspIsObjectsChanged = QSP_TRUE;
	if (state->IsVarsDescChanged) qspIsVarsDescChanged = QSP_TRUE;
	if (state->IsMainDescChanged) qspIsMainDescChanged = QSP_TRUE;
}
