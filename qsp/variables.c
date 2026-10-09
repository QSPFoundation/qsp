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

INLINE int qspValuePositionsAscCompare(const void *arg1, const void *arg2);
INLINE int qspValuePositionsDescCompare(const void *arg1, const void *arg2);
INLINE QSPVarSlot *qspNewVarSlots(int capacity);
INLINE QSPVarSlot *qspGetVarSlot(QSPVarsScope *scope, QSPString name, unsigned int nameHash);
INLINE QSPVarSlot *qspGetEmptyVarSlot(QSPVarsScope *scope, unsigned int nameHash);
INLINE void qspResizeVarsScope(QSPVarsScope *scope, int newCapacity);
INLINE void qspResizeVarsNames(QSPVarsScope *scope, int newCapacity);
INLINE QSPVar *qspCreateNewVar(QSPVarsScope *scope, QSPString name, unsigned int nameHash);
INLINE QSPVar *qspAddVarToLocals(QSPString name);
INLINE void qspSetVarValuesByReference(QSPVar *var, QSPVariant *vals, int count, QSP_BOOL toMove);
INLINE int *qspNewVarIndicesSlots(int capacity);
INLINE int *qspGetVarIndexSlot(QSPVar *var, QSPString key, unsigned int hash);
INLINE int *qspGetEmptyVarIndexSlot(QSPVar *var, unsigned int hash);
INLINE int *qspGetVarIndexSlotByPos(QSPVar *var, int indexPos);
INLINE void qspResizeVarIndices(QSPVar *var, int newCapacity);
INLINE void qspRemoveVarIndex(QSPVar *var, int indexPos);
INLINE void qspRemoveArrayItem(QSPVar *var, int index);
INLINE QSPVar *qspGetVarData(QSPCachedTarget *target, QSPVariant *index, QSP_BOOL toCreate);
INLINE QSP_BOOL qspGetVarValueByReference(QSPVar *var, int ind, QSP_TINYINT baseType, QSPVariant *res);
INLINE void qspResetVarValue(QSPCachedTarget *target);
INLINE void qspSetVarValueByReference(QSPVar *var, int ind, QSPVariant *val);
INLINE void qspSetVarValueByIndex(QSPString varName, QSPVariant index, QSPVariant *val);
INLINE void qspSetFirstVarValue(QSPString varName, QSPVariant *val);
INLINE void qspSetVarValue(QSPCachedTarget *target, QSPVariant *val, QSP_CHAR op);
INLINE void qspMoveTupleToArray(QSPVar *dest, QSPTuple *src, int start, int count);
INLINE void qspCopyArray(QSPVar *dest, QSPVar *src, int start, int count);
INLINE void qspSortArray(QSPVar *var, QSP_TINYINT baseValType, QSP_BOOL isAscending);
INLINE void qspSetVarsValues(QSPCachedAssignment *assignment, QSPVariant *v, QSP_CHAR op);

void qspInitVarTypes(void)
{
    int i;
    for (i = 0; i < sizeof(qspSpecToBaseTypeTable); ++i)
        qspSpecToBaseTypeTable[i] = QSP_TYPE_NUM;

    qspSpecToBaseTypeTable[QSP_NUMTYPE_CHAR] = QSP_TYPE_NUM;
    qspSpecToBaseTypeTable[QSP_STRTYPE_CHAR] = QSP_TYPE_STR;
    qspSpecToBaseTypeTable[QSP_TUPLETYPE_CHAR] = QSP_TYPE_TUPLE;
}

INLINE int qspValuePositionsAscCompare(const void *arg1, const void *arg2)
{
    return qspVariantsCompare(*(QSPVariant **)arg1, *(QSPVariant **)arg2); /* base types of values should be the same */
}

INLINE int qspValuePositionsDescCompare(const void *arg1, const void *arg2)
{
    return qspVariantsCompare(*(QSPVariant **)arg2, *(QSPVariant **)arg1); /* base types of values should be the same */
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
        scope->VarSlots = 0;
        scope->VarsCount = 0;
        scope->Capacity = 0;
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

INLINE QSPVarSlot *qspNewVarSlots(int capacity)
{
    int i;
    QSPVarSlot *slots = (QSPVarSlot *)malloc(capacity * sizeof(QSPVarSlot));
    for (i = 0; i < capacity; ++i)
        slots[i].Name = qspNullString;
    return slots;
}

void qspInitVarsScope(QSPVarsScope *scope, int capacity)
{
    int namesCapacity = capacity * QSP_VARSNAMECHARSPERSLOT;
    scope->VarsCount = 0;
    scope->Capacity = capacity;
    scope->VarSlots = qspNewVarSlots(capacity);
    scope->NamesLen = 0;
    scope->NamesCapacity = namesCapacity;
    scope->Names = (QSP_CHAR *)malloc(namesCapacity * sizeof(QSP_CHAR));
}

void qspClearVarsScope(QSPVarsScope *scope)
{
    /* Remove all variables & free the buffers */
    if (scope->VarSlots)
    {
        qspClearVars(scope);
        free(scope->VarSlots);
        free(scope->Names);
    }
}

void qspClearVars(QSPVarsScope *scope)
{
    /* Remove all variables & keep the buffers */
    QSPVarSlot *slot = scope->VarSlots;
    while (scope->VarsCount > 0)
    {
        if (slot->Name.Str)
        {
            qspEmptyVar(&slot->Var);
            slot->Name = qspNullString;
            --scope->VarsCount;
        }
        ++slot;
    }
    scope->NamesLen = 0;
}

void qspClearLocalVarsScopes(QSPVarsScopeChunk *chunk)
{
    QSPVarsScopeChunk *parentChunk;
    while (chunk)
    {
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

INLINE QSPVarSlot *qspGetVarSlot(QSPVarsScope *scope, QSPString name, unsigned int nameHash)
{
    QSPVarSlot *slot;
    int mask = QSP_CAPACITYMASK(scope->Capacity), ind = (int)(nameHash & mask);
    while ((slot = scope->VarSlots + ind)->Name.Str && (slot->NameHash != nameHash || qspStrsCompare(slot->Name, name)))
        ind = (ind + 1) & mask;
    return slot;
}

INLINE QSPVarSlot *qspGetEmptyVarSlot(QSPVarsScope *scope, unsigned int nameHash)
{
    int mask = QSP_CAPACITYMASK(scope->Capacity), ind = (int)(nameHash & mask);
    while (scope->VarSlots[ind].Name.Str) ind = (ind + 1) & mask;
    return scope->VarSlots + ind;
}

INLINE void qspResizeVarsScope(QSPVarsScope *scope, int newCapacity)
{
    QSPVarSlot *slot, *oldSlot, *oldSlots = scope->VarSlots;
    int i, oldCapacity = scope->Capacity;
    scope->Capacity = newCapacity;
    scope->VarSlots = qspNewVarSlots(newCapacity);
    for (i = 0, oldSlot = oldSlots; i < oldCapacity; ++i, ++oldSlot)
    {
        if (oldSlot->Name.Str)
        {
            slot = qspGetEmptyVarSlot(scope, oldSlot->NameHash);
            *slot = *oldSlot;
        }
    }
    free(oldSlots);
}

INLINE void qspResizeVarsNames(QSPVarsScope *scope, int newCapacity)
{
    QSPVarSlot *slot;
    int count = scope->Capacity;
    /* The old names have to stay valid until the slots are updated */
    QSP_CHAR *names = (QSP_CHAR *)malloc(newCapacity * sizeof(QSP_CHAR));
    memcpy(names, scope->Names, scope->NamesLen * sizeof(QSP_CHAR));
    for (slot = scope->VarSlots; count > 0; --count, ++slot)
    {
        if (slot->Name.Str)
            slot->Name = qspStringFromLen(names + (slot->Name.Str - scope->Names), qspStrLen(slot->Name));
    }
    free(scope->Names);
    scope->Names = names;
    scope->NamesCapacity = newCapacity;
}

INLINE QSPVar *qspCreateNewVar(QSPVarsScope *scope, QSPString name, unsigned int nameHash)
{
    QSPVarSlot *slot;
    int newNamesLen = scope->NamesLen + qspStrLen(name);
    if (scope->VarsCount * 2 >= scope->Capacity)
        qspResizeVarsScope(scope, scope->Capacity * 2);
    if (newNamesLen > scope->NamesCapacity)
        qspResizeVarsNames(scope, newNamesLen * 2);
    slot = qspGetEmptyVarSlot(scope, nameHash);
    slot->Name = qspCopyToText(scope->Names + scope->NamesLen, name);
    slot->NameHash = nameHash;
    scope->NamesLen = newNamesLen;
    qspInitVarData(&slot->Var);
    ++scope->VarsCount;
    return &slot->Var;
}

INLINE QSPVar *qspAddVarToLocals(QSPString name)
{
    unsigned int nameHash;
    QSPVarsScope *scope;
    QSPVarSlot *slot;
    name = qspPrepareVarName(name, &nameHash);
    if (qspIsEmpty(name))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return 0;
    }

    if (qspCurrentLocalVars)
        scope = qspCurrentLocalVars->Slots + qspCurrentLocalVars->SlotsCount - 1;
    else
        scope = qspAllocateLocalScope();

    if (!scope->VarSlots)
        qspInitVarsScope(scope, QSP_VARSLOCALCAPACITY); /* init the scope the first time it's used */

    /* Check if the variable already exists in the current scope */
    slot = qspGetVarSlot(scope, name, nameHash);
    if (slot->Name.Str) return &slot->Var;

    /* It doesn't exist yet, so we have to add it */
    if (scope->VarsCount >= QSP_MAXLOCALVARS)
    {
        qspSetError(QSP_ERR_TOOMANYVARS);
        return 0;
    }
    return qspCreateNewVar(scope, name, nameHash);
}

QSPVar *qspAddVarToScope(QSPVarsScope *scope, QSPString name)
{
    /* Special & loaded variables: the name isn't validated or looked up */
    unsigned int nameHash = qspGetTextHash(name);
    return qspCreateNewVar(scope, name, nameHash);
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
    if (!scope->VarSlots)
        qspInitVarsScope(scope, QSP_VARSLOCALCAPACITY); /* init the uninitialized scope */

    varArgs = qspAddVarToScope(scope, QSP_STATIC_STR(QSP_VARARGS));
    qspSetVarValuesByReference(varArgs, args, count, toMove);

    qspAddVarToScope(scope, QSP_STATIC_STR(QSP_VARRES));

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
    unsigned int nameHash;
    QSPVarsScopeChunk *chunk;
    QSPVarsScope *scope;
    QSPVarSlot *slot;
    name = qspPrepareVarName(name, &nameHash);
    if (qspIsEmpty(name))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return 0;
    }

    /* Check all local scopes starting the latest */
    chunk = qspCurrentLocalVars;
    while (chunk)
    {
        scope = chunk->Slots + chunk->SlotsCount;
        while (scope > chunk->Slots)
        {
            --scope;
            if (scope->VarsCount)
            {
                slot = qspGetVarSlot(scope, name, nameHash);
                if (slot->Name.Str) return &slot->Var;
            }
        }
        chunk = chunk->ParentChunk;
    }

    /* Check the global scope */
    slot = qspGetVarSlot(&qspGlobalVars, name, nameHash);
    if (slot->Name.Str) return &slot->Var;

    if (toCreate)
    {
        /* Create it in the global scope */
        if (qspGlobalVars.VarsCount >= QSP_MAXGLOBALVARS)
        {
            qspSetError(QSP_ERR_TOOMANYVARS);
            return 0;
        }
        return qspCreateNewVar(&qspGlobalVars, name, nameHash);
    }
    return &qspNullVar;
}

INLINE int *qspNewVarIndicesSlots(int capacity)
{
    int i, *slots = (int *)malloc(capacity * sizeof(int));
    for (i = 0; i < capacity; ++i)
        slots[i] = -1;
    return slots;
}

INLINE int *qspGetVarIndexSlot(QSPVar *var, QSPString key, unsigned int hash)
{
    QSPVarIndex *curIndex;
    int *slot, mask = QSP_CAPACITYMASK(var->IndsCapacity * 2), ind = (int)(hash & mask);
    while (*(slot = var->IndsSlots + ind) >= 0)
    {
        curIndex = var->Indices + *slot;
        if (curIndex->Hash == hash && qspStrsEqual(curIndex->Str, key))
            return slot;
        ind = (ind + 1) & mask;
    }
    return slot;
}

INLINE int *qspGetEmptyVarIndexSlot(QSPVar *var, unsigned int hash)
{
    int mask = QSP_CAPACITYMASK(var->IndsCapacity * 2), ind = (int)(hash & mask);
    while (var->IndsSlots[ind] >= 0)
        ind = (ind + 1) & mask;
    return var->IndsSlots + ind;
}

INLINE int *qspGetVarIndexSlotByPos(QSPVar *var, int indexPos)
{
    int mask = QSP_CAPACITYMASK(var->IndsCapacity * 2), ind = (int)(var->Indices[indexPos].Hash & mask);
    while (var->IndsSlots[ind] != indexPos)
        ind = (ind + 1) & mask;
    return var->IndsSlots + ind;
}

INLINE void qspResizeVarIndices(QSPVar *var, int newCapacity)
{
    int i, *slot;
    var->IndsCapacity = newCapacity;
    var->Indices = (QSPVarIndex *)realloc(var->Indices, newCapacity * sizeof(QSPVarIndex));
    free(var->IndsSlots);
    var->IndsSlots = qspNewVarIndicesSlots(newCapacity * 2); /* the slots table stays at most half full */
    for (i = 0; i < var->IndsCount; ++i)
    {
        slot = qspGetEmptyVarIndexSlot(var, var->Indices[i].Hash);
        *slot = i;
    }
}

INLINE void qspRemoveVarIndex(QSPVar *var, int indexPos)
{
    int lastIndexPos, mask, curInd, curIndexPos, *slot, *slots;
    /* Remove the index & empty the slot */
    qspFreeString(&var->Indices[indexPos].Str);
    slot = qspGetVarIndexSlotByPos(var, indexPos);
    *slot = -1;
    /* Re-insert the following indices up to the next empty slot, lookups stop at empty slots */
    slots = var->IndsSlots;
    mask = QSP_CAPACITYMASK(var->IndsCapacity * 2);
    curInd = ((int)(slot - slots) + 1) & mask;
    while ((curIndexPos = slots[curInd]) >= 0)
    {
        slots[curInd] = -1;
        slot = qspGetEmptyVarIndexSlot(var, var->Indices[curIndexPos].Hash);
        *slot = curIndexPos;
        curInd = (curInd + 1) & mask;
    }
    /* Keep the indices dense, the last one fills the gap */
    lastIndexPos = var->IndsCount - 1;
    if (indexPos != lastIndexPos)
    {
        var->Indices[indexPos] = var->Indices[lastIndexPos]; /* fill the gap */
        slot = qspGetVarIndexSlotByPos(var, lastIndexPos);
        *slot = indexPos; /* its slot points to the new position */
    }
    var->IndsCount = lastIndexPos;
}

INLINE void qspRemoveArrayItem(QSPVar *var, int index)
{
    int count, removedIndexPos;
    QSPVarIndex *ind;
    if (index < 0 || index >= var->ValsCount) return;
    qspFreeVariant(var->Values + index);
    var->ValsCount--;
    memmove(var->Values + index, var->Values + index + 1, (var->ValsCount - index) * sizeof(QSPVariant));
    /* Update positions of items, they aren't ordered */
    count = var->IndsCount;
    for (ind = var->Indices; count > 0 && ind->Index != index; --count, ++ind)
        ind->Index -= (ind->Index > index);
    /* Update the following indices, then remove the index of the removed item */
    if (count > 0)
    {
        removedIndexPos = (int)(ind - var->Indices);
        ++ind;
        while (--count > 0)
        {
            ind->Index -= (ind->Index > index);
            ++ind;
        }
        qspRemoveVarIndex(var, removedIndexPos);
    }
}

QSPVarIndex *qspAddVarIndex(QSPVar *var, unsigned int hash)
{
    int *slot;
    QSPVarIndex *ind;
    if (var->IndsCount >= var->IndsCapacity)
    {
        int newCapacity = (var->IndsCapacity ? var->IndsCapacity * 2 : QSP_VARSINDICESCAPACITY);
        qspResizeVarIndices(var, newCapacity);
    }
    slot = qspGetEmptyVarIndexSlot(var, hash);
    *slot = var->IndsCount;
    ind = var->Indices + var->IndsCount;
    ind->Hash = hash;
    ++var->IndsCount;
    return ind;
}

int qspGetVarIndex(QSPVar *var, QSPVariant index, QSP_BOOL toCreate)
{
    unsigned int hash;
    QSPString key;
    if (QSP_ISNUM(index.Type))
        return QSP_TOINT(QSP_NUM(index));
    key = qspGetVariantAsIndexString(&index);
    qspUpperStr(&key);
    hash = qspGetTextHash(key);
    if (var->IndsCount > 0)
    {
        int *slot = qspGetVarIndexSlot(var, key, hash);
        if (*slot >= 0)
        {
            qspFreeString(&key);
            return var->Indices[*slot].Index;
        }
    }
    if (toCreate)
    {
        QSPVarIndex *ind = qspAddVarIndex(var, hash);
        ind->Str = qspCopyToNewText(key); /* get exact string to save memory */
        ind->Index = var->ValsCount; /* point to the new array item */
        qspFreeString(&key);
        return ind->Index;
    }
    qspFreeString(&key);
    return -1;
}

INLINE QSPVar *qspGetVarData(QSPCachedTarget *target, QSPVariant *index, QSP_BOOL toCreate)
{
    QSPVar *var;
    int oldLocationState;
    switch (target->Index.Type)
    {
    case qspArgNone: /* plain variable */
        var = qspVarReference(target->Name, toCreate);
        if (var) *index = qspNumVariant(0);
        return var;
    case qspArgEmpty: /* x[] means a new item */
        var = qspVarReference(target->Name, toCreate);
        if (var) *index = qspNumVariant(var->ValsCount);
        return var;
    }
    /* x[expr], the index code can remove variables, so it goes first */
    oldLocationState = qspLocationState;
    *index = qspCalculateArgValue(&target->Index);
    if (qspLocationState != oldLocationState) return 0;
    var = qspVarReference(target->Name, toCreate);
    if (var) return var;
    qspFreeVariant(index);
    return 0;
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

INLINE void qspResetVarValue(QSPCachedTarget *target)
{
    int arrIndex;
    QSPVariant index;
    QSPVar *var = qspGetVarData(target, &index, QSP_FALSE); /* missing items stay empty, so we don't create them */
    if (!var) return;
    arrIndex = qspGetVarIndex(var, index, QSP_FALSE);
    qspFreeVariant(&index);
    if (arrIndex >= 0 && arrIndex < var->ValsCount)
    {
        QSPVariant *curValue = var->Values + arrIndex;
        if (QSP_ISDEF(curValue->Type))
        {
            qspFreeVariant(curValue);
            qspInitVariant(curValue, QSP_TYPE_UNDEF);
        }
    }
}

INLINE void qspSetVarValueByReference(QSPVar *var, int ind, QSPVariant *val)
{
    /* The value has to be converted to the type of the variable already */
    int oldCount = var->ValsCount;
    if (ind >= oldCount)
    {
        /* A new item */
        QSPVariant *curValue;
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
    if (!qspConvertVariantTo(val, varType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return;
    }
    arrIndex = qspGetVarIndex(var, index, QSP_TRUE);
    qspSetVarValueByReference(var, arrIndex, val);
}

INLINE void qspSetFirstVarValue(QSPString varName, QSPVariant *val)
{
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_TRUE);
    if (!var) return;
    varType = qspGetVarType(varName);
    if (!qspConvertVariantTo(val, varType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return;
    }
    qspSetVarValueByReference(var, 0, val);
}

INLINE void qspSetVarValue(QSPCachedTarget *target, QSPVariant *val, QSP_CHAR op)
{
    QSPVariant index;
    QSP_TINYINT varType;
    int arrIndex = -1;
    QSPVar *var = qspGetVarData(target, &index, QSP_TRUE);
    if (!var) return;
    varType = qspGetVarType(target->Name);
    if (op != QSP_EQUAL_CHAR)
    {
        /* Replace the value with its combination with the current value */
        QSPVariant oldVal, res;
        arrIndex = qspGetVarIndex(var, index, QSP_FALSE);
        qspGetVarValueByReference(var, arrIndex, varType, &oldVal);
        if (!qspAutoConvertCombine(&oldVal, val, op, &res))
        {
            qspFreeVariant(&oldVal);
            qspFreeVariant(&index);
            return;
        }
        qspFreeVariant(&oldVal);
        qspFreeVariant(val);
        qspMoveToNewVariant(val, &res);
    }
    /* We create a new index only when the value can be assigned */
    if (!qspConvertVariantTo(val, varType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        qspFreeVariant(&index);
        return;
    }
    if (arrIndex < 0) arrIndex = qspGetVarIndex(var, index, QSP_TRUE);
    qspFreeVariant(&index);
    qspSetVarValueByReference(var, arrIndex, val);
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
    QSPVarIndex *destIndex;
    int i, itemsToCopy, srcIndsCount, newInd;
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
    srcIndsCount = src->IndsCount;
    if ((src->ValsCount - itemsToCopy) * 8 < srcIndsCount) /* skipped items bound the indices to remove, break-even is about 1/8 */
    {
        /* Almost all indices fit, copy the table & remove the rest */
        dest->Indices = (QSPVarIndex *)malloc(src->IndsCapacity * sizeof(QSPVarIndex));
        dest->IndsSlots = (int *)malloc((src->IndsCapacity * 2) * sizeof(int));
        memcpy(dest->Indices, src->Indices, srcIndsCount * sizeof(QSPVarIndex));
        memcpy(dest->IndsSlots, src->IndsSlots, (src->IndsCapacity * 2) * sizeof(int));
        dest->IndsCount = srcIndsCount;
        dest->IndsCapacity = src->IndsCapacity;
        /* Go backwards, every removal moves the last index into its gap */
        i = srcIndsCount;
        while (--i >= 0)
        {
            destIndex = dest->Indices + i;
            newInd = destIndex->Index - start;
            if (newInd >= 0 && newInd < itemsToCopy)
            {
                destIndex->Str = qspCopyToNewText(destIndex->Str);
                destIndex->Index = newInd;
            }
            else
            {
                destIndex->Str = qspNullString; /* the string belongs to source */
                qspRemoveVarIndex(dest, i);
            }
        }
    }
    else if (srcIndsCount > 0)
    {
        /* Allocate the final table at once */
        int maxCount, capacity = QSP_VARSINDICESCAPACITY;
        QSPVarIndex *srcIndex = src->Indices;
        maxCount = (itemsToCopy < srcIndsCount ? itemsToCopy : srcIndsCount);
        while (capacity < maxCount) capacity *= 2;
        qspResizeVarIndices(dest, capacity);
        for (i = 0; i < srcIndsCount; ++i, ++srcIndex)
        {
            newInd = srcIndex->Index - start;
            if (newInd >= 0 && newInd < itemsToCopy)
            {
                destIndex = qspAddVarIndex(dest, srcIndex->Hash);
                destIndex->Str = qspCopyToNewText(srcIndex->Str);
                destIndex->Index = newInd;
            }
        }
    }
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
    var->ValsCapacity = valsCount;
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

INLINE void qspSetVarsValues(QSPCachedAssignment *assignment, QSPVariant *v, QSP_CHAR op)
{
    int i, oldLocationState, varsCount = assignment->TargetsCount;
    QSPCachedTarget *targets = assignment->Targets;
    if (varsCount == 1)
    {
        qspSetVarValue(targets, v, op);
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
                    qspSetVarValue(targets + i, QSP_PTUPLE(v).Vals + i, op);
                    if (qspLocationState != oldLocationState)
                        return;
                }
                /* Only 1 variable left, fill it with a tuple containing all the values left */
                v2 = qspTupleVariant(qspMoveToNewTuple(QSP_PTUPLE(v).Vals + i, QSP_PTUPLE(v).ValsCount - i));
                qspSetVarValue(targets + lastVarIndex, &v2, op);
                qspFreeVariant(&v2);
            }
            else
            {
                /* Assign all values to the variables */
                for (i = 0; i < valuesCount; ++i)
                {
                    qspSetVarValue(targets + i, QSP_PTUPLE(v).Vals + i, op);
                    if (qspLocationState != oldLocationState)
                        return;
                }
                /* No values left, reset the rest of vars with default values */
                while (i < varsCount)
                {
                    qspResetVarValue(targets + i);
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
        qspSetVarValue(targets, v, op);
        if (qspLocationState != oldLocationState)
            return;
        for (i = 1; i < varsCount; ++i)
        {
            qspResetVarValue(targets + i);
            if (qspLocationState != oldLocationState)
                return;
        }
        break;
    }
}

void qspStatementSetVarsValues(QSPCachedStat *stat)
{
    QSPVariant v;
    QSPCachedAssignment *assignment;
    int oldLocationState;
    assignment = stat->Data.Assignment;
    oldLocationState = qspLocationState;
    v = qspCalculateArgValue(&assignment->Value);
    if (qspLocationState != oldLocationState) return;
    qspSetVarsValues(assignment, &v, assignment->Operation);
    qspFreeVariant(&v);
}

void qspStatementLocal(QSPCachedStat *stat)
{
    int i;
    QSPVariant v;
    QSPCachedTarget *target;
    QSPCachedAssignment *assignment = stat->Data.Assignment;
    if (assignment->Operation)
    {
        int oldLocationState = qspLocationState;
        /* We have to evaluate expression before allocation of local vars */
        v = qspCalculateArgValue(&assignment->Value);
        if (qspLocationState != oldLocationState) return;
    }
    else
        v = qspGetEmptyVariant(QSP_TYPE_UNDEF);
    for (i = assignment->TargetsCount, target = assignment->Targets; i > 0; --i, ++target)
    {
        if (!qspAddVarToLocals(target->Name))
        {
            qspFreeVariant(&v);
            return;
        }
    }
    if (assignment->Operation)
    {
        qspSetVarsValues(assignment, &v, QSP_EQUAL_CHAR);
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
