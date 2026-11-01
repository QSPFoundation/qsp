/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "tuples.h"
#include "names.h"
#include "text.h"
#include "variant.h"

QSPTuple qspNullTuple;

INLINE int qspTupleNamesCompare(QSPTuple first, QSPTuple second);

QSPTuple qspNewTuple(int count)
{
    QSPTuple tuple;
    if (count)
    {
        tuple.Vals = (QSPField *)malloc(count * sizeof(QSPField));
        tuple.ValsCount = count;
    }
    else
    {
        tuple.Vals = 0;
        tuple.ValsCount = 0;
    }
    return tuple;
}

void qspFreeTupleFields(QSPField *fields, int count)
{
    while (--count >= 0)
    {
        qspFreeVariant(&fields->Value);
        ++fields;
    }
}

void qspFreeTuple(QSPTuple *tuple)
{
    if (tuple->Vals)
    {
        qspFreeTupleFields(tuple->Vals, tuple->ValsCount);
        free(tuple->Vals);
    }
}

QSP_BOOL qspIsTupleNumber(QSPTuple tuple)
{
    if (tuple.ValsCount == 1)
    {
        if (tuple.Vals[0].NameIndex > 0) /* tuple with named fields is not a single scalar value */
            return QSP_FALSE;

        switch (QSP_BASETYPE(tuple.Vals[0].Value.Type))
        {
        case QSP_TYPE_TUPLE:
            return qspIsTupleNumber(QSP_TUPLE(tuple.Vals[0].Value));
        case QSP_TYPE_NUM:
            return QSP_TRUE;
        case QSP_TYPE_STR:
            return qspIsStrNumber(QSP_STR(tuple.Vals[0].Value));
        }
    }
    return QSP_FALSE;
}

QSP_BIGINT qspTupleToNum(QSPTuple tuple, QSP_BOOL *isValid)
{
    switch (tuple.ValsCount)
    {
    case 0: /* a special case, i.e. an empty tuple must be convertible to 0 */
        if (isValid) *isValid = QSP_TRUE;
        return 0;
    case 1:
        if (tuple.Vals[0].NameIndex > 0) /* tuple with named fields is not a single scalar value */
            break;

        switch (QSP_BASETYPE(tuple.Vals[0].Value.Type))
        {
        case QSP_TYPE_UNDEF:
            if (isValid) *isValid = QSP_TRUE;
            return 0;
        case QSP_TYPE_TUPLE:
            return qspTupleToNum(QSP_TUPLE(tuple.Vals[0].Value), isValid);
        case QSP_TYPE_NUM:
            if (isValid) *isValid = QSP_TRUE;
            return QSP_NUM(tuple.Vals[0].Value);
        case QSP_TYPE_STR:
            return qspStrToNum(QSP_STR(tuple.Vals[0].Value), isValid);
        }
        break;
    }

    if (isValid) *isValid = QSP_FALSE;
    return 0;
}

QSPTuple qspCopyToNewTuple(QSPVariant *values, int count)
{
    QSPField *newItem;
    QSPTuple tuple = qspNewTuple(count);
    for (newItem = tuple.Vals; count > 0; --count, ++values, ++newItem)
    {
        newItem->NameIndex = 0;
        qspCopyToNewVariant(&newItem->Value, values);
    }
    return tuple;
}

QSPTuple qspMoveToNewTuple(QSPVariant *values, int count)
{
    QSPField *newItem;
    QSPTuple tuple = qspNewTuple(count);
    for (newItem = tuple.Vals; count > 0; --count, ++values, ++newItem)
    {
        newItem->NameIndex = 0;
        qspMoveToNewVariant(&newItem->Value, values);
    }
    return tuple;
}

void qspAssignNamesToTupleItems(QSPTuple *tuple, QSPField *fields, int count)
{
    QSPField *item;
    for (item = tuple->Vals; count > 0; --count, ++fields, ++item)
        item->NameIndex = fields->NameIndex;
}

QSPTuple qspCopyFieldsToNewTuple(QSPField *fields, int count)
{
    QSPField *newItem;
    QSPTuple tuple = qspNewTuple(count);
    for (newItem = tuple.Vals; count > 0; --count, ++fields, ++newItem)
    {
        newItem->NameIndex = fields->NameIndex;
        qspCopyToNewVariant(&newItem->Value, &fields->Value);
    }
    return tuple;
}

QSPTuple qspMoveFieldsToNewTuple(QSPField *fields, int count)
{
    QSPField *newItem;
    QSPTuple tuple = qspNewTuple(count);
    for (newItem = tuple.Vals; count > 0; --count, ++fields, ++newItem)
    {
        newItem->NameIndex = fields->NameIndex;
        qspMoveToNewVariant(&newItem->Value, &fields->Value);
    }
    return tuple;
}

QSPTuple qspAppendValueToNewTuple(QSPTuple *tuple, QSPVariant *value)
{
    /* Builds new tuple with combined values, old items are moved to the result */
    QSPField *item;
    QSPTuple result = qspNewTuple(tuple->ValsCount + 1);
    if (tuple->Vals)
    {
        memcpy(result.Vals, tuple->Vals, tuple->ValsCount * sizeof(QSPField));
        free(tuple->Vals);
    }
    item = result.Vals + tuple->ValsCount;
    item->NameIndex = 0;
    qspMoveToNewVariant(&item->Value, value);
    *tuple = qspNullTuple;
    return result;
}

QSPTuple qspPrependValueToNewTuple(QSPTuple *tuple, QSPVariant *value)
{
    /* Builds new tuple with combined values, old items are moved to the result */
    QSPField *item;
    QSPTuple result = qspNewTuple(tuple->ValsCount + 1);
    if (tuple->Vals)
    {
        memcpy(result.Vals + 1, tuple->Vals, tuple->ValsCount * sizeof(QSPField));
        free(tuple->Vals);
    }
    item = result.Vals;
    item->NameIndex = 0;
    qspMoveToNewVariant(&item->Value, value);
    *tuple = qspNullTuple;
    return result;
}

QSPTuple qspMergeToNewTuple(QSPTuple *first, QSPTuple *second)
{
    /* Old items are moved to the result, named items of the second tuple replace items with the same name */
    int appendedCount;
    QSPVariant *fieldValue;
    QSPField *item, *itemsEnd, *newItem;
    QSPTuple result = *first;
    *first = qspNullTuple;
    /* Preprocess the second tuple */
    newItem = second->Vals;
    itemsEnd = second->Vals + second->ValsCount;
    for (item = second->Vals; item < itemsEnd; ++item)
    {
        fieldValue = qspGetTupleFieldValue(result, item->NameIndex);
        if (fieldValue)
        {
            /* Replace previous value */
            qspFreeVariant(fieldValue);
            *fieldValue = item->Value;
        }
        else
        {
            /* Move item within second tuple */
            *newItem = *item;
            ++newItem;
        }
    }
    /* Append items of the second tuple */
    appendedCount = (int)(newItem - second->Vals);
    if (appendedCount)
    {
        QSPTuple merged = qspNewTuple(result.ValsCount + appendedCount);
        if (result.Vals)
        {
            memcpy(merged.Vals, result.Vals, result.ValsCount * sizeof(QSPField));
            free(result.Vals);
        }
        memcpy(merged.Vals + result.ValsCount, second->Vals, appendedCount * sizeof(QSPField));
        result = merged;
    }
    /* All items are moved to the result */
    if (second->Vals) free(second->Vals);
    *second = qspNullTuple;
    return result;
}

void qspReserveTupleItems(QSPTuple *tuple, int count)
{
    if (count > tuple->ValsCount)
    {
        QSPField *item, *itemsEnd;
        QSPTuple result = qspNewTuple(count);
        if (tuple->Vals)
        {
            memcpy(result.Vals, tuple->Vals, tuple->ValsCount * sizeof(QSPField));
            free(tuple->Vals);
        }
        itemsEnd = result.Vals + count;
        for (item = result.Vals + tuple->ValsCount; item < itemsEnd; ++item)
        {
            item->NameIndex = 0;
            qspInitVariant(&item->Value, QSP_TYPE_UNDEF);
        }
        *tuple = result;
    }
}

QSPVariant *qspReserveTupleFieldValue(QSPTuple *tuple, int nameIndex)
{
    QSPField *item;
    QSPTuple result;
    QSPVariant *fieldValue = qspGetTupleFieldValue(*tuple, nameIndex);
    if (fieldValue) return fieldValue;
    result = qspNewTuple(tuple->ValsCount + 1);
    if (tuple->Vals)
    {
        memcpy(result.Vals, tuple->Vals, tuple->ValsCount * sizeof(QSPField));
        free(tuple->Vals);
    }
    item = result.Vals + tuple->ValsCount;
    item->NameIndex = nameIndex;
    qspInitVariant(&item->Value, QSP_TYPE_UNDEF);
    *tuple = result;
    return &item->Value;
}

QSPVariant *qspReserveTupleFieldValueByPath(QSPVariant *value, QSPField *fields, int count)
{
    /* Creates missing fields, values on the way become tuples */
    while (--count >= 0)
    {
        if (!QSP_ISTUPLE(value->Type))
        {
            qspFreeVariant(value);
            qspInitVariant(value, QSP_TYPE_TUPLE);
        }
        value = qspReserveTupleFieldValue(&QSP_PTUPLE(value), fields->NameIndex);
        ++fields;
    }
    return value;
}

void qspRemoveTupleField(QSPTuple *tuple, int nameIndex)
{
    QSPTuple result;
    int itemsCount, fieldIndex = qspGetTupleFieldIndex(*tuple, nameIndex);
    if (fieldIndex < 0) return;
    qspFreeVariant(&tuple->Vals[fieldIndex].Value);
    itemsCount = tuple->ValsCount - 1;
    result = qspNewTuple(itemsCount);
    if (itemsCount)
    {
        memcpy(result.Vals, tuple->Vals, fieldIndex * sizeof(QSPField));
        memcpy(result.Vals + fieldIndex, tuple->Vals + fieldIndex + 1, (itemsCount - fieldIndex) * sizeof(QSPField));
    }
    free(tuple->Vals);
    *tuple = result;
}

QSPVariant *qspGetTupleFieldValueByPath(QSPVariant *value, QSPField *fields, int count)
{
    while (value && --count >= 0)
    {
        if (!QSP_ISTUPLE(value->Type)) return 0;
        value = qspGetTupleFieldValue(QSP_PTUPLE(value), fields->NameIndex);
        ++fields;
    }
    return value;
}

int qspTupleValueCompare(QSPTuple tuple, QSPVariant *value)
{
    switch (QSP_BASETYPE(value->Type))
    {
    case QSP_TYPE_UNDEF:
        return (tuple.ValsCount ? 1 : 0); /* undefined value is the empty tuple here */
    case QSP_TYPE_TUPLE:
        return qspTuplesCompare(tuple, QSP_PTUPLE(value));
    case QSP_TYPE_NUM:
    case QSP_TYPE_STR:
        {
            int delta;
            if (tuple.ValsCount == 0)
                return -1; /* an empty tuple is always smaller than any value */
            delta = qspVariantsCompare(&tuple.Vals->Value, value); /* compare the first tuple item with the value */
            if (delta) return delta;
            if (tuple.ValsCount > 1 || tuple.Vals->NameIndex > 0)
                return 1; /* the tuple is bigger if it contains more than 1 item or the item has a name */
            break;
        }
    }
    return 0;
}

INLINE int qspTupleNamesCompare(QSPTuple first, QSPTuple second)
{
    QSPField *pos1 = first.Vals, *pos2 = second.Vals;
    QSPField *end1 = first.Vals + first.ValsCount, *end2 = second.Vals + second.ValsCount;
    while (pos2 < end2 && pos1 < end1)
    {
        if (pos1->NameIndex != pos2->NameIndex) /* the same name has the same index */
            return qspStrsCompare(qspGetNameText(pos1->NameIndex), qspGetNameText(pos2->NameIndex));
        ++pos1, ++pos2;
    }
    if (pos1 < end1) return 1;
    if (pos2 < end2) return -1;
    return 0;
}

int qspTuplesCompare(QSPTuple first, QSPTuple second)
{
    int delta;
    QSPField *pos1 = first.Vals, *pos2 = second.Vals;
    QSPField *end1 = first.Vals + first.ValsCount, *end2 = second.Vals + second.ValsCount;
    while (pos2 < end2 && pos1 < end1)
    {
        delta = qspVariantsCompare(&pos1->Value, &pos2->Value);
        if (delta) return delta;
        ++pos1, ++pos2;
    }
    if (pos1 < end1) return 1;
    if (pos2 < end2) return -1;
    return qspTupleNamesCompare(first, second);
}

QSP_BOOL qspTuplesEqual(QSPTuple first, QSPTuple second)
{
    if (first.ValsCount == second.ValsCount)
    {
        QSPField *pos1 = first.Vals, *pos2 = second.Vals;
        QSPField *end1 = first.Vals + first.ValsCount;
        while (pos1 < end1)
        {
            if (pos1->NameIndex != pos2->NameIndex || !qspVariantsEqual(&pos1->Value, &pos2->Value))
                return QSP_FALSE;
            ++pos1, ++pos2;
        }
        return QSP_TRUE;
    }
    return QSP_FALSE;
}

void qspAppendTupleToString(QSPTuple tuple, QSPBufString *res)
{
    QSP_CHAR buf[QSP_MAX_BIGINT_LEN];
    QSPString temp;
    QSPVariant *value;
    QSPField *item = tuple.Vals, *itemsEnd = item + tuple.ValsCount;
    qspAddBufText(res, QSP_STATIC_STR(QSP_TUPLEDISPLAY_START));
    while (item < itemsEnd)
    {
        if (item->NameIndex > 0)
        {
            qspAddBufText(res, qspGetNameText(item->NameIndex));
            qspAddBufText(res, QSP_STATIC_STR(QSP_TUPLEDISPLAY_FIELDDELIM));
        }
        value = &item->Value;
        switch (QSP_BASETYPE(value->Type))
        {
        case QSP_TYPE_UNDEF:
            qspAddBufText(res, QSP_STATIC_STR(QSP_UNDEFTYPE));
            break;
        case QSP_TYPE_TUPLE:
            qspAppendTupleToString(QSP_PTUPLE(value), res);
            break;
        case QSP_TYPE_NUM:
            qspAddBufText(res, qspNumToStr(buf, QSP_PNUM(value)));
            break;
        case QSP_TYPE_STR:
            qspAddBufText(res, QSP_STATIC_STR(QSP_DEFQUOT));
            temp = qspReplaceText(QSP_PSTR(value), QSP_STATIC_STR(QSP_DEFQUOT), QSP_STATIC_STR(QSP_ESCDEFQUOT), INT_MAX, QSP_TRUE);
            qspAddBufText(res, temp);
            qspFreeNewString(&temp, &QSP_PSTR(value));
            qspAddBufText(res, QSP_STATIC_STR(QSP_DEFQUOT));
            break;
        }
        if (++item == itemsEnd) break;
        qspAddBufText(res, QSP_STATIC_STR(QSP_TUPLEDISPLAY_DELIM));
    }
    qspAddBufText(res, QSP_STATIC_STR(QSP_TUPLEDISPLAY_END));
}
