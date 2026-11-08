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
    #define QSP_VARSGLOBALCAPACITY 512 /* has to be a power of 2 */
    #define QSP_VARSLOCALCAPACITY 64
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
    } QSPVarData;

    typedef struct
    {
        QSPVarData Data;
        int RefsCount;
    } QSPSharedVarData;

    typedef struct
    {
        int NameIndex; /* 0 for an empty entry */
        int LocalIndex; /* previous or the last local variable with the same name (index in qspLocalVars, 1-based), 0 for none */
        int GlobalIndex; /* global var of the name for local variables (index in qspGlobalVars, 1-based) */
        QSP_BOOL IsRef;
        union
        {
            QSPVarData Own;
            QSPSharedVarData *Shared;
        } Data;
    } QSPVar;

    typedef struct
    {
        QSPVarData *VarData;
        QSPVariant Index;
        QSPField *Fields;
    } QSPResolvedTarget;

    enum
    {
        QSP_REF_GET,
        QSP_REF_GETORCREATE,
        QSP_REF_GETSILENTLY /* no errors reported */
    };

    extern QSPVar qspNullVar;
    extern QSPVar *qspGlobalVars; /* hash table by name indices, QSP_SLOTSCOUNT(qspGlobalVarsCapacity) entries */
    extern int qspGlobalVarsCount;
    extern int qspGlobalVarsCapacity; /* has to be a power of 2 */

    extern QSP_TINYINT qspSpecToBaseTypeTable[128];

    /* External functions */
    void qspInitVarTypes(void);
    void qspInitVars(void);
    void qspTerminateVars(void);
    void qspClearGlobalVars(void);
    void qspClearLocalVars(void);
    void qspClearAllVars(QSP_BOOL toInit);
    void qspRegisterSpecialVariables(void);
    int qspAllocateLocalScope(void);
    void qspReleaseLocalScope(int oldLocalScopeBase);
    QSPVarData *qspAddGlobalVarByName(QSPString name);
    int qspAllocateLocalScopeWithArgs(QSPVariant *args, int count, QSP_BOOL toMove);
    QSP_BOOL qspSetArgs(QSPVariant *args, int count, QSP_BOOL toMove);
    QSP_BOOL qspApplyResult(QSPVariant *res);
    int qspHideLocalVars(void);
    void qspRestoreHiddenLocalVars(int oldLocalVarsBase);
    void qspClearHiddenLocalVars(int oldLocalVarsBase);
    QSPVar *qspVarReferenceByName(QSPString name, QSP_TINYINT mode);
    QSPVarKey *qspAddVarKey(QSPVarData *varData, unsigned int hash, int valIndex);
    int qspGetVarKeyIndex(QSPVarData *varData, int valIndex);
    int qspGetVarItemIndex(QSPVarData *varData, QSPVariant index, QSP_BOOL toCreate);
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
    QSPVarData *qspVarDataByName(QSPString name);
    QSPString qspGetVarStrValue(QSPString name, int valIndex);
    QSP_BIGINT qspGetVarNumValue(QSPString name, int valIndex);
    QSPTuple qspCopyArrayToNewTuple(QSPVarData *src, int start, int count);
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

    INLINE void qspInitVarData(QSPVarData *varData)
    {
        varData->Values = 0;
        varData->ValsCount = 0;
        varData->ValsCapacity = 0;
        varData->Keys = 0;
        varData->KeysSlots = 0;
        varData->KeysCount = 0;
        varData->KeysCapacity = 0;
    }

    INLINE void qspMoveVarData(QSPVarData *dest, QSPVarData *src)
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

    INLINE void qspEmptyVarData(QSPVarData *varData)
    {
        if (varData->Values)
        {
            qspFreeVariants(varData->Values, varData->ValsCount);
            free(varData->Values);
        }
        if (varData->Keys)
        {
            QSPVarKey *curKey;
            int count = varData->KeysCount;
            for (curKey = varData->Keys; count > 0; --count, ++curKey)
                qspFreeVariant(&curKey->Key);
            free(varData->Keys);
            free(varData->KeysSlots);
        }
        qspInitVarData(varData);
    }

    INLINE QSPVarData *qspGetVarData(QSPVar *var)
    {
        return (var->IsRef ? &var->Data.Shared->Data : &var->Data.Own);
    }

    INLINE QSPVarData *qspVarDataReferenceByName(QSPString name, QSP_TINYINT mode)
    {
        QSPVar *var = qspVarReferenceByName(name, mode);
        return (var ? qspGetVarData(var) : 0);
    }

    INLINE QSPVar qspGetUnknownVar(void)
    {
        QSPVar var;
        var.NameIndex = 0;
        var.LocalIndex = 0;
        var.GlobalIndex = 0;
        var.IsRef = QSP_FALSE;
        qspInitVarData(&var.Data.Own);
        return var;
    }

#endif
