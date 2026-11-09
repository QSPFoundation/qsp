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

QSPVar *qspGlobalVars = 0;
int qspGlobalVarsCount = 0;
int qspGlobalVarsCapacity = 0;

static QSPVar qspNullVar;

/* Special variables */
static int qspArgsNameIndex = 0;
static int qspResultNameIndex = 0;

/* Local variables */
static QSPVar *qspLocalVars = 0; /* all local variables */
static int qspLocalVarsCount = 0;
static int qspLocalVarsCapacity = 0;
static int qspLocalScopeBase = 0; /* defines the current local scope (index in qspLocalVars) */
static int qspLocalVarsBase = 0; /* local variables below it are hidden from the running code */

QSP_TINYINT qspSpecToBaseTypeTable[128];

INLINE int qspValuePositionsAscCompare(const void *arg1, const void *arg2);
INLINE int qspValuePositionsDescCompare(const void *arg1, const void *arg2);
INLINE QSP_BOOL qspGetArrayValsType(QSPVarData *varData, QSP_TINYINT *valsType);
INLINE void qspInitVar(QSPVar *var, int nameIndex);
INLINE QSPVar *qspNewVars(int capacity);
INLINE QSPVar *qspGetGlobalVar(int nameIndex, QSP_BOOL toCreate);
INLINE QSPVar *qspGetEmptyGlobalVar(int nameIndex);
INLINE void qspRebuildGlobalVars(int newCapacity);
INLINE QSPVar *qspCreateGlobalVar(int nameIndex);
INLINE QSPVar *qspCreateLocalVar(QSPVar *globalVar);
INLINE void qspReleaseLocalVars(int varsBase);
INLINE void qspReleaseSharedVarData(QSPSharedVarData *sharedData);
INLINE void qspResetVar(QSPVar *var);
INLINE QSPSharedVarData *qspShareVarByName(QSPString name);
INLINE void qspMoveReferenceToVar(QSPVar *var, QSPSharedVarData *sharedData);
INLINE QSPVar *qspAddLocalVar(int nameIndex);
INLINE void qspInitVarValues(QSPVarData *varData, QSPVariant *vals, int count, QSP_BOOL toMove);
INLINE QSPVar *qspVarReference(int nameIndex, QSP_TINYINT mode);
INLINE QSPVarData *qspVarDataReference(int nameIndex, QSP_TINYINT mode);
INLINE int *qspGetVarKeySlot(QSPVarData *varData, QSPVariant *key, unsigned int hash);
INLINE int *qspGetEmptyVarKeySlot(QSPVarData *varData, unsigned int hash);
INLINE int *qspGetVarKeySlotByIndex(QSPVarData *varData, int keyIndex);
INLINE void qspRebuildVarKeysSlots(QSPVarData *varData);
INLINE void qspResizeVarKeys(QSPVarData *varData, int newCapacity);
INLINE void qspRemoveVarKey(QSPVarData *varData, int keyIndex);
INLINE void qspMoveVarKey(QSPVarData *varData, int keyIndex, int newKeyIndex);
INLINE void qspMoveVarItem(QSPVarData *varData, int valIndex, int newIndex);
INLINE void qspRemoveVarItem(QSPVarData *varData, int valIndex);
INLINE int qspGetVarKeyLowerBound(QSPVarData *varData, int valIndex);
INLINE QSP_BOOL qspIsPosIndex(QSPVariant *index);
INLINE QSP_BOOL qspResolveTarget(QSPResolvedTarget *resolved, QSPCachedTarget *target, QSP_TINYINT mode);
INLINE void qspFreeResolvedTarget(QSPResolvedTarget *resolved, QSPCachedTarget *target);
INLINE void qspGetVarItemValue(QSPVarData *varData, int valIndex, QSP_TINYINT varType, QSPVariant *res);
INLINE void qspGetVarItemFieldValue(QSPVarData *varData, int valIndex, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *res);
INLINE void qspResetTargetValue(QSPCachedTarget *target);
INLINE QSPVariant *qspReserveVarItem(QSPVarData *varData, int valIndex);
INLINE void qspSetVarItemValue(QSPVarData *varData, int valIndex, QSPVariant *val);
INLINE void qspSetVarItemFieldValue(QSPVarData *varData, int valIndex, QSPField *fields, int count, QSPVariant *val);
INLINE void qspSetVarFieldValue(QSPVarData *varData, QSPVariant index, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *val, QSP_CHAR op);
INLINE void qspSetTargetValue(QSPCachedTarget *target, QSPVariant *val, QSP_CHAR op);
INLINE void qspMoveTupleToArray(QSPVarData *dest, QSPTuple *src, int start, int count);
INLINE void qspCopyArray(QSPVarData *dest, QSPVarData *src, int start, int count);
INLINE void qspMoveArray(QSPVarData *dest, QSPVarData *src, int start, int count);
INLINE void qspSortArray(QSPVarData *varData, QSP_TINYINT varType, QSP_BOOL isAscending);
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

INLINE QSP_BOOL qspGetArrayValsType(QSPVarData *varData, QSP_TINYINT *valsType)
{
    QSP_TINYINT type = QSP_TYPE_UNDEF;
    QSPVariant *curValue = varData->Values, *end = varData->Values + varData->ValsCount;
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

INLINE void qspInitVar(QSPVar *var, int nameIndex)
{
    var->NameIndex = nameIndex;
    var->LocalIndex = 0;
    var->GlobalIndex = 0;
    var->IsRef = QSP_FALSE;
    qspInitVarData(&var->Data.Own);
}

INLINE QSPVar *qspNewVars(int capacity)
{
    QSPVar *vars = (QSPVar *)malloc(capacity * sizeof(QSPVar));
    memset(vars, 0, capacity * sizeof(QSPVar));
    return vars;
}

void qspInitVars(void)
{
    qspInitVar(&qspNullVar, 0);
    qspGlobalVarsCount = 0;
    qspGlobalVarsCapacity = QSP_VARSGLOBALCAPACITY;
    qspGlobalVars = qspNewVars(QSP_SLOTSCOUNT(qspGlobalVarsCapacity));
    qspLocalVarsCount = 0;
    qspLocalVarsCapacity = QSP_VARSLOCALCAPACITY;
    qspLocalVars = (QSPVar *)malloc(qspLocalVarsCapacity * sizeof(QSPVar));
    qspLocalScopeBase = 0;
    qspLocalVarsBase = 0;
}

void qspTerminateVars(void)
{
    /* Remove all variables & free the buffers */
    qspReleaseLocalVars(0);
    qspClearGlobalVars();
    free(qspGlobalVars);
    free(qspLocalVars);
}

void qspClearGlobalVars(void)
{
    /* Remove all global variables, variables referenced by aliases get emptied */
    int count = qspGlobalVarsCount;
    QSPVar *var = qspGlobalVars;
    while (count > 0)
    {
        if (var->NameIndex)
        {
            if (var->IsRef && var->Data.Shared->RefsCount > 1)
                qspEmptyVarData(&var->Data.Shared->Data);
            else
            {
                /* Keep the global var if local variables reference it */
                qspResetVar(var);
                if (!var->LocalIndex)
                {
                    var->NameIndex = 0;
                    --qspGlobalVarsCount;
                }
            }
            --count;
        }
        ++var;
    }
    /* Removed vars break references of the rest, so we need to rebuild them */
    if (qspGlobalVarsCount) qspRebuildGlobalVars(qspGlobalVarsCapacity);
}

void qspClearLocalVars(void)
{
    /* Remove local variables the code can access */
    qspReleaseLocalVars(qspLocalVarsBase);
    qspLocalScopeBase = qspLocalVarsBase;
}

void qspClearAllVars(QSP_BOOL toInit)
{
    if (!toInit)
    {
        qspClearLocalVars();
        qspClearGlobalVars();
    }
}

void qspRegisterSpecialVariables(void)
{
    qspArgsNameIndex = qspGetNameIndex(QSP_STATIC_STR(QSP_VAR_ARGS), QSP_TRUE);
    qspResultNameIndex = qspGetNameIndex(QSP_STATIC_STR(QSP_VAR_RESULT), QSP_TRUE);
}

INLINE QSPVar *qspGetGlobalVar(int nameIndex, QSP_BOOL toCreate)
{
    QSPVar *var;
    int mask = QSP_SLOTSMASK(qspGlobalVarsCapacity), ind = (int)(qspGetIndexHash(nameIndex) & mask);
    while ((var = qspGlobalVars + ind)->NameIndex && var->NameIndex != nameIndex)
        ind = (ind + 1) & mask;
    if (toCreate && !var->NameIndex) return qspCreateGlobalVar(nameIndex);
    return var;
}

INLINE QSPVar *qspGetEmptyGlobalVar(int nameIndex)
{
    int mask = QSP_SLOTSMASK(qspGlobalVarsCapacity), ind = (int)(qspGetIndexHash(nameIndex) & mask);
    while (qspGlobalVars[ind].NameIndex) ind = (ind + 1) & mask;
    return qspGlobalVars + ind;
}

INLINE void qspRebuildGlobalVars(int newCapacity)
{
    QSPVar *var, *oldVar, *oldVars = qspGlobalVars;
    int i, prevLocalIndex, newGlobalIndex, oldSlotsCount = QSP_SLOTSCOUNT(qspGlobalVarsCapacity);
    qspGlobalVarsCapacity = newCapacity;
    qspGlobalVars = qspNewVars(QSP_SLOTSCOUNT(newCapacity));
    for (i = 0, oldVar = oldVars; i < oldSlotsCount; ++i, ++oldVar)
    {
        if (oldVar->NameIndex)
        {
            var = qspGetEmptyGlobalVar(oldVar->NameIndex);
            *var = *oldVar;
            /* Update global index for the local variables */
            newGlobalIndex = (int)(var - qspGlobalVars) + 1;
            prevLocalIndex = var->LocalIndex;
            while (prevLocalIndex)
            {
                var = qspLocalVars + prevLocalIndex - 1;
                var->GlobalIndex = newGlobalIndex;
                prevLocalIndex = var->LocalIndex;
            }
        }
    }
    free(oldVars);
}

INLINE QSPVar *qspCreateGlobalVar(int nameIndex)
{
    QSPVar *var;
    if (qspGlobalVarsCount >= qspGlobalVarsCapacity)
        qspRebuildGlobalVars(qspGlobalVarsCapacity * 2);
    var = qspGetEmptyGlobalVar(nameIndex);
    qspInitVar(var, nameIndex);
    ++qspGlobalVarsCount;
    return var;
}

INLINE void qspReleaseSharedVarData(QSPSharedVarData *sharedData)
{
    if (--sharedData->RefsCount == 0)
    {
        qspEmptyVarData(&sharedData->Data);
        free(sharedData);
    }
}

INLINE void qspResetVar(QSPVar *var)
{
    if (var->IsRef)
    {
        qspReleaseSharedVarData(var->Data.Shared);
        var->IsRef = QSP_FALSE;
        qspInitVarData(&var->Data.Own);
    }
    else
        qspEmptyVarData(&var->Data.Own);
}

INLINE QSPSharedVarData *qspShareVarByName(QSPString name)
{
    QSPSharedVarData *sharedData;
    QSPVar *var = qspVarReferenceByName(name, QSP_REF_GETORCREATE);
    if (!var) return 0;
    if (var->IsRef)
        sharedData = var->Data.Shared;
    else
    {
        sharedData = (QSPSharedVarData *)malloc(sizeof(QSPSharedVarData));
        qspMoveVarData(&sharedData->Data, &var->Data.Own);
        sharedData->RefsCount = 1;
        var->IsRef = QSP_TRUE;
        var->Data.Shared = sharedData;
    }
    ++sharedData->RefsCount; /* reference for the caller */
    return sharedData;
}

INLINE void qspMoveReferenceToVar(QSPVar *var, QSPSharedVarData *sharedData)
{
    /* The var owns the reference */
    qspResetVar(var);
    var->IsRef = QSP_TRUE;
    var->Data.Shared = sharedData;
}

INLINE QSPVar *qspCreateLocalVar(QSPVar *globalVar)
{
    QSPVar *localVar;
    if (qspLocalVarsCount >= qspLocalVarsCapacity)
    {
        qspLocalVarsCapacity *= 2;
        qspLocalVars = (QSPVar *)realloc(qspLocalVars, qspLocalVarsCapacity * sizeof(QSPVar));
    }
    localVar = qspLocalVars + qspLocalVarsCount;
    localVar->NameIndex = globalVar->NameIndex;
    localVar->LocalIndex = globalVar->LocalIndex;
    localVar->GlobalIndex = (int)(globalVar - qspGlobalVars) + 1;
    localVar->IsRef = QSP_FALSE;
    qspInitVarData(&localVar->Data.Own);
    globalVar->LocalIndex = ++qspLocalVarsCount; /* 1-based */
    return localVar;
}

INLINE void qspReleaseLocalVars(int varsBase)
{
    /* Restore previous bindings in reverse order */
    QSPVar *globalVar, *localVar = qspLocalVars + qspLocalVarsCount - 1;
    while (qspLocalVarsCount > varsBase)
    {
        globalVar = qspGlobalVars + localVar->GlobalIndex - 1;
        globalVar->LocalIndex = localVar->LocalIndex;
        qspResetVar(localVar);
        --qspLocalVarsCount;
        --localVar;
    }
}

int qspAllocateLocalScope(void)
{
    int oldLocalScopeBase = qspLocalScopeBase;
    qspLocalScopeBase = qspLocalVarsCount;
    return oldLocalScopeBase;
}

void qspReleaseLocalScope(int oldLocalScopeBase)
{
    qspReleaseLocalVars(qspLocalScopeBase);
    qspLocalScopeBase = oldLocalScopeBase;
}

INLINE QSPVar *qspAddLocalVar(int nameIndex)
{
    /* Check if the variable already exists in the current scope */
    QSPVar *globalVar = qspGetGlobalVar(nameIndex, QSP_FALSE);
    if (globalVar->LocalIndex > qspLocalScopeBase)
        return qspLocalVars + globalVar->LocalIndex - 1;

    /* It doesn't exist yet, so we have to add it */
    if (qspLocalVarsCount - qspLocalScopeBase >= QSP_MAXLOCALVARS)
    {
        qspSetError(QSP_ERR_TOOMANYVARS);
        return 0;
    }
    if (!globalVar->NameIndex)
        globalVar = qspCreateGlobalVar(nameIndex);
    return qspCreateLocalVar(globalVar);
}

QSPVarData *qspAddGlobalVarByName(QSPString name)
{
    QSPVar *var;
    unsigned int nameHash;
    name = qspPrepareVarName(name, &nameHash); /* validates the name */
    if (qspIsEmpty(name))
    {
        qspSetError(QSP_ERR_INCORRECTVARNAME);
        return 0;
    }
    var = qspGetGlobalVar(qspGetExactNameIndex(name, nameHash, QSP_TRUE), QSP_TRUE);
    return qspGetVarData(var);
}

INLINE void qspInitVarValues(QSPVarData *varData, QSPVariant *vals, int count, QSP_BOOL toMove)
{
    if (count)
    {
        int i;
        varData->ValsCapacity = varData->ValsCount = count;
        varData->Values = (QSPVariant *)malloc(count * sizeof(QSPVariant));
        if (toMove)
        {
            for (i = 0; i < count; ++i)
                qspMoveToNewVariant(varData->Values + i, vals + i);
        }
        else
        {
            for (i = 0; i < count; ++i)
                qspCopyToNewVariant(varData->Values + i, vals + i);
        }
    }
}

int qspAllocateLocalScopeWithArgs(QSPVariant *args, int count, QSP_BOOL toMove)
{
    QSPVar *var;
    int oldLocalScopeBase = qspAllocateLocalScope();

    var = qspCreateLocalVar(qspGetGlobalVar(qspArgsNameIndex, QSP_TRUE));
    qspInitVarValues(qspGetVarData(var), args, count, toMove);

    qspCreateLocalVar(qspGetGlobalVar(qspResultNameIndex, QSP_TRUE));
    return oldLocalScopeBase;
}

QSP_BOOL qspSetArgs(QSPVariant *args, int count, QSP_BOOL toMove)
{
    QSPVarData *varData = qspVarDataReference(qspArgsNameIndex, QSP_REF_GETORCREATE);
    if (!varData) return QSP_FALSE;

    qspEmptyVarData(varData);
    qspInitVarValues(varData, args, count, toMove);
    return QSP_TRUE;
}

QSP_BOOL qspApplyResult(QSPVariant *res)
{
    QSPVarData *varData = qspVarDataReference(qspResultNameIndex, QSP_REF_GET);
    if (!varData) return QSP_FALSE;

    if (varData->ValsCount)
        qspCopyToNewVariant(res, varData->Values);
    else
        qspInitVariant(res, QSP_TYPE_UNDEF);
    return QSP_TRUE;
}

int qspHideLocalVars(void)
{
    int oldLocalVarsBase = qspLocalVarsBase;
    qspLocalVarsBase = qspLocalVarsCount;
    return oldLocalVarsBase;
}

void qspRestoreHiddenLocalVars(int oldLocalVarsBase)
{
    /* Local scopes should be released already */
    qspLocalVarsBase = oldLocalVarsBase;
}

void qspClearHiddenLocalVars(int oldLocalVarsBase)
{
    qspLocalVarsBase = oldLocalVarsBase;
    qspClearLocalVars();
}

INLINE QSPVar *qspVarReference(int nameIndex, QSP_TINYINT mode)
{
    QSPVar *var = qspGetGlobalVar(nameIndex, QSP_FALSE);
    if (var->NameIndex)
    {
        /* Local variable with this name hides the global one */
        if (var->LocalIndex > qspLocalVarsBase)
            return qspLocalVars + var->LocalIndex - 1;
        return var;
    }

    if (mode == QSP_REF_GETORCREATE)
    {
        /* Create it in the global scope */
        if (qspGlobalVarsCount >= QSP_MAXGLOBALVARS)
        {
            qspSetError(QSP_ERR_TOOMANYVARS);
            return 0;
        }
        return qspCreateGlobalVar(nameIndex);
    }
    return &qspNullVar;
}

INLINE QSPVarData *qspVarDataReference(int nameIndex, QSP_TINYINT mode)
{
    QSPVar *var = qspVarReference(nameIndex, mode);
    return (var ? qspGetVarData(var) : 0);
}

QSPVar *qspVarReferenceByName(QSPString name, QSP_TINYINT mode)
{
    int nameIndex;
    unsigned int nameHash;
    name = qspPrepareVarName(name, &nameHash); /* validates the name */
    if (qspIsEmpty(name))
    {
        if (mode != QSP_REF_GETSILENTLY)
            qspSetError(QSP_ERR_INCORRECTVARNAME);
        return 0;
    }
    nameIndex = qspGetExactNameIndex(name, nameHash, (mode == QSP_REF_GETORCREATE));
    if (nameIndex < 0) return &qspNullVar;
    return qspVarReference(nameIndex, mode);
}

INLINE int *qspGetVarKeySlot(QSPVarData *varData, QSPVariant *key, unsigned int hash)
{
    QSPVarKey *curKey;
    int *slot, mask = QSP_SLOTSMASK(varData->KeysCapacity), ind = (int)(hash & mask);
    while (*(slot = varData->KeysSlots + ind) >= 0)
    {
        curKey = varData->Keys + *slot;
        if (curKey->Hash == hash && qspVariantKeysEqual(&curKey->Key, key))
            return slot;
        ind = (ind + 1) & mask;
    }
    return slot;
}

INLINE int *qspGetEmptyVarKeySlot(QSPVarData *varData, unsigned int hash)
{
    int mask = QSP_SLOTSMASK(varData->KeysCapacity), ind = (int)(hash & mask);
    while (varData->KeysSlots[ind] >= 0)
        ind = (ind + 1) & mask;
    return varData->KeysSlots + ind;
}

INLINE int *qspGetVarKeySlotByIndex(QSPVarData *varData, int keyIndex)
{
    int mask = QSP_SLOTSMASK(varData->KeysCapacity), ind = (int)(varData->Keys[keyIndex].Hash & mask);
    while (varData->KeysSlots[ind] != keyIndex)
        ind = (ind + 1) & mask;
    return varData->KeysSlots + ind;
}

INLINE void qspRebuildVarKeysSlots(QSPVarData *varData)
{
    int i, *slot, keysCount = varData->KeysCount, slotsCount = QSP_SLOTSCOUNT(varData->KeysCapacity);
    for (i = 0; i < slotsCount; ++i)
        varData->KeysSlots[i] = -1;
    for (i = 0; i < keysCount; ++i)
    {
        slot = qspGetEmptyVarKeySlot(varData, varData->Keys[i].Hash);
        *slot = i;
    }
}

INLINE void qspResizeVarKeys(QSPVarData *varData, int newCapacity)
{
    varData->KeysCapacity = newCapacity;
    varData->Keys = (QSPVarKey *)realloc(varData->Keys, newCapacity * sizeof(QSPVarKey));
    varData->KeysSlots = (int *)realloc(varData->KeysSlots, QSP_SLOTSCOUNT(newCapacity) * sizeof(int));
    qspRebuildVarKeysSlots(varData);
}

QSPVarKey *qspAddVarKey(QSPVarData *varData, unsigned int hash, int valIndex)
{
    int *slot;
    QSPVarKey *key;
    if (varData->KeysCount >= varData->KeysCapacity)
    {
        int newCapacity = (varData->KeysCapacity ? varData->KeysCapacity * 2 : QSP_VARSKEYSCAPACITY);
        qspResizeVarKeys(varData, newCapacity);
    }
    slot = qspGetEmptyVarKeySlot(varData, hash);
    *slot = varData->KeysCount; /* store the new key index */
    key = varData->Keys + varData->KeysCount;
    key->Hash = hash;
    key->Index = valIndex;
    ++varData->KeysCount;
    return key;
}

INLINE void qspRemoveVarKey(QSPVarData *varData, int keyIndex)
{
    int lastKeyIndex, mask, curInd, curKeyIndex, *slot, *slots;
    if (keyIndex < 0 || keyIndex >= varData->KeysCount) return;
    /* Remove the key & empty the slot */
    qspFreeVariant(&varData->Keys[keyIndex].Key);
    slot = qspGetVarKeySlotByIndex(varData, keyIndex);
    *slot = -1;
    /* Re-insert the following keys up to the next empty slot, lookups stop at empty slots */
    slots = varData->KeysSlots;
    mask = QSP_SLOTSMASK(varData->KeysCapacity);
    curInd = ((int)(slot - slots) + 1) & mask;
    while ((curKeyIndex = slots[curInd]) >= 0)
    {
        slots[curInd] = -1;
        slot = qspGetEmptyVarKeySlot(varData, varData->Keys[curKeyIndex].Hash);
        *slot = curKeyIndex;
        curInd = (curInd + 1) & mask;
    }
    /* The following keys move down, the slots store key indices & have to follow */
    lastKeyIndex = varData->KeysCount - 1;
    if (lastKeyIndex - keyIndex <= varData->KeysCapacity / 16)
    {
        /* Few keys follow, finding their slots is cheaper, break-even is about 1/16 */
        for (curKeyIndex = keyIndex + 1; curKeyIndex <= lastKeyIndex; ++curKeyIndex)
        {
            slot = qspGetVarKeySlotByIndex(varData, curKeyIndex);
            --(*slot);
        }
    }
    else
    {
        /* Update all slots */
        int i, slotsCount = QSP_SLOTSCOUNT(varData->KeysCapacity);
        for (i = 0; i < slotsCount; ++i)
            slots[i] -= (slots[i] > keyIndex);
    }
    memmove(varData->Keys + keyIndex, varData->Keys + keyIndex + 1, (lastKeyIndex - keyIndex) * sizeof(QSPVarKey));
    varData->KeysCount = lastKeyIndex;
}

INLINE void qspMoveVarKey(QSPVarData *varData, int keyIndex, int newKeyIndex)
{
    /* It trusts the passed indices */
    QSPVarKey movedKey = varData->Keys[keyIndex];
    int *keySlot = qspGetVarKeySlotByIndex(varData, keyIndex);
    if (newKeyIndex < keyIndex)
    {
        /* Keys from new position to the old position move up */
        if (keyIndex - newKeyIndex <= varData->KeysCapacity / 16)
        {
            int *curSlot, curKeyIndex;
            for (curKeyIndex = keyIndex - 1; curKeyIndex >= newKeyIndex; --curKeyIndex)
            {
                curSlot = qspGetVarKeySlotByIndex(varData, curKeyIndex);
                ++(*curSlot);
            }
        }
        else
        {
            int i, lastKeyIndex = varData->KeysCount - 1, *slots = varData->KeysSlots, slotsCount = QSP_SLOTSCOUNT(varData->KeysCapacity);
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
        memmove(varData->Keys + newKeyIndex + 1, varData->Keys + newKeyIndex, (keyIndex - newKeyIndex) * sizeof(QSPVarKey));
    }
    else
    {
        /* Keys after the old position up to the new position move down */
        if (newKeyIndex - keyIndex <= varData->KeysCapacity / 16)
        {
            int *curSlot, curKeyIndex;
            for (curKeyIndex = keyIndex + 1; curKeyIndex <= newKeyIndex; ++curKeyIndex)
            {
                curSlot = qspGetVarKeySlotByIndex(varData, curKeyIndex);
                --(*curSlot);
            }
        }
        else
        {
            int i, lastKeyIndex = varData->KeysCount - 1, *slots = varData->KeysSlots, slotsCount = QSP_SLOTSCOUNT(varData->KeysCapacity);
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
        memmove(varData->Keys + keyIndex, varData->Keys + keyIndex + 1, (newKeyIndex - keyIndex) * sizeof(QSPVarKey));
    }
    varData->Keys[newKeyIndex] = movedKey;
    *keySlot = newKeyIndex;
}

INLINE void qspMoveVarItem(QSPVarData *varData, int valIndex, int newIndex)
{
    QSPVariant movedValue;
    int keyIndex, newKeyIndex;
    if (valIndex < 0 || newIndex < 0) return;
    if (valIndex >= varData->ValsCount)
    {
        /* Prepare the value to move */
        QSPVariant undefinedValue = qspGetEmptyVariant(QSP_TYPE_UNDEF);
        valIndex = varData->ValsCount;
        qspSetVarItemValue(varData, valIndex, &undefinedValue);
    }
    if (newIndex == valIndex) return;
    if (newIndex >= varData->ValsCount)
    {
        /* Prepare space for the new value */
        QSPVariant undefinedValue = qspGetEmptyVariant(QSP_TYPE_UNDEF);
        qspSetVarItemValue(varData, newIndex, &undefinedValue);
    }
    keyIndex = qspGetVarKeyIndex(varData, valIndex);
    movedValue = varData->Values[valIndex];
    if (newIndex < valIndex)
    {
        /* Items from new position to the old position move up */
        int i, startKeyIndex, endKeyIndex;
        memmove(varData->Values + newIndex + 1, varData->Values + newIndex, (valIndex - newIndex) * sizeof(QSPVariant));
        startKeyIndex = qspGetVarKeyLowerBound(varData, newIndex);
        endKeyIndex = qspGetVarKeyLowerBound(varData, valIndex);
        newKeyIndex = startKeyIndex;
        for (i = startKeyIndex; i < endKeyIndex; ++i)
            varData->Keys[i].Index++;
    }
    else
    {
        /* Items after the old position up to new position move down */
        int i, startKeyIndex, endKeyIndex;
        memmove(varData->Values + valIndex, varData->Values + valIndex + 1, (newIndex - valIndex) * sizeof(QSPVariant));
        startKeyIndex = qspGetVarKeyLowerBound(varData, valIndex + 1);
        endKeyIndex = qspGetVarKeyLowerBound(varData, newIndex + 1);
        newKeyIndex = endKeyIndex - 1;
        for (i = startKeyIndex; i < endKeyIndex; ++i)
            varData->Keys[i].Index--;
    }
    varData->Values[newIndex] = movedValue;
    if (keyIndex >= 0) /* -1 for keyless item */
    {
        qspMoveVarKey(varData, keyIndex, newKeyIndex);
        varData->Keys[newKeyIndex].Index = newIndex;
    }
}

INLINE void qspRemoveVarItem(QSPVarData *varData, int valIndex)
{
    int keyIndex, lastValIndex = varData->ValsCount - 1;
    if (valIndex < 0 || valIndex > lastValIndex) return;
    /* Move the item to the end */
    qspMoveVarItem(varData, valIndex, lastValIndex);
    /* Remove it */
    keyIndex = qspGetVarKeyIndex(varData, lastValIndex);
    if (keyIndex >= 0) /* -1 for keyless item */
        qspRemoveVarKey(varData, keyIndex);
    qspFreeVariant(varData->Values + lastValIndex);
    varData->ValsCount = lastValIndex;
}

INLINE int qspGetVarKeyLowerBound(QSPVarData *varData, int valIndex)
{
    /* Value index has to be validated already */
    int low, high, keysCount = varData->KeysCount, keylessCount = varData->ValsCount - keysCount;
    /* Search from valIndex - keylessCount to valIndex inclusive */
    low = (valIndex > keylessCount ? valIndex - keylessCount : 0);
    high = (valIndex < keysCount ? valIndex : keysCount);
    if (low < high)
    {
        int keyIndex;
        QSPVarKey *keys = varData->Keys;
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

int qspGetVarKeyIndex(QSPVarData *varData, int valIndex)
{
    /* Value index has to be validated already */
    int keyIndex = qspGetVarKeyLowerBound(varData, valIndex);
    if (keyIndex < varData->KeysCount && varData->Keys[keyIndex].Index == valIndex)
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

int qspGetVarItemIndex(QSPVarData *varData, QSPVariant index, QSP_BOOL toCreate)
{
    unsigned int hash;
    if (qspIsPosIndex(&index))
        return QSP_TOINT(QSP_NUM(index));

    hash = qspGetVariantKeyHash(&index);
    if (varData->KeysCount > 0)
    {
        int *slot = qspGetVarKeySlot(varData, &index, hash);
        if (*slot >= 0) return varData->Keys[*slot].Index;
    }
    if (toCreate)
    {
        /* The key points to a new array item */
        QSPVarKey *key = qspAddVarKey(varData, hash, varData->ValsCount);
        qspCopyToNewVariant(&key->Key, &index);
        /* Make sure the item exists */
        qspReserveVarItem(varData, key->Index);
        return key->Index;
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
        if (!(resolved->VarData = qspVarDataReference(target->NameIndex, mode))) break;
        resolved->Index = qspNumVariant(0);
        return QSP_TRUE;
    case QSP_ARG_EMPTY: /* x[] means a new item */
        if (target->DynamicFields && !qspCalculateTargetFields(target, resolved->Fields, mode)) break;
        if (!(resolved->VarData = qspVarDataReference(target->NameIndex, mode))) break;
        resolved->Index = qspNumVariant(resolved->VarData->ValsCount);
        return QSP_TRUE;
    default: /* regular x[expr] */
        if (!qspCalculateArgValue(&target->Index, &resolved->Index)) break;
        if (target->DynamicFields && !qspCalculateTargetFields(target, resolved->Fields, mode))
        {
            qspFreeVariant(&resolved->Index);
            break;
        }
        if (!(resolved->VarData = qspVarDataReference(target->NameIndex, mode)))
        {
            qspFreeVariant(&resolved->Index);
            break;
        }
        return QSP_TRUE;
    }
    if (target->DynamicFields) qspReleaseMemory(resolved->Fields);
    return QSP_FALSE;
}

INLINE void qspFreeResolvedTarget(QSPResolvedTarget *resolved, QSPCachedTarget *target)
{
    if (target->DynamicFields) qspReleaseMemory(resolved->Fields);
    qspFreeVariant(&resolved->Index);
}

INLINE void qspGetVarItemValue(QSPVarData *varData, int valIndex, QSP_TINYINT varType, QSPVariant *res)
{
    QSPVariant *value = ((valIndex >= 0 && valIndex < varData->ValsCount) ? varData->Values + valIndex : 0);
    qspInitVariantFrom(res, value, varType);
}

INLINE void qspGetVarItemFieldValue(QSPVarData *varData, int valIndex, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *res)
{
    /* The type prefix of the name or the last field specifies the type of the value */
    QSPVariant *value = ((valIndex >= 0 && valIndex < varData->ValsCount) ? varData->Values + valIndex : 0);
    if (count && !QSP_ISDEF(varType)) varType = fields[count - 1].Value.Type;
    value = qspGetTupleFieldValueByPath(value, fields, count);
    qspInitVariantFrom(res, value, varType);
}

void qspGetVarValueByIndex(int nameIndex, QSP_TINYINT varType, QSPVariant index, QSPVariant *res)
{
    int arrIndex;
    QSPVarData *varData = qspVarDataReference(nameIndex, QSP_REF_GET);
    if (!varData)
    {
        qspInitVariant(res, varType);
        return;
    }
    arrIndex = qspGetVarItemIndex(varData, index, QSP_FALSE);
    qspGetVarItemValue(varData, arrIndex, varType, res);
}

void qspGetFirstVarValue(int nameIndex, QSP_TINYINT varType, QSPVariant *res)
{
    QSPVarData *varData = qspVarDataReference(nameIndex, QSP_REF_GET);
    if (!varData)
    {
        qspInitVariant(res, varType);
        return;
    }
    qspGetVarItemValue(varData, 0, varType, res);
}

void qspGetLastVarValue(int nameIndex, QSP_TINYINT varType, QSPVariant *res)
{
    QSPVarData *varData = qspVarDataReference(nameIndex, QSP_REF_GET);
    if (!varData)
    {
        qspInitVariant(res, varType);
        return;
    }
    qspGetVarItemValue(varData, varData->ValsCount - 1, varType, res);
}

void qspGetVarFieldValueByIndex(int nameIndex, QSP_TINYINT varType, QSPVariant index, QSPField *fields, int count, QSPVariant *res)
{
    int arrIndex;
    QSPVarData *varData = qspVarDataReference(nameIndex, QSP_REF_GET);
    if (!varData)
    {
        qspInitVariant(res, varType);
        return;
    }
    arrIndex = qspGetVarItemIndex(varData, index, QSP_FALSE);
    qspGetVarItemFieldValue(varData, arrIndex, varType, fields, count, res);
}

void qspGetFirstVarFieldValue(int nameIndex, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *res)
{
    QSPVarData *varData = qspVarDataReference(nameIndex, QSP_REF_GET);
    if (!varData)
    {
        qspInitVariant(res, varType);
        return;
    }
    qspGetVarItemFieldValue(varData, 0, varType, fields, count, res);
}

void qspGetLastVarFieldValue(int nameIndex, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *res)
{
    QSPVarData *varData = qspVarDataReference(nameIndex, QSP_REF_GET);
    if (!varData)
    {
        qspInitVariant(res, varType);
        return;
    }
    qspGetVarItemFieldValue(varData, varData->ValsCount - 1, varType, fields, count, res);
}

void qspGetVarValueByName(QSPString varName, QSPVariant index, QSPVariant *res)
{
    int arrIndex;
    QSP_TINYINT varType;
    QSPVarData *varData = qspVarDataReferenceByName(varName, QSP_REF_GET);
    if (!varData)
    {
        qspInitVariant(res, QSP_TYPE_UNDEF);
        return;
    }
    arrIndex = qspGetVarItemIndex(varData, index, QSP_FALSE);
    varType = qspGetVarType(varName);
    qspGetVarItemValue(varData, arrIndex, varType, res);
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
    QSPVarData *varData = qspVarDataReferenceByName(varName, QSP_REF_GET);
    if (!varData)
    {
        qspInitVariant(res, QSP_TYPE_UNDEF);
        return;
    }
    count = qspGetFieldsFromPath(path, fields, QSP_REF_GET);
    if (count < 0)
    {
        qspInitVariant(res, QSP_TYPE_UNDEF);
        return;
    }
    arrIndex = qspGetVarItemIndex(varData, index, QSP_FALSE);
    varType = qspGetVarType(varName);
    qspGetVarItemFieldValue(varData, arrIndex, varType, fields, count, res);
}

QSPVarData *qspVarDataByName(QSPString name)
{
    QSPVarData *varData;
    QSPString upperName = qspUpperText(name);
    varData = qspVarDataReferenceByName(upperName, QSP_REF_GETSILENTLY);
    qspFreeNewString(&upperName, &name);
    return varData;
}

QSPString qspGetVarStrValue(QSPString name, int valIndex)
{
    QSPVarData *varData = qspVarDataByName(name);
    if (varData && valIndex >= 0 && valIndex < varData->ValsCount)
        return qspGetVariantAsString(varData->Values + valIndex);
    return qspNullString;
}

QSP_BIGINT qspGetVarNumValue(QSPString name, int valIndex)
{
    QSPVarData *varData = qspVarDataByName(name);
    if (varData && valIndex >= 0 && valIndex < varData->ValsCount)
        return qspGetVariantAsNum(varData->Values + valIndex, 0);
    return 0;
}

INLINE void qspResetTargetValue(QSPCachedTarget *target)
{
    QSPResolvedTarget resolved;
    int arrIndex;
    /* Missing items have nothing to reset, so we don't create them */
    if (!qspResolveTarget(&resolved, target, QSP_REF_GET)) return;
    arrIndex = qspGetVarItemIndex(resolved.VarData, resolved.Index, QSP_FALSE);
    if (arrIndex >= 0 && arrIndex < resolved.VarData->ValsCount)
    {
        QSPVariant *curValue = qspGetTupleFieldValueByPath(resolved.VarData->Values + arrIndex, resolved.Fields, target->FieldsCount);
        if (curValue && QSP_ISDEF(curValue->Type))
        {
            qspFreeVariant(curValue);
            qspInitVariant(curValue, QSP_TYPE_UNDEF);
        }
    }
    qspFreeResolvedTarget(&resolved, target);
}

INLINE QSPVariant *qspReserveVarItem(QSPVarData *varData, int valIndex)
{
    if (valIndex >= 0)
    {
        int oldCount = varData->ValsCount;
        if (valIndex >= oldCount)
        {
            if (valIndex >= varData->ValsCapacity)
            {
                varData->ValsCapacity = (valIndex > 0 ? valIndex + 4 : 1); /* allocate only 1 item for the first value */
                varData->Values = (QSPVariant *)realloc(varData->Values, varData->ValsCapacity * sizeof(QSPVariant));
            }
            varData->ValsCount = valIndex + 1;
            qspInitUndefinedVariants(varData->Values + oldCount, valIndex - oldCount + 1);
        }
        return varData->Values + valIndex;
    }
    return 0;
}

INLINE void qspSetVarItemValue(QSPVarData *varData, int valIndex, QSPVariant *val)
{
    /* The value has to be converted to the type of the variable already */
    QSPVariant *item = qspReserveVarItem(varData, valIndex);
    if (!item) return;
    qspFreeVariant(item);
    qspMoveToNewVariant(item, val);
}

INLINE void qspSetVarItemFieldValue(QSPVarData *varData, int valIndex, QSPField *fields, int count, QSPVariant *val)
{
    /* The value has to be converted to the type of the variable already */
    QSPVariant *item = qspReserveVarItem(varData, valIndex);
    if (!item) return;
    item = qspReserveTupleFieldValueByPath(item, fields, count);
    qspFreeVariant(item);
    qspMoveToNewVariant(item, val);
}

INLINE void qspSetVarFieldValue(QSPVarData *varData, QSPVariant index, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *val, QSP_CHAR op)
{
    int arrIndex = -1;
    if (count && !QSP_ISDEF(varType)) varType = fields[count - 1].Value.Type;
    if (op != QSP_EQUAL_CHAR)
    {
        /* Replace the value with its combination with the current value */
        QSPVariant oldVal, res;
        arrIndex = qspGetVarItemIndex(varData, index, QSP_FALSE);
        qspGetVarItemFieldValue(varData, arrIndex, varType, fields, count, &oldVal);
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
        arrIndex = qspGetVarItemIndex(varData, index, QSP_TRUE);
    qspSetVarItemFieldValue(varData, arrIndex, fields, count, val);
}

INLINE void qspSetTargetValue(QSPCachedTarget *target, QSPVariant *val, QSP_CHAR op)
{
    QSPResolvedTarget resolved;
    int arrIndex = -1;
    if (!qspResolveTarget(&resolved, target, QSP_REF_GETORCREATE)) return;
    if (target->FieldsCount)
    {
        qspSetVarFieldValue(resolved.VarData, resolved.Index, target->VarType, resolved.Fields, target->FieldsCount, val, op);
        qspFreeResolvedTarget(&resolved, target);
        return;
    }
    if (op != QSP_EQUAL_CHAR)
    {
        /* Replace the value with its combination with the current value */
        QSPVariant oldVal, res;
        arrIndex = qspGetVarItemIndex(resolved.VarData, resolved.Index, QSP_FALSE);
        qspGetVarItemValue(resolved.VarData, arrIndex, target->VarType, &oldVal);
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
    if (!qspConvertVariantTo(val, target->VarType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        qspFreeResolvedTarget(&resolved, target);
        return;
    }
    if (arrIndex < 0)
        arrIndex = qspGetVarItemIndex(resolved.VarData, resolved.Index, QSP_TRUE);
    qspSetVarItemValue(resolved.VarData, arrIndex, val);
    qspFreeResolvedTarget(&resolved, target);
}

INLINE void qspMoveTupleToArray(QSPVarData *dest, QSPTuple *src, int start, int count)
{
    int i, itemsToMove, keysCount;
    QSPVariant keyVal;
    QSPVarKey *key;
    QSPField *field;
    /* Clear the dest array anyway */
    qspEmptyVarData(dest);
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

INLINE void qspCopyArray(QSPVarData *dest, QSPVarData *src, int start, int count)
{
    int i, itemsToCopy, keyIndex, keysCount;
    /* Clear the dest array anyway */
    qspEmptyVarData(dest);
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

INLINE void qspMoveArray(QSPVarData *dest, QSPVarData *src, int start, int count)
{
    int i, itemsToMove, keyIndex, keysCount;
    /* Clear the dest array anyway */
    qspEmptyVarData(dest);
    /* Validate parameters */
    if (count <= 0) return;
    if (start < 0) start = 0;
    itemsToMove = src->ValsCount - start;
    if (itemsToMove <= 0) return;
    if (count < itemsToMove) itemsToMove = count;
    if (itemsToMove == src->ValsCount)
    {
        /* Move everything */
        qspMoveVarData(dest, src);
        return;
    }
    /* Get key range of the moved items */
    keyIndex = qspGetVarKeyLowerBound(src, start);
    keysCount = qspGetVarKeyLowerBound(src, start + itemsToMove) - keyIndex;
    /* Move array values, the following values get shifted */
    dest->ValsCapacity = dest->ValsCount = itemsToMove;
    dest->Values = (QSPVariant *)malloc(itemsToMove * sizeof(QSPVariant));
    src->ValsCount -= itemsToMove;
    memcpy(dest->Values, src->Values + start, itemsToMove * sizeof(QSPVariant));
    memmove(src->Values + start, src->Values + start + itemsToMove, (src->ValsCount - start) * sizeof(QSPVariant));
    if (keysCount > 0)
    {
        if (keysCount == src->KeysCount)
        {
            /* Move all keys */
            for (i = 0; i < keysCount; ++i)
                src->Keys[i].Index -= start;
            dest->Keys = src->Keys;
            dest->KeysSlots = src->KeysSlots;
            dest->KeysCount = keysCount;
            dest->KeysCapacity = src->KeysCapacity;
            src->Keys = 0;
            src->KeysSlots = 0;
            src->KeysCount = 0;
            src->KeysCapacity = 0;
        }
        else
        {
            /* Move keys, the following keys get shifted */
            int capacity = QSP_VARSKEYSCAPACITY;
            QSPVarKey *destKey, *srcKey = src->Keys + keyIndex;
            while (capacity < keysCount) capacity *= 2;
            qspResizeVarKeys(dest, capacity);
            for (i = 0; i < keysCount; ++i, ++srcKey)
            {
                destKey = qspAddVarKey(dest, srcKey->Hash, srcKey->Index - start);
                qspMoveToNewVariant(&destKey->Key, &srcKey->Key);
            }
            src->KeysCount -= keysCount;
            memmove(src->Keys + keyIndex, src->Keys + keyIndex + keysCount, (src->KeysCount - keyIndex) * sizeof(QSPVarKey));
            qspRebuildVarKeysSlots(src);
        }
    }
    /* Items of the following keys get shifted */
    for (i = keyIndex; i < src->KeysCount; ++i)
        src->Keys[i].Index -= itemsToMove;
}

QSPTuple qspCopyArrayToNewTuple(QSPVarData *src, int start, int count)
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

INLINE void qspSortArray(QSPVarData *varData, QSP_TINYINT varType, QSP_BOOL isAscending)
{
    QSP_TINYINT valsType;
    QSPVariant *curValue, *sortedValues, **valuePositions;
    int i, keysCount, valsCount;
    valsCount = varData->ValsCount;
    if (valsCount < 2) return;
    if (!qspGetArrayValsType(varData, &valsType))
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
    curValue = varData->Values;
    for (i = 0; i < valsCount; ++i, ++curValue)
        valuePositions[i] = curValue;
    /* Sort positions of values by comparing values */
    if (isAscending)
        qsort(valuePositions, valsCount, sizeof(QSPVariant *), qspValuePositionsAscCompare);
    else
        qsort(valuePositions, valsCount, sizeof(QSPVariant *), qspValuePositionsDescCompare);
    /* Reorder keys, they follow their items & stay ordered by index in Values */
    keysCount = varData->KeysCount;
    if (keysCount > 0)
    {
        QSPVarKey *curKey, *sortedKeys;
        int keyIndex, *keyIndices;
        /* Key index of every item, -1 without a key */
        keyIndices = (int *)malloc(valsCount * sizeof(int));
        for (i = 0; i < valsCount; ++i)
            keyIndices[i] = -1;
        for (i = 0; i < keysCount; ++i)
            keyIndices[varData->Keys[i].Index] = i;
        /* Place the keys in the order of their sorted items */
        sortedKeys = (QSPVarKey *)malloc(varData->KeysCapacity * sizeof(QSPVarKey));
        curKey = sortedKeys;
        for (i = 0; i < valsCount; ++i)
        {
            keyIndex = keyIndices[valuePositions[i] - varData->Values];
            if (keyIndex >= 0)
            {
                *curKey = varData->Keys[keyIndex];
                curKey->Index = i;
                ++curKey;
            }
        }
        free(keyIndices);
        free(varData->Keys);
        varData->Keys = sortedKeys;
        qspRebuildVarKeysSlots(varData); /* the slots follow the new order */
    }
    /* Reorder values */
    sortedValues = (QSPVariant *)malloc(valsCount * sizeof(QSPVariant));
    curValue = sortedValues;
    for (i = 0; i < valsCount; ++i, ++curValue)
        qspMoveToNewVariant(curValue, valuePositions[i]);
    free(valuePositions);
    free(varData->Values);
    varData->Values = sortedValues;
    varData->ValsCapacity = valsCount;
}

int qspArraySize(QSPString varName)
{
    QSPVarData *varData = qspVarDataReferenceByName(varName, QSP_REF_GET);
    if (!varData) return 0;
    return varData->ValsCount;
}

int qspArrayPos(QSPString varName, QSPVariant *val, int ind)
{
    QSP_TINYINT varType;
    QSPVariant defaultValue, *curValue;
    QSPVarData *varData = qspVarDataReferenceByName(varName, QSP_REF_GET);
    if (!varData) return -1;
    varType = qspGetVarType(varName);
    if (!qspConvertVariantTo(val, varType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return -1;
    }
    defaultValue = qspGetEmptyVariant(QSP_BASETYPE(val->Type));
    if (ind < 0) ind = 0;
    while (ind < varData->ValsCount)
    {
        curValue = varData->Values + ind;
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
    QSPVarData *varData = qspVarDataReferenceByName(varName, QSP_REF_GET);
    if (!varData) return -1;
    regExp = qspRegExpGetCompiled(regExpStr);
    if (!regExp) return -1;
    defaultValue = qspGetEmptyVariant(QSP_TYPE_STR);
    if (ind < 0) ind = 0;
    while (ind < varData->ValsCount)
    {
        curValue = varData->Values + ind;
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
    QSPVarData *varData = qspVarDataReferenceByName(varName, QSP_REF_GET);
    if (!varData) return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    varType = qspGetVarType(varName);
    if (QSP_ISDEF(varType))
        valsType = varType;
    else if (!qspGetArrayValsType(varData, &valsType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    }
    resultValue = qspGetEmptyVariant(valsType);
    if (!QSP_ISDEF(valsType)) return resultValue; /* nothing but holes */
    bestValue = 0;
    for (i = 0; i < varData->ValsCount; ++i)
    {
        curValue = varData->Values + i;
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
    QSPVariant v;
    QSPCachedAssignment *assignment = stat->Data.Assignment;
    if (!qspCalculateArgValue(&assignment->Value, &v)) return;
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
        /* We have to evaluate expression before allocation of local vars */
        if (!qspCalculateArgValue(&assignment->Value, &v)) return;
    }
    else
        v = qspGetEmptyVariant(QSP_TYPE_UNDEF);
    for (i = assignment->TargetsCount, target = assignment->Targets; i > 0; --i, ++target)
    {
        if (!qspAddLocalVar(target->NameIndex))
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
    int i, namesCount;
    QSPVariant names;
    QSPVar *var;
    QSPSharedVarData *sharedVars[QSP_MAXSTATARGS];
    QSPCachedAssignment *assignment = stat->Data.Assignment;
    /* We have to evaluate expression before allocation of local vars */
    if (!qspCalculateArgValue(&assignment->Value, &names)) return;
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
                while (--i >= 0) qspReleaseSharedVarData(sharedVars[i]);
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
        if (!((var = qspAddLocalVar(assignment->Targets[i].NameIndex))))
        {
            while (i < namesCount)
            {
                qspReleaseSharedVarData(sharedVars[i]);
                ++i;
            }
            return;
        }
        qspMoveReferenceToVar(var, sharedVars[i]);
    }
}

void qspStatementSetVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPField fields[QSP_MAXMATHOPARGS];
    QSPVariant index;
    QSP_TINYINT varType;
    int fieldsCount = 0;
    QSPVarData *varData = qspVarDataReferenceByName(QSP_STR(args[0]), QSP_REF_GETORCREATE);
    if (!varData) return;
    varType = qspGetVarType(QSP_STR(args[0]));
    if (count == 4)
    {
        fieldsCount = qspGetFieldsFromPath(QSP_TUPLE(args[3]), fields, QSP_REF_GETORCREATE);
        if (fieldsCount < 0) return;
    }
    index = (count == 2 ? qspNumVariant(0) : args[2]);
    qspSetVarFieldValue(varData, index, varType, fields, fieldsCount, args + 1, QSP_EQUAL_CHAR);
}

void qspStatementInsVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    int valIndex, newIndex;
    QSP_TINYINT varType;
    QSPVarData *varData = qspVarDataReferenceByName(QSP_STR(args[0]), QSP_REF_GETORCREATE);
    if (!varData) return;
    varType = qspGetVarType(QSP_STR(args[0]));
    if (!qspConvertVariantTo(args + 1, varType))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return;
    }
    if (count == 2)
    {
        /* New last item */
        qspSetVarItemValue(varData, varData->ValsCount, args + 1);
        return;
    }
    if (qspIsPosIndex(args + 2))
    {
        /* Insert by position */
        if (count == 3)
        {
            /* New last item, move it to the specified position */
            valIndex = varData->ValsCount;
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
            valIndex = qspGetVarItemIndex(varData, args[2], QSP_TRUE);
            newIndex = valIndex;
        }
        else
        {
            /* The item of the key ends up at the new position */
            newIndex = QSP_TOINT(QSP_NUM(args[3]));
            if (newIndex < 0) return;
            valIndex = qspGetVarItemIndex(varData, args[2], QSP_TRUE);
        }
    }
    /* Move item to the new position and update the value */
    qspMoveVarItem(varData, valIndex, newIndex);
    qspSetVarItemValue(varData, newIndex, args + 1);
}

void qspStatementUnpackArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    int startInd, maxCount;
    QSPTuple *src;
    QSPVarData *dest = qspVarDataReferenceByName(QSP_STR(args[0]), QSP_REF_GETORCREATE);
    if (!dest) return;
    src = &QSP_TUPLE(args[1]);
    startInd = (count >= 3 ? QSP_TOINT(QSP_NUM(args[2])) : 0);
    maxCount = (count == 4 ? QSP_TOINT(QSP_NUM(args[3])) : src->FieldsCount);
    qspMoveTupleToArray(dest, src, startInd, maxCount);
}

void qspStatementMoveArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg)
{
    QSPVarData *dest, *src;
    if (!((dest = qspVarDataReferenceByName(QSP_STR(args[0]), QSP_REF_GETORCREATE)))) return;
    if (!((src = qspVarDataReferenceByName(QSP_STR(args[1]), QSP_REF_GET)))) return;
    if (dest != src)
    {
        int startInd = (count >= 3 ? QSP_TOINT(QSP_NUM(args[2])) : 0);
        int maxCount = (count == 4 ? QSP_TOINT(QSP_NUM(args[3])) : src->ValsCount);
        switch (extArg)
        {
        case QSP_STAT_COPYARR:
            qspCopyArray(dest, src, startInd, maxCount);
            break;
        case QSP_STAT_MOVEARR:
            qspMoveArray(dest, src, startInd, maxCount);
            break;
        }
    }
}

void qspStatementSortArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSP_TINYINT varType;
    QSPVarData *varData = qspVarDataReferenceByName(QSP_STR(args[0]), QSP_REF_GET); /* we don't create a new array */
    if (!varData) return;
    varType = qspGetVarType(QSP_STR(args[0]));
    if (count == 2)
        qspSortArray(varData, varType, QSP_ISFALSE(QSP_NUM(args[1])));
    else
        qspSortArray(varData, varType, QSP_TRUE);
}

void qspStatementScanStr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPRegExp *regExp;
    QSP_CHAR *foundPos;
    QSPString text;
    QSPVariant foundString;
    int curInd, groupInd, foundLen;
    QSPVarData *varData = qspVarDataReferenceByName(QSP_STR(args[0]), QSP_REF_GETORCREATE);
    if (!varData) return;
    regExp = qspRegExpGetCompiled(QSP_STR(args[2]));
    if (!regExp) return;
    text = QSP_STR(args[1]);
    groupInd = (count == 4 ? QSP_TOINT(QSP_NUM(args[3])) : 0);
    qspEmptyVarData(varData); /* clear the dest array anyway */
    foundString.Type = QSP_TYPE_STR;
    curInd = 0;
    foundPos = qspRegExpStrSearch(regExp, text, 0, groupInd, &foundLen);
    while (foundPos && foundLen)
    {
        QSP_STR(foundString) = qspStringFromLen(foundPos, foundLen);
        if (curInd >= varData->ValsCapacity)
        {
            varData->ValsCapacity = curInd + 8;
            varData->Values = (QSPVariant *)realloc(varData->Values, varData->ValsCapacity * sizeof(QSPVariant));
        }
        qspCopyToNewVariant(varData->Values + curInd, &foundString);
        ++curInd;
        foundPos = qspRegExpStrSearch(regExp, text, foundPos + foundLen, groupInd, &foundLen);
    }
    varData->ValsCount = curInd;
}

void qspStatementKillVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (count)
    {
        QSPVarData *varData = qspVarDataReferenceByName(QSP_STR(args[0]), QSP_REF_GET); /* we don't create a new array */
        if (!varData) return;
        if (count > 1)
        {
            /* The item or its field */
            QSPField fields[QSP_MAXMATHOPARGS];
            int fieldsCount = 0, arrIndex = qspGetVarItemIndex(varData, args[1], QSP_FALSE);
            if (count == 3)
            {
                fieldsCount = qspGetFieldsFromPath(QSP_TUPLE(args[2]), fields, QSP_REF_GET);
                if (fieldsCount < 0) return;
            }
            if (arrIndex >= 0 && arrIndex < varData->ValsCount)
            {
                if (fieldsCount)
                {
                    /* The last field is the one that was specified */
                    QSPVariant *value = qspGetTupleFieldValueByPath(varData->Values + arrIndex, fields, fieldsCount - 1);
                    if (value && QSP_ISTUPLE(value->Type))
                        qspRemoveTupleField(&QSP_PTUPLE(value), fields[fieldsCount - 1].NameIndex);
                }
                else
                {
                    /* Empty path removes the item itself */
                    qspRemoveVarItem(varData, arrIndex);
                }
            }
        }
        else
            qspEmptyVarData(varData);
    }
    else
        qspClearGlobalVars();
}
