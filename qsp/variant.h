/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "text.h"
#include "tuples.h"

#ifndef QSP_VARIANTDEFINES
    #define QSP_VARIANTDEFINES

    #define QSP_IND_DELIM QSP_FMT("\0")

    /* External functions */
    QSPString qspGetVariantAsString(QSPVariant *val);
    QSP_BIGINT qspGetVariantAsNum(QSPVariant *val, QSP_BOOL *isValid);
    QSP_BOOL qspConvertVariantTo(QSPVariant *val, QSP_TINYINT type);
    int qspVariantsCompare(QSPVariant *first, QSPVariant *second);
    QSP_BOOL qspVariantsEqual(QSPVariant *first, QSPVariant *second);
    void qspAutoConvertAppend(QSPVariant *arg1, QSPVariant *arg2, QSPVariant *res);
    QSP_BOOL qspAutoConvertCombine(QSPVariant *arg1, QSPVariant *arg2, QSP_CHAR op, QSPVariant *res);
    void qspAppendVariantToIndexString(QSPVariant *val, QSPBufString *res);

    INLINE void qspFreeVariant(QSPVariant *val)
    {
        switch (QSP_BASETYPE(val->Type))
        {
        case QSP_TYPE_TUPLE:
            qspFreeTuple(&QSP_PTUPLE(val));
            break;
        case QSP_TYPE_STR:
            qspFreeString(&QSP_PSTR(val));
            break;
        }
    }

    INLINE void qspFreeVariants(QSPVariant *args, int count)
    {
        while (--count >= 0)
        {
            qspFreeVariant(args);
            ++args;
        }
    }

    INLINE void qspInitVariant(QSPVariant *value, QSP_TINYINT type)
    {
        /* Works fine with all types including QSP_TYPE_UNDEF */
        memset(&value->Val, 0, sizeof(value->Val));
        value->Type = type;
    }

    INLINE QSPVariant qspGetEmptyVariant(QSP_TINYINT type)
    {
        QSPVariant ret;
        qspInitVariant(&ret, type);
        return ret;
    }

    INLINE QSPVariant qspNumVariant(QSP_BIGINT value)
    {
        QSPVariant ret;
        ret.Type = QSP_TYPE_NUM;
        QSP_NUM(ret) = value;
        return ret;
    }

    INLINE QSPVariant qspStrVariant(QSPString value, QSP_TINYINT type)
    {
        QSPVariant ret;
        ret.Type = type;
        QSP_STR(ret) = value;
        return ret;
    }

    INLINE QSPVariant qspTupleVariant(QSPTuple value)
    {
        QSPVariant ret;
        ret.Type = QSP_TYPE_TUPLE;
        QSP_TUPLE(ret) = value;
        return ret;
    }

    INLINE void qspCopyToNewVariant(QSPVariant *dest, QSPVariant *src)
    {
        switch (QSP_BASETYPE(dest->Type = src->Type))
        {
        case QSP_TYPE_TUPLE:
            QSP_PTUPLE(dest) = qspCopyToNewTuple(QSP_PTUPLE(src).Vals, QSP_PTUPLE(src).ValsCount);
            break;
        case QSP_TYPE_NUM:
            QSP_PNUM(dest) = QSP_PNUM(src);
            break;
        case QSP_TYPE_STR:
            QSP_PSTR(dest) = qspCopyToNewText(QSP_PSTR(src));
            break;
        }
    }

    INLINE void qspMoveToNewVariant(QSPVariant *dest, QSPVariant *src)
    {
        dest->Val = src->Val;
        dest->Type = src->Type;
        qspInitVariant(src, src->Type);
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

    INLINE QSPString qspGetVariantAsIndexString(QSPVariant *val)
    {
        QSPBufString buf = qspNewBufString(16);
        qspAppendVariantToIndexString(val, &buf);
        return qspBufTextToString(buf);
    }

#endif
