/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "callbacks.h"
#include "actions.h"
#include "common.h"
#include "errors.h"
#include "locations.h"
#include "objects.h"

QSP_CALLBACK qspCallbacks[QSP_CALL_DUMMY];
QSP_BOOL qspIsInCallback = QSP_FALSE;

void qspPrepareCallback(QSPCallState *state, QSP_BOOL toRefreshUI)
{
    state->LocationState = qspLocationState;
    state->IsInCallback = qspIsInCallback;
    /* Save the state of changes */
    state->WindowsChangedState = qspCurWindowsChangedState;
    /* Save the execution state */
    state->RealCurLoc = qspRealCurLoc;
    state->RealActIndex = qspRealActIndex;
    state->RealLineNum = qspRealLineNum;
    state->RealLine = qspRealLine;
    /* Switch to the callback mode */
    qspIsInCallback = QSP_TRUE;

    if (toRefreshUI) qspCallRefreshInt(QSP_TRUE);
}

QSP_BOOL qspFinalizeCallback(QSPCallState *state, QSP_BOOL toResetLocationState)
{
    /* Restore the previous mode */
    qspIsInCallback = state->IsInCallback;
    /* Restore the execution state */
    /* It's still fine to restore old values even when a new game was started
     * because we exit the old code & reset the state anyway */
    qspRealCurLoc = state->RealCurLoc;
    qspRealActIndex = state->RealActIndex;
    qspRealLineNum = state->RealLineNum;
    qspRealLine = state->RealLine;
    /* Restore the state of changes without resetting the current state */
    qspCurWindowsChangedState = (QSP_TINYINT)(qspCurWindowsChangedState | state->WindowsChangedState);

    if (qspLocationState != state->LocationState)
    {
        if (toResetLocationState)
        {
            /* Reset the location state */
            qspLocationState = state->LocationState;
            qspResetError(QSP_FALSE);
        }
        return QSP_FALSE;
    }
    return QSP_TRUE;
}
