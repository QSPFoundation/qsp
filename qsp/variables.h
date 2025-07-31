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

    #define QSP_VARSMAXSCOPES 100
    #define QSP_VARSGLOBALBUCKETS 512
    #define QSP_VARSLOCALBUCKETS 64
    #define QSP_VARSMAXBUCKETSIZE 32
    #define QSP_VARGROUPSBATCHSIZE 256
    #define QSP_VARARGS QSP_FMT("ARGS")
    #define QSP_VARRES QSP_FMT("RESULT")

    typedef struct
    {
        int Index;
        QSPString Str;
    } QSPVarIndex;

    typedef struct
    {
        QSPString Name;
        QSPVariant *Values;
        int ValsCount;
        int ValsCapacity;
        QSPVarIndex *Indices;
        int IndsCount;
        int IndsCapacity;
    } QSPVar;

    typedef struct
    {
        QSPVar *Vars;
        int VarsCount;
        int Capacity;
    } QSPVarsBucket;

    typedef struct QSPVarsScope_s QSPVarsScope;

    typedef struct QSPVarsScope_s
    {
        QSPVarsScope *ParentScope;
        QSPVarsBucket *Buckets;
        int BucketsCount;
    } QSPVarsScope;

    extern QSPVar qspNullVar;
    extern QSPVarsScope *qspGlobalVars; /* there's only one global scope, we don't recreate it */
    extern QSPVarsScope *qspCurrentLocalVars; /* local scopes can be recreated */

    extern QSP_TINYINT qspSpecToBaseTypeTable[128];

    /* External functions */
    void qspInitVarTypes(void);
    void qspClearVars(QSPVarsScope *scope);
    void qspClearLocalVarsScopes(QSPVarsScope *scope);
    void qspInitGlobalVarsScope();
    void qspClearGlobalVarsScope();
    void qspClearAllVars(QSP_BOOL toInit);
    QSPVarsScope *qspSaveLocalVarsAndRestoreGlobals(void);
    void qspClearSavedLocalVars(QSPVarsScope *scope);
    void qspRestoreSavedLocalVars(QSPVarsScope *scope);
    QSPVar *qspVarReference(QSPString name, QSP_BOOL toCreate);
    int qspGetVarIndex(QSPVar *var, QSPVariant index, QSP_BOOL toCreate);
    QSP_BOOL qspGetVarValueByIndex(QSPString varName, QSPVariant index, QSPVariant *res);
    QSP_BOOL qspGetFirstVarValue(QSPString varName, QSPVariant *res);
    QSP_BOOL qspGetLastVarValue(QSPString varName, QSPVariant *res);
    QSPString qspGetVarStrValue(QSPString name);
    QSP_BIGINT qspGetVarNumValue(QSPString name);
    int qspArraySize(QSPString varName);
    int qspArrayPos(QSPString varName, QSPVariant *val, int ind, QSP_BOOL isRegExp);
    QSPVariant qspArrayMinMaxItem(QSPString varName, QSP_BOOL isMin);
    QSP_BOOL qspSetArgs(QSPVariant *args, int count, QSP_BOOL toMove);
    QSP_BOOL qspApplyResult(QSPVariant *res);
    /* Statements */
    void qspStatementSetVarsValues(QSPString s, QSPCachedStat *stat);
    void qspStatementLocal(QSPString s, QSPCachedStat *stat);
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

    INLINE void qspInitVarData(QSPVar *var)
    {
        /* We deliberately don't touch var's name here */
        var->Values = 0;
        var->ValsCount = 0;
        var->ValsCapacity = 0;
        var->Indices = 0;
        var->IndsCount = 0;
        var->IndsCapacity = 0;
    }

    INLINE void qspMoveVar(QSPVar *dest, QSPVar *src)
    {
        /* We deliberately don't touch var's name here */
        dest->Values = src->Values;
        dest->ValsCount = src->ValsCount;
        dest->ValsCapacity = src->ValsCapacity;
        dest->Indices = src->Indices;
        dest->IndsCount = src->IndsCount;
        dest->IndsCapacity = src->IndsCapacity;
        qspInitVarData(src);
    }

    INLINE void qspEmptyVar(QSPVar *var)
    {
        /* We deliberately don't touch var's name here */
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
        }
        qspInitVarData(var);
    }

    INLINE QSPVar qspGetUnknownVar(void)
    {
        QSPVar var;
        var.Name = qspNullString;
        qspInitVarData(&var);
        return var;
    }

    INLINE void qspInitVarsScope(QSPVarsScope *scope)
    {
        int i;
        QSPVarsBucket *bucket;
        /* The parent scope had to be set already */
        scope->BucketsCount = (scope->ParentScope ? QSP_VARSLOCALBUCKETS : QSP_VARSGLOBALBUCKETS);
        bucket = scope->Buckets = (QSPVarsBucket *)malloc(scope->BucketsCount * sizeof(QSPVarsBucket));
        for (i = scope->BucketsCount; i > 0; --i, ++bucket)
        {
            bucket->Vars = 0;
            bucket->Capacity = bucket->VarsCount = 0;
        }
    }

    INLINE void qspClearVarsScope(QSPVarsScope *scope)
    {
        if (scope->Buckets)
        {
            qspClearVars(scope);
            free(scope->Buckets);
        }
        free(scope);
    }

    INLINE QSPVarsScope *qspAllocateLocalScope(void)
    {
        /* We allocate the scope, but don't initialize it */
        QSPVarsScope *scope = (QSPVarsScope *)malloc(sizeof(QSPVarsScope));
        scope->ParentScope = qspCurrentLocalVars;
        scope->Buckets = 0;
        scope->BucketsCount = 0;

        qspCurrentLocalVars = scope;
        return scope;
    }

    INLINE void qspRemoveLocalScope(void)
    {
        QSPVarsScope *scope = qspCurrentLocalVars;
        if (scope && scope->ParentScope)
        {
            qspCurrentLocalVars = scope->ParentScope;

            qspClearVarsScope(scope);
        }
    }

#endif
