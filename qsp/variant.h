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

#ifndef QSP_VARIANTDEFINES
    #define QSP_VARIANTDEFINES

    #define QSP_STR(a) (a).Val.Str
    #define QSP_NUM(a) (a).Val.Num
    #define QSP_PSTR(a) (a)->Val.Str
    #define QSP_PNUM(a) (a)->Val.Num

    enum
    {
        QSP_TYPE_NUMBER = 0,
        QSP_TYPE_STRING = 1,
        QSP_TYPE_CODE = 2,
        QSP_TYPE_TUPLE = 3,
        QSP_TYPE_VARREF = 4,
        QSP_TYPE_DEFINED_TYPES = 5, /* represents a number of defined values */
        QSP_TYPE_UNDEFINED = 64, /* not used for values, it has to be a string-based type */
    };

    #define QSP_ISDEF(a) ((a) != QSP_TYPE_UNDEFINED)
    #define QSP_ISNUM(a) ((a) == QSP_TYPE_NUMBER)
    #define QSP_ISSTR(a) ((a) > QSP_TYPE_NUMBER)
    #define QSP_BASETYPE(a) ((a) > QSP_TYPE_NUMBER) /* QSP_TYPE_STRING | QSP_TYPE_NUMBER */

    typedef struct
    {
        union
        {
            QSPString Str;
            int Num;
        } Val;
        QSP_TINYINT Type;
    } QSPVariant;

    /* External functions */
    QSP_BOOL qspConvertVariantTo(QSPVariant *val, QSP_TINYINT type);
    int qspAutoConvertCompare(QSPVariant *v1, QSPVariant *v2);
    void qspUpdateVariantValue(QSPVariant *dest, QSPVariant *src);

    INLINE void qspFreeVariants(QSPVariant *args, int count)
    {
        while (--count >= 0)
            if (QSP_ISSTR(args[count].Type)) qspFreeString(QSP_STR(args[count]));
    }

    INLINE void qspInitVariant(QSPVariant *value, QSP_TINYINT type)
    {
        if (QSP_ISSTR(value->Type = type))
            QSP_PSTR(value) = qspNullString;
        else
            QSP_PNUM(value) = 0;
    }

    INLINE QSPVariant qspGetEmptyVariant(QSP_TINYINT type)
    {
        QSPVariant ret;
        qspInitVariant(&ret, type);
        return ret;
    }

    INLINE void qspCopyToNewVariant(QSPVariant *dest, QSPVariant *src)
    {
        if (QSP_ISSTR(dest->Type = src->Type))
            QSP_PSTR(dest) = qspGetNewText(QSP_PSTR(src));
        else
            QSP_PNUM(dest) = QSP_PNUM(src);
    }

    INLINE QSP_BOOL qspCanConvertToNum(QSPVariant *val)
    {
        QSP_BOOL isValid;
        if (QSP_ISSTR(val->Type))
        {
            qspStrToNum(QSP_PSTR(val), &isValid);
            if (!isValid) return QSP_FALSE;
        }
        return QSP_TRUE;
    }

#endif
