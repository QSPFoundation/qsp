/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
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

    typedef struct
    {
        union
        {
            QSPString Str;
            int Num;
        } Val;
        QSP_BOOL IsStr;
    } QSPVariant;

    /* External functions */
    QSP_BOOL qspConvertVariantTo(QSPVariant *val, QSP_BOOL isToString);
    int qspAutoConvertCompare(QSPVariant *, QSPVariant *);

    INLINE void qspFreeVariants(QSPVariant *args, int count)
    {
        while (--count >= 0)
            if (args[count].IsStr) qspFreeString(QSP_STR(args[count]));
    }

    INLINE QSPVariant qspGetEmptyVariant(QSP_BOOL isStringType)
    {
        QSPVariant ret;
        if (ret.IsStr = isStringType)
            QSP_STR(ret) = qspNewEmptyString();
        else
            QSP_NUM(ret) = 0;
        return ret;
    }

    INLINE void qspCopyVariant(QSPVariant *dest, QSPVariant *src)
    {
        if (dest->IsStr = src->IsStr)
            QSP_PSTR(dest) = qspGetNewText(QSP_PSTR(src));
        else
            QSP_PNUM(dest) = QSP_PNUM(src);
    }

    INLINE QSP_BOOL qspIsCanConvertToNum(QSPVariant *val)
    {
        QSP_BOOL isValid;
        if (val->IsStr)
        {
            qspStrToNum(QSP_PSTR(val), &isValid);
            if (!isValid) return QSP_FALSE;
        }
        return QSP_TRUE;
    }

#endif
