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

#include "tuples.h"
#include "text.h"
#include "variant.h"

QSPTuple qspNullTuple;

void qspFreeTuple(QSPTuple tuple)
{
    if (tuple.Vals)
    {
        int count = tuple.Items;
        while (--count >= 0)
        {
            switch (QSP_BASETYPE(tuple.Vals[count].Type))
            {
                case QSP_TYPE_TUPLE:
                    qspFreeTuple(QSP_TUPLE(tuple.Vals[count]));
                    break;
                case QSP_TYPE_STR:
                    qspFreeString(QSP_STR(tuple.Vals[count]));
                    break;
            }
        }
        free(tuple.Vals);
    }
}

int qspTupleToNum(QSPTuple tuple, QSP_BOOL *isValid)
{
    if (tuple.Items == 1)
    {
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
    }

    if (isValid) *isValid = QSP_FALSE;
    return 0;
}

QSPString qspTupleToStr(QSPTuple tuple)
{
    if (tuple.Items == 1)
    {
        switch (QSP_BASETYPE(tuple.Vals[0].Type))
        {
            case QSP_TYPE_TUPLE:
                return qspTupleToStr(QSP_TUPLE(tuple.Vals[0]));
            case QSP_TYPE_NUM:
            {
                QSP_CHAR buf[QSP_NUMTOSTRBUF];
                return qspGetNewText(qspNumToStr(buf, QSP_NUM(tuple.Vals[0])));
            }
            case QSP_TYPE_STR:
                return qspGetNewText(QSP_STR(tuple.Vals[0]));
        }
    }

    return qspNullString;
}

QSPTuple qspGetNewTuple(QSPVariant *values, int count)
{
    QSPTuple tuple;

    tuple.Vals = (QSPVariant *)malloc(count * sizeof(QSPVariant));
    tuple.Items = count;

    while (--count >= 0)
    {
        QSP_TINYINT type = values[count].Type;
        switch (QSP_BASETYPE(type))
        {
            case QSP_TYPE_TUPLE:
                QSP_TUPLE(tuple.Vals[count]) = qspGetNewTuple(QSP_TUPLE(values[count]).Vals, QSP_TUPLE(values[count]).Items);
                break;
            case QSP_TYPE_NUM:
                QSP_NUM(tuple.Vals[count]) = QSP_NUM(values[count]);
                break;
            case QSP_TYPE_STR:
                QSP_STR(tuple.Vals[count]) = qspGetNewText(QSP_STR(values[count]));
                break;
        }
        tuple.Vals[count].Type = type;
    }

    return tuple;
}

int qspTuplesComp(QSPTuple first, QSPTuple second)
{
    QSP_BOOL isValid;
    QSPString str;
    QSP_CHAR buf[QSP_NUMTOSTRBUF];
    int num, delta = 0;
    QSPVariant *pos1 = first.Vals, *pos2 = second.Vals;
    QSPVariant *end1 = first.Vals + first.Items, *end2 = second.Vals + second.Items;
    while (pos2 < end2 && pos1 < end1)
    {
        switch (QSP_BASETYPE(pos1->Type))
        {
            case QSP_TYPE_TUPLE:
                switch (QSP_BASETYPE(pos2->Type))
                {
                    case QSP_TYPE_TUPLE:
                        delta = qspTuplesComp(QSP_PTUPLE(pos1), QSP_PTUPLE(pos2));
                        break;
                    case QSP_TYPE_NUM:
                        num = qspTupleToNum(QSP_PTUPLE(pos1), &isValid);
                        if (isValid)
                        {
                            delta = (num > QSP_PNUM(pos2) ? 1 : (num < QSP_PNUM(pos2) ? -1 : 0));
                        }
                        else
                        {
                            str = qspTupleToStr(QSP_PTUPLE(pos1));
                            delta = qspStrsComp(str, qspNumToStr(buf, QSP_PNUM(pos2)));
                            qspFreeString(str);
                        }
                        break;
                    case QSP_TYPE_STR:
                        str = qspTupleToStr(QSP_PTUPLE(pos1));
                        delta = qspStrsComp(str, QSP_PSTR(pos2));
                        qspFreeString(str);
                        break;
                }
                break;
            case QSP_TYPE_NUM:
                switch (QSP_BASETYPE(pos2->Type))
                {
                    case QSP_TYPE_TUPLE:
                        num = qspTupleToNum(QSP_PTUPLE(pos2), &isValid);
                        if (isValid)
                        {
                            delta = (QSP_PNUM(pos1) > num ? 1 : (QSP_PNUM(pos1) < num ? -1 : 0));
                        }
                        else
                        {
                            str = qspTupleToStr(QSP_PTUPLE(pos2));
                            delta = qspStrsComp(qspNumToStr(buf, QSP_PNUM(pos1)), str);
                            qspFreeString(str);
                        }
                        break;
                    case QSP_TYPE_NUM:
                        delta = (QSP_PNUM(pos1) > QSP_PNUM(pos2) ? 1 : (QSP_PNUM(pos1) < QSP_PNUM(pos2) ? -1 : 0));
                        break;
                    case QSP_TYPE_STR:
                        num = qspStrToNum(QSP_PSTR(pos2), &isValid);
                        if (isValid)
                        {
                            delta = (QSP_PNUM(pos1) > num ? 1 : (QSP_PNUM(pos1) < num ? -1 : 0));
                        }
                        else
                        {
                            delta = qspStrsComp(qspNumToStr(buf, QSP_PNUM(pos1)), QSP_PSTR(pos2));
                        }
                        break;
                }
                break;
            case QSP_TYPE_STR:
                switch (QSP_BASETYPE(pos2->Type))
                {
                    case QSP_TYPE_TUPLE:
                        str = qspTupleToStr(QSP_PTUPLE(pos2));
                        delta = qspStrsComp(QSP_PSTR(pos1), str);
                        qspFreeString(str);
                        break;
                    case QSP_TYPE_NUM:
                        num = qspStrToNum(QSP_PSTR(pos1), &isValid);
                        if (isValid)
                        {
                            delta = (num > QSP_PNUM(pos2) ? 1 : (num < QSP_PNUM(pos2) ? -1 : 0));
                        }
                        else
                        {
                            delta = qspStrsComp(QSP_PSTR(pos1), qspNumToStr(buf, QSP_PNUM(pos2)));
                        }
                        break;
                    case QSP_TYPE_STR:
                        delta = qspStrsComp(QSP_PSTR(pos1), QSP_PSTR(pos2));
                        break;
                }
                break;
        }
        if (delta) break;
        ++pos1, ++pos2;
    }
    if (delta) return delta;
    return (pos1 == end1) ? ((pos2 == end2) ? 0 : -1) : 1;
}

void qspExtendTuple(QSPTuple *tuple, QSPVariant *values, int count)
{
    QSPVariant *dest;
    int i, oldCount = tuple->Items;
    tuple->Items = oldCount + count;
    tuple->Vals = (QSPVariant *)realloc(tuple->Vals, tuple->Items * sizeof(QSPVariant));
    dest = tuple->Vals + oldCount;
    for (i = 0; i < count; ++i)
    {
        qspCopyToNewVariant(dest, values);
        ++dest;
        ++values;
    }
}
