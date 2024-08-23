/* Copyright (C) 2001-2024 Val Argunov (byte AT qsp DOT org) */
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
#include "text.h"

#ifndef QSP_TUPLESDEFINES
    #define QSP_TUPLESDEFINES

    #define QSP_TUPLEDISPLAY_START QSP_LSBRACK
    #define QSP_TUPLEDISPLAY_END QSP_RSBRACK
    #define QSP_TUPLEDISPLAY_DELIM QSP_COMMA

    #define QSP_TUPLEIND_STARTID QSP_FMT("\x02")
    #define QSP_TUPLEIND_ENDID QSP_FMT("\x03")
    #define QSP_TUPLEIND_STRID QSP_FMT("\x01")
    #define QSP_TUPLEIND_DELIM QSP_FMT("\x1F")

    extern QSPTuple qspNullTuple;

    /* External functions */
    void qspFreeTuple(QSPTuple *tuple);
    int qspTupleToNum(QSPTuple tuple, QSP_BOOL *isValid);
    QSPTuple qspCopyToNewTuple(QSPVariant *values, int count);
    QSPTuple qspMoveToNewTuple(QSPVariant *values, int count);
    QSPTuple qspMergeToNewTuple(QSPVariant *list1, int count1, QSPVariant *list2, int count2);
    int qspTuplesComp(QSPTuple first, QSPTuple second);
    void qspAppendTupleToDisplayString(QSPBufString *res, QSPTuple tuple);
    void qspAppendTupleToIndexString(QSPBufString *res, QSPTuple tuple);

    INLINE QSPString qspTupleToDisplayString(QSPTuple tuple)
    {
        QSPBufString buf = qspNewBufString(16);
        qspAppendTupleToDisplayString(&buf, tuple);
        return qspBufTextToString(buf);
    }

    INLINE QSPString qspTupleToIndexString(QSPTuple tuple)
    {
        QSPBufString buf = qspNewBufString(16);
        qspAppendTupleToIndexString(&buf, tuple);
        return qspBufTextToString(buf);
    }

#endif
