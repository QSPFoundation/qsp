/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "time.h"
#include "callbacks.h"

QSP_BIGINT qspMSCount = 0;

void qspResetTime(QSP_BIGINT msecs)
{
    qspMSCount = msecs;
    qspCallGetMSCount();
}

QSP_BIGINT qspGetTime(void)
{
    if ((qspMSCount += qspCallGetMSCount()) < 0) qspMSCount = 0;
    return qspMSCount;
}
