/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"

#ifndef QSP_TIMEDEFINES
    #define QSP_TIMEDEFINES

    extern QSP_BIGINT qspMSCount;

    /* External functions */
    void qspResetTime(QSP_BIGINT msecs);
    QSP_BIGINT qspGetTime(void);

#endif
