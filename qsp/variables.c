/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "variables.h"
#include "codetools.h"
#include "coding.h"
#include "common.h"
#include "errors.h"
#include "locations.h"
#include "mathops.h"
#include "regexp.h"

QSPVar qspNullVar;
QSPVarsScope qspGlobalVars;
QSPVarsScopeChunk *qspCurrentLocalVars = 0;

QSP_TINYINT qspSpecToBaseTypeTable[128];

INLINE int qspIndStringCompare(const void *name, const void *compareTo);
INLINE int qspIndStringFloorCompare(const void *name, const void *compareTo);
INLINE int qspValuePositionsAscCompare(const void *arg1, const void *arg2);
INLINE int qspValuePositionsDescCompare(const void *arg1, const void *arg2);
INLINE unsigned int qspGetNameHash(QSPString name);
INLINE QSPVar *qspCreateNewVar(QSPVarsBucket *bucket, QSPString name, int capacityIncrement);
INLINE QSPVar *qspGetVar(QSPVarsBucket *bucket, QSPString name);
INLINE QSPVar *qspAddVarToLocals(QSPString name);
INLINE QSPVar *qspAddSpecialVarToScope(QSPVarsScope *scope, QSPString name);
INLINE void qspSetVarValuesByReference(QSPVar *var, QSPVariant *vals, int count, QSP_BOOL toMove);
INLINE void qspRemoveArrayItem(QSPVar *var, int index);
INLINE QSPVar *qspGetVarData(QSPString s, int *index, QSP_BOOL isSetOperation);
INLINE QSP_BOOL qspGetVarValueByReference(QSPVar *var, int ind, QSP_TINYINT baseType, QSPVariant *res);
INLINE void qspResetVar(QSPString varName);
INLINE void qspSetVarValueByReference(QSPVar *var, int ind, QSP_TINYINT baseType, QSPVariant *val);
INLINE void qspSetVarValueByIndex(QSPString varName, QSPVariant index, QSPVariant *val);
INLINE void qspSetFirstVarValue(QSPString varName, QSPVariant *val);
INLINE void qspSetVarValue(QSPString varName, QSPVariant *val, QSP_CHAR op);
INLINE void qspMoveTupleToArray(QSPVar *dest, QSPTuple *src, int start, int count);
INLINE void qspCopyArray(QSPVar *dest, QSPVar *src, int start, int count);
INLINE void qspSortArray(QSPVar *var, QSP_TINYINT baseValType, QSP_BOOL isAscending);
INLINE int qspGetVarsNames(QSPString str, QSPString *varNames, int maxNames);
INLINE void qspSetVarsValues(QSPString *varNames, int varsCount, QSPVariant *v, QSP_CHAR op);
INLINE QSPString qspGetVarNameOnly(QSPString s);

void qspInitVarTypes(void)
{
    int i;
    for (i = 0; i < sizeof(qspSpecToBaseTypeTable); ++i)
        qspSpecToBaseTypeTable[i] = QSP_TYPE_NUM;

    qspSpecToBaseTypeTable[QSP_NUMTYPE_CHAR] = QSP_TYPE_NUM;
    qspSpecToBaseTypeTable[QSP_STRTYPE_CHAR] = QSP_TYPE_STR;
    qspSpecToBaseTypeTable[QSP_TUPLETYPE_CHAR] = QSP_TYPE_TUPLE;
}

INLINE int qspIndStringCompare(const void *name, const void *compareTo)
{
    return qspStrsCompare(*(QSPString *)name, ((QSPVarIndex *)compareTo)->Str);
}

INLINE int qspIndStringFloorCompare(const void *name, const void *compareTo)
{
    QSPString key = *(QSPString *)name;
    QSPVarIndex *item = (QSPVarIndex *)compareTo;

    /* It's safe to check (item + 1) because the item never points to the last array item */
    if (qspStrsCompare(key, (item + 1)->Str) < 0 && qspStrsCompare(key, item->Str) >= 0)
        return 0;

    return qspStrsCompare(key, item->Str);
}

INLINE int qspValuePositionsAscCompare(const void *arg1, const void *arg2)
{
    return qspVariantsCompare(*(QSPVariant **)arg1, *(QSPVariant **)arg2); /* base types of values should be the same */
}

INLINE int qspValuePositionsDescCompare(const void *arg1, const void *arg2)
{
    return qspVariantsCompare(*(QSPVariant **)arg2, *(QSPVariant **)arg1); /* base types of values should be the same */
}

INLINE unsigned int qspGetNameHash(QSPString name)
{
    QSP_CHAR *pos;
    unsigned int nameHash = 7;
    for (pos = name.Str; pos < name.End; ++pos)
        nameHash = nameHash * 31 + (unsigned char)*pos;

    return nameHash;
}

QSPVarsScopeChunk *qspAllocateVarsScopeChunk(QSPVarsScopeChunk *parentChunk)
{
    int i;
    QSPVarsScope *scope;
    QSPVarsScopeChunk *chunk = (QSPVarsScopeChunk *)malloc(sizeof(QSPVarsScopeChunk));
    chunk->ParentChunk = parentChunk;
    chunk->SlotsCount = 0;

    scope = chunk->Slots;
    for (i = 0; i < QSP_VARSSCOPECHUNKSIZE; ++i, ++scope)
    {
        /* We don't initialize the allocated scopes */
        scope->Buckets = 0;
        scope->BucketsCount = 0;
    }

    return chunk;
}

void qspClearVarsScopeChunk(QSPVarsScopeChunk *chunk)
{
    int i;
    QSPVarsScope *scope = chunk->Slots;
    for (i = 0; i < QSP_VARSSCOPECHUNKSIZE; ++i, ++scope)
        qspClearVarsScope(scope);
    free(chunk);
}

void qspInitVarsScope(QSPVarsScope *scope, int buckets)
{
    int i;
    QSPVarsBucket *bucket;
    scope->BucketsCount = buckets;
    bucket = scope->Buckets = (QSPVarsBucket *)malloc(scope->BucketsCount * sizeof(QSPVarsBucket));
    for (i = scope->BucketsCount; i > 0; --i, ++bucket)
    {
        bucket->Vars = 0;
        bucket->Capacity = bucket->VarsCount = 0;
    }
}

void qspClearVarsScope(QSPVarsScope *scope)
{
    int i;
    QSPVarsBucket *bucket = scope->Buckets;
    if (bucket)
    {
        for (i = scope->BucketsCount; i > 0; --i, ++bucket)
        {
            if (bucket->Vars) free(bucket->Vars);
        }
        free(scope->Buckets);
    }
}

void qspClearVars(QSPVarsScope *scope)
{
    /* Remove all variables & keep the buckets */
    int i, j;
    QSPVar *var;
    QSPVarsBucket *bucket = scope->Buckets;
    for (i = scope->BucketsCount; i > 0; --i, ++bucket)
    {
        var = bucket->Vars;
        if (var)
        {
            /* Remove vars & keep the allocated vars buffer */
            for (j = bucket->VarsCount; j > 0; --j, ++var)
            {
                qspFreeString(&var->Name);
                qspEmptyVar(var);
            }
            bucket->VarsCount = 0;
        }
    }
}

void qspClearLocalVarsScopes(QSPVarsScopeChunk *chunk)
{
    int i;
    QSPVarsScope *scope;
    QSPVarsScopeChunk *parentChunk;
    while (chunk)
    {
        scope = chunk->Slots;
        for (i = chunk->SlotsCount; i > 0; --i, ++scope)
            qspClearVars(scope);

        parentChunk = chunk->ParentChunk;
        qspClearVarsScopeChunk(chunk);
        chunk = parentChunk;
    }
}

void qspClearAllVars(QSP_BOOL toInit)
{
    if (!toInit)
    {
        /* Clear all local scopes */
        qspClearLocalVarsScopes(qspCurrentLocalVars);
        /* Clear global variables & keep the global scope */
        qspClearVars(&qspGlobalVars);
    }
    qspCurrentLocalVars = 0;
}

INLINE QSPVar *qspCreateNewVar(QSPVarsBucket *bucket, QSPString name, int capacityIncrement)
{
    QSPVar *var;
    if (bucket->VarsCount >= bucket->Capacity)
    {
        bucket->Capacity = bucket->VarsCount + capacityIncrement;
        bucket->Vars = (QSPVar *)realloc(bucket->Vars, bucket->Capacity * sizeof(QSPVar));
    }
    var = bucket->Vars + bucket->VarsCount;
    var->Name = qspCopyToNewText(name);
    qspInitVarData(var);

    bucket->VarsCount++;
    return var;
}

INLINE QSPVar *qspGetVar(QSPVarsBucket *bucket, QSPString name)
{
    int i;
    QSPVar *var = bucket->Vars;
    for (i = bucket->VarsCount; i > 0; --i)
    {
        if (qspStrsEqual(var->Name, name)) return var;
        ++var;
    }
    return 0;
}

INLINE QSPVar *qspAddVarToLocals(QSPString name)
{
    unsigned int nameHash;
    QSPVarsScope *scope;
    QSPVarsBucket *bucket;
    QSPVar *var;
    if (qspIsEmpty(name))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return 0;
    }

    /* Ignore type prefix */
    if (qspIsInClass(*name.Str, QSP_CHAR_TYPEPREFIX))
        name.Str++;

    if (qspIsEmpty(name) || qspIsInClass(*name.Str, QSP_CHAR_DIGIT) || qspStrCharClass(name, QSP_CHAR_DELIM))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return 0;
    }

    nameHash = qspGetNameHash(name);
    scope = qspCurrentLocalVars->Slots + qspCurrentLocalVars->SlotsCount - 1;
    if (!scope->Buckets)
        qspInitVarsScope(scope, QSP_VARSLOCALBUCKETS); /* init the scope the first time it's used */

    /* Check if the variable already exists in the current scope */
    bucket = scope->Buckets + (nameHash % scope->BucketsCount);
    var = qspGetVar(bucket, name);
    if (var) return var;

    /* It doesn't exist yet, so we have to add it */
    if (bucket->VarsCount >= QSP_VARSMAXBUCKETSIZE)
    {
        qspSetError(QSP_ERR_TOOMANYVARS);
        return 0;
    }
    return qspCreateNewVar(bucket, name, 8);
}

INLINE QSPVar *qspAddSpecialVarToScope(QSPVarsScope *scope, QSPString name)
{
    unsigned int nameHash = qspGetNameHash(name); /* we don't validate the name */
    QSPVarsBucket *bucket = scope->Buckets + (nameHash % scope->BucketsCount);

    return qspCreateNewVar(bucket, name, 2); /* small increment for special vars */
}

INLINE void qspSetVarValuesByReference(QSPVar *var, QSPVariant *vals, int count, QSP_BOOL toMove)
{
    if (count)
    {
        int i;
        var->ValsCapacity = var->ValsCount = count;
        var->Values = (QSPVariant *)malloc(count * sizeof(QSPVariant));
        if (toMove)
        {
            for (i = 0; i < count; ++i)
                qspMoveToNewVariant(var->Values + i, vals + i);
        }
        else
        {
            for (i = 0; i < count; ++i)
                qspCopyToNewVariant(var->Values + i, vals + i);
        }
    }
}

QSPVarsScope *qspAllocateLocalScopeWithArgs(QSPVariant *args, int count, QSP_BOOL toMove)
{
    QSPVar *varArgs;
    QSPVarsScope *scope = qspAllocateLocalScope();
    if (!scope->Buckets)
        qspInitVarsScope(scope, QSP_VARSLOCALBUCKETS); /* init the uninitialized scope */

    varArgs = qspAddSpecialVarToScope(scope, QSP_STATIC_STR(QSP_VARARGS));
    qspSetVarValuesByReference(varArgs, args, count, toMove);

    qspAddSpecialVarToScope(scope, QSP_STATIC_STR(QSP_VARRES));

    return scope;
}

QSP_BOOL qspSetArgs(QSPVariant *args, int count, QSP_BOOL toMove)
{
    QSPVar *varArgs = qspVarReference(QSP_STATIC_STR(QSP_VARARGS), QSP_TRUE);
    if (!varArgs) return QSP_FALSE;

    qspEmptyVar(varArgs);
    qspSetVarValuesByReference(varArgs, args, count, toMove);
    return QSP_TRUE;
}

QSP_BOOL qspApplyResult(QSPVariant *res)
{
    QSPVar *varRes = qspVarReference(QSP_STATIC_STR(QSP_VARRES), QSP_FALSE);
    if (!varRes) return QSP_FALSE;

    if (varRes->ValsCount)
        qspCopyToNewVariant(res, varRes->Values);
    else
        qspInitVariant(res, QSP_TYPE_UNDEF);
    return QSP_TRUE;
}

QSPVarsScopeChunk *qspSaveLocalVarsAndRestoreGlobals(void)
{
    QSPVarsScopeChunk *previousVarsChunk = qspCurrentLocalVars;
    qspCurrentLocalVars = 0;
    return previousVarsChunk;
}

void qspRestoreSavedLocalVars(QSPVarsScopeChunk *chunk)
{
    qspClearLocalVarsScopes(qspCurrentLocalVars);
    qspCurrentLocalVars = chunk;
}

QSPVar *qspVarReference(QSPString name, QSP_BOOL toCreate)
{
    int i;
    unsigned int nameHash;
    QSPVarsScopeChunk *chunk;
    QSPVarsScope *scope;
    QSPVarsBucket *bucket;
    QSPVar *var;
    if (qspIsEmpty(name))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return 0;
    }

    /* Ignore type prefix */
    if (qspIsInClass(*name.Str, QSP_CHAR_TYPEPREFIX))
        name.Str++;

    if (qspIsEmpty(name) || qspIsInClass(*name.Str, QSP_CHAR_DIGIT) || qspStrCharClass(name, QSP_CHAR_DELIM))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return 0;
    }

    /* Check all local scopes starting the latest */
    nameHash = qspGetNameHash(name);
    chunk = qspCurrentLocalVars;
    while (chunk)
    {
        scope = chunk->Slots + chunk->SlotsCount - 1;
        for (i = chunk->SlotsCount; i > 0; --i, --scope)
        {
            if (scope->Buckets)
            {
                bucket = scope->Buckets + (nameHash % scope->BucketsCount);
                var = qspGetVar(bucket, name);
                if (var) return var;
            }
        }
        chunk = chunk->ParentChunk;
    }

    /* Check the global scope */
    bucket = qspGlobalVars.Buckets + (nameHash % qspGlobalVars.BucketsCount);
    var = qspGetVar(bucket, name);
    if (var) return var;

    if (toCreate)
    {
        /* Create it in the global scope */
        if (bucket->VarsCount >= QSP_VARSMAXBUCKETSIZE)
        {
            qspSetError(QSP_ERR_TOOMANYVARS);
            return 0;
        }
        return qspCreateNewVar(bucket, name, 8);
    }
    return &qspNullVar;
}

INLINE void qspRemoveArrayItem(QSPVar *var, int index)
{
    int i;
    QSP_BOOL toRemove;
    QSPVarIndex *ind;
    if (index < 0 || index >= var->ValsCount) return;
    qspFreeVariant(var->Values + index);
    var->ValsCount--;
    i = index;
    while (i < var->ValsCount)
    {
        var->Values[i] = var->Values[i + 1];
        ++i;
    }
    toRemove = QSP_FALSE;
    ind = var->Indices;
    for (i = 0; i < var->IndsCount; ++i)
    {
        if (ind->Index == index)
        {
            qspFreeString(&ind->Str);
            var->IndsCount--;
            if (i == var->IndsCount) break;
            toRemove = QSP_TRUE;
        }
        if (toRemove) *ind = *(ind + 1);
        if (ind->Index > index) ind->Index--;
        ++ind;
    }
}

int qspGetVarIndex(QSPVar *var, QSPVariant index, QSP_BOOL toCreate)
{
    int indsCount;
    QSPString uStr;
    if (QSP_ISNUM(index.Type)) return QSP_TOINT(QSP_NUM(index));
    uStr = qspGetVariantAsIndexString(&index);
    qspUpperStr(&uStr);
    indsCount = var->IndsCount;
    if (indsCount > 0)
    {
        QSPVarIndex *ind = (QSPVarIndex *)bsearch(&uStr, var->Indices, indsCount, sizeof(QSPVarIndex), qspIndStringCompare);
        if (ind)
        {
            qspFreeString(&uStr);
            return ind->Index;
        }
    }
    if (toCreate)
    {
        /* Find the first item that's smaller than uStr */
        int floorItem = indsCount - 1;
        if (indsCount > 0)
        {
            QSPVarIndex *lastItem = var->Indices + floorItem;
            if (qspStrsCompare(uStr, lastItem->Str) < 0)
            {
                QSPVarIndex *ind = (QSPVarIndex *)bsearch(&uStr, var->Indices, floorItem, sizeof(QSPVarIndex), qspIndStringFloorCompare);
                floorItem = (ind ? (int)(ind - var->Indices) : -1);
            }
        }
        /* Prepare buffer & shift existing items to allocate extra space */
        if (indsCount >= var->IndsCapacity)
        {
            var->IndsCapacity = indsCount + 8;
            var->Indices = (QSPVarIndex *)realloc(var->Indices, var->IndsCapacity * sizeof(QSPVarIndex));
        }
        ++floorItem;
        while (indsCount > floorItem)
        {
            var->Indices[indsCount] = var->Indices[indsCount - 1];
            --indsCount;
        }
        /* Add new index item */
        indsCount = var->ValsCount; /* point to the new array item */
        var->Indices[floorItem].Str = uStr;
        var->Indices[floorItem].Index = indsCount;
        var->IndsCount++;
        return indsCount;
    }
    qspFreeString(&uStr);
    return -1;
}

INLINE QSPVar *qspGetVarData(QSPString s, int *index, QSP_BOOL isSetOperation)
{
    QSP_CHAR *nameEnd = qspStrCharClass(s, QSP_CHAR_DELIM);
    if (nameEnd)
    {
        QSP_CHAR *startPos = s.Str;
        s.Str = nameEnd;
        qspSkipSpaces(&s);
        if (!qspIsEmpty(s) && *s.Str == QSP_LSBRACK_CHAR)
        {
            QSPVar *var;
            QSP_CHAR *rPos = qspDelimPos(s, QSP_RSBRACK_CHAR);
            if (!rPos)
            {
                qspSetError(QSP_ERR_BRACKNOTFOUND);
                return 0;
            }
            var = qspVarReference(qspStringFromPair(startPos, nameEnd), isSetOperation);
            if (!var) return 0;
            s.Str += QSP_CHAR_LEN;
            qspSkipSpaces(&s);
            if (s.Str == rPos)
            {
                if (isSetOperation)
                    *index = var->ValsCount; /* new item */
                else
                    *index = (var->ValsCount ? var->ValsCount - 1 : 0); /* last item */
            }
            else
            {
                QSPVariant ind;
                int oldLocationState = qspLocationState;
                ind = qspCalculateExprValue(qspStringFromPair(s.Str, rPos));
                if (qspLocationState != oldLocationState) return 0;
                *index = qspGetVarIndex(var, ind, isSetOperation);
                qspFreeVariant(&ind);
            }
            return var;
        }
        else
        {
            qspSetError(QSP_ERR_INCORRECTNAME);
            return 0;
        }
    }
    *index = 0;
    return qspVarReference(s, isSetOperation);
}

INLINE QSP_BOOL qspGetVarValueByReference(QSPVar *var, int ind, QSP_TINYINT baseType, QSPVariant *res)
{
    if (ind >= 0 && ind < var->ValsCount)
    {
        QSP_TINYINT varType = var->Values[ind].Type;
        if (QSP_ISDEF(varType) && QSP_BASETYPE(varType) == baseType)
        {
            qspCopyToNewVariant(res, var->Values + ind);
            return QSP_TRUE;
        }
    }
    qspInitVariant(res, baseType);
    return QSP_TRUE;
}

QSP_BOOL qspGetVarValueByIndex(QSPString varName, QSPVariant index, QSPVariant *res)
{
    int arrIndex;
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    if (!var) return QSP_FALSE;
    arrIndex = qspGetVarIndex(var, index, QSP_FALSE);
    varType = qspGetVarType(varName);
    return qspGetVarValueByReference(var, arrIndex, varType, res);
}

QSP_BOOL qspGetFirstVarValue(QSPString varName, QSPVariant *res)
{
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    if (!var) return QSP_FALSE;
    varType = qspGetVarType(varName);
    return qspGetVarValueByReference(var, 0, varType, res);
}

QSP_BOOL qspGetLastVarValue(QSPString varName, QSPVariant *res)
{
    int arrIndex;
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    if (!var) return QSP_FALSE;
    arrIndex = var->ValsCount - 1;
    varType = qspGetVarType(varName);
    return qspGetVarValueByReference(var, arrIndex, varType, res);
}

QSPString qspGetVarStrValue(QSPString name)
{
    int oldLocationState = qspLocationState;
    QSPVar *var = qspVarReference(name, QSP_FALSE);
    if (var)
    {
        if (var->ValsCount && QSP_ISSTR(var->Values[0].Type))
            return QSP_STR(var->Values[0]);
    }
    else
    {
        /* Reset the location state */
        qspLocationState = oldLocationState;
        qspResetError(QSP_FALSE);
    }
    return qspNullString;
}

QSP_BIGINT qspGetVarNumValue(QSPString name)
{
    int oldLocationState = qspLocationState;
    QSPVar *var = qspVarReference(name, QSP_FALSE);
    if (var)
    {
        if (var->ValsCount && QSP_ISNUM(var->Values[0].Type))
            return QSP_NUM(var->Values[0]);
    }
    else
    {
        /* Reset the location state */
        qspLocationState = oldLocationState;
        qspResetError(QSP_FALSE);
    }
    return 0;
}

INLINE void qspResetVar(QSPString varName)
{
    int index;
    QSPVar *var = qspGetVarData(varName, &index, QSP_TRUE);
    if (!var) return;
    if (index >= 0 && index < var->ValsCount)
    {
        QSPVariant *curValue = var->Values + index;
        if (QSP_ISDEF(curValue->Type))
        {
            qspFreeVariant(curValue);
            qspInitVariant(curValue, QSP_TYPE_UNDEF);
        }
    }
}

INLINE void qspSetVarValueByReference(QSPVar *var, int ind, QSP_TINYINT baseType, QSPVariant *val)
{
    int oldCount = var->ValsCount;
    if (ind >= oldCount)
    {
        /* A new item */
        QSPVariant *curValue;
        if (!qspConvertVariantTo(val, baseType))
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            return;
        }
        if (ind >= var->ValsCapacity)
        {
            if (ind > 0)
                var->ValsCapacity = ind + 4;
            else
                var->ValsCapacity = 1; /* allocate only 1 item for the first value */
            var->Values = (QSPVariant *)realloc(var->Values, var->ValsCapacity * sizeof(QSPVariant));
        }
        var->ValsCount = ind + 1;
        /* Init new values */
        for (curValue = var->Values + oldCount; oldCount < ind; ++curValue, ++oldCount)
            qspInitVariant(curValue, QSP_TYPE_UNDEF);
        qspMoveToNewVariant(var->Values + ind, val);
    }
    else if (ind >= 0)
    {
        /* Replace an existing item */
        if (!qspConvertVariantTo(val, baseType))
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            return;
        }
        qspFreeVariant(var->Values + ind);
        qspMoveToNewVariant(var->Values + ind, val);
    }
}

INLINE void qspSetVarValueByIndex(QSPString varName, QSPVariant index, QSPVariant *val)
{
    int arrIndex;
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_TRUE);
    if (!var) return;
    varType = qspGetVarType(varName);
    arrIndex = qspGetVarIndex(var, index, QSP_TRUE);
    qspSetVarValueByReference(var, arrIndex, varType, val);
}

INLINE void qspSetFirstVarValue(QSPString varName, QSPVariant *val)
{
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_TRUE);
    if (!var) return;
    varType = qspGetVarType(varName);
    qspSetVarValueByReference(var, 0, varType, val);
}

INLINE void qspSetVarValue(QSPString varName, QSPVariant *val, QSP_CHAR op)
{
    int index;
    QSP_TINYINT varType;
    QSPVar *var = qspGetVarData(varName, &index, QSP_TRUE);
    if (!var) return;
    varType = qspGetVarType(varName);
    if (op == QSP_EQUAL_CHAR)
        qspSetVarValueByReference(var, index, varType, val);
    else if (qspIsInClass(op, QSP_CHAR_SIMPLEOP))
    {
        QSPVariant oldVal, res;
        qspGetVarValueByReference(var, index, varType, &oldVal);
        if (!qspAutoConvertCombine(&oldVal, val, op, &res))
        {
            qspFreeVariant(&oldVal);
            return;
        }
        qspSetVarValueByReference(var, index, varType, &res);
        qspFreeVariant(&res);
        qspFreeVariant(&oldVal);
    }
    else
    {
        qspSetError(QSP_ERR_UNKNOWNACTION);
        return;
    }
}

INLINE void qspMoveTupleToArray(QSPVar *dest, QSPTuple *src, int start, int count)
{
    int i, itemsToMove;
    /* Clear the dest array anyway */
    qspEmptyVar(dest);
    /* Validate parameters */
    if (count <= 0) return;
    if (start < 0) start = 0;
    itemsToMove = src->ValsCount - start;
    if (itemsToMove <= 0) return;
    if (count < itemsToMove) itemsToMove = count;
    /* Move tuple items */
    dest->ValsCapacity = dest->ValsCount = itemsToMove;
    dest->Values = (QSPVariant *)malloc(itemsToMove * sizeof(QSPVariant));
    for (i = 0; i < itemsToMove; ++i)
        qspMoveToNewVariant(dest->Values + i, src->Vals + start + i);
}

INLINE void qspCopyArray(QSPVar *dest, QSPVar *src, int start, int count)
{
    int i, itemsToCopy, newInd;
    /* Clear the dest array anyway */
    qspEmptyVar(dest);
    /* Validate parameters */
    if (count <= 0) return;
    if (start < 0) start = 0;
    itemsToCopy = src->ValsCount - start;
    if (itemsToCopy <= 0) return;
    if (count < itemsToCopy) itemsToCopy = count;
    /* Copy array values */
    dest->ValsCapacity = dest->ValsCount = itemsToCopy;
    dest->Values = (QSPVariant *)malloc(itemsToCopy * sizeof(QSPVariant));
    for (i = 0; i < itemsToCopy; ++i)
        qspCopyToNewVariant(dest->Values + i, src->Values + start + i);
    /* Copy array indices */
    dest->IndsCapacity = 0;
    dest->Indices = 0;
    count = 0;
    for (i = 0; i < src->IndsCount; ++i)
    {
        newInd = src->Indices[i].Index - start;
        if (newInd >= 0 && newInd < itemsToCopy)
        {
            if (count >= dest->IndsCapacity)
            {
                dest->IndsCapacity = count + 16;
                dest->Indices = (QSPVarIndex *)realloc(dest->Indices, dest->IndsCapacity * sizeof(QSPVarIndex));
            }
            dest->Indices[count].Index = newInd;
            dest->Indices[count].Str = qspCopyToNewText(src->Indices[i].Str);
            ++count;
        }
    }
    dest->IndsCount = count;
}

INLINE void qspSortArray(QSPVar *var, QSP_TINYINT baseValType, QSP_BOOL isAscending)
{
    QSPVariant *curValue, *sortedValues, **valuePositions;
    int i, *indexMapping, indsCount, valsCount;
    valsCount = var->ValsCount;
    if (valsCount < 2) return;
    valuePositions = (QSPVariant **)malloc(valsCount * sizeof(QSPVariant *));
    curValue = var->Values;
    for (i = 0; i < valsCount; ++i, ++curValue)
    {
        if (QSP_BASETYPE(curValue->Type) != baseValType)
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            free(valuePositions);
            return;
        }
        valuePositions[i] = curValue;
    }
    /* Sort positions of values by comparing values */
    if (isAscending)
        qsort(valuePositions, valsCount, sizeof(QSPVariant *), qspValuePositionsAscCompare);
    else
        qsort(valuePositions, valsCount, sizeof(QSPVariant *), qspValuePositionsDescCompare);
    /* Create mapping from old positions to new positions */
    indexMapping = (int *)malloc(valsCount * sizeof(int));
    for (i = 0; i < valsCount; ++i)
        indexMapping[valuePositions[i] - var->Values] = i;
    /* Reorder indices */
    indsCount = var->IndsCount;
    for (i = 0; i < indsCount; ++i)
        var->Indices[i].Index = indexMapping[var->Indices[i].Index];
    free(indexMapping);
    /* Reorder values */
    sortedValues = (QSPVariant *)malloc(valsCount * sizeof(QSPVariant));
    curValue = sortedValues;
    for (i = 0; i < valsCount; ++i, ++curValue)
        qspMoveToNewVariant(curValue, valuePositions[i]);
    free(valuePositions);
    free(var->Values);
    var->Values = sortedValues;
}

int qspArraySize(QSPString varName)
{
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    if (!var) return 0;
    return var->ValsCount;
}

int qspArrayPos(QSPString varName, QSPVariant *val, int ind)
{
    QSP_TINYINT varType;
    QSPVariant defaultValue, *curValue;
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    if (!var) return -1;
    varType = qspGetVarType(varName);
    if (!qspConvertVariantTo(val, varType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return -1;
    }
    defaultValue = qspGetEmptyVariant(varType);
    if (ind < 0) ind = 0;
    while (ind < var->ValsCount)
    {
        curValue = var->Values + ind;
        if (!QSP_ISDEF(curValue->Type)) curValue = &defaultValue; /* check undefined values */
        if (qspVariantsEqual(val, curValue)) return ind;
        ++ind;
    }
    return -1;
}

int qspArrayPosRegExp(QSPString varName, QSPString regExpStr, int ind)
{
    QSPVariant defaultValue, *curValue;
    QSPRegExp *regExp;
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    if (!var) return -1;
    regExp = qspRegExpGetCompiled(regExpStr);
    if (!regExp) return -1;
    defaultValue = qspGetEmptyVariant(QSP_TYPE_STR);
    if (ind < 0) ind = 0;
    while (ind < var->ValsCount)
    {
        curValue = var->Values + ind;
        if (!QSP_ISDEF(curValue->Type)) curValue = &defaultValue; /* check undefined values */
        if (QSP_ISSTR(curValue->Type) && qspRegExpStrMatch(regExp, QSP_PSTR(curValue))) return ind;
        ++ind;
    }
    return -1;
}

QSPVariant qspArrayMinMaxItem(QSPString varName, QSP_BOOL isMin)
{
    int i;
    QSPVariant resultValue, *bestValue, *curValue;
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    if (!var) return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    varType = qspGetVarType(varName);
    resultValue = qspGetEmptyVariant(varType);
    bestValue = 0;
    for (i = 0; i < var->ValsCount; ++i)
    {
        curValue = var->Values + i;
        if (!QSP_ISDEF(curValue->Type)) curValue = &resultValue; /* check undefined values */
        if (QSP_BASETYPE(curValue->Type) == varType)
        {
            if (bestValue)
            {
                switch (varType)
                {
                case QSP_TYPE_TUPLE:
                    if (isMin)
                    {
                        if (qspTuplesCompare(QSP_PTUPLE(curValue), QSP_PTUPLE(bestValue)) < 0)
                            bestValue = curValue;
                    }
                    else if (qspTuplesCompare(QSP_PTUPLE(curValue), QSP_PTUPLE(bestValue)) > 0)
                        bestValue = curValue;
                    break;
                case QSP_TYPE_STR:
                    if (isMin)
                    {
                        if (qspStrsCompare(QSP_PSTR(curValue), QSP_PSTR(bestValue)) < 0)
                            bestValue = curValue;
                    }
                    else if (qspStrsCompare(QSP_PSTR(curValue), QSP_PSTR(bestValue)) > 0)
                        bestValue = curValue;
                    break;
                case QSP_TYPE_NUM:
                    if (isMin)
                    {
                        if (QSP_PNUM(curValue) < QSP_PNUM(bestValue))
                            bestValue = curValue;
                    }
                    else if (QSP_PNUM(curValue) > QSP_PNUM(bestValue))
                        bestValue = curValue;
                    break;
                }
            }
            else
                bestValue = curValue;
        }
    }
    if (bestValue) qspCopyToNewVariant(&resultValue, bestValue);
    return resultValue;
}

INLINE int qspGetVarsNames(QSPString str, QSPString *varNames, int maxNames)
{
    QSP_CHAR *pos;
    int count = 0;
    while (1)
    {
        qspSkipSpaces(&str);
        if (qspIsEmpty(str))
        {
            qspSetError(QSP_ERR_SYNTAX);
            return 0;
        }
        if (count >= maxNames)
        {
            qspSetError(QSP_ERR_ARGSCOUNT);
            return 0;
        }
        pos = qspDelimPos(str, QSP_COMMA_CHAR);
        if (pos)
        {
            varNames[count] = qspDelSpc(qspStringFromPair(str.Str, pos));
            ++count;
        }
        else
        {
            varNames[count] = qspDelSpc(str);
            ++count;
            break;
        }
        str.Str = pos + QSP_CHAR_LEN;
    }
    return count;
}

INLINE void qspSetVarsValues(QSPString *varNames, int varsCount, QSPVariant *v, QSP_CHAR op)
{
    int i, oldLocationState;
    if (varsCount == 1)
    {
        qspSetVarValue(varNames[0], v, op);
        return;
    }
    /* Examples:
     * a,b=2,3
     * a,b=5
     * a,b,c=4,5
     * a,b=5,6,7
     * a,b=[]
     * */
    oldLocationState = qspLocationState;
    switch (QSP_BASETYPE(v->Type))
    {
    case QSP_TYPE_TUPLE:
        {
            int valuesCount = QSP_PTUPLE(v).ValsCount;
            if (varsCount < valuesCount)
            {
                /* Assign variables that contain single values */
                QSPVariant v2;
                int lastVarIndex = varsCount - 1;
                for (i = 0; i < lastVarIndex; ++i)
                {
                    qspSetVarValue(varNames[i], QSP_PTUPLE(v).Vals + i, op);
                    if (qspLocationState != oldLocationState)
                        return;
                }
                /* Only 1 variable left, fill it with a tuple containing all the values left */
                v2 = qspTupleVariant(qspMoveToNewTuple(QSP_PTUPLE(v).Vals + i, QSP_PTUPLE(v).ValsCount - i));
                qspSetVarValue(varNames[lastVarIndex], &v2, op);
                qspFreeVariant(&v2);
            }
            else
            {
                /* Assign all values to the variables */
                for (i = 0; i < valuesCount; ++i)
                {
                    qspSetVarValue(varNames[i], QSP_PTUPLE(v).Vals + i, op);
                    if (qspLocationState != oldLocationState)
                        return;
                }
                /* No values left, reset the rest of vars with default values */
                while (i < varsCount)
                {
                    qspResetVar(varNames[i]);
                    if (qspLocationState != oldLocationState)
                        return;
                    ++i;
                }
            }
            break;
        }
    case QSP_TYPE_NUM:
    case QSP_TYPE_STR:
        /* Consider it a tuple with 1 item */
        qspSetVarValue(varNames[0], v, op);
        if (qspLocationState != oldLocationState)
            return;
        for (i = 1; i < varsCount; ++i)
        {
            qspResetVar(varNames[i]);
            if (qspLocationState != oldLocationState)
                return;
        }
        break;
    }
}

void qspStatementSetVarsValues(QSPString s, QSPCachedStat *stat)
{
    QSPVariant v;
    QSPString names[QSP_SETMAXVARS];
    int namesCount, oldLocationState;
    QSP_CHAR op;
    if (stat->ErrorCode)
    {
        qspSetError(stat->ErrorCode);
        return;
    }
    if (stat->ArgsCount < 3)
    {
        qspSetError(QSP_ERR_EQNOTFOUND);
        return;
    }
    oldLocationState = qspLocationState;
    v = qspCalculateExprValue(qspStringFromPair(s.Str + stat->Args[2].StartPos, s.Str + stat->Args[2].EndPos));
    if (qspLocationState != oldLocationState) return;
    namesCount = qspGetVarsNames(qspStringFromPair(s.Str + stat->Args[0].StartPos, s.Str + stat->Args[0].EndPos), names, QSP_SETMAXVARS);
    if (!namesCount)
    {
        qspFreeVariant(&v);
        return;
    }
    op = *(s.Str + stat->Args[1].StartPos); /* contains one of QSP_CHAR_SIMPLEOP characters */
    qspSetVarsValues(names, namesCount, &v, op);
    qspFreeVariant(&v);
}

INLINE QSPString qspGetVarNameOnly(QSPString s)
{
    QSP_CHAR *nameEnd = qspStrCharClass(s, QSP_CHAR_DELIM);
    if (nameEnd) s.End = nameEnd;
    return s;
}

void qspStatementLocal(QSPString s, QSPCachedStat *stat)
{
    QSPVariant v;
    QSPString varName, names[QSP_SETMAXVARS];
    int i, namesCount;
    if (stat->ErrorCode)
    {
        qspSetError(stat->ErrorCode);
        return;
    }
    if (stat->ArgsCount > 1)
    {
        int oldLocationState = qspLocationState;
        if (*(s.Str + stat->Args[1].StartPos) != QSP_EQUAL_CHAR)
        {
            qspSetError(QSP_ERR_SYNTAX);
            return;
        }
        /* We have to evaluate expression before allocation of local vars */
        v = qspCalculateExprValue(qspStringFromPair(s.Str + stat->Args[2].StartPos, s.Str + stat->Args[2].EndPos));
        if (qspLocationState != oldLocationState) return;
    }
    else
        v = qspGetEmptyVariant(QSP_TYPE_UNDEF);
    namesCount = qspGetVarsNames(qspStringFromPair(s.Str + stat->Args[0].StartPos, s.Str + stat->Args[0].EndPos), names, QSP_SETMAXVARS);
    if (!namesCount)
    {
        qspFreeVariant(&v);
        return;
    }
    for (i = 0; i < namesCount; ++i)
    {
        varName = qspGetVarNameOnly(names[i]);
        if (!qspAddVarToLocals(varName))
        {
            qspFreeVariant(&v);
            return;
        }
    }
    if (stat->ArgsCount > 1)
    {
        qspSetVarsValues(names, namesCount, &v, QSP_EQUAL_CHAR);
        qspFreeVariant(&v);
    }
}

void qspStatementSetVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (count == 2)
        qspSetFirstVarValue(QSP_STR(args[0]), args + 1);
    else
        qspSetVarValueByIndex(QSP_STR(args[0]), args[2], args + 1);
}

void qspStatementUnpackArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    int startInd, maxCount;
    QSPTuple *src;
    QSPVar *dest = qspVarReference(QSP_STR(args[0]), QSP_TRUE);
    if (!dest) return;
    src = &QSP_TUPLE(args[1]);
    startInd = (count >= 3 ? QSP_TOINT(QSP_NUM(args[2])) : 0);
    maxCount = (count == 4 ? QSP_TOINT(QSP_NUM(args[3])) : src->ValsCount);
    qspMoveTupleToArray(dest, src, startInd, maxCount);
}

void qspStatementCopyArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPVar *dest, *src;
    if (!((dest = qspVarReference(QSP_STR(args[0]), QSP_TRUE)))) return;
    if (!((src = qspVarReference(QSP_STR(args[1]), QSP_FALSE)))) return;
    if (dest != src)
    {
        int startInd = (count >= 3 ? QSP_TOINT(QSP_NUM(args[2])) : 0);
        int maxCount = (count == 4 ? QSP_TOINT(QSP_NUM(args[3])) : src->ValsCount);
        qspCopyArray(dest, src, startInd, maxCount);
    }
}

void qspStatementSortArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_FALSE); /* we don't create a new array */
    if (!var) return;
    varType = qspGetVarType(QSP_STR(args[0]));
    if (count == 2)
        qspSortArray(var, varType, QSP_ISFALSE(QSP_NUM(args[1])));
    else
        qspSortArray(var, varType, QSP_TRUE);
}

void qspStatementScanStr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPRegExp *regExp;
    QSP_CHAR *foundPos;
    QSPString text;
    QSPVariant foundString;
    int curInd, groupInd, foundLen;
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_TRUE);
    if (!var) return;
    regExp = qspRegExpGetCompiled(QSP_STR(args[2]));
    if (!regExp) return;
    text = QSP_STR(args[1]);
    groupInd = (count == 4 ? QSP_TOINT(QSP_NUM(args[3])) : 0);
    qspEmptyVar(var); /* clear the dest array anyway */
    foundString.Type = QSP_TYPE_STR;
    curInd = 0;
    foundPos = qspRegExpStrSearch(regExp, text, 0, groupInd, &foundLen);
    while (foundPos && foundLen)
    {
        QSP_STR(foundString) = qspStringFromLen(foundPos, foundLen);
        if (curInd >= var->ValsCapacity)
        {
            var->ValsCapacity = curInd + 8;
            var->Values = (QSPVariant *)realloc(var->Values, var->ValsCapacity * sizeof(QSPVariant));
        }
        qspCopyToNewVariant(var->Values + curInd, &foundString);
        ++curInd;
        foundPos = qspRegExpStrSearch(regExp, text, foundPos + foundLen, groupInd, &foundLen);
    }
    var->ValsCount = curInd;
}

void qspStatementKillVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (count)
    {
        QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_FALSE); /* we don't create a new array */
        if (!var) return;
        if (count == 1)
            qspEmptyVar(var);
        else
        {
            int arrIndex = qspGetVarIndex(var, args[1], QSP_FALSE);
            qspRemoveArrayItem(var, arrIndex);
        }
    }
    else
        qspClearAllVars(QSP_FALSE);
}
