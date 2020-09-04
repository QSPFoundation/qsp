/* Copyright (C) 2001-2020 Valeriy Argunov (val AT time DOT guru) */
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

QSP_BOOL qspConvertVariantTo(QSPVariant *val, QSP_BOOL isToString)
{
    int num;
    QSP_CHAR buf[12];
    QSP_BOOL isValid;
    if (val->IsStr)
    {
        if (!isToString)
        {
            num = qspStrToNum(QSP_PSTR(val), &isValid);
            if (!isValid) return QSP_TRUE;
            qspFreeString(QSP_PSTR(val));
            QSP_PNUM(val) = num;
            val->IsStr = QSP_FALSE;
        }
    }
    else if (isToString)
    {
        QSP_PSTR(val) = qspGetNewText(qspNumToStr(buf, QSP_PNUM(val)));
        val->IsStr = QSP_TRUE;
    }
    return QSP_FALSE;
}

int qspAutoConvertCompare(QSPVariant *v1, QSPVariant *v2)
{
    int res;
    if (v1->IsStr != v2->IsStr)
    {
        if (v2->IsStr)
        {
            if (qspIsCanConvertToNum(v2))
                qspConvertVariantTo(v2, QSP_FALSE);
            else
                qspConvertVariantTo(v1, QSP_TRUE);
        }
        else
        {
            if (qspIsCanConvertToNum(v1))
                qspConvertVariantTo(v1, QSP_FALSE);
            else
                qspConvertVariantTo(v2, QSP_TRUE);
        }
    }
    if (v1->IsStr)
        res = qspStrsComp(QSP_PSTR(v1), QSP_PSTR(v2));
    else
        res = (QSP_PNUM(v1) > QSP_PNUM(v2) ? 1 : (QSP_PNUM(v1) < QSP_PNUM(v2) ? -1 : 0));
    return res;
}
