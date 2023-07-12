/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
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
#include "common.h"
#include "errors.h"
#include "objects.h"

QSP_CALLBACK qspCallBacks[QSP_CALL_DUMMY];
QSP_BOOL qspIsInCallBack = QSP_FALSE;
QSP_BOOL qspToDisableCodeExec = QSP_FALSE; /* blocks major state changes, so we can skip some checks */

void qspPrepareCallBack(QSPCallState *state, QSP_BOOL toDisableCodeExec, QSP_BOOL toRefreshUI)
{
    state->IsInCallBack = qspIsInCallBack;
    /* save a state of changes */
    state->ToDisableCodeExec = qspToDisableCodeExec;
    state->IsMainDescChanged = qspIsMainDescChanged;
    state->IsVarsDescChanged = qspIsVarsDescChanged;
    state->IsObjectsChanged = qspIsObjectsChanged;
    state->IsActionsChanged = qspIsActionsChanged;
    /* save an execution state */
    state->RealCurLoc = qspRealCurLoc;
    state->RealActIndex = qspRealActIndex;
    state->RealLine = qspRealLine;
    /* switch to a callback mode */
    qspIsInCallBack = QSP_TRUE;
    qspToDisableCodeExec = toDisableCodeExec;

    if (toRefreshUI && qspCallBacks[QSP_CALL_REFRESHINT])
        qspCallBacks[QSP_CALL_REFRESHINT](QSP_TRUE);
}

void qspFinalizeCallBack(QSPCallState *state)
{
    /* restore a previous mode */
    qspToDisableCodeExec = state->ToDisableCodeExec;
    qspIsInCallBack = state->IsInCallBack;
    /* restore an execution state */
    /* it's still fine to restore old values even when a new game was started
     * because we exit the old code & reset the state anyway */
    qspRealCurLoc = state->RealCurLoc;
    qspRealActIndex = state->RealActIndex;
    qspRealLine = state->RealLine;
    /* restore a state of changes */
    if (state->IsActionsChanged) qspIsActionsChanged = QSP_TRUE;
    if (state->IsObjectsChanged) qspIsObjectsChanged = QSP_TRUE;
    if (state->IsVarsDescChanged) qspIsVarsDescChanged = QSP_TRUE;
    if (state->IsMainDescChanged) qspIsMainDescChanged = QSP_TRUE;
}
