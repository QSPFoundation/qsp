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
#include "tuples.h"

#ifndef QSP_VARIANTDEFINES
    #define QSP_VARIANTDEFINES

    /* External functions */
    QSP_BOOL qspConvertVariantTo(QSPVariant *val, QSP_TINYINT type);
    int qspAutoConvertCompare(QSPVariant *v1, QSPVariant *v2);
    void qspUpdateVariantValue(QSPVariant *dest, QSPVariant *src);
    QSP_BOOL qspAutoConvertCombine(QSPVariant *arg1, QSPVariant *arg2, QSP_CHAR op, QSPVariant *res);

    INLINE void qspFreeVariants(QSPVariant *args, int count)
    {
        while (--count >= 0)
        {
            switch (QSP_BASETYPE(args[count].Type))
            {
                case QSP_TYPE_TUPLE:
                    qspFreeTuple(QSP_TUPLE(args[count]));
                    break;
                case QSP_TYPE_STR:
                    qspFreeString(QSP_STR(args[count]));
                    break;
            }
        }
    }

    INLINE void qspInitVariant(QSPVariant *value, QSP_TINYINT type)
    {
        switch (QSP_BASETYPE(value->Type = type))
        {
            case QSP_TYPE_TUPLE:
                QSP_PTUPLE(value) = qspNullTuple;
                break;
            case QSP_TYPE_NUM:
                QSP_PNUM(value) = 0;
                break;
            case QSP_TYPE_STR:
                QSP_PSTR(value) = qspNullString;
                break;
        }
    }

    INLINE QSPVariant qspGetEmptyVariant(QSP_TINYINT type)
    {
        QSPVariant ret;
        qspInitVariant(&ret, type);
        return ret;
    }

    INLINE void qspCopyToNewVariant(QSPVariant *dest, QSPVariant *src)
    {
        switch (QSP_BASETYPE(dest->Type = src->Type))
        {
            case QSP_TYPE_TUPLE:
                QSP_PTUPLE(dest) = qspGetNewTuple(QSP_PTUPLE(src).Vals, QSP_PTUPLE(src).Items);
                break;
            case QSP_TYPE_NUM:
                QSP_PNUM(dest) = QSP_PNUM(src);
                break;
            case QSP_TYPE_STR:
                QSP_PSTR(dest) = qspGetNewText(QSP_PSTR(src));
                break;
        }
    }

    INLINE QSP_BOOL qspCanConvertToNum(QSPVariant *val)
    {
        switch (QSP_BASETYPE(val->Type))
        {
            case QSP_TYPE_TUPLE:
            {
                QSP_BOOL isValid;
                qspTupleToNum(QSP_PTUPLE(val), &isValid);
                if (!isValid) return QSP_FALSE;
                break;
            }
            case QSP_TYPE_STR:
            {
                QSP_BOOL isValid;
                qspStrToNum(QSP_PSTR(val), &isValid);
                if (!isValid) return QSP_FALSE;
                break;
            }
        }

        return QSP_TRUE;
    }

#endif
