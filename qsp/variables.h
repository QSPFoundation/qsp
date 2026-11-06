/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "codetools.h"
#include "names.h"
#include "text.h"
#include "variant.h"

#ifndef QSP_VARSDEFINES
    #define QSP_VARSDEFINES

    #define QSP_MAXGLOBALVARS 100000
    #define QSP_MAXLOCALVARS 500
    #define QSP_VARSGLOBALCAPACITY 512
    #define QSP_VARSLOCALCAPACITY 8
    #define QSP_VARSSCOPECHUNKSIZE 128
    #define QSP_VARSKEYSCAPACITY 4 /* has to be a power of 2 */

    #define QSP_VAR_ARGS QSP_FMT("ARGS")
    #define QSP_VAR_RESULT QSP_FMT("RESULT")
    #define QSP_VAR_DEBUG QSP_FMT("DEBUG")

    typedef struct
    {
        QSPVariant Key;
        unsigned int Hash;
        int Index; /* index in Values */
    } QSPVarKey;

    typedef struct
    {
        QSPVariant *Values;
        int ValsCount;
        int ValsCapacity;
        QSPVarKey *Keys; /* ordered by index in Values */
        int *KeysSlots; /* hash table by keys, QSP_SLOTSCOUNT(KeysCapacity) slots */
        int KeysCount;
        int KeysCapacity; /* has to be a power of 2 */
    } QSPVar;

    typedef struct
    {
        QSPVar Var;
        int RefsCount;
    } QSPSharedVar;

    typedef struct
    {
        int NameIndex; /* 0 for an empty slot */
        QSP_BOOL IsRef;
        union
        {
            QSPVar Var;
            QSPSharedVar *Shared;
        } Data;
    } QSPVarSlot;

    typedef struct
    {
        QSPVarSlot *VarSlots; /* hash table by name indices, QSP_SLOTSCOUNT(Capacity) slots */
        int VarsCount;
        int Capacity; /* has to be a power of 2 */
    } QSPVarsScope;

    typedef struct QSPVarsScopeChunk_s QSPVarsScopeChunk;

    typedef struct QSPVarsScopeChunk_s
    {
        QSPVarsScope Slots[QSP_VARSSCOPECHUNKSIZE];
        int SlotsCount;
        QSPVarsScopeChunk *ParentChunk;
    } QSPVarsScopeChunk;

    typedef struct
    {
        QSPVar *Var;
        QSPVariant Index;
        QSPField *Fields;
    } QSPResolvedTarget;

    enum
    {
        QSP_REF_GET,
        QSP_REF_GETORCREATE,
        QSP_REF_GETSILENTLY /* no errors reported */
    };

    extern QSPVarSlot qspNullVarSlot;
    extern QSPVarsScope qspGlobalVars; /* there's only one global scope, we don't recreate it */
    extern QSPVarsScopeChunk *qspCurrentLocalVars; /* local scopes can be recreated */

    extern QSP_TINYINT qspSpecToBaseTypeTable[128];

    /* External functions */
    void qspInitVarTypes(void);
    QSPVarsScopeChunk *qspAllocateVarsScopeChunk(QSPVarsScopeChunk *parentChunk);
    void qspClearVarsScopeChunk(QSPVarsScopeChunk *chunk);
    void qspInitVarsScope(QSPVarsScope *scope, int capacity);
    void qspClearVarsScope(QSPVarsScope *scope);
    void qspClearVars(QSPVarsScope *scope);
    void qspClearLocalVarsScopes(QSPVarsScopeChunk *chunk);
    void qspClearAllVars(QSP_BOOL toInit);
    void qspRegisterSpecialVariables(void);
    QSPVar *qspAddVarToScope(QSPVarsScope *scope, QSPString name);
    QSPVarsScope *qspAllocateLocalScopeWithArgs(QSPVariant *args, int count, QSP_BOOL toMove);
    QSP_BOOL qspSetArgs(QSPVariant *args, int count, QSP_BOOL toMove);
    QSP_BOOL qspApplyResult(QSPVariant *res);
    QSPVarsScopeChunk *qspSaveLocalVarsAndRestoreGlobals(void);
    void qspRestoreSavedLocalVars(QSPVarsScopeChunk *chunk);
    QSPVarSlot *qspVarSlotReferenceByName(QSPString name, QSP_TINYINT mode);
    QSPVarKey *qspAddVarKey(QSPVar *var, unsigned int hash, int valIndex);
    int qspGetVarKeyIndex(QSPVar *var, int valIndex);
    int qspGetVarItemIndex(QSPVar *var, QSPVariant index, QSP_BOOL toCreate);
    void qspGetVarValueByIndex(int nameIndex, QSP_TINYINT varType, QSPVariant index, QSPVariant *res);
    void qspGetFirstVarValue(int nameIndex, QSP_TINYINT varType, QSPVariant *res);
    void qspGetLastVarValue(int nameIndex, QSP_TINYINT varType, QSPVariant *res);
    void qspGetVarFieldValueByIndex(int nameIndex, QSP_TINYINT varType, QSPVariant index, QSPField *fields, int count, QSPVariant *res);
    void qspGetFirstVarFieldValue(int nameIndex, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *res);
    void qspGetLastVarFieldValue(int nameIndex, QSP_TINYINT varType, QSPField *fields, int count, QSPVariant *res);
    void qspGetVarValueByName(QSPString varName, QSPVariant index, QSPVariant *res);
    QSP_BOOL qspGetFieldNameIndex(QSPString name, QSP_TINYINT mode, int *nameIndex);
    QSP_BOOL qspInitPlainField(QSPField *field, QSPString name);
    QSP_BOOL qspInitQuotedField(QSPField *field, QSPString name, QSP_TINYINT fieldType);
    QSP_BOOL qspParseFieldName(QSPString *s, QSPField *field, QSPString *code);
    int qspParseFieldsPath(QSPString *s, QSP_TINYINT varType, QSPField *fields, QSPString *dynamicNames);
    int qspGetFieldsFromPath(QSPTuple path, QSPField *fields, QSP_TINYINT mode);
    void qspGetVarFieldValueByPath(QSPString varName, QSPVariant index, QSPTuple path, QSPVariant *res);
    QSPVar *qspVarByName(QSPString name);
    QSPString qspGetVarStrValue(QSPString name, int valIndex);
    QSP_BIGINT qspGetVarNumValue(QSPString name, int valIndex);
    QSPTuple qspCopyArrayToNewTuple(QSPVar *src, int start, int count);
    int qspArraySize(QSPString varName);
    int qspArrayPos(QSPString varName, QSPVariant *val, int ind);
    int qspArrayPosRegExp(QSPString varName, QSPString regExpStr, int ind);
    QSPVariant qspArrayMinMaxItem(QSPString varName, QSP_BOOL isMin);
    /* Statements */
    void qspStatementSetVarsValues(QSPCachedStat *stat);
    void qspStatementLocal(QSPCachedStat *stat);
    void qspStatementAlias(QSPCachedStat *stat);
    void qspStatementSetVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementInsVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementUnpackArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementCopyArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementSortArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementScanStr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementKillVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

    INLINE QSP_TINYINT qspGetVarType(QSPString name)
    {
        QSP_CHAR specSymbol = *name.Str;
        if (specSymbol < sizeof(qspSpecToBaseTypeTable))
            return qspSpecToBaseTypeTable[specSymbol];

        return QSP_TYPE_UNDEF;
    }

    INLINE QSPString qspPrepareVarName(QSPString name, unsigned int *nameHash)
    {
        QSP_CHAR *pos = name.Str, *end = name.End;

        /* Validate the name, skip its type prefix & hash it */
        if (pos < end && qspIsInClass(*pos, QSP_CHAR_TYPEPREFIX))
            name.Str = ++pos;

        if (pos == end || qspIsInClass(*pos, QSP_CHAR_DIGIT))
            return qspNullString;

        if (nameHash)
        {
            unsigned int hash = QSP_HASHSEED;
            do
            {
                if (qspIsInClass(*pos, QSP_CHAR_DELIM | QSP_CHAR_FIELDREF))
                    return qspNullString;
                hash = qspAddValueToHash(hash, *pos);
            } while (++pos < end);
            *nameHash = qspFinalizeHash(hash);
        }
        else
        {
            do
            {
                if (qspIsInClass(*pos, QSP_CHAR_DELIM | QSP_CHAR_FIELDREF))
                    return qspNullString;
            } while (++pos < end);
        }
        return name;
    }

    INLINE void qspInitVarData(QSPVar *var)
    {
        var->Values = 0;
        var->ValsCount = 0;
        var->ValsCapacity = 0;
        var->Keys = 0;
        var->KeysSlots = 0;
        var->KeysCount = 0;
        var->KeysCapacity = 0;
    }

    INLINE void qspMoveVar(QSPVar *dest, QSPVar *src)
    {
        dest->Values = src->Values;
        dest->ValsCount = src->ValsCount;
        dest->ValsCapacity = src->ValsCapacity;
        dest->Keys = src->Keys;
        dest->KeysSlots = src->KeysSlots;
        dest->KeysCount = src->KeysCount;
        dest->KeysCapacity = src->KeysCapacity;
        qspInitVarData(src);
    }

    INLINE void qspEmptyVar(QSPVar *var)
    {
        if (var->Values)
        {
            qspFreeVariants(var->Values, var->ValsCount);
            free(var->Values);
        }
        if (var->Keys)
        {
            QSPVarKey *curKey;
            int count = var->KeysCount;
            for (curKey = var->Keys; count > 0; --count, ++curKey)
                qspFreeVariant(&curKey->Key);
            free(var->Keys);
            free(var->KeysSlots);
        }
        qspInitVarData(var);
    }

    INLINE QSPVar *qspGetVarFromSlot(QSPVarSlot *slot)
    {
        return (slot->IsRef ? &slot->Data.Shared->Var : &slot->Data.Var);
    }

    INLINE QSPVar *qspVarReferenceByName(QSPString name, QSP_TINYINT mode)
    {
        QSPVarSlot *slot = qspVarSlotReferenceByName(name, mode);
        return (slot ? qspGetVarFromSlot(slot) : 0);
    }

    INLINE QSPVarSlot qspGetUnknownVarSlot(void)
    {
        QSPVarSlot slot;
        slot.NameIndex = 0;
        slot.IsRef = QSP_FALSE;
        qspInitVarData(&slot.Data.Var);
        return slot;
    }

    INLINE QSPVarsScope *qspAllocateLocalScope(void)
    {
        QSPVarsScopeChunk *chunk = qspCurrentLocalVars;
        if (chunk && chunk->SlotsCount < QSP_VARSSCOPECHUNKSIZE)
            return &chunk->Slots[chunk->SlotsCount++];

        chunk = qspAllocateVarsScopeChunk(chunk);
        chunk->SlotsCount = 1;
        qspCurrentLocalVars = chunk;
        return chunk->Slots;
    }

    INLINE void qspReleaseLastLocalScope(void)
    {
        QSPVarsScopeChunk *chunk = qspCurrentLocalVars;
        if (chunk)
        {
            QSPVarsScope *scope = &chunk->Slots[--chunk->SlotsCount];
            if (scope->Capacity > QSP_VARSLOCALCAPACITY)
            {
                qspClearVarsScope(scope); /* the next scope starts small */
                scope->VarSlots = 0;
                scope->Capacity = 0;
            }
            else if (scope->VarsCount)
                qspClearVars(scope);

            if (!chunk->SlotsCount)
            {
                qspCurrentLocalVars = chunk->ParentChunk;
                qspClearVarsScopeChunk(chunk);
            }
        }
    }

#endif
