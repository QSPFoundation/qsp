/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "text.h"

#ifndef QSP_TUPLESDEFINES
    #define QSP_TUPLESDEFINES

    #define QSP_TUPLEDISPLAY_START QSP_LSBRACK
    #define QSP_TUPLEDISPLAY_END QSP_RSBRACK
    #define QSP_TUPLEDISPLAY_DELIM QSP_COMMA QSP_FMT(" ")
    #define QSP_TUPLEDISPLAY_FIELDDELIM QSP_FIELDVALUEDELIM QSP_FMT(" ")

    extern QSPTuple qspNullTuple;

    /* External functions */
    QSPTuple qspNewTuple(int count);
    void qspFreeTupleFields(QSPField *fields, int count);
    void qspFreeTuple(QSPTuple *tuple);
    QSP_BOOL qspIsTupleNumber(QSPTuple tuple);
    QSP_BIGINT qspTupleToNum(QSPTuple tuple, QSP_BOOL *isValid);
    QSPTuple qspCopyToNewTuple(QSPVariant *values, int count);
    QSPTuple qspMoveToNewTuple(QSPVariant *values, int count);
    void qspAssignNamesToTupleItems(QSPTuple *tuple, QSPField *fields, int count);
    QSPTuple qspCopyFieldsToNewTuple(QSPField *fields, int count);
    QSPTuple qspMoveFieldsToNewTuple(QSPField *fields, int count);
    QSPTuple qspAppendValueToNewTuple(QSPTuple *tuple, QSPVariant *value);
    QSPTuple qspPrependValueToNewTuple(QSPTuple *tuple, QSPVariant *value);
    QSPTuple qspMergeToNewTuple(QSPTuple *first, QSPTuple *second);
    void qspReserveTupleItems(QSPTuple *tuple, int count);
    QSPVariant *qspReserveTupleFieldValue(QSPTuple *tuple, int nameIndex);
    QSPVariant *qspReserveTupleFieldValueByPath(QSPVariant *value, QSPField *fields, int count);
    void qspRemoveTupleField(QSPTuple *tuple, int nameIndex);
    QSPVariant *qspGetTupleFieldValueByPath(QSPVariant *value, QSPField *fields, int count);
    int qspTupleValueCompare(QSPTuple tuple, QSPVariant *value);
    int qspTuplesCompare(QSPTuple first, QSPTuple second);
    QSP_BOOL qspTuplesEqual(QSPTuple first, QSPTuple second);
    void qspAppendTupleToString(QSPTuple tuple, QSPBufString *res);

    INLINE int qspGetTupleFieldIndex(QSPTuple tuple, int nameIndex)
    {
        if (nameIndex > 0)
        {
            int i;
            for (i = 0; i < tuple.FieldsCount; ++i)
                if (tuple.Fields[i].NameIndex == nameIndex) return i;
        }
        return -1;
    }

    INLINE QSPVariant *qspGetTupleFieldValue(QSPTuple tuple, int nameIndex)
    {
        int fieldIndex = qspGetTupleFieldIndex(tuple, nameIndex);
        return (fieldIndex >= 0 ? &tuple.Fields[fieldIndex].Value : 0);
    }

    INLINE QSP_BOOL qspTupleHasNames(QSPTuple tuple)
    {
        QSPField *item = tuple.Fields, *itemsEnd = item + tuple.FieldsCount;
        while (item < itemsEnd)
        {
            if (item->NameIndex > 0) return QSP_TRUE;
            ++item;
        }
        return QSP_FALSE;
    }

    INLINE QSPString qspGetTupleAsString(QSPTuple tuple)
    {
        QSPBufString buf = qspNewBufString(0, 16);
        qspAppendTupleToString(tuple, &buf);
        return qspBufStringToString(buf);
    }

#endif
