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
            QSPString temp = qspCopyToNewText(qspDelSpc(QSP_PSTR(val)));
            qspUpperStr(&temp);
            qspFreeString(&QSP_PSTR(val));
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

QSPString qspGetVariantAsString(QSPVariant *val)
{
    switch (QSP_BASETYPE(val->Type))
    {
        case QSP_TYPE_TUPLE:
            return qspGetTupleAsString(QSP_PTUPLE(val));
        case QSP_TYPE_NUM:
        {
            QSP_CHAR buf[QSP_NUMTOSTRBUF];
            return qspCopyToNewText(qspNumToStr(buf, QSP_PNUM(val)));
        }
        case QSP_TYPE_STR:
            return qspCopyToNewText(QSP_PSTR(val));
    }
    return qspNullString;
}

int qspGetVariantAsNum(QSPVariant *val, QSP_BOOL *isValid)
{
    switch (QSP_BASETYPE(val->Type))
    {
    case QSP_TYPE_TUPLE:
        return qspTupleToNum(QSP_PTUPLE(val), isValid);
    case QSP_TYPE_NUM:
        if (isValid) *isValid = QSP_TRUE;
        return QSP_PNUM(val);
    case QSP_TYPE_STR:
        return qspStrToNum(QSP_PSTR(val), isValid);
    }
    if (isValid) *isValid = QSP_FALSE; /* shouldn't happen */
    return 0;
}

QSP_BOOL qspConvertVariantTo(QSPVariant *val, QSP_TINYINT type)
{
    if (val->Type != type && qspTypeConversionTable[val->Type][type])
    {
        QSP_TINYINT toBaseType = QSP_BASETYPE(type);
        if (QSP_BASETYPE(val->Type) != toBaseType)
        {
            switch (toBaseType)
            {
                case QSP_TYPE_TUPLE:
                    QSP_PTUPLE(val) = qspMoveToNewTuple(val, 1);
                    break;
                case QSP_TYPE_NUM:
                {
                    QSP_BOOL isValid;
                    int result = qspGetVariantAsNum(val, &isValid);
                    if (!isValid) return QSP_FALSE;
                    qspFreeVariant(val);
                    QSP_PNUM(val) = result;
                    break;
                }
                case QSP_TYPE_STR:
                {
                    QSPString result = qspGetVariantAsString(val);
                    qspFreeVariant(val);
                    QSP_PSTR(val) = result;
                    break;
                }
            }
        }
        val->Type = type;
        qspFormatVariant(val);
    }
    return QSP_TRUE;
}

int qspAutoConvertCompare(QSPVariant *v1, QSPVariant *v2)
{
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
        return qspTuplesComp(QSP_PTUPLE(v1), QSP_PTUPLE(v2));
    case QSP_TYPE_NUM:
        return (QSP_PNUM(v1) > QSP_PNUM(v2) ? 1 : (QSP_PNUM(v1) < QSP_PNUM(v2) ? -1 : 0));
    case QSP_TYPE_STR:
        return qspStrsComp(QSP_PSTR(v1), QSP_PSTR(v2));
    }
    return 0;
}

void qspAutoConvertAppend(QSPVariant *arg1, QSPVariant *arg2, QSPVariant *res)
{
    switch (QSP_BASETYPE(arg1->Type))
    {
    case QSP_TYPE_TUPLE:
        switch (QSP_BASETYPE(arg2->Type))
        {
        case QSP_TYPE_TUPLE:
            QSP_PTUPLE(res) = qspMergeToNewTuple(QSP_PTUPLE(arg1).Vals, QSP_PTUPLE(arg1).Items, QSP_PTUPLE(arg2).Vals, QSP_PTUPLE(arg2).Items);
            res->Type = QSP_TYPE_TUPLE;
            break;
        case QSP_TYPE_NUM:
        case QSP_TYPE_STR:
            QSP_PTUPLE(res) = qspMergeToNewTuple(QSP_PTUPLE(arg1).Vals, QSP_PTUPLE(arg1).Items, arg2, 1);
            res->Type = QSP_TYPE_TUPLE;
            break;
        }
        break;
    case QSP_TYPE_NUM:
    case QSP_TYPE_STR:
        switch (QSP_BASETYPE(arg2->Type))
        {
        case QSP_TYPE_TUPLE:
            QSP_PTUPLE(res) = qspMergeToNewTuple(arg1, 1, QSP_PTUPLE(arg2).Vals, QSP_PTUPLE(arg2).Items);
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
        QSPTuple *tuple = &QSP_PTUPLE(arg1);
        if (tuple->Vals)
        {
            int i;
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
        }
        else
        {
            QSP_PTUPLE(res).Vals = 0;
            QSP_PTUPLE(res).Items = 0;
        }
        res->Type = QSP_TYPE_TUPLE;
        return QSP_TRUE;
    }
    if (QSP_BASETYPE(arg2->Type) == QSP_TYPE_TUPLE)
    {
        QSPTuple *tuple = &QSP_PTUPLE(arg2);
        if (tuple->Vals)
        {
            int i;
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
        }
        else
        {
            QSP_PTUPLE(res).Vals = 0;
            QSP_PTUPLE(res).Items = 0;
        }
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

void qspAppendVariantToIndexString(QSPBufString *res, QSPVariant *val)
{
    QSP_CHAR buf[QSP_NUMTOSTRBUF];
    switch (QSP_BASETYPE(val->Type))
    {
        case QSP_TYPE_TUPLE:
        {
            int items = QSP_PTUPLE(val).Items;
            qspAddBufText(res, qspNumToStr(buf, items));
            qspAddBufText(res, QSP_STATIC_STR(QSP_IND_DELIM));
            if (items > 0)
            {
                QSPVariant *item = QSP_PTUPLE(val).Vals;
                while (--items > 0)
                {
                    qspAppendVariantToIndexString(res, item);
                    qspAddBufText(res, QSP_STATIC_STR(QSP_IND_DELIM));
                    ++item;
                }
                qspAppendVariantToIndexString(res, item);
            }
            break;
        }
        case QSP_TYPE_NUM:
            qspAddBufText(res, QSP_STATIC_STR(QSP_IND_NUMID)); /* type id */
            qspAddBufText(res, qspNumToStr(buf, QSP_PNUM(val)));
            break;
        case QSP_TYPE_STR:
            qspAddBufText(res, QSP_STATIC_STR(QSP_IND_STRID)); /* type id */
            qspAddBufText(res, QSP_PSTR(val));
            break;
    }
}
