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
#include "variant.h"

#ifndef QSP_TUPLESDEFINES
    #define QSP_TUPLESDEFINES

    extern QSPTuple qspNullTuple;

    /* External functions */
    void qspFreeTuple(QSPTuple tuple);
    int qspTupleToNum(QSPTuple tuple, QSP_BOOL *isValid);
    QSPString qspTupleToStr(QSPTuple tuple);
    QSPTuple qspMergeToTuple(QSPVariant *list1, int count1, QSPVariant *list2, int count2);
    int qspTuplesComp(QSPTuple first, QSPTuple second);

    INLINE QSPTuple qspGetNewTuple(QSPVariant *values, int count)
    {
        QSPTuple tuple;
        QSPVariant *newItem, *item;
        tuple.Items = count;
        tuple.Vals = (QSPVariant *)malloc(count * sizeof(QSPVariant));
        newItem = tuple.Vals;
        for (item = values; count > 0; --count, ++item, ++newItem)
            qspCopyToNewVariant(newItem, item);
        return tuple;
    }

#endif
