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

#include "variant.h"
#include "text.h"

INLINE void qspFormatVariant(QSPVariant *val);

INLINE void qspFormatVariant(QSPVariant *val)
{
    QSPString temp;
    switch (val->Type)
    {
        case QSP_TYPE_VARREF:
            temp = qspGetNewText(qspDelSpc(QSP_PSTR(val)));
            qspUpperStr(&temp);
            qspFreeString(QSP_PSTR(val));
            QSP_PSTR(val) = temp;
            break;
    }
}

QSP_BOOL qspConvertVariantTo(QSPVariant *val, QSP_TINYINT type)
{
    int num;
    QSP_CHAR buf[12];
    QSP_BOOL isValid;
    if (val->Type != type)
    {
        if (QSP_ISNUM(type))
        {
            if (QSP_ISSTR(val->Type))
            {
                num = qspStrToNum(QSP_PSTR(val), &isValid);
                if (!isValid) return QSP_FALSE;
                qspFreeString(QSP_PSTR(val));
                QSP_PNUM(val) = num;
            }
        }
        else
        {
            if (QSP_ISNUM(val->Type))
            {
                QSP_PSTR(val) = qspGetNewText(qspNumToStr(buf, QSP_PNUM(val)));
            }
        }
        val->Type = type;
        qspFormatVariant(val);
    }
    return QSP_TRUE;
}

int qspAutoConvertCompare(QSPVariant *v1, QSPVariant *v2)
{
    int res;
    if (QSP_BASETYPE(v1->Type) != QSP_BASETYPE(v2->Type))
    {
        if (QSP_ISSTR(v2->Type))
        {
            if (qspIsCanConvertToNum(v2))
                qspConvertVariantTo(v2, v1->Type);
            else
                qspConvertVariantTo(v1, v2->Type);
        }
        else
        {
            if (qspIsCanConvertToNum(v1))
                qspConvertVariantTo(v1, v2->Type);
            else
                qspConvertVariantTo(v2, v1->Type);
        }
    }
    if (QSP_ISSTR(v1->Type))
        res = qspStrsComp(QSP_PSTR(v1), QSP_PSTR(v2));
    else
        res = (QSP_PNUM(v1) > QSP_PNUM(v2) ? 1 : (QSP_PNUM(v1) < QSP_PNUM(v2) ? -1 : 0));
    return res;
}

void qspUpdateVariantValue(QSPVariant *dest, QSPVariant *src)
{
    if (QSP_BASETYPE(src->Type) != QSP_BASETYPE(dest->Type))
    {
        if (QSP_ISSTR(dest->Type = src->Type))
            QSP_PSTR(dest) = qspGetNewText(QSP_PSTR(src));
        else
        {
            qspFreeString(QSP_PSTR(dest));
            QSP_PNUM(dest) = QSP_PNUM(src);
        }
    }
    else
    {
        if (QSP_ISSTR(dest->Type = src->Type))
            qspUpdateText(&QSP_PSTR(dest), QSP_PSTR(src));
        else
            QSP_PNUM(dest) = QSP_PNUM(src);
    }
}
