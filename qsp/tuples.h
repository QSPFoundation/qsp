/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "text.h"

#ifndef QSP_TUPLESDEFINES
    #define QSP_TUPLESDEFINES

    #define QSP_TUPLEDISPLAY_START QSP_LSBRACK
    #define QSP_TUPLEDISPLAY_END QSP_RSBRACK
    #define QSP_TUPLEDISPLAY_DELIM QSP_COMMA

    extern QSPTuple qspNullTuple;

    /* External functions */
    void qspFreeTuple(QSPTuple *tuple);
    QSP_BOOL qspIsTupleNumber(QSPTuple tuple);
    QSP_BIGINT qspTupleToNum(QSPTuple tuple, QSP_BOOL *isValid);
    QSPTuple qspCopyToNewTuple(QSPVariant *values, int count);
    QSPTuple qspMoveToNewTuple(QSPVariant *values, int count);
    QSPTuple qspMergeToNewTuple(QSPVariant *list1, int count1, QSPVariant *list2, int count2);
    int qspTupleValueCompare(QSPTuple tuple, QSPVariant *value);
    int qspTuplesCompare(QSPTuple first, QSPTuple second);
    void qspAppendTupleToString(QSPTuple tuple, QSPBufString *res);

    INLINE QSPString qspGetTupleAsString(QSPTuple tuple)
    {
        QSPBufString buf = qspNewBufString(16);
        qspAppendTupleToString(tuple, &buf);
        return qspBufTextToString(buf);
    }

#endif
