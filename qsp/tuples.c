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

INLINE QSPVariant *qspReserveTupleFieldValue(QSPTuple *tuple, int nameIndex);
INLINE int qspTupleNamesCompare(QSPTuple first, QSPTuple second);
INLINE QSP_BOOL qspIsPlainFieldName(QSPString name);

QSPTuple qspNewTuple(int count)
{
    QSPTuple tuple;
    if (count)
    {
        tuple.Fields = (QSPField *)malloc(count * sizeof(QSPField));
        tuple.FieldsCount = count;
    }
    else
    {
        tuple.Fields = 0;
        tuple.FieldsCount = 0;
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
    if (tuple->Fields)
    {
        qspFreeTupleFields(tuple->Fields, tuple->FieldsCount);
        free(tuple->Fields);
    }
}

QSP_BOOL qspIsTupleNumber(QSPTuple tuple)
{
    if (tuple.FieldsCount == 1)
    {
        if (tuple.Fields[0].NameIndex > 0) /* tuple with named field is not a single scalar value */
            return QSP_FALSE;

        switch (QSP_BASETYPE(tuple.Fields[0].Value.Type))
        {
        case QSP_TYPE_TUPLE:
            return qspIsTupleNumber(QSP_TUPLE(tuple.Fields[0].Value));
        case QSP_TYPE_NUM:
            return QSP_TRUE;
        case QSP_TYPE_STR:
            return qspIsStrNumber(QSP_STR(tuple.Fields[0].Value));
        }
    }
    return QSP_FALSE;
}

QSP_BIGINT qspTupleToNum(QSPTuple tuple, QSP_BOOL *isValid)
{
    switch (tuple.FieldsCount)
    {
    case 0: /* a special case, i.e. an empty tuple must be convertible to 0 */
        if (isValid) *isValid = QSP_TRUE;
        return 0;
    case 1:
        if (tuple.Fields[0].NameIndex > 0) /* tuple with named field is not a single scalar value */
            break;

        switch (QSP_BASETYPE(tuple.Fields[0].Value.Type))
        {
        case QSP_TYPE_UNDEF:
            if (isValid) *isValid = QSP_TRUE;
            return 0;
        case QSP_TYPE_TUPLE:
            return qspTupleToNum(QSP_TUPLE(tuple.Fields[0].Value), isValid);
        case QSP_TYPE_NUM:
            if (isValid) *isValid = QSP_TRUE;
            return QSP_NUM(tuple.Fields[0].Value);
        case QSP_TYPE_STR:
            return qspStrToNum(QSP_STR(tuple.Fields[0].Value), isValid);
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
    for (newItem = tuple.Fields; count > 0; --count, ++values, ++newItem)
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
    for (newItem = tuple.Fields; count > 0; --count, ++values, ++newItem)
    {
        newItem->NameIndex = 0;
        qspMoveToNewVariant(&newItem->Value, values);
    }
    return tuple;
}

QSPTuple qspCopyFieldsToNewTuple(QSPField *fields, int count)
{
    QSPField *newItem;
    QSPTuple tuple = qspNewTuple(count);
    for (newItem = tuple.Fields; count > 0; --count, ++fields, ++newItem)
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
    for (newItem = tuple.Fields; count > 0; --count, ++fields, ++newItem)
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
    QSPTuple result = qspNewTuple(tuple->FieldsCount + 1);
    if (tuple->Fields)
    {
        memcpy(result.Fields, tuple->Fields, tuple->FieldsCount * sizeof(QSPField));
        free(tuple->Fields);
    }
    item = result.Fields + tuple->FieldsCount;
    item->NameIndex = 0;
    qspMoveToNewVariant(&item->Value, value);
    *tuple = qspNullTuple;
    return result;
}

QSPTuple qspPrependValueToNewTuple(QSPTuple *tuple, QSPVariant *value)
{
    /* Builds new tuple with combined values, old items are moved to the result */
    QSPField *item;
    QSPTuple result = qspNewTuple(tuple->FieldsCount + 1);
    if (tuple->Fields)
    {
        memcpy(result.Fields + 1, tuple->Fields, tuple->FieldsCount * sizeof(QSPField));
        free(tuple->Fields);
    }
    item = result.Fields;
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
    newItem = second->Fields;
    itemsEnd = second->Fields + second->FieldsCount;
    for (item = second->Fields; item < itemsEnd; ++item)
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
    appendedCount = (int)(newItem - second->Fields);
    if (appendedCount)
    {
        QSPTuple merged = qspNewTuple(result.FieldsCount + appendedCount);
        if (result.Fields)
        {
            memcpy(merged.Fields, result.Fields, result.FieldsCount * sizeof(QSPField));
            free(result.Fields);
        }
        memcpy(merged.Fields + result.FieldsCount, second->Fields, appendedCount * sizeof(QSPField));
        result = merged;
    }
    /* All items are moved to the result */
    if (second->Fields) free(second->Fields);
    *second = qspNullTuple;
    return result;
}

void qspReserveTupleItems(QSPTuple *tuple, int count)
{
    if (count > tuple->FieldsCount)
    {
        QSPField *item, *itemsEnd;
        QSPTuple result = qspNewTuple(count);
        if (tuple->Fields)
        {
            memcpy(result.Fields, tuple->Fields, tuple->FieldsCount * sizeof(QSPField));
            free(tuple->Fields);
        }
        itemsEnd = result.Fields + count;
        for (item = result.Fields + tuple->FieldsCount; item < itemsEnd; ++item)
        {
            item->NameIndex = 0;
            qspInitVariant(&item->Value, QSP_TYPE_UNDEF);
        }
        *tuple = result;
    }
}

INLINE QSPVariant *qspReserveTupleFieldValue(QSPTuple *tuple, int nameIndex)
{
    QSPField *item;
    QSPTuple result;
    QSPVariant *fieldValue = qspGetTupleFieldValue(*tuple, nameIndex);
    if (fieldValue) return fieldValue;
    result = qspNewTuple(tuple->FieldsCount + 1);
    if (tuple->Fields)
    {
        memcpy(result.Fields, tuple->Fields, tuple->FieldsCount * sizeof(QSPField));
        free(tuple->Fields);
    }
    item = result.Fields + tuple->FieldsCount;
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
    qspFreeVariant(&tuple->Fields[fieldIndex].Value);
    itemsCount = tuple->FieldsCount - 1;
    result = qspNewTuple(itemsCount);
    if (itemsCount)
    {
        memcpy(result.Fields, tuple->Fields, fieldIndex * sizeof(QSPField));
        memcpy(result.Fields + fieldIndex, tuple->Fields + fieldIndex + 1, (itemsCount - fieldIndex) * sizeof(QSPField));
    }
    free(tuple->Fields);
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
        return (tuple.FieldsCount ? 1 : 0); /* undefined value is the empty tuple here */
    case QSP_TYPE_TUPLE:
        return qspTuplesCompare(tuple, QSP_PTUPLE(value));
    case QSP_TYPE_NUM:
    case QSP_TYPE_STR:
        {
            int delta;
            if (tuple.FieldsCount == 0)
                return -1; /* an empty tuple is always smaller than any value */
            delta = qspVariantsCompare(&tuple.Fields->Value, value); /* compare the first tuple item with the value */
            if (delta) return delta;
            if (tuple.FieldsCount > 1 || tuple.Fields->NameIndex > 0)
                return 1; /* the tuple is bigger if it contains more than 1 item or the item has a name */
            break;
        }
    }
    return 0;
}

INLINE int qspTupleNamesCompare(QSPTuple first, QSPTuple second)
{
    QSPField *pos1 = first.Fields, *pos2 = second.Fields;
    QSPField *end1 = first.Fields + first.FieldsCount, *end2 = second.Fields + second.FieldsCount;
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
    QSPField *pos1 = first.Fields, *pos2 = second.Fields;
    QSPField *end1 = first.Fields + first.FieldsCount, *end2 = second.Fields + second.FieldsCount;
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
    if (first.FieldsCount == second.FieldsCount)
    {
        QSPField *pos1 = first.Fields, *pos2 = second.Fields;
        QSPField *end1 = first.Fields + first.FieldsCount;
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

INLINE QSP_BOOL qspIsPlainFieldName(QSPString name)
{
    if (qspIsEmpty(name)) return QSP_FALSE;
    if (qspIsInClass(*name.Str, QSP_CHAR_DIGIT | QSP_CHAR_TYPEPREFIX)) return QSP_FALSE;
    if (qspStrCharClass(name, QSP_CHAR_DELIM | QSP_CHAR_FIELDREF)) return QSP_FALSE;
    return QSP_TRUE;
}

void qspAppendTupleToString(QSPTuple tuple, QSPBufString *res)
{
    QSP_CHAR buf[QSP_MAX_BIGINT_LEN];
    QSPString name;
    QSPVariant *value;
    QSPField *item = tuple.Fields, *itemsEnd = item + tuple.FieldsCount;
    qspAddBufText(res, QSP_STATIC_STR(QSP_TUPLEDISPLAY_START));
    while (item < itemsEnd)
    {
        if (item->NameIndex > 0)
        {
            name = qspGetNameText(item->NameIndex);
            if (qspIsPlainFieldName(name))
                qspAddBufText(res, name);
            else
                qspAddBufQuotedText(res, name, QSP_STATIC_STR(QSP_DEFQUOT));
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
            qspAddBufQuotedText(res, QSP_PSTR(value), QSP_STATIC_STR(QSP_DEFQUOT));
            break;
        }
        if (++item == itemsEnd) break;
        qspAddBufText(res, QSP_STATIC_STR(QSP_TUPLEDISPLAY_DELIM));
    }
    qspAddBufText(res, QSP_STATIC_STR(QSP_TUPLEDISPLAY_END));
}
