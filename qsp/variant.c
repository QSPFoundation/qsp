/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "variant.h"
#include "errors.h"
#include "mathops.h"
#include "text.h"
#include "tuples.h"

static const QSP_BOOL qspTypeConversionTable[QSP_TYPE_DEFINED_TYPES][QSP_TYPE_DEFINED_TYPES] =
{
    /*             TUPLE     NUMBER     BOOL      STRING     CODE      VARREF */
    /* TUPLE */  { QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE },
    /* NUMBER */ { QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE },
    /* BOOL */   { QSP_TRUE, QSP_FALSE, QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE },
    /* STRING */ { QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE },
    /* CODE */   { QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_FALSE, QSP_TRUE, QSP_TRUE },
    /* VARREF */ { QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_FALSE, QSP_TRUE, QSP_TRUE },
    /* UNDEF */  { QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE,  QSP_TRUE, QSP_TRUE },
};

INLINE QSP_BOOL qspSumSimpleVariants(QSPVariant *arg1, QSPVariant *arg2, QSPVariant *res);
INLINE QSPString qspGetVariantAsVarRef(QSPVariant *val);
INLINE QSP_BOOL qspGetVariantAsBool(QSPVariant *val);

INLINE QSP_BOOL qspSumSimpleVariants(QSPVariant *arg1, QSPVariant *arg2, QSPVariant *res)
{
    switch (QSP_BASETYPE(arg1->Type))
    {
    case QSP_TYPE_TUPLE:
        return QSP_FALSE; /* tuples get processed before calling this function */
    case QSP_TYPE_STR:
        switch (QSP_BASETYPE(arg2->Type))
        {
        case QSP_TYPE_TUPLE:
            return QSP_FALSE; /* tuples get processed before calling this function */
        case QSP_TYPE_STR:
            QSP_PSTR(res) = qspConcatText(QSP_PSTR(arg1), QSP_PSTR(arg2));
            res->Type = QSP_TYPE_STR;
            break;
        case QSP_TYPE_NUM:
            if (qspCanConvertToNum(arg1))
            {
                qspConvertVariantTo(arg1, QSP_TYPE_NUM);
                *res = qspNumVariant(QSP_PNUM(arg1) + QSP_PNUM(arg2));
            }
            else
            {
                qspConvertVariantTo(arg2, QSP_TYPE_STR);
                QSP_PSTR(res) = qspConcatText(QSP_PSTR(arg1), QSP_PSTR(arg2));
                res->Type = QSP_TYPE_STR;
            }
            break;
        }
        break;
    case QSP_TYPE_NUM:
        switch (QSP_BASETYPE(arg2->Type))
        {
        case QSP_TYPE_TUPLE:
            return QSP_FALSE; /* tuples get processed before calling this function */
        case QSP_TYPE_STR:
            if (qspCanConvertToNum(arg2))
            {
                qspConvertVariantTo(arg2, QSP_TYPE_NUM);
                *res = qspNumVariant(QSP_PNUM(arg1) + QSP_PNUM(arg2));
            }
            else
            {
                qspConvertVariantTo(arg1, QSP_TYPE_STR);
                QSP_PSTR(res) = qspConcatText(QSP_PSTR(arg1), QSP_PSTR(arg2));
                res->Type = QSP_TYPE_STR;
            }
            break;
        case QSP_TYPE_NUM:
            *res = qspNumVariant(QSP_PNUM(arg1) + QSP_PNUM(arg2));
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
            QSP_CHAR buf[QSP_MAX_BIGINT_LEN];
            return qspCopyToNewText(qspNumToStr(buf, QSP_PNUM(val)));
        }
    case QSP_TYPE_STR:
        return qspCopyToNewText(QSP_PSTR(val));
    }
    return qspNullString;
}

INLINE QSPString qspGetVariantAsVarRef(QSPVariant *val)
{
    switch (QSP_BASETYPE(val->Type))
    {
    case QSP_TYPE_TUPLE:
    case QSP_TYPE_NUM:
        return qspGetVariantAsString(val); /* return invalid variable name */
    case QSP_TYPE_STR:
        {
            QSPString result = qspCopyToNewText(qspDelSpc(QSP_PSTR(val)));
            qspUpperStr(&result);
            return result;
        }
    }
    return qspNullString;
}

QSP_BIGINT qspGetVariantAsNum(QSPVariant *val, QSP_BOOL *isValid)
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

INLINE QSP_BOOL qspGetVariantAsBool(QSPVariant *val)
{
    switch (QSP_BASETYPE(val->Type))
    {
    case QSP_TYPE_TUPLE:
        return QSP_ISTRUE(QSP_PTUPLE(val).ValsCount);
    case QSP_TYPE_NUM:
        return QSP_ISTRUE(QSP_PNUM(val));
    case QSP_TYPE_STR:
        return QSP_ISTRUE(qspStrLen(QSP_PSTR(val)));
    }
    return QSP_FALSE;
}

QSP_BOOL qspConvertVariantTo(QSPVariant *val, QSP_TINYINT type)
{
    /* NB: val->Type can be QSP_TYPE_UNDEF */
    if (val->Type != type && qspTypeConversionTable[val->Type][type])
    {
        switch (type)
        {
        case QSP_TYPE_TUPLE:
            QSP_PTUPLE(val) = qspMoveToNewTuple(val, 1);
            break;
        case QSP_TYPE_NUM:
            {
                QSP_BOOL isValid;
                QSP_BIGINT result = qspGetVariantAsNum(val, &isValid);
                if (!isValid) return QSP_FALSE;
                qspFreeVariant(val);
                QSP_PNUM(val) = result;
                break;
            }
        case QSP_TYPE_BOOL:
            {
                QSP_BOOL result = qspGetVariantAsBool(val);
                qspFreeVariant(val);
                QSP_PNUM(val) = QSP_TOBOOL(result);
                break;
            }
        case QSP_TYPE_STR:
        case QSP_TYPE_CODE:
            {
                QSPString result = qspGetVariantAsString(val);
                qspFreeVariant(val);
                QSP_PSTR(val) = result;
                break;
            }
        case QSP_TYPE_VARREF:
            {
                QSPString result = qspGetVariantAsVarRef(val);
                qspFreeVariant(val);
                QSP_PSTR(val) = result;
                break;
            }
        }
        val->Type = type;
    }
    return QSP_TRUE;
}

int qspVariantsCompare(QSPVariant *first, QSPVariant *second)
{
    switch (QSP_BASETYPE(first->Type))
    {
    case QSP_TYPE_TUPLE:
        return qspTupleValueCompare(QSP_PTUPLE(first), second);
    case QSP_TYPE_NUM:
        switch (QSP_BASETYPE(second->Type))
        {
        case QSP_TYPE_TUPLE:
            return -qspTupleValueCompare(QSP_PTUPLE(second), first);
        case QSP_TYPE_NUM:
            return (QSP_PNUM(first) > QSP_PNUM(second) ? 1 : (QSP_PNUM(first) < QSP_PNUM(second) ? -1 : 0));
        case QSP_TYPE_STR:
            {
                QSP_BOOL isValid;
                QSP_BIGINT num = qspStrToNum(QSP_PSTR(second), &isValid);
                if (!isValid)
                {
                    QSP_CHAR buf[QSP_MAX_BIGINT_LEN];
                    return qspStrsCompare(qspNumToStr(buf, QSP_PNUM(first)), QSP_PSTR(second));
                }
                return (QSP_PNUM(first) > num ? 1 : (QSP_PNUM(first) < num ? -1 : 0));
            }
        }
        break;
    case QSP_TYPE_STR:
        switch (QSP_BASETYPE(second->Type))
        {
        case QSP_TYPE_TUPLE:
            return -qspTupleValueCompare(QSP_PTUPLE(second), first);
        case QSP_TYPE_NUM:
            {
                QSP_BOOL isValid;
                QSP_BIGINT num = qspStrToNum(QSP_PSTR(first), &isValid);
                if (!isValid)
                {
                    QSP_CHAR buf[QSP_MAX_BIGINT_LEN];
                    return qspStrsCompare(QSP_PSTR(first), qspNumToStr(buf, QSP_PNUM(second)));
                }
                return (num > QSP_PNUM(second) ? 1 : (num < QSP_PNUM(second) ? -1 : 0));
            }
        case QSP_TYPE_STR:
            return qspStrsCompare(QSP_PSTR(first), QSP_PSTR(second));
        }
        break;
    }
    return 0;
}

QSP_BOOL qspVariantsEqual(QSPVariant *first, QSPVariant *second)
{
    switch (QSP_BASETYPE(first->Type))
    {
    case QSP_TYPE_TUPLE:
        return (QSP_ISTUPLE(second->Type) && qspTuplesEqual(QSP_PTUPLE(first), QSP_PTUPLE(second)));
    case QSP_TYPE_NUM:
        return (QSP_ISNUM(second->Type) && QSP_PNUM(first) == QSP_PNUM(second));
    case QSP_TYPE_STR:
        return (QSP_ISSTR(second->Type) && qspStrsEqual(QSP_PSTR(first), QSP_PSTR(second)));
    }
    return QSP_FALSE;
}

void qspAutoConvertAppend(QSPVariant *arg1, QSPVariant *arg2, QSPVariant *res)
{
    switch (QSP_BASETYPE(arg1->Type))
    {
    case QSP_TYPE_TUPLE:
        switch (QSP_BASETYPE(arg2->Type))
        {
        case QSP_TYPE_TUPLE:
            *res = qspTupleVariant(qspMergeToNewTuple(QSP_PTUPLE(arg1).Vals, QSP_PTUPLE(arg1).ValsCount, QSP_PTUPLE(arg2).Vals, QSP_PTUPLE(arg2).ValsCount));
            break;
        case QSP_TYPE_NUM:
        case QSP_TYPE_STR:
            *res = qspTupleVariant(qspMergeToNewTuple(QSP_PTUPLE(arg1).Vals, QSP_PTUPLE(arg1).ValsCount, arg2, 1));
            break;
        }
        break;
    case QSP_TYPE_NUM:
    case QSP_TYPE_STR:
        switch (QSP_BASETYPE(arg2->Type))
        {
        case QSP_TYPE_TUPLE:
            *res = qspTupleVariant(qspMergeToNewTuple(arg1, 1, QSP_PTUPLE(arg2).Vals, QSP_PTUPLE(arg2).ValsCount));
            break;
        case QSP_TYPE_NUM:
        case QSP_TYPE_STR:
            qspConvertVariantTo(arg1, QSP_TYPE_STR);
            qspConvertVariantTo(arg2, QSP_TYPE_STR);

            QSP_PSTR(res) = qspConcatText(QSP_PSTR(arg1), QSP_PSTR(arg2));
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
            QSPVariant *vals = (QSPVariant *)malloc(tuple->ValsCount * sizeof(QSPVariant));
            for (i = 0; i < tuple->ValsCount; ++i)
            {
                if (!qspAutoConvertCombine(tuple->Vals + i, arg2, op, vals + i))
                {
                    qspFreeVariants(vals, i);
                    free(vals);
                    return QSP_FALSE;
                }
            }
            QSP_PTUPLE(res).Vals = vals;
            QSP_PTUPLE(res).ValsCount = tuple->ValsCount;
        }
        else
        {
            QSP_PTUPLE(res).Vals = 0;
            QSP_PTUPLE(res).ValsCount = 0;
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
            QSPVariant *vals = (QSPVariant *)malloc(tuple->ValsCount * sizeof(QSPVariant));
            for (i = 0; i < tuple->ValsCount; ++i)
            {
                if (!qspAutoConvertCombine(arg1, tuple->Vals + i, op, vals + i))
                {
                    qspFreeVariants(vals, i);
                    free(vals);
                    return QSP_FALSE;
                }
            }
            QSP_PTUPLE(res).Vals = vals;
            QSP_PTUPLE(res).ValsCount = tuple->ValsCount;
        }
        else
        {
            QSP_PTUPLE(res).Vals = 0;
            QSP_PTUPLE(res).ValsCount = 0;
        }
        res->Type = QSP_TYPE_TUPLE;
        return QSP_TRUE;
    }

    if (op == QSP_ADD_CHAR)
        return qspSumSimpleVariants(arg1, arg2, res);

    if (!qspConvertVariantTo(arg1, QSP_TYPE_NUM) || !qspConvertVariantTo(arg2, QSP_TYPE_NUM))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return QSP_FALSE;
    }

    switch (op)
    {
    case QSP_SUB_CHAR:
        QSP_PNUM(res) = QSP_PNUM(arg1) - QSP_PNUM(arg2);
        break;
    case QSP_MUL_CHAR:
        QSP_PNUM(res) = QSP_PNUM(arg1) * QSP_PNUM(arg2);
        break;
    case QSP_DIV_CHAR:
        if (QSP_PNUM(arg2) == 0)
        {
            qspSetError(QSP_ERR_DIVBYZERO);
            return QSP_FALSE;
        }
        QSP_PNUM(res) = QSP_PNUM(arg1) / QSP_PNUM(arg2);
        break;
    default:
        qspSetError(QSP_ERR_UNKNOWNACTION);
        return QSP_FALSE;
    }
    res->Type = QSP_TYPE_NUM;
    return QSP_TRUE;
}

void qspAppendVariantToIndexString(QSPVariant *val, QSPBufString *res)
{
    QSP_CHAR buf[QSP_MAX_BIGINT_LEN];
    switch (QSP_BASETYPE(val->Type))
    {
    case QSP_TYPE_TUPLE:
        {
            int count = QSP_PTUPLE(val).ValsCount;
            qspAddBufText(res, qspNumToStr(buf, count));
            if (count > 0)
            {
                QSPVariant *item = QSP_PTUPLE(val).Vals;
                qspAddBufText(res, QSP_STATIC_STR(QSP_IND_DELIM));
                while (--count > 0)
                {
                    qspAppendVariantToIndexString(item, res);
                    qspAddBufText(res, QSP_STATIC_STR(QSP_IND_DELIM));
                    ++item;
                }
                qspAppendVariantToIndexString(item, res);
            }
            break;
        }
    case QSP_TYPE_NUM:
        qspAddBufText(res, QSP_STATIC_STR(QSP_NUMTYPE)); /* type id */
        qspAddBufText(res, qspNumToStr(buf, QSP_PNUM(val)));
        break;
    case QSP_TYPE_STR:
        qspAddBufText(res, QSP_STATIC_STR(QSP_STRTYPE)); /* type id */
        qspAddBufText(res, QSP_PSTR(val));
        break;
    }
}
