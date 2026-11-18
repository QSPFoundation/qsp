/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "callbacks.h"
#include "actions.h"
#include "errors.h"
#include "locations.h"
#include "names.h"
#include "objects.h"

QSP_CALLBACK qspCallbacks[QSP_CALL_DUMMY];
QSP_BOOL qspIsInCallback = QSP_FALSE;

void qspPrepareCallback(QSPCallState *state, QSP_BOOL toRefreshUI)
{
    state->LocationState = qspLocationState;
    state->NamesGeneration = qspNamesGeneration;
    state->IsInCallback = qspIsInCallback;
    /* Switch to the callback mode */
    qspIsInCallback = QSP_TRUE;

    if (toRefreshUI) qspCallRefreshInt(QSP_TRUE);
}

QSP_BOOL qspFinalizeCallback(QSPCallState *state, QSP_BOOL toResetLocationState)
{
    /* Restore the previous mode */
    qspIsInCallback = state->IsInCallback;

    if (qspLocationState != state->LocationState)
    {
        if (toResetLocationState)
        {
            if (qspNamesGeneration == state->NamesGeneration) /* don't execute code with outdated names */
                qspLocationState = state->LocationState;
            qspResetError(QSP_FALSE);
        }
        return QSP_FALSE;
    }
    return QSP_TRUE;
}
