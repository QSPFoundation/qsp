/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "codetools.h"
#include "text.h"
#include "variant.h"

#ifndef QSP_VARSDEFINES
    #define QSP_VARSDEFINES

    #define QSP_MAXGLOBALVARS 100000
    #define QSP_MAXLOCALVARS 500
    #define QSP_VARSGLOBALCAPACITY 1024
    #define QSP_VARSLOCALCAPACITY 16
    #define QSP_VARSNAMECHARSPERSLOT 2
    #define QSP_VARSSCOPECHUNKSIZE 128
    #define QSP_VARSINDICESCAPACITY 4
    #define QSP_VARARGS QSP_FMT("ARGS")
    #define QSP_VARRES QSP_FMT("RESULT")

    typedef struct
    {
        QSPString Str;
        unsigned int Hash;
        int Index; /* in Values */
    } QSPVarIndex;

    typedef struct
    {
        QSPVariant *Values;
        int ValsCount;
        int ValsCapacity;
        QSPVarIndex *Indices;
        int *IndsSlots; /* hash table by text indices, IndsCapacity * 2 slots */
        int IndsCount;
        int IndsCapacity; /* has to be a power of 2 */
    } QSPVar;

    typedef struct
    {
        QSPString Name; /* points into the scope's Names, Str is 0 for an empty slot */
        unsigned int NameHash;
        QSPVar Var;
    } QSPVarSlot;

    typedef struct
    {
        QSPVarSlot *VarSlots;
        int VarsCount;
        int Capacity; /* has to be a power of 2 */
        QSP_CHAR *Names;
        int NamesLen;
        int NamesCapacity;
    } QSPVarsScope;

    typedef struct QSPVarsScopeChunk_s QSPVarsScopeChunk;

    typedef struct QSPVarsScopeChunk_s
    {
        QSPVarsScope Slots[QSP_VARSSCOPECHUNKSIZE];
        int SlotsCount;
        QSPVarsScopeChunk *ParentChunk;
    } QSPVarsScopeChunk;

    extern QSPVar qspNullVar;
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
    QSPVar *qspAddVarToScope(QSPVarsScope *scope, QSPString name);
    QSPVarsScope *qspAllocateLocalScopeWithArgs(QSPVariant *args, int count, QSP_BOOL toMove);
    QSP_BOOL qspSetArgs(QSPVariant *args, int count, QSP_BOOL toMove);
    QSP_BOOL qspApplyResult(QSPVariant *res);
    QSPVarsScopeChunk *qspSaveLocalVarsAndRestoreGlobals(void);
    void qspRestoreSavedLocalVars(QSPVarsScopeChunk *chunk);
    QSPVar *qspVarReference(QSPString name, QSP_BOOL toCreate);
    QSPVarIndex *qspAddVarIndex(QSPVar *var, unsigned int hash);
    int qspGetVarIndex(QSPVar *var, QSPVariant index, QSP_BOOL toCreate);
    QSP_BOOL qspGetVarValueByIndex(QSPString varName, QSPVariant index, QSPVariant *res);
    QSP_BOOL qspGetFirstVarValue(QSPString varName, QSPVariant *res);
    QSP_BOOL qspGetLastVarValue(QSPString varName, QSPVariant *res);
    QSPString qspGetVarStrValue(QSPString name);
    QSP_BIGINT qspGetVarNumValue(QSPString name);
    int qspArraySize(QSPString varName);
    int qspArrayPos(QSPString varName, QSPVariant *val, int ind);
    int qspArrayPosRegExp(QSPString varName, QSPString regExpStr, int ind);
    QSPVariant qspArrayMinMaxItem(QSPString varName, QSP_BOOL isMin);
    /* Statements */
    void qspStatementSetVarsValues(QSPCachedStat *stat);
    void qspStatementLocal(QSPCachedStat *stat);
    void qspStatementSetVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementUnpackArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementCopyArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementSortArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementScanStr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
    void qspStatementKillVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

    INLINE QSP_TINYINT qspGetVarType(QSPString str)
    {
        QSP_CHAR specSymbol = *str.Str;
        if (specSymbol < sizeof(qspSpecToBaseTypeTable))
            return qspSpecToBaseTypeTable[specSymbol];

        return QSP_TYPE_NUM;
    }

    INLINE QSPString qspPrepareVarName(QSPString name, unsigned int *nameHash)
    {
        unsigned int hash = QSP_TEXTHASHSEED;
        QSP_CHAR *pos = name.Str, *end = name.End;

        /* Validate the name, skip its type prefix & hash it */
        if (pos < end && qspIsInClass(*pos, QSP_CHAR_TYPEPREFIX))
            name.Str = ++pos;

        if (pos == end || qspIsInClass(*pos, QSP_CHAR_DIGIT))
            return qspNullString;
        do
        {
            if (qspIsInClass(*pos, QSP_CHAR_DELIM))
                return qspNullString;
            hash = qspAddCharToTextHash(hash, *pos);
        } while (++pos < end);
        *nameHash = qspFinalizeTextHash(hash);
        return name;
    }

    INLINE void qspInitVarData(QSPVar *var)
    {
        var->Values = 0;
        var->ValsCount = 0;
        var->ValsCapacity = 0;
        var->Indices = 0;
        var->IndsSlots = 0;
        var->IndsCount = 0;
        var->IndsCapacity = 0;
    }

    INLINE void qspMoveVar(QSPVar *dest, QSPVar *src)
    {
        dest->Values = src->Values;
        dest->ValsCount = src->ValsCount;
        dest->ValsCapacity = src->ValsCapacity;
        dest->Indices = src->Indices;
        dest->IndsSlots = src->IndsSlots;
        dest->IndsCount = src->IndsCount;
        dest->IndsCapacity = src->IndsCapacity;
        qspInitVarData(src);
    }

    INLINE void qspEmptyVar(QSPVar *var)
    {
        if (var->Values)
        {
            qspFreeVariants(var->Values, var->ValsCount);
            free(var->Values);
        }
        if (var->Indices)
        {
            QSPVarIndex *curIndex;
            int count = var->IndsCount;
            for (curIndex = var->Indices; count > 0; --count, ++curIndex)
                qspFreeString(&curIndex->Str);
            free(var->Indices);
            free(var->IndsSlots);
        }
        qspInitVarData(var);
    }

    INLINE QSPVar qspGetUnknownVar(void)
    {
        QSPVar var;
        qspInitVarData(&var);
        return var;
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
