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
#include "errors.h"
#include "text.h"
#include "tuples.h"

static QSP_BOOL qspTypeConversionTable[QSP_TYPE_DEFINED_TYPES][QSP_TYPE_DEFINED_TYPES] =
{
    /*             TUPLE     NUMBER    STRING     CODE      VARREF */
    /* TUPLE */  { QSP_TRUE, QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE },
    /* NUMBER */ { QSP_TRUE, QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE },
    /* STRING */ { QSP_TRUE, QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE },
    /* CODE */   { QSP_TRUE, QSP_TRUE, QSP_FALSE, QSP_TRUE, QSP_TRUE },
    /* VARREF */ { QSP_TRUE, QSP_TRUE, QSP_FALSE, QSP_TRUE, QSP_TRUE },
};

INLINE void qspFormatVariant(QSPVariant *val);
INLINE QSP_BOOL qspSumSimpleVariants(QSPVariant *arg1, QSPVariant *arg2, QSPVariant *res);

INLINE void qspFormatVariant(QSPVariant *val)
{
    switch (val->Type)
    {
        case QSP_TYPE_VARREF:
        {
            QSPString temp = qspGetNewText(qspDelSpc(QSP_PSTR(val)));
            qspUpperStr(&temp);
            qspFreeString(QSP_PSTR(val));
            QSP_PSTR(val) = temp;
            break;
        }
    }
}

INLINE QSP_BOOL qspSumSimpleVariants(QSPVariant *arg1, QSPVariant *arg2, QSPVariant *res)
{
    switch (QSP_BASETYPE(arg1->Type))
    {
    case QSP_TYPE_TUPLE:
        return QSP_FALSE;
    case QSP_TYPE_STR:
        switch (QSP_BASETYPE(arg2->Type))
        {
        case QSP_TYPE_TUPLE:
            return QSP_FALSE;
        case QSP_TYPE_STR:
            qspAddText(&QSP_PSTR(res), QSP_PSTR(arg1), QSP_TRUE);
            qspAddText(&QSP_PSTR(res), QSP_PSTR(arg2), QSP_FALSE);
            res->Type = QSP_TYPE_STR;
            break;
        case QSP_TYPE_NUM:
            if (qspCanConvertToNum(arg1))
            {
                qspConvertVariantTo(arg1, QSP_TYPE_NUM);
                QSP_PNUM(res) = QSP_PNUM(arg1) + QSP_PNUM(arg2);
                res->Type = QSP_TYPE_NUM;
            }
            else
            {
                qspConvertVariantTo(arg2, QSP_TYPE_STR);
                qspAddText(&QSP_PSTR(res), QSP_PSTR(arg1), QSP_TRUE);
                qspAddText(&QSP_PSTR(res), QSP_PSTR(arg2), QSP_FALSE);
                res->Type = QSP_TYPE_STR;
            }
            break;
        }
        break;
    case QSP_TYPE_NUM:
        switch (QSP_BASETYPE(arg2->Type))
        {
        case QSP_TYPE_TUPLE:
            return QSP_FALSE;
        case QSP_TYPE_STR:
            if (qspCanConvertToNum(arg2))
            {
                qspConvertVariantTo(arg2, QSP_TYPE_NUM);
                QSP_PNUM(res) = QSP_PNUM(arg1) + QSP_PNUM(arg2);
                res->Type = QSP_TYPE_NUM;
            }
            else
            {
                qspConvertVariantTo(arg1, QSP_TYPE_STR);
                qspAddText(&QSP_PSTR(res), QSP_PSTR(arg1), QSP_TRUE);
                qspAddText(&QSP_PSTR(res), QSP_PSTR(arg2), QSP_FALSE);
                res->Type = QSP_TYPE_STR;
            }
            break;
        case QSP_TYPE_NUM:
            QSP_PNUM(res) = QSP_PNUM(arg1) + QSP_PNUM(arg2);
            res->Type = QSP_TYPE_NUM;
            break;
        }
        break;
    }
    return QSP_TRUE;
}

QSP_BOOL qspConvertVariantTo(QSPVariant *val, QSP_TINYINT type)
{
    if (val->Type != type && qspTypeConversionTable[val->Type][type])
    {
        QSP_TINYINT fromBaseType = QSP_BASETYPE(val->Type), toBaseType = QSP_BASETYPE(type);
        switch (fromBaseType)
        {
        case QSP_TYPE_TUPLE:
            switch (toBaseType)
            {
                case QSP_TYPE_NUM:
                {
                    QSP_BOOL isValid;
                    int num = qspTupleToNum(QSP_PTUPLE(val), &isValid);
                    if (!isValid) return QSP_FALSE;
                    qspFreeTuple(QSP_PTUPLE(val));
                    QSP_PNUM(val) = num;
                    break;
                }
                case QSP_TYPE_STR:
                {
                    QSPString str = qspTupleToDisplayString(QSP_PTUPLE(val));
                    qspFreeTuple(QSP_PTUPLE(val));
                    QSP_PSTR(val) = str;
                    break;
                }
            }
            break;
        case QSP_TYPE_NUM:
            switch (toBaseType)
            {
                case QSP_TYPE_TUPLE:
                    QSP_PTUPLE(val) = qspGetNewTuple(val, 1);
                    break;
                case QSP_TYPE_STR:
                {
                    QSP_CHAR buf[QSP_NUMTOSTRBUF];
                    QSP_PSTR(val) = qspGetNewText(qspNumToStr(buf, QSP_PNUM(val)));
                    break;
                }
            }
            break;
        case QSP_TYPE_STR:
            switch (toBaseType)
            {
                case QSP_TYPE_TUPLE:
                {
                    QSPTuple tuple = qspGetNewTuple(val, 1);
                    qspFreeString(QSP_PSTR(val));
                    QSP_PTUPLE(val) = tuple;
                    break;
                }
                case QSP_TYPE_NUM:
                {
                    QSP_BOOL isValid;
                    int num = qspStrToNum(QSP_PSTR(val), &isValid);
                    if (!isValid) return QSP_FALSE;
                    qspFreeString(QSP_PSTR(val));
                    QSP_PNUM(val) = num;
                    break;
                }
            }
            break;
        }
        val->Type = type;
        qspFormatVariant(val);
    }
    return QSP_TRUE;
}

int qspAutoConvertCompare(QSPVariant *v1, QSPVariant *v2)
{
    int res;
    QSP_TINYINT firstBaseType = QSP_BASETYPE(v1->Type), secondBaseType = QSP_BASETYPE(v2->Type);
    if (firstBaseType != secondBaseType)
    {
        switch (firstBaseType)
        {
        case QSP_TYPE_TUPLE:
            switch (secondBaseType)
            {
            case QSP_TYPE_NUM:
                if (qspCanConvertToNum(v1))
                    qspConvertVariantTo(v1, v2->Type);
                else
                {
                    qspConvertVariantTo(v1, QSP_TYPE_STR);
                    qspConvertVariantTo(v2, QSP_TYPE_STR);
                }
                break;
            case QSP_TYPE_STR:
                qspConvertVariantTo(v1, v2->Type);
                break;
            }
            break;
        case QSP_TYPE_NUM:
            switch (secondBaseType)
            {
            case QSP_TYPE_TUPLE:
                if (qspCanConvertToNum(v2))
                    qspConvertVariantTo(v2, v1->Type);
                else
                {
                    qspConvertVariantTo(v1, QSP_TYPE_STR);
                    qspConvertVariantTo(v2, QSP_TYPE_STR);
                }
                break;
            case QSP_TYPE_STR:
                if (qspCanConvertToNum(v2))
                    qspConvertVariantTo(v2, v1->Type);
                else
                    qspConvertVariantTo(v1, v2->Type);
                break;
            }
            break;
        case QSP_TYPE_STR:
            switch (secondBaseType)
            {
            case QSP_TYPE_TUPLE:
                qspConvertVariantTo(v2, v1->Type);
                break;
            case QSP_TYPE_NUM:
                if (qspCanConvertToNum(v1))
                    qspConvertVariantTo(v1, v2->Type);
                else
                    qspConvertVariantTo(v2, v1->Type);
                break;
            }
            break;
        }
    }
    switch (QSP_BASETYPE(v1->Type))
    {
    case QSP_TYPE_TUPLE:
        res = qspTuplesComp(QSP_PTUPLE(v1), QSP_PTUPLE(v2));
        break;
    case QSP_TYPE_NUM:
        res = (QSP_PNUM(v1) > QSP_PNUM(v2) ? 1 : (QSP_PNUM(v1) < QSP_PNUM(v2) ? -1 : 0));
        break;
    case QSP_TYPE_STR:
        res = qspStrsComp(QSP_PSTR(v1), QSP_PSTR(v2));
        break;
    }
    return res;
}

void qspUpdateVariantValue(QSPVariant *dest, QSPVariant *src)
{
    switch (QSP_BASETYPE(dest->Type))
    {
    case QSP_TYPE_TUPLE:
        qspFreeTuple(QSP_PTUPLE(dest));
        break;
    case QSP_TYPE_STR:
        qspFreeString(QSP_PSTR(dest));
        break;
    }
    qspCopyToNewVariant(dest, src);
}

void qspAutoConvertAppend(QSPVariant *arg1, QSPVariant *arg2, QSPVariant *res)
{
    switch (QSP_BASETYPE(arg1->Type))
    {
    case QSP_TYPE_TUPLE:
        switch (QSP_BASETYPE(arg2->Type))
        {
        case QSP_TYPE_TUPLE:
            QSP_PTUPLE(res) = qspMergeToTuple(QSP_PTUPLE(arg1).Vals, QSP_PTUPLE(arg1).Items, QSP_PTUPLE(arg2).Vals, QSP_PTUPLE(arg2).Items);
            res->Type = QSP_TYPE_TUPLE;
            break;
        case QSP_TYPE_NUM:
        case QSP_TYPE_STR:
            QSP_PTUPLE(res) = qspMergeToTuple(QSP_PTUPLE(arg1).Vals, QSP_PTUPLE(arg1).Items, arg2, 1);
            res->Type = QSP_TYPE_TUPLE;
            break;
        }
        break;
    case QSP_TYPE_NUM:
    case QSP_TYPE_STR:
        switch (QSP_BASETYPE(arg2->Type))
        {
        case QSP_TYPE_TUPLE:
            QSP_PTUPLE(res) = qspMergeToTuple(arg1, 1, QSP_PTUPLE(arg2).Vals, QSP_PTUPLE(arg2).Items);
            res->Type = QSP_TYPE_TUPLE;
            break;
        case QSP_TYPE_NUM:
        case QSP_TYPE_STR:
            qspConvertVariantTo(arg1, QSP_TYPE_STR);
            qspConvertVariantTo(arg2, QSP_TYPE_STR);

            qspAddText(&QSP_PSTR(res), QSP_PSTR(arg1), QSP_TRUE);
            qspAddText(&QSP_PSTR(res), QSP_PSTR(arg2), QSP_FALSE);
            res->Type = QSP_TYPE_STR;
            break;
        }
        break;
    }
}

QSP_BOOL qspAutoConvertCombine(QSPVariant *arg1, QSPVariant *arg2, QSP_CHAR op, QSPVariant *res)
{
    if (QSP_BASETYPE(arg1->Type) == QSP_TYPE_TUPLE)
    {
        int i;
        QSPTuple *tuple = &QSP_PTUPLE(arg1);
        QSPVariant *vals = (QSPVariant *)malloc(tuple->Items * sizeof(QSPVariant));
        for (i = 0; i < tuple->Items; ++i)
        {
            if (!qspAutoConvertCombine(tuple->Vals + i, arg2, op, vals + i))
            {
                qspFreeVariants(vals, i);
                free(vals);
                return QSP_FALSE;
            }
        }
        QSP_PTUPLE(res).Vals = vals;
        QSP_PTUPLE(res).Items = tuple->Items;
        res->Type = QSP_TYPE_TUPLE;
        return QSP_TRUE;
    }
    if (QSP_BASETYPE(arg2->Type) == QSP_TYPE_TUPLE)
    {
        int i;
        QSPTuple *tuple = &QSP_PTUPLE(arg2);
        QSPVariant *vals = (QSPVariant *)malloc(tuple->Items * sizeof(QSPVariant));
        for (i = 0; i < tuple->Items; ++i)
        {
            if (!qspAutoConvertCombine(arg1, tuple->Vals + i, op, vals + i))
            {
                qspFreeVariants(vals, i);
                free(vals);
                return QSP_FALSE;
            }
        }
        QSP_PTUPLE(res).Vals = vals;
        QSP_PTUPLE(res).Items = tuple->Items;
        res->Type = QSP_TYPE_TUPLE;
        return QSP_TRUE;
    }

    if (op == QSP_ADD[0])
        return qspSumSimpleVariants(arg1, arg2, res);

    if (!qspConvertVariantTo(arg1, QSP_TYPE_NUM) || !qspConvertVariantTo(arg2, QSP_TYPE_NUM))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return QSP_FALSE;
    }

    if (op == QSP_SUB[0])
    {
        QSP_PNUM(res) = QSP_PNUM(arg1) - QSP_PNUM(arg2);
    }
    else if (op == QSP_MUL[0])
    {
        QSP_PNUM(res) = QSP_PNUM(arg1) * QSP_PNUM(arg2);
    }
    else /* QSP_DIV */
    {
        if (QSP_PNUM(arg2) == 0)
        {
            qspSetError(QSP_ERR_DIVBYZERO);
            return QSP_FALSE;
        }
        QSP_PNUM(res) = QSP_PNUM(arg1) / QSP_PNUM(arg2);
    }
    res->Type = QSP_TYPE_NUM;
    return QSP_TRUE;
}
