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

#include "tuples.h"
#include "text.h"
#include "variant.h"

QSPTuple qspNullTuple;

void qspFreeTuple(QSPTuple *tuple)
{
    if (tuple->Vals)
    {
        qspFreeVariants(tuple->Vals, tuple->Items);
        free(tuple->Vals);
    }
}

QSP_BOOL qspIsTupleNumber(QSPTuple tuple)
{
    if (tuple.Items == 1)
    {
        switch (QSP_BASETYPE(tuple.Vals[0].Type))
        {
        case QSP_TYPE_TUPLE:
            return qspIsTupleNumber(QSP_TUPLE(tuple.Vals[0]));
        case QSP_TYPE_NUM:
            return QSP_TRUE;
        case QSP_TYPE_STR:
            return qspIsStrNumber(QSP_STR(tuple.Vals[0]));
        }
    }
    return QSP_FALSE;
}

QSP_BIGINT qspTupleToNum(QSPTuple tuple, QSP_BOOL *isValid)
{
    switch (tuple.Items)
    {
    case 0: /* a special case, i.e. an empty tuple must be convertible to 0 */
        if (isValid) *isValid = QSP_TRUE;
        return 0;
    case 1:
        switch (QSP_BASETYPE(tuple.Vals[0].Type))
        {
        case QSP_TYPE_TUPLE:
            return qspTupleToNum(QSP_TUPLE(tuple.Vals[0]), isValid);
        case QSP_TYPE_NUM:
            if (isValid) *isValid = QSP_TRUE;
            return QSP_NUM(tuple.Vals[0]);
        case QSP_TYPE_STR:
            return qspStrToNum(QSP_STR(tuple.Vals[0]), isValid);
        }
        break;
    }

    if (isValid) *isValid = QSP_FALSE;
    return 0;
}

QSPTuple qspCopyToNewTuple(QSPVariant *values, int count)
{
    QSPTuple tuple;
    if (count)
    {
        QSPVariant *newItem, *srcItem;
        tuple.Vals = (QSPVariant *)malloc(count * sizeof(QSPVariant));
        tuple.Items = count;
        newItem = tuple.Vals;
        for (srcItem = values; count > 0; --count, ++srcItem, ++newItem)
            qspCopyToNewVariant(newItem, srcItem);
    }
    else
    {
        tuple.Vals = 0;
        tuple.Items = 0;
    }
    return tuple;
}

QSPTuple qspMoveToNewTuple(QSPVariant *values, int count)
{
    QSPTuple tuple;
    if (count)
    {
        QSPVariant *newItem, *srcItem;
        tuple.Vals = (QSPVariant *)malloc(count * sizeof(QSPVariant));
        tuple.Items = count;
        newItem = tuple.Vals;
        for (srcItem = values; count > 0; --count, ++srcItem, ++newItem)
            qspMoveToNewVariant(newItem, srcItem);
    }
    else
    {
        tuple.Vals = 0;
        tuple.Items = 0;
    }
    return tuple;
}

QSPTuple qspMergeToNewTuple(QSPVariant *list1, int count1, QSPVariant *list2, int count2)
{
    QSPTuple tuple;
    int newCount = count1 + count2;
    if (newCount)
    {
        QSPVariant *newItem, *item;
        tuple.Vals = (QSPVariant *)malloc(newCount * sizeof(QSPVariant));
        tuple.Items = newCount;
        newItem = tuple.Vals;
        for (item = list1; count1 > 0; --count1, ++item, ++newItem)
            qspMoveToNewVariant(newItem, item);
        for (item = list2; count2 > 0; --count2, ++item, ++newItem)
            qspMoveToNewVariant(newItem, item);
    }
    else
    {
        tuple.Vals = 0;
        tuple.Items = 0;
    }
    return tuple;
}

int qspTupleValueCompare(QSPTuple tuple, QSPVariant *value)
{
    switch (QSP_BASETYPE(value->Type))
    {
    case QSP_TYPE_TUPLE:
        return qspTuplesCompare(tuple, QSP_PTUPLE(value));
    case QSP_TYPE_NUM:
    case QSP_TYPE_STR:
        {
            int delta;
            if (tuple.Items == 0)
                return -1; /* an empty tuple is always smaller than any value */
            delta = qspVariantsCompare(tuple.Vals, value); /* compare the first tuple item with the value */
            if (delta) return delta;
            if (tuple.Items > 1)
                return 1; /* the tuple is bigger if it contains more than 1 item */
            break;
        }
    }
    return 0;
}

int qspTuplesCompare(QSPTuple first, QSPTuple second)
{
    int delta;
    QSPVariant *pos1 = first.Vals, *pos2 = second.Vals;
    QSPVariant *end1 = first.Vals + first.Items, *end2 = second.Vals + second.Items;
    while (pos2 < end2 && pos1 < end1)
    {
        delta = qspVariantsCompare(pos1, pos2);
        if (delta) return delta;
        ++pos1, ++pos2;
    }
    return (pos1 == end1) ? ((pos2 == end2) ? 0 : -1) : 1;
}

void qspAppendTupleToString(QSPTuple tuple, QSPBufString *res)
{
    QSP_CHAR buf[QSP_MAX_BIGINT_LEN];
    QSPString temp;
    QSPVariant *item = tuple.Vals, *itemsEnd = item + tuple.Items;
    qspAddBufText(res, QSP_STATIC_STR(QSP_TUPLEDISPLAY_START));
    while (item < itemsEnd)
    {
        switch (QSP_BASETYPE(item->Type))
        {
        case QSP_TYPE_TUPLE:
            qspAppendTupleToString(QSP_PTUPLE(item), res);
            break;
        case QSP_TYPE_NUM:
            qspAddBufText(res, qspNumToStr(buf, QSP_PNUM(item)));
            break;
        case QSP_TYPE_STR:
            qspAddBufText(res, QSP_STATIC_STR(QSP_DEFQUOT));
            temp = qspReplaceText(QSP_PSTR(item), QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
            qspAddBufText(res, temp);
            qspFreeNewString(&temp, &QSP_PSTR(item));
            qspAddBufText(res, QSP_STATIC_STR(QSP_DEFQUOT));
            break;
        }
        if (++item == itemsEnd) break;
        qspAddBufText(res, QSP_STATIC_STR(QSP_TUPLEDISPLAY_DELIM));
    }
    qspAddBufText(res, QSP_STATIC_STR(QSP_TUPLEDISPLAY_END));
}
