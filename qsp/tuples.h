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
#include "text.h"

#ifndef QSP_TUPLESDEFINES
    #define QSP_TUPLESDEFINES

    #define QSP_TUPLEDISPLAYSTART QSP_FMT("(")
    #define QSP_TUPLEDISPLAYDELIM QSP_FMT(",")
    #define QSP_TUPLEDISPLAYEND QSP_FMT(")")
    #define QSP_TUPLEINDSTART QSP_FMT("\x02")
    #define QSP_TUPLEINDDELIM QSP_FMT("\x1F")
    #define QSP_TUPLEINDEND QSP_FMT("\x03")

    extern QSPTuple qspNullTuple;

    /* External functions */
    void qspFreeTuple(QSPTuple tuple);
    int qspTupleToNum(QSPTuple tuple, QSP_BOOL *isValid);
    QSPTuple qspGetNewTuple(QSPVariant *values, int count);
    QSPTuple qspMergeToTuple(QSPVariant *list1, int count1, QSPVariant *list2, int count2);
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
