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
#include "memory.h"
#include "regexp.h"
#include "statements.h"

QSPVarSlot qspNullVarSlot;
QSPVarsScope qspGlobalVars;
QSPVarsScopeChunk *qspCurrentLocalVars = 0;

QSP_TINYINT qspSpecToBaseTypeTable[128];

INLINE int qspValuePositionsAscCompare(const void *arg1, const void *arg2);
INLINE int qspValuePositionsDescCompare(const void *arg1, const void *arg2);
INLINE QSP_BOOL qspGetArrayValsType(QSPVar *var, QSP_TINYINT *valsType);
INLINE QSPVarSlot *qspNewVarSlots(int capacity);
INLINE QSPVarSlot *qspGetVarSlot(QSPVarsScope *scope, QSPString name, unsigned int nameHash);
INLINE QSPVarSlot *qspGetEmptyVarSlot(QSPVarsScope *scope, unsigned int nameHash);
INLINE void qspResizeVarsScope(QSPVarsScope *scope, int newCapacity);
INLINE void qspResizeVarsNames(QSPVarsScope *scope, int newCapacity);
INLINE QSPVarSlot *qspCreateNewVar(QSPVarsScope *scope, QSPString name, unsigned int nameHash);
INLINE void qspReleaseSharedVar(QSPSharedVar *sharedVar);
INLINE void qspEmptyVarSlot(QSPVarSlot *slot);
INLINE QSPSharedVar *qspShareVarByName(QSPString name);
INLINE void qspMoveReferenceToVarSlot(QSPVarSlot *slot, QSPSharedVar *sharedVar);
INLINE QSPVarSlot *qspAddVarToLocals(QSPString name);
INLINE void qspInitVarValues(QSPVar *var, QSPVariant *vals, int count, QSP_BOOL toMove);
INLINE int *qspGetVarKeySlot(QSPVar *var, QSPVariant *key, unsigned int hash);
INLINE int *qspGetEmptyVarKeySlot(QSPVar *var, unsigned int hash);
INLINE int *qspGetVarKeySlotByIndex(QSPVar *var, int keyIndex);
INLINE void qspRebuildVarKeysSlots(QSPVar *var);
INLINE void qspResizeVarKeys(QSPVar *var, int newCapacity);
INLINE void qspRemoveVarKey(QSPVar *var, int keyIndex);
INLINE void qspMoveVarKey(QSPVar *var, int keyIndex, int newKeyIndex);
INLINE void qspMoveVarItem(QSPVar *var, int valIndex, int newIndex);
INLINE void qspRemoveVarItem(QSPVar *var, int valIndex);
INLINE int qspGetVarKeyLowerBound(QSPVar *var, int valIndex);
INLINE QSP_BOOL qspIsPosIndex(QSPVariant *index);
INLINE QSP_BOOL qspResolveTarget(QSPResolvedTarget *resolved, QSPCachedTarget *target, QSP_TINYINT mode);
INLINE void qspFreeResolvedTarget(QSPResolvedTarget *resolved, QSPCachedTarget *target);
INLINE void qspGetVarItemValue(QSPVar *var, int valIndex, QSP_TINYINT varType, QSPVariant *res);
INLINE void qspGetVarItemFieldValue(QSPVar *var, int valIndex, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *res);
INLINE void qspResetTargetValue(QSPCachedTarget *target);
INLINE QSPVariant *qspReserveVarItem(QSPVar *var, int valIndex);
INLINE void qspSetVarItemValue(QSPVar *var, int valIndex, QSPVariant *val);
INLINE void qspSetVarItemFieldValue(QSPVar *var, int valIndex, QSPField *fields, int count, QSPVariant *val);
INLINE void qspSetVarFieldValue(QSPVar *var, QSPVariant index, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *val, QSP_CHAR op);
INLINE void qspSetTargetValue(QSPCachedTarget *target, QSPVariant *val, QSP_CHAR op);
INLINE void qspMoveTupleToArray(QSPVar *dest, QSPTuple *src, int start, int count);
INLINE void qspCopyArray(QSPVar *dest, QSPVar *src, int start, int count);
INLINE void qspSortArray(QSPVar *var, QSP_TINYINT varType, QSP_BOOL isAscending);
INLINE void qspSetTargetsValues(QSPCachedAssignment *assignment, QSPVariant *val, QSP_CHAR op);

void qspInitVarTypes(void)
{
    int i;
    for (i = 0; i < sizeof(qspSpecToBaseTypeTable); ++i)
        qspSpecToBaseTypeTable[i] = QSP_TYPE_UNDEF; /* no prefix: values as they are */

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

INLINE QSP_BOOL qspGetArrayValsType(QSPVar *var, QSP_TINYINT *valsType)
{
    QSP_TINYINT type = QSP_TYPE_UNDEF;
    QSPVariant *curValue = var->Values, *end = var->Values + var->ValsCount;
    while (curValue < end)
    {
        if (QSP_ISDEF(curValue->Type))
        {
            if (!QSP_ISDEF(type))
                type = QSP_BASETYPE(curValue->Type);
            else if (QSP_BASETYPE(curValue->Type) != type)
            {
                *valsType = QSP_TYPE_UNDEF;
                return QSP_FALSE;
            }
        }
        ++curValue;
    }
    *valsType = type;
    return QSP_TRUE;
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
    int namesCapacity = capacity * QSP_VARSNAMECHARSPERVAR;
    scope->VarsCount = 0;
    scope->Capacity = capacity;
    scope->VarSlots = qspNewVarSlots(QSP_SLOTSCOUNT(capacity));
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
            qspEmptyVarSlot(slot);
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
    int mask = QSP_SLOTSMASK(scope->Capacity), ind = (int)(nameHash & mask);
    while ((slot = scope->VarSlots + ind)->Name.Str && (slot->NameHash != nameHash || !qspStrsEqual(slot->Name, name)))
        ind = (ind + 1) & mask;
    return slot;
}

INLINE QSPVarSlot *qspGetEmptyVarSlot(QSPVarsScope *scope, unsigned int nameHash)
{
    int mask = QSP_SLOTSMASK(scope->Capacity), ind = (int)(nameHash & mask);
    while (scope->VarSlots[ind].Name.Str) ind = (ind + 1) & mask;
    return scope->VarSlots + ind;
}

INLINE void qspResizeVarsScope(QSPVarsScope *scope, int newCapacity)
{
    int oldSlotsCount = QSP_SLOTSCOUNT(scope->Capacity);
    QSPVarSlot *oldSlots = scope->VarSlots;
    scope->Capacity = newCapacity;
    scope->VarSlots = qspNewVarSlots(QSP_SLOTSCOUNT(newCapacity));
    if (oldSlots)
    {
        int i;
        QSPVarSlot *slot, *oldSlot = oldSlots;
        for (i = 0; i < oldSlotsCount; ++i, ++oldSlot)
        {
            if (oldSlot->Name.Str)
            {
                slot = qspGetEmptyVarSlot(scope, oldSlot->NameHash);
                *slot = *oldSlot;
            }
        }
        free(oldSlots);
    }
}

INLINE void qspResizeVarsNames(QSPVarsScope *scope, int newCapacity)
{
    QSP_CHAR *oldNames = scope->Names;
    scope->NamesCapacity = newCapacity;
    scope->Names = (QSP_CHAR *)malloc(newCapacity * sizeof(QSP_CHAR));
    if (oldNames)
    {
        int i, nameOffset;
        QSPVarSlot *slot = scope->VarSlots;
        /* The old names have to stay valid until the slots are updated */
        memcpy(scope->Names, oldNames, scope->NamesLen * sizeof(QSP_CHAR));
        for (i = QSP_SLOTSCOUNT(scope->Capacity); i > 0; --i, ++slot)
        {
            if (slot->Name.Str)
            {
                nameOffset = (int)(slot->Name.Str - oldNames);
                slot->Name = qspStringFromLen(scope->Names + nameOffset, qspStrLen(slot->Name));
            }
        }
        free(oldNames);
    }
}

INLINE QSPVarSlot *qspCreateNewVar(QSPVarsScope *scope, QSPString name, unsigned int nameHash)
{
    QSPVarSlot *slot;
    int newNamesLen = scope->NamesLen + qspStrLen(name);
    if (scope->VarsCount >= scope->Capacity)
        qspResizeVarsScope(scope, scope->Capacity * 2);
    if (newNamesLen > scope->NamesCapacity)
        qspResizeVarsNames(scope, newNamesLen * 2);
    slot = qspGetEmptyVarSlot(scope, nameHash);
    slot->Name = qspCopyToText(scope->Names + scope->NamesLen, name);
    slot->NameHash = nameHash;
    slot->IsRef = QSP_FALSE;
    scope->NamesLen = newNamesLen;
    qspInitVarData(&slot->Data.Var);
    ++scope->VarsCount;
    return slot;
}

INLINE void qspReleaseSharedVar(QSPSharedVar *sharedVar)
{
    if (--sharedVar->RefsCount == 0)
    {
        qspEmptyVar(&sharedVar->Var);
        free(sharedVar);
    }
}

INLINE void qspEmptyVarSlot(QSPVarSlot *slot)
{
    if (slot->IsRef)
    {
        qspReleaseSharedVar(slot->Data.Shared);
        slot->IsRef = QSP_FALSE;
        qspInitVarData(&slot->Data.Var);
    }
    else
        qspEmptyVar(&slot->Data.Var);
}

INLINE QSPSharedVar *qspShareVarByName(QSPString name)
{
    QSPSharedVar *sharedVar;
    QSPVarSlot *slot = qspVarSlotReference(name, QSP_REF_GETORCREATE);
    if (!slot) return 0;
    if (slot->IsRef)
        sharedVar = slot->Data.Shared;
    else
    {
        sharedVar = (QSPSharedVar *)malloc(sizeof(QSPSharedVar));
        qspMoveVar(&sharedVar->Var, &slot->Data.Var);
        sharedVar->RefsCount = 1;
        slot->IsRef = QSP_TRUE;
        slot->Data.Shared = sharedVar;
    }
    ++sharedVar->RefsCount; /* reference for the caller */
    return sharedVar;
}

INLINE void qspMoveReferenceToVarSlot(QSPVarSlot *slot, QSPSharedVar *sharedVar)
{
    /* Slot owns the reference */
    qspEmptyVarSlot(slot);
    slot->IsRef = QSP_TRUE;
    slot->Data.Shared = sharedVar;
}

INLINE QSPVarSlot *qspAddVarToLocals(QSPString name)
{
    unsigned int nameHash;
    QSPVarsScope *scope;
    QSPVarSlot *slot;
    name = qspPrepareVarName(name, &nameHash); /* validates the name */
    if (qspIsEmpty(name))
    {
        qspSetError(QSP_ERR_INCORRECTVARNAME);
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
    if (slot->Name.Str) return slot;

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
    /* Special variables: the name isn't validated or looked up */
    unsigned int nameHash = qspGetTextHash(name);
    QSPVarSlot *slot = qspCreateNewVar(scope, name, nameHash);
    return &slot->Data.Var;
}

INLINE void qspInitVarValues(QSPVar *var, QSPVariant *vals, int count, QSP_BOOL toMove)
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

    varArgs = qspAddVarToScope(scope, QSP_STATIC_STR(QSP_VAR_ARGS));
    qspInitVarValues(varArgs, args, count, toMove);

    qspAddVarToScope(scope, QSP_STATIC_STR(QSP_VAR_RESULT));

    return scope;
}

QSP_BOOL qspSetArgs(QSPVariant *args, int count, QSP_BOOL toMove)
{
    QSPVar *varArgs = qspVarReference(QSP_STATIC_STR(QSP_VAR_ARGS), QSP_REF_GETORCREATE);
    if (!varArgs) return QSP_FALSE;

    qspEmptyVar(varArgs);
    qspInitVarValues(varArgs, args, count, toMove);
    return QSP_TRUE;
}

QSP_BOOL qspApplyResult(QSPVariant *res)
{
    QSPVar *varRes = qspVarReference(QSP_STATIC_STR(QSP_VAR_RESULT), QSP_REF_GET);
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

QSPVarSlot *qspVarSlotReference(QSPString name, QSP_TINYINT mode)
{
    unsigned int nameHash;
    QSPVarsScopeChunk *chunk;
    QSPVarsScope *scope;
    QSPVarSlot *slot;
    name = qspPrepareVarName(name, &nameHash); /* validates the name */
    if (qspIsEmpty(name))
    {
        if (mode != QSP_REF_GETSILENTLY)
            qspSetError(QSP_ERR_INCORRECTVARNAME);
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
                if (slot->Name.Str) return slot;
            }
        }
        chunk = chunk->ParentChunk;
    }

    /* Check the global scope */
    slot = qspGetVarSlot(&qspGlobalVars, name, nameHash);
    if (slot->Name.Str) return slot;

    if (mode == QSP_REF_GETORCREATE)
    {
        /* Create it in the global scope */
        if (qspGlobalVars.VarsCount >= QSP_MAXGLOBALVARS)
        {
            qspSetError(QSP_ERR_TOOMANYVARS);
            return 0;
        }
        return qspCreateNewVar(&qspGlobalVars, name, nameHash);
    }
    return &qspNullVarSlot;
}

INLINE int *qspGetVarKeySlot(QSPVar *var, QSPVariant *key, unsigned int hash)
{
    QSPVarKey *curKey;
    int *slot, mask = QSP_SLOTSMASK(var->KeysCapacity), ind = (int)(hash & mask);
    while (*(slot = var->KeysSlots + ind) >= 0)
    {
        curKey = var->Keys + *slot;
        if (curKey->Hash == hash && qspVariantKeysEqual(&curKey->Key, key))
            return slot;
        ind = (ind + 1) & mask;
    }
    return slot;
}

INLINE int *qspGetEmptyVarKeySlot(QSPVar *var, unsigned int hash)
{
    int mask = QSP_SLOTSMASK(var->KeysCapacity), ind = (int)(hash & mask);
    while (var->KeysSlots[ind] >= 0)
        ind = (ind + 1) & mask;
    return var->KeysSlots + ind;
}

INLINE int *qspGetVarKeySlotByIndex(QSPVar *var, int keyIndex)
{
    int mask = QSP_SLOTSMASK(var->KeysCapacity), ind = (int)(var->Keys[keyIndex].Hash & mask);
    while (var->KeysSlots[ind] != keyIndex)
        ind = (ind + 1) & mask;
    return var->KeysSlots + ind;
}

INLINE void qspRebuildVarKeysSlots(QSPVar *var)
{
    int i, *slot, keysCount = var->KeysCount, slotsCount = QSP_SLOTSCOUNT(var->KeysCapacity);
    for (i = 0; i < slotsCount; ++i)
        var->KeysSlots[i] = -1;
    for (i = 0; i < keysCount; ++i)
    {
        slot = qspGetEmptyVarKeySlot(var, var->Keys[i].Hash);
        *slot = i;
    }
}

INLINE void qspResizeVarKeys(QSPVar *var, int newCapacity)
{
    var->KeysCapacity = newCapacity;
    var->Keys = (QSPVarKey *)realloc(var->Keys, newCapacity * sizeof(QSPVarKey));
    var->KeysSlots = (int *)realloc(var->KeysSlots, QSP_SLOTSCOUNT(newCapacity) * sizeof(int));
    qspRebuildVarKeysSlots(var);
}

QSPVarKey *qspAddVarKey(QSPVar *var, unsigned int hash, int valIndex)
{
    int *slot;
    QSPVarKey *key;
    if (var->KeysCount >= var->KeysCapacity)
    {
        int newCapacity = (var->KeysCapacity ? var->KeysCapacity * 2 : QSP_VARSKEYSCAPACITY);
        qspResizeVarKeys(var, newCapacity);
    }
    slot = qspGetEmptyVarKeySlot(var, hash);
    *slot = var->KeysCount; /* store the new key index */
    key = var->Keys + var->KeysCount;
    key->Hash = hash;
    key->Index = valIndex;
    ++var->KeysCount;
    return key;
}

INLINE void qspRemoveVarKey(QSPVar *var, int keyIndex)
{
    int lastKeyIndex, mask, curInd, curKeyIndex, *slot, *slots;
    if (keyIndex < 0 || keyIndex >= var->KeysCount) return;
    /* Remove the key & empty the slot */
    qspFreeVariant(&var->Keys[keyIndex].Key);
    slot = qspGetVarKeySlotByIndex(var, keyIndex);
    *slot = -1;
    /* Re-insert the following keys up to the next empty slot, lookups stop at empty slots */
    slots = var->KeysSlots;
    mask = QSP_SLOTSMASK(var->KeysCapacity);
    curInd = ((int)(slot - slots) + 1) & mask;
    while ((curKeyIndex = slots[curInd]) >= 0)
    {
        slots[curInd] = -1;
        slot = qspGetEmptyVarKeySlot(var, var->Keys[curKeyIndex].Hash);
        *slot = curKeyIndex;
        curInd = (curInd + 1) & mask;
    }
    /* The following keys move down, the slots store key indices & have to follow */
    lastKeyIndex = var->KeysCount - 1;
    if (lastKeyIndex - keyIndex <= var->KeysCapacity / 16)
    {
        /* Few keys follow, finding their slots is cheaper, break-even is about 1/16 */
        for (curKeyIndex = keyIndex + 1; curKeyIndex <= lastKeyIndex; ++curKeyIndex)
        {
            slot = qspGetVarKeySlotByIndex(var, curKeyIndex);
            --(*slot);
        }
    }
    else
    {
        /* Update all slots */
        int i, slotsCount = QSP_SLOTSCOUNT(var->KeysCapacity);
        for (i = 0; i < slotsCount; ++i)
            slots[i] -= (slots[i] > keyIndex);
    }
    memmove(var->Keys + keyIndex, var->Keys + keyIndex + 1, (lastKeyIndex - keyIndex) * sizeof(QSPVarKey));
    var->KeysCount = lastKeyIndex;
}

INLINE void qspMoveVarKey(QSPVar *var, int keyIndex, int newKeyIndex)
{
    /* It trusts the passed indices */
    QSPVarKey movedKey = var->Keys[keyIndex];
    int *keySlot = qspGetVarKeySlotByIndex(var, keyIndex);
    if (newKeyIndex < keyIndex)
    {
        /* Keys from new position to the old position move up */
        if (keyIndex - newKeyIndex <= var->KeysCapacity / 16)
        {
            int *curSlot, curKeyIndex;
            for (curKeyIndex = keyIndex - 1; curKeyIndex >= newKeyIndex; --curKeyIndex)
            {
                curSlot = qspGetVarKeySlotByIndex(var, curKeyIndex);
                ++(*curSlot);
            }
        }
        else
        {
            int i, lastKeyIndex = var->KeysCount - 1, *slots = var->KeysSlots, slotsCount = QSP_SLOTSCOUNT(var->KeysCapacity);
            if (keyIndex < lastKeyIndex)
            {
                for (i = 0; i < slotsCount; ++i)
                    slots[i] += (slots[i] >= newKeyIndex && slots[i] < keyIndex);
            }
            else
            {
                for (i = 0; i < slotsCount; ++i)
                    slots[i] += (slots[i] >= newKeyIndex);
            }
        }
        memmove(var->Keys + newKeyIndex + 1, var->Keys + newKeyIndex, (keyIndex - newKeyIndex) * sizeof(QSPVarKey));
    }
    else
    {
        /* Keys after the old position up to the new position move down */
        if (newKeyIndex - keyIndex <= var->KeysCapacity / 16)
        {
            int *curSlot, curKeyIndex;
            for (curKeyIndex = keyIndex + 1; curKeyIndex <= newKeyIndex; ++curKeyIndex)
            {
                curSlot = qspGetVarKeySlotByIndex(var, curKeyIndex);
                --(*curSlot);
            }
        }
        else
        {
            int i, lastKeyIndex = var->KeysCount - 1, *slots = var->KeysSlots, slotsCount = QSP_SLOTSCOUNT(var->KeysCapacity);
            if (newKeyIndex < lastKeyIndex)
            {
                for (i = 0; i < slotsCount; ++i)
                    slots[i] -= (slots[i] > keyIndex && slots[i] <= newKeyIndex);
            }
            else
            {
                for (i = 0; i < slotsCount; ++i)
                    slots[i] -= (slots[i] > keyIndex);
            }
        }
        memmove(var->Keys + keyIndex, var->Keys + keyIndex + 1, (newKeyIndex - keyIndex) * sizeof(QSPVarKey));
    }
    var->Keys[newKeyIndex] = movedKey;
    *keySlot = newKeyIndex;
}

INLINE void qspMoveVarItem(QSPVar *var, int valIndex, int newIndex)
{
    QSPVariant movedValue;
    int keyIndex, newKeyIndex;
    if (valIndex < 0 || newIndex < 0) return;
    if (valIndex >= var->ValsCount)
    {
        /* Prepare the value to move */
        QSPVariant undefinedValue = qspGetEmptyVariant(QSP_TYPE_UNDEF);
        valIndex = var->ValsCount;
        qspSetVarItemValue(var, valIndex, &undefinedValue);
    }
    if (newIndex == valIndex) return;
    if (newIndex >= var->ValsCount)
    {
        /* Prepare space for the new value */
        QSPVariant undefinedValue = qspGetEmptyVariant(QSP_TYPE_UNDEF);
        qspSetVarItemValue(var, newIndex, &undefinedValue);
    }
    keyIndex = qspGetVarKeyIndex(var, valIndex);
    movedValue = var->Values[valIndex];
    if (newIndex < valIndex)
    {
        /* Items from new position to the old position move up */
        int i, startKeyIndex, endKeyIndex;
        memmove(var->Values + newIndex + 1, var->Values + newIndex, (valIndex - newIndex) * sizeof(QSPVariant));
        startKeyIndex = qspGetVarKeyLowerBound(var, newIndex);
        endKeyIndex = qspGetVarKeyLowerBound(var, valIndex);
        newKeyIndex = startKeyIndex;
        for (i = startKeyIndex; i < endKeyIndex; ++i)
            var->Keys[i].Index++;
    }
    else
    {
        /* Items after the old position up to new position move down */
        int i, startKeyIndex, endKeyIndex;
        memmove(var->Values + valIndex, var->Values + valIndex + 1, (newIndex - valIndex) * sizeof(QSPVariant));
        startKeyIndex = qspGetVarKeyLowerBound(var, valIndex + 1);
        endKeyIndex = qspGetVarKeyLowerBound(var, newIndex + 1);
        newKeyIndex = endKeyIndex - 1;
        for (i = startKeyIndex; i < endKeyIndex; ++i)
            var->Keys[i].Index--;
    }
    var->Values[newIndex] = movedValue;
    if (keyIndex >= 0) /* -1 for keyless item */
    {
        qspMoveVarKey(var, keyIndex, newKeyIndex);
        var->Keys[newKeyIndex].Index = newIndex;
    }
}

INLINE void qspRemoveVarItem(QSPVar *var, int valIndex)
{
    int keyIndex, lastValIndex = var->ValsCount - 1;
    if (valIndex < 0 || valIndex > lastValIndex) return;
    /* Move the item to the end */
    qspMoveVarItem(var, valIndex, lastValIndex);
    /* Remove it */
    keyIndex = qspGetVarKeyIndex(var, lastValIndex);
    if (keyIndex >= 0) /* -1 for keyless item */
        qspRemoveVarKey(var, keyIndex);
    qspFreeVariant(var->Values + lastValIndex);
    var->ValsCount = lastValIndex;
}

INLINE int qspGetVarKeyLowerBound(QSPVar *var, int valIndex)
{
    /* Value index has to be validated already */
    int low, high, keysCount = var->KeysCount, keylessCount = var->ValsCount - keysCount;
    /* Search from valIndex - keylessCount to valIndex inclusive */
    low = (valIndex > keylessCount ? valIndex - keylessCount : 0);
    high = (valIndex < keysCount ? valIndex : keysCount);
    if (low < high)
    {
        int keyIndex;
        QSPVarKey *keys = var->Keys;
        /* Search only when keyed and keyless items are mixed on both sides of the value index */
        if (keys[low].Index >= valIndex) return low; /* all keys from low are at or after value index */
        if (keys[high - 1].Index < valIndex) return high; /* all keys before high are before value index */
        while (low < high)
        {
            keyIndex = (low + high) / 2;
            if (keys[keyIndex].Index < valIndex)
                low = keyIndex + 1;
            else
                high = keyIndex;
        }
    }
    return low; /* the first key at or after the item */
}

int qspGetVarKeyIndex(QSPVar *var, int valIndex)
{
    /* Value index has to be validated already */
    int keyIndex = qspGetVarKeyLowerBound(var, valIndex);
    if (keyIndex < var->KeysCount && var->Keys[keyIndex].Index == valIndex)
        return keyIndex;
    return -1;
}

INLINE QSP_BOOL qspIsPosIndex(QSPVariant *index)
{
    /* Numbers specify positions */
    switch (QSP_BASETYPE(index->Type))
    {
    case QSP_TYPE_UNDEF:
    case QSP_TYPE_NUM:
        return QSP_TRUE;
    }
    return QSP_FALSE;
}

int qspGetVarItemIndex(QSPVar *var, QSPVariant index, QSP_BOOL toCreate)
{
    unsigned int hash;
    if (qspIsPosIndex(&index))
        return QSP_TOINT(QSP_NUM(index));

    hash = qspGetVariantKeyHash(&index);
    if (var->KeysCount > 0)
    {
        int *slot = qspGetVarKeySlot(var, &index, hash);
        if (*slot >= 0) return var->Keys[*slot].Index;
    }
    if (toCreate)
    {
        QSPVarKey *key = qspAddVarKey(var, hash, var->ValsCount); /* point to the new array item */
        qspCopyToNewVariant(&key->Key, &index);
        return var->ValsCount;
    }
    return -1;
}

INLINE QSP_BOOL qspResolveTarget(QSPResolvedTarget *resolved, QSPCachedTarget *target, QSP_TINYINT mode)
{
    /* Init target fields no matter what mode we use */
    if (target->Path.Str && !target->Fields && !qspParseTargetFields(target)) return QSP_FALSE;
    if (target->DynamicFields)
        resolved->Fields = (QSPField *)qspAllocateMemory(target->FieldsCount * sizeof(QSPField));
    else
        resolved->Fields = target->Fields;

    /* The index & field names have to be calculated first */
    switch (target->Index.Type)
    {
    case QSP_ARG_NONE: /* plain variable */
        if (target->DynamicFields && !qspCalculateTargetFields(target, resolved->Fields, mode)) break;
        if (!(resolved->Var = qspVarReference(target->Name, mode))) break;
        resolved->Index = qspNumVariant(0);
        return QSP_TRUE;
    case QSP_ARG_EMPTY: /* x[] means a new item */
        if (target->DynamicFields && !qspCalculateTargetFields(target, resolved->Fields, mode)) break;
        if (!(resolved->Var = qspVarReference(target->Name, mode))) break;
        resolved->Index = qspNumVariant(resolved->Var->ValsCount);
        return QSP_TRUE;
    default: /* regular x[expr] */
        {
            int oldLocationState = qspLocationState;
            resolved->Index = qspCalculateArgValue(&target->Index);
            if (qspLocationState != oldLocationState) break;
            if (target->DynamicFields && !qspCalculateTargetFields(target, resolved->Fields, mode))
            {
                qspFreeVariant(&resolved->Index);
                break;
            }
            if (!(resolved->Var = qspVarReference(target->Name, mode)))
            {
                qspFreeVariant(&resolved->Index);
                break;
            }
            return QSP_TRUE;
        }
    }
    if (target->DynamicFields) qspReleaseMemory(resolved->Fields);
    return QSP_FALSE;
}

INLINE void qspFreeResolvedTarget(QSPResolvedTarget *resolved, QSPCachedTarget *target)
{
    if (target->DynamicFields) qspReleaseMemory(resolved->Fields);
    qspFreeVariant(&resolved->Index);
}

INLINE void qspGetVarItemValue(QSPVar *var, int valIndex, QSP_TINYINT varType, QSPVariant *res)
{
    QSPVariant *value = ((valIndex >= 0 && valIndex < var->ValsCount) ? var->Values + valIndex : 0);
    qspInitVariantFrom(res, value, varType);
}

INLINE void qspGetVarItemFieldValue(QSPVar *var, int valIndex, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *res)
{
    /* The type prefix of the name or the last field specifies the type of the value */
    QSPVariant *value = ((valIndex >= 0 && valIndex < var->ValsCount) ? var->Values + valIndex : 0);
    if (count && !QSP_ISDEF(varType)) varType = fields[count - 1].Value.Type;
    value = qspGetTupleFieldValueByPath(value, fields, count);
    qspInitVariantFrom(res, value, varType);
}

void qspGetVarValueByIndex(QSPString varName, QSPVariant index, QSPVariant *res)
{
    int arrIndex;
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
    if (!var) return;
    arrIndex = qspGetVarItemIndex(var, index, QSP_FALSE);
    varType = qspGetVarType(varName);
    qspGetVarItemValue(var, arrIndex, varType, res);
}

void qspGetFirstVarValue(QSPString varName, QSPVariant *res)
{
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
    if (!var) return;
    varType = qspGetVarType(varName);
    qspGetVarItemValue(var, 0, varType, res);
}

void qspGetLastVarValue(QSPString varName, QSPVariant *res)
{
    int arrIndex;
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
    if (!var) return;
    arrIndex = var->ValsCount - 1;
    varType = qspGetVarType(varName);
    qspGetVarItemValue(var, arrIndex, varType, res);
}

void qspGetVarFieldValueByIndex(QSPString varName, QSPVariant index, QSPField *fields, int count, QSPVariant *res)
{
    int arrIndex;
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
    if (!var) return;
    arrIndex = qspGetVarItemIndex(var, index, QSP_FALSE);
    varType = qspGetVarType(varName);
    qspGetVarItemFieldValue(var, arrIndex, varType, fields, count, res);
}

void qspGetFirstVarFieldValue(QSPString varName, QSPField *fields, int count, QSPVariant *res)
{
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
    if (!var) return;
    varType = qspGetVarType(varName);
    qspGetVarItemFieldValue(var, 0, varType, fields, count, res);
}

void qspGetLastVarFieldValue(QSPString varName, QSPField *fields, int count, QSPVariant *res)
{
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
    if (!var) return;
    varType = qspGetVarType(varName);
    qspGetVarItemFieldValue(var, var->ValsCount - 1, varType, fields, count, res);
}

QSP_BOOL qspGetFieldNameIndex(QSPString name, QSP_TINYINT mode, int *nameIndex)
{
    /* Any text except the empty one can be a field name */
    if (qspIsEmpty(name))
    {
        if (mode != QSP_REF_GETSILENTLY)
            qspSetError(QSP_ERR_INCORRECTFIELDNAME);
        return QSP_FALSE;
    }
    *nameIndex = qspGetNameIndex(name, (mode == QSP_REF_GETORCREATE));
    return QSP_TRUE;
}

QSP_BOOL qspInitPlainField(QSPField *field, QSPString name)
{
    QSPString fieldName = qspPrepareVarName(name, 0); /* validates the name */
    if (qspIsEmpty(fieldName) || qspIsInClass(*fieldName.Str, QSP_CHAR_TYPEPREFIX))
    {
        /* Field names can't double the type prefix */
        qspSetError(QSP_ERR_INCORRECTFIELDNAME);
        return QSP_FALSE;
    }
    field->Value = qspGetEmptyVariant(qspGetVarType(name)); /* type of value if the field is missing */
    return qspGetFieldNameIndex(fieldName, QSP_REF_GETORCREATE, &field->NameIndex);
}

QSP_BOOL qspInitQuotedField(QSPField *field, QSPString name, QSP_TINYINT fieldType)
{
    field->Value = qspGetEmptyVariant(fieldType); /* type of value if the field is missing */
    if (qspStrStr(name, QSP_STATIC_STR(QSP_LSUBEX)))
    {
        field->NameIndex = -1; /* calculated on every usage */
        return QSP_TRUE;
    }
    return qspGetFieldNameIndex(name, QSP_REF_GETORCREATE, &field->NameIndex);
}

QSP_BOOL qspParseFieldName(QSPString *s, QSPField *field, QSPString *code)
{
    QSPString name;
    QSP_CHAR *namePos = s->Str;
    /* Type prefix of a quoted name is outside the quotes */
    if (namePos < s->End && qspIsInClass(*namePos, QSP_CHAR_TYPEPREFIX)) ++namePos;
    if (namePos < s->End && qspIsInClass(*namePos, QSP_CHAR_QUOT))
    {
        QSP_TINYINT fieldType = qspGetVarType(*s);
        s->Str = namePos;
        if (!qspUnquoteToNewText(s, &name)) return QSP_FALSE;
        if (!qspInitQuotedField(field, name, fieldType))
        {
            qspFreeString(&name);
            return QSP_FALSE;
        }
        qspFreeString(&name);
        *code = qspStringFromPair(namePos, s->Str);
        return QSP_TRUE;
    }
    /* Plain name with its own type prefix */
    *code = qspNullString;
    name = qspParseToDelim(s, QSP_CHAR_DELIM | QSP_CHAR_FIELDREF);
    return qspInitPlainField(field, name);
}

int qspParseFieldsPath(QSPString *s, QSP_TINYINT varType, QSPField *fields, QSPString *dynamicNames)
{
    QSP_TINYINT fieldType;
    int count = 0;
    while (1)
    {
        if (count == QSP_MAXMATHOPARGS)
        {
            qspSetError(QSP_ERR_COMPLEXEXPRESSION);
            return -1;
        }
        qspSkipSpaces(s);
        if (!qspParseFieldName(s, fields + count, dynamicNames + count))
            return -1;
        fieldType = fields[count].Value.Type; /* specified type prefix */
        ++count;
        qspSkipSpaces(s);
        if (!qspParseChar(s, QSP_FIELDDELIM_CHAR)) break;
        if (QSP_ISDEF(fieldType) && fieldType != QSP_TYPE_TUPLE)
        {
            qspSetError(QSP_ERR_INCORRECTFIELDNAME); /* we don't allow scalar value prefixes in between */
            return -1;
        }
    }
    if (QSP_ISDEF(varType) && QSP_ISDEF(fieldType) && fieldType != varType)
    {
        qspSetError(QSP_ERR_INCORRECTFIELDNAME); /* type prefixes should match */
        return -1;
    }
    return count;
}

int qspGetFieldsFromPath(QSPTuple path, QSPField *fields, QSP_TINYINT mode)
{
    /* Path items are the exact field names */
    int i;
    QSPField *item;
    if (path.FieldsCount > QSP_MAXMATHOPARGS)
    {
        qspSetError(QSP_ERR_COMPLEXEXPRESSION);
        return -1;
    }
    for (i = 0, item = path.Fields; i < path.FieldsCount; ++i, ++item)
    {
        if (!QSP_ISSTR(item->Value.Type))
        {
            qspSetError(QSP_ERR_INCORRECTFIELDNAME); /* only strings are names */
            return -1;
        }
        fields[i].Value = qspGetEmptyVariant(QSP_TYPE_UNDEF);
        if (!qspGetFieldNameIndex(QSP_STR(item->Value), mode, &fields[i].NameIndex)) return -1;
    }
    return path.FieldsCount;
}

void qspGetVarFieldValueByPath(QSPString varName, QSPVariant index, QSPTuple path, QSPVariant *res)
{
    QSPField fields[QSP_MAXMATHOPARGS];
    int count, arrIndex;
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
    if (!var) return;
    varType = qspGetVarType(varName);
    count = qspGetFieldsFromPath(path, fields, QSP_REF_GET);
    if (count < 0) return;
    arrIndex = qspGetVarItemIndex(var, index, QSP_FALSE);
    qspGetVarItemFieldValue(var, arrIndex, varType, fields, count, res);
}

QSPVar *qspVarByName(QSPString name)
{
    QSPVar *var;
    QSPString upperName = qspUpperText(name);
    var = qspVarReference(upperName, QSP_REF_GETSILENTLY);
    qspFreeNewString(&upperName, &name);
    return var;
}

QSPString qspGetVarStrValue(QSPString name, int valIndex)
{
    QSPVar *var = qspVarByName(name);
    if (var && valIndex >= 0 && valIndex < var->ValsCount)
        return qspGetVariantAsString(var->Values + valIndex);
    return qspNullString;
}

QSP_BIGINT qspGetVarNumValue(QSPString name, int valIndex)
{
    QSPVar *var = qspVarByName(name);
    if (var && valIndex >= 0 && valIndex < var->ValsCount)
        return qspGetVariantAsNum(var->Values + valIndex, 0);
    return 0;
}

INLINE void qspResetTargetValue(QSPCachedTarget *target)
{
    QSPResolvedTarget resolved;
    int arrIndex;
    /* Missing items have nothing to reset, so we don't create them */
    if (!qspResolveTarget(&resolved, target, QSP_REF_GET)) return;
    arrIndex = qspGetVarItemIndex(resolved.Var, resolved.Index, QSP_FALSE);
    if (arrIndex >= 0 && arrIndex < resolved.Var->ValsCount)
    {
        QSPVariant *curValue = qspGetTupleFieldValueByPath(resolved.Var->Values + arrIndex, resolved.Fields, target->FieldsCount);
        if (curValue && QSP_ISDEF(curValue->Type))
        {
            qspFreeVariant(curValue);
            qspInitVariant(curValue, QSP_TYPE_UNDEF);
        }
    }
    qspFreeResolvedTarget(&resolved, target);
}

INLINE QSPVariant *qspReserveVarItem(QSPVar *var, int valIndex)
{
    if (valIndex >= 0)
    {
        int oldCount = var->ValsCount;
        if (valIndex >= oldCount)
        {
            if (valIndex >= var->ValsCapacity)
            {
                var->ValsCapacity = (valIndex > 0 ? valIndex + 4 : 1); /* allocate only 1 item for the first value */
                var->Values = (QSPVariant *)realloc(var->Values, var->ValsCapacity * sizeof(QSPVariant));
            }
            var->ValsCount = valIndex + 1;
            qspInitUndefinedVariants(var->Values + oldCount, valIndex - oldCount + 1);
        }
        return var->Values + valIndex;
    }
    return 0;
}

INLINE void qspSetVarItemValue(QSPVar *var, int valIndex, QSPVariant *val)
{
    /* The value has to be converted to the type of the variable already */
    QSPVariant *item = qspReserveVarItem(var, valIndex);
    if (!item) return;
    qspFreeVariant(item);
    qspMoveToNewVariant(item, val);
}

INLINE void qspSetVarItemFieldValue(QSPVar *var, int valIndex, QSPField *fields, int count, QSPVariant *val)
{
    /* The value has to be converted to the type of the variable already */
    QSPVariant *item = qspReserveVarItem(var, valIndex);
    if (!item) return;
    item = qspReserveTupleFieldValueByPath(item, fields, count);
    qspFreeVariant(item);
    qspMoveToNewVariant(item, val);
}

INLINE void qspSetVarFieldValue(QSPVar *var, QSPVariant index, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *val, QSP_CHAR op)
{
    int arrIndex = -1;
    if (count && !QSP_ISDEF(varType)) varType = fields[count - 1].Value.Type;
    if (op != QSP_EQUAL_CHAR)
    {
        /* Replace the value with its combination with the current value */
        QSPVariant oldVal, res;
        arrIndex = qspGetVarItemIndex(var, index, QSP_FALSE);
        qspGetVarItemFieldValue(var, arrIndex, varType, fields, count, &oldVal);
        if (!qspAutoConvertCombine(&oldVal, val, op, &res))
        {
            qspFreeVariant(&oldVal);
            return;
        }
        qspFreeVariant(&oldVal);
        qspFreeVariant(val);
        qspMoveToNewVariant(val, &res);
    }
    /* We create a new key only when the value can be assigned */
    if (!qspConvertVariantTo(val, varType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return;
    }
    if (arrIndex < 0)
        arrIndex = qspGetVarItemIndex(var, index, QSP_TRUE);
    qspSetVarItemFieldValue(var, arrIndex, fields, count, val);
}

INLINE void qspSetTargetValue(QSPCachedTarget *target, QSPVariant *val, QSP_CHAR op)
{
    QSPResolvedTarget resolved;
    QSP_TINYINT varType;
    int arrIndex = -1;
    if (!qspResolveTarget(&resolved, target, QSP_REF_GETORCREATE)) return;
    varType = qspGetVarType(target->Name);
    if (target->FieldsCount)
    {
        qspSetVarFieldValue(resolved.Var, resolved.Index, varType, resolved.Fields, target->FieldsCount, val, op);
        qspFreeResolvedTarget(&resolved, target);
        return;
    }
    if (op != QSP_EQUAL_CHAR)
    {
        /* Replace the value with its combination with the current value */
        QSPVariant oldVal, res;
        arrIndex = qspGetVarItemIndex(resolved.Var, resolved.Index, QSP_FALSE);
        qspGetVarItemValue(resolved.Var, arrIndex, varType, &oldVal);
        if (!qspAutoConvertCombine(&oldVal, val, op, &res))
        {
            qspFreeVariant(&oldVal);
            qspFreeResolvedTarget(&resolved, target);
            return;
        }
        qspFreeVariant(&oldVal);
        qspFreeVariant(val);
        qspMoveToNewVariant(val, &res);
    }
    /* We create a new key only when the value can be assigned */
    if (!qspConvertVariantTo(val, varType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        qspFreeResolvedTarget(&resolved, target);
        return;
    }
    if (arrIndex < 0)
        arrIndex = qspGetVarItemIndex(resolved.Var, resolved.Index, QSP_TRUE);
    qspSetVarItemValue(resolved.Var, arrIndex, val);
    qspFreeResolvedTarget(&resolved, target);
}

INLINE void qspMoveTupleToArray(QSPVar *dest, QSPTuple *src, int start, int count)
{
    int i, itemsToMove, keysCount;
    QSPVariant keyVal;
    QSPVarKey *key;
    QSPField *field;
    /* Clear the dest array anyway */
    qspEmptyVar(dest);
    /* Validate parameters */
    if (count <= 0) return;
    if (start < 0) start = 0;
    itemsToMove = src->FieldsCount - start;
    if (itemsToMove <= 0) return;
    if (count < itemsToMove) itemsToMove = count;
    /* Move tuple items, field names become text keys */
    dest->ValsCapacity = dest->ValsCount = itemsToMove;
    dest->Values = (QSPVariant *)malloc(itemsToMove * sizeof(QSPVariant));
    field = src->Fields + start;
    keysCount = 0;
    for (i = 0; i < itemsToMove; ++i)
        if (field[i].NameIndex > 0) ++keysCount;
    if (keysCount)
    {
        int capacity = QSP_VARSKEYSCAPACITY;
        while (capacity < keysCount) capacity *= 2;
        qspResizeVarKeys(dest, capacity);
    }
    for (i = 0; i < itemsToMove; ++i, ++field)
    {
        qspMoveToNewVariant(dest->Values + i, &field->Value);
        if (field->NameIndex > 0)
        {
            keyVal = qspStrVariant(qspCopyToNewText(qspGetNameText(field->NameIndex)), QSP_TYPE_STR);
            key = qspAddVarKey(dest, qspGetVariantKeyHash(&keyVal), i);
            key->Key = keyVal;
        }
    }
}

INLINE void qspCopyArray(QSPVar *dest, QSPVar *src, int start, int count)
{
    int i, itemsToCopy, keyIndex, keysCount;
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
    /* Copy keys of the copied items, they stay ordered */
    keyIndex = qspGetVarKeyLowerBound(src, start);
    keysCount = qspGetVarKeyLowerBound(src, start + itemsToCopy) - keyIndex;
    if (keysCount > 0)
    {
        QSPVarKey *destKey, *srcKey = src->Keys + keyIndex;
        if (keysCount == src->KeysCount)
        {
            /* All keys stay the same & the slots can be copied */
            qspResizeVarKeys(dest, src->KeysCapacity);
            memcpy(dest->KeysSlots, src->KeysSlots, QSP_SLOTSCOUNT(src->KeysCapacity) * sizeof(int));
            destKey = dest->Keys;
            for (i = 0; i < keysCount; ++i, ++srcKey, ++destKey)
            {
                destKey->Hash = srcKey->Hash;
                destKey->Index = srcKey->Index - start;
                qspCopyToNewVariant(&destKey->Key, &srcKey->Key);
            }
            dest->KeysCount = keysCount;
        }
        else
        {
            int capacity = QSP_VARSKEYSCAPACITY;
            while (capacity < keysCount) capacity *= 2;
            qspResizeVarKeys(dest, capacity);
            for (i = 0; i < keysCount; ++i, ++srcKey)
            {
                destKey = qspAddVarKey(dest, srcKey->Hash, srcKey->Index - start);
                qspCopyToNewVariant(&destKey->Key, &srcKey->Key);
            }
        }
    }
}

QSPTuple qspCopyArrayToNewTuple(QSPVar *src, int start, int count)
{
    int itemsToCopy, keyIndex, keysCount;
    QSPTuple tuple;
    /* Validate parameters */
    if (count <= 0) return qspNullTuple;
    if (start < 0) start = 0;
    itemsToCopy = src->ValsCount - start;
    if (itemsToCopy <= 0) return qspNullTuple;
    if (count < itemsToCopy) itemsToCopy = count;
    tuple = qspCopyToNewTuple(src->Values + start, itemsToCopy);
    /* String keys of the copied items become field names */
    keyIndex = qspGetVarKeyLowerBound(src, start);
    keysCount = qspGetVarKeyLowerBound(src, start + itemsToCopy) - keyIndex;
    if (keysCount > 0)
    {
        QSPVarKey *key;
        for (key = src->Keys + keyIndex; keysCount > 0; --keysCount, ++key)
        {
            if (QSP_ISSTR(key->Key.Type))
                tuple.Fields[key->Index - start].NameIndex = qspGetNameIndex(QSP_STR(key->Key), QSP_TRUE);
        }
    }
    return tuple;
}

INLINE void qspSortArray(QSPVar *var, QSP_TINYINT varType, QSP_BOOL isAscending)
{
    QSP_TINYINT valsType;
    QSPVariant *curValue, *sortedValues, **valuePositions;
    int i, keysCount, valsCount;
    valsCount = var->ValsCount;
    if (valsCount < 2) return;
    if (!qspGetArrayValsType(var, &valsType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return;
    }
    if (!QSP_ISDEF(valsType)) return; /* nothing but holes */
    if (QSP_ISDEF(varType) && varType != valsType)
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return;
    }
    valuePositions = (QSPVariant **)malloc(valsCount * sizeof(QSPVariant *));
    curValue = var->Values;
    for (i = 0; i < valsCount; ++i, ++curValue)
        valuePositions[i] = curValue;
    /* Sort positions of values by comparing values */
    if (isAscending)
        qsort(valuePositions, valsCount, sizeof(QSPVariant *), qspValuePositionsAscCompare);
    else
        qsort(valuePositions, valsCount, sizeof(QSPVariant *), qspValuePositionsDescCompare);
    /* Reorder keys, they follow their items & stay ordered by index in Values */
    keysCount = var->KeysCount;
    if (keysCount > 0)
    {
        QSPVarKey *curKey, *sortedKeys;
        int keyIndex, *keyIndices;
        /* Key index of every item, -1 without a key */
        keyIndices = (int *)malloc(valsCount * sizeof(int));
        for (i = 0; i < valsCount; ++i)
            keyIndices[i] = -1;
        for (i = 0; i < keysCount; ++i)
            keyIndices[var->Keys[i].Index] = i;
        /* Place the keys in the order of their sorted items */
        sortedKeys = (QSPVarKey *)malloc(var->KeysCapacity * sizeof(QSPVarKey));
        curKey = sortedKeys;
        for (i = 0; i < valsCount; ++i)
        {
            keyIndex = keyIndices[valuePositions[i] - var->Values];
            if (keyIndex >= 0)
            {
                *curKey = var->Keys[keyIndex];
                curKey->Index = i;
                ++curKey;
            }
        }
        free(keyIndices);
        free(var->Keys);
        var->Keys = sortedKeys;
        qspRebuildVarKeysSlots(var); /* the slots follow the new order */
    }
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
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
    if (!var) return 0;
    return var->ValsCount;
}

int qspArrayPos(QSPString varName, QSPVariant *val, int ind)
{
    QSP_TINYINT varType;
    QSPVariant defaultValue, *curValue;
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
    if (!var) return -1;
    varType = qspGetVarType(varName);
    if (!qspConvertVariantTo(val, varType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return -1;
    }
    defaultValue = qspGetEmptyVariant(QSP_BASETYPE(val->Type));
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
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
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
    QSP_TINYINT varType, valsType;
    QSPVar *var = qspVarReference(varName, QSP_REF_GET);
    if (!var) return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    varType = qspGetVarType(varName);
    if (QSP_ISDEF(varType))
        valsType = varType;
    else if (!qspGetArrayValsType(var, &valsType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    }
    resultValue = qspGetEmptyVariant(valsType);
    if (!QSP_ISDEF(valsType)) return resultValue; /* nothing but holes */
    bestValue = 0;
    for (i = 0; i < var->ValsCount; ++i)
    {
        curValue = var->Values + i;
        if (!QSP_ISDEF(curValue->Type)) curValue = &resultValue;
        if (QSP_BASETYPE(curValue->Type) == valsType)
        {
            if (bestValue)
            {
                if (isMin)
                {
                    if (qspVariantsCompare(curValue, bestValue) < 0)
                        bestValue = curValue;
                }
                else if (qspVariantsCompare(curValue, bestValue) > 0)
                    bestValue = curValue;
            }
            else
                bestValue = curValue;
        }
    }
    if (bestValue) qspCopyToNewVariant(&resultValue, bestValue);
    return resultValue;
}

INLINE void qspSetTargetsValues(QSPCachedAssignment *assignment, QSPVariant *val, QSP_CHAR op)
{
    int i, oldLocationState, varsCount = assignment->TargetsCount;
    QSPCachedTarget *targets = assignment->Targets;
    if (varsCount == 1)
    {
        qspSetTargetValue(targets, val, op);
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
    switch (QSP_BASETYPE(val->Type))
    {
    case QSP_TYPE_TUPLE:
        {
            QSPTuple *tuple = &QSP_PTUPLE(val);
            int valuesCount = tuple->FieldsCount;
            if (varsCount < valuesCount)
            {
                /* Assign variables that contain single values */
                QSPVariant restValues;
                int lastVarIndex = varsCount - 1;
                for (i = 0; i < lastVarIndex; ++i)
                {
                    qspSetTargetValue(targets + i, &tuple->Fields[i].Value, op);
                    if (qspLocationState != oldLocationState)
                        return;
                }
                /* Only 1 variable left, fill it with a tuple containing all the values left */
                restValues = qspTupleVariant(qspMoveFieldsToNewTuple(tuple->Fields + i, valuesCount - i));
                qspSetTargetValue(targets + lastVarIndex, &restValues, op);
                qspFreeVariant(&restValues);
            }
            else
            {
                /* Assign all values to the variables */
                for (i = 0; i < valuesCount; ++i)
                {
                    qspSetTargetValue(targets + i, &tuple->Fields[i].Value, op);
                    if (qspLocationState != oldLocationState)
                        return;
                }
                /* No values left, reset the rest of vars with default values */
                while (i < varsCount)
                {
                    qspResetTargetValue(targets + i);
                    if (qspLocationState != oldLocationState)
                        return;
                    ++i;
                }
            }
            break;
        }
    case QSP_TYPE_UNDEF:
    case QSP_TYPE_NUM:
    case QSP_TYPE_STR:
        /* Consider it a tuple with 1 item */
        qspSetTargetValue(targets, val, op);
        if (qspLocationState != oldLocationState)
            return;
        for (i = 1; i < varsCount; ++i)
        {
            qspResetTargetValue(targets + i);
            if (qspLocationState != oldLocationState)
                return;
        }
        break;
    }
}

void qspStatementSetVarsValues(QSPCachedStat *stat)
{
    QSPCachedAssignment *assignment = stat->Data.Assignment;
    int oldLocationState = qspLocationState;
    QSPVariant v = qspCalculateArgValue(&assignment->Value);
    if (qspLocationState != oldLocationState) return;
    qspSetTargetsValues(assignment, &v, assignment->Operation);
    qspFreeVariant(&v);
}

void qspStatementLocal(QSPCachedStat *stat)
{
    int i;
    QSPVariant v;
    QSPCachedTarget *target;
    QSPCachedAssignment *assignment = stat->Data.Assignment;
    if (assignment->Value.Type != QSP_ARG_NONE)
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
        qspSetTargetsValues(assignment, &v, QSP_EQUAL_CHAR);
        qspFreeVariant(&v);
    }
}

void qspStatementAlias(QSPCachedStat *stat)
{
    QSPVariant names;
    QSPVarSlot *slot;
    QSPSharedVar *sharedVars[QSP_MAXSTATARGS];
    QSPCachedAssignment *assignment = stat->Data.Assignment;
    int i, namesCount, oldLocationState = qspLocationState;
    /* We have to evaluate expression before allocation of local vars */
    names = qspCalculateArgValue(&assignment->Value);
    if (qspLocationState != oldLocationState) return;
    namesCount = (QSP_ISTUPLE(names.Type) ? QSP_TUPLE(names).FieldsCount : 1);
    if (namesCount != assignment->TargetsCount)
    {
        qspSetError(QSP_ERR_ARGSCOUNT);
        qspFreeVariant(&names);
        return;
    }
    /* Share the arrays before the local variables exist */
    if (QSP_ISTUPLE(names.Type))
    {
        QSPVariant *name;
        for (i = 0; i < namesCount; ++i)
        {
            name = &QSP_TUPLE(names).Fields[i].Value;
            qspConvertVariantTo(name, QSP_TYPE_VARREF);
            if (!((sharedVars[i] = qspShareVarByName(QSP_PSTR(name)))))
            {
                while (--i >= 0) qspReleaseSharedVar(sharedVars[i]);
                qspFreeVariant(&names);
                return;
            }
        }
    }
    else
    {
        qspConvertVariantTo(&names, QSP_TYPE_VARREF);
        if (!((sharedVars[0] = qspShareVarByName(QSP_STR(names)))))
        {
            qspFreeVariant(&names);
            return;
        }
    }
    qspFreeVariant(&names);
    for (i = 0; i < namesCount; ++i)
    {
        if (!((slot = qspAddVarToLocals(assignment->Targets[i].Name))))
        {
            while (i < namesCount)
            {
                qspReleaseSharedVar(sharedVars[i]);
                ++i;
            }
            return;
        }
        qspMoveReferenceToVarSlot(slot, sharedVars[i]);
    }
}

void qspStatementSetVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPField fields[QSP_MAXMATHOPARGS];
    QSPVariant index;
    QSP_TINYINT varType;
    int fieldsCount = 0;
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_REF_GETORCREATE);
    if (!var) return;
    varType = qspGetVarType(QSP_STR(args[0]));
    if (count == 4)
    {
        fieldsCount = qspGetFieldsFromPath(QSP_TUPLE(args[3]), fields, QSP_REF_GETORCREATE);
        if (fieldsCount < 0) return;
    }
    index = (count == 2 ? qspNumVariant(0) : args[2]);
    qspSetVarFieldValue(var, index, varType, fields, fieldsCount, args + 1, QSP_EQUAL_CHAR);
}

void qspStatementInsVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    int valIndex, newIndex;
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_REF_GETORCREATE);
    if (!var) return;
    varType = qspGetVarType(QSP_STR(args[0]));
    if (!qspConvertVariantTo(args + 1, varType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return;
    }
    if (count == 2)
    {
        /* New last item */
        qspSetVarItemValue(var, var->ValsCount, args + 1);
        return;
    }
    if (qspIsPosIndex(args + 2))
    {
        /* Insert by position */
        if (count == 3)
        {
            /* New last item, move it to the specified position */
            valIndex = var->ValsCount;
            newIndex = QSP_TOINT(QSP_NUM(args[2]));
        }
        else
        {
            /* The item at the position ends up at the new position */
            valIndex = QSP_TOINT(QSP_NUM(args[2]));
            newIndex = QSP_TOINT(QSP_NUM(args[3]));
        }
        if (valIndex < 0 || newIndex < 0) return;
    }
    else
    {
        /* Insert by key */
        if (count == 3)
        {
            /* Same as SETVAR */
            valIndex = qspGetVarItemIndex(var, args[2], QSP_TRUE);
            newIndex = valIndex;
        }
        else
        {
            /* The item of the key ends up at the new position */
            newIndex = QSP_TOINT(QSP_NUM(args[3]));
            if (newIndex < 0) return;
            valIndex = qspGetVarItemIndex(var, args[2], QSP_TRUE);
        }
    }
    /* Move item to the new position and update the value */
    qspMoveVarItem(var, valIndex, newIndex);
    qspSetVarItemValue(var, newIndex, args + 1);
}

void qspStatementUnpackArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    int startInd, maxCount;
    QSPTuple *src;
    QSPVar *dest = qspVarReference(QSP_STR(args[0]), QSP_REF_GETORCREATE);
    if (!dest) return;
    src = &QSP_TUPLE(args[1]);
    startInd = (count >= 3 ? QSP_TOINT(QSP_NUM(args[2])) : 0);
    maxCount = (count == 4 ? QSP_TOINT(QSP_NUM(args[3])) : src->FieldsCount);
    qspMoveTupleToArray(dest, src, startInd, maxCount);
}

void qspStatementCopyArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPVar *dest, *src;
    if (!((dest = qspVarReference(QSP_STR(args[0]), QSP_REF_GETORCREATE)))) return;
    if (!((src = qspVarReference(QSP_STR(args[1]), QSP_REF_GET)))) return;
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
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_REF_GET); /* we don't create a new array */
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
    QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_REF_GETORCREATE);
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
        QSPVar *var = qspVarReference(QSP_STR(args[0]), QSP_REF_GET); /* we don't create a new array */
        if (!var) return;
        if (count > 1)
        {
            /* The item or its field */
            QSPField fields[QSP_MAXMATHOPARGS];
            int fieldsCount = 0, arrIndex = qspGetVarItemIndex(var, args[1], QSP_FALSE);
            if (count == 3)
            {
                fieldsCount = qspGetFieldsFromPath(QSP_TUPLE(args[2]), fields, QSP_REF_GET);
                if (fieldsCount < 0) return;
            }
            if (arrIndex >= 0 && arrIndex < var->ValsCount)
            {
                if (fieldsCount)
                {
                    /* The last field is the one that was specified */
                    QSPVariant *value = qspGetTupleFieldValueByPath(var->Values + arrIndex, fields, fieldsCount - 1);
                    if (value && QSP_ISTUPLE(value->Type))
                        qspRemoveTupleField(&QSP_PTUPLE(value), fields[fieldsCount - 1].NameIndex);
                }
                else
                {
                    /* Empty path removes the item itself */
                    qspRemoveVarItem(var, arrIndex);
                }
            }
        }
        else
            qspEmptyVar(var);
    }
    else
        qspClearVars(&qspGlobalVars);
}
