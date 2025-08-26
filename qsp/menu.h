/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "variant.h"

#ifndef QSP_MENUDEFINES
    #define QSP_MENUDEFINES

    #define QSP_MENUDELIM QSP_FMT(":")
    QSP_DEFINE_SPECIAL_CHAR(QSP_MENUDELIM, QSP_FMT(':'))

    #define QSP_MAXMENUITEMS 100

    /* Statements */
    void qspStatementShowMenu(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

#endif
