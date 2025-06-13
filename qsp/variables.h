/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "declarations.h"
#include "codetools.h"
#include "text.h"
#include "variant.h"

#ifndef QSP_VARSDEFINES
    #define QSP_VARSDEFINES

    #define QSP_VARSBUCKETS 1024
    #define QSP_VARSBUCKETSIZE 32
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
    } QSPVarsBucket;

    typedef struct
    {
        QSP_BOOL HasSpecialVars;
        QSPVar ArgsVar;
        QSPVar ResultVar;
        QSPVar *Vars;
        int VarsCount;
        int Capacity;
    } QSPVarsGroup;

    extern QSPVar qspNullVar;
    extern QSPVarsBucket qspVars[QSP_VARSBUCKETS];
    extern QSPVarsGroup *qspSavedVarGroups;
    extern int qspSavedVarGroupsCount;
    extern int qspSavedVarGroupsBufSize;
    extern QSPVar *qspArgsVar;
    extern QSPVar *qspResultVar;

    extern QSP_TINYINT qspSpecToBaseTypeTable[128];

    /* External functions */
    void qspInitVarTypes(void);
    QSPVar *qspVarReference(QSPString name, QSP_BOOL toCreate);
    void qspClearAllVars(QSP_BOOL toInit);
    QSP_BOOL qspInitSpecialVars(void);
    int qspGetVarIndex(QSPVar *var, QSPVariant index, QSP_BOOL toCreate);
    QSP_BOOL qspGetVarValueByIndex(QSPString varName, QSPVariant index, QSPVariant *res);
    QSP_BOOL qspGetFirstVarValue(QSPString varName, QSPVariant *res);
    QSP_BOOL qspGetLastVarValue(QSPString varName, QSPVariant *res);
    QSPString qspGetVarStrValue(QSPString name);
    QSP_BIGINT qspGetVarNumValue(QSPString name);
    void qspRestoreGlobalVars(void);
    int qspSaveLocalVarsAndRestoreGlobals(QSPVarsGroup **savedVarGroups);
    void qspClearSavedLocalVars(QSPVarsGroup *varGroups, int groupsCount);
    void qspRestoreSavedLocalVars(QSPVarsGroup *varGroups, int groupsCount);
    void qspRestoreVars(QSPVar *vars, int count);
    void qspClearVars(QSPVar *vars, int count);
    void qspRestoreSpecialVars(QSPVarsGroup *varGroup);
    void qspClearSpecialVars(QSPVarsGroup *varGroup);
    int qspArraySize(QSPString varName);
    int qspArrayPos(QSPString varName, QSPVariant *val, int ind, QSP_BOOL isRegExp);
    QSPVariant qspArrayMinMaxItem(QSPString varName, QSP_BOOL isMin);
    void qspSetArgs(QSPVariant *args, int count, QSP_BOOL toMove);
    void qspApplyResult(QSPVariant *res);
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

    INLINE QSPVarsGroup *qspAllocateSavedVarsGroup()
    {
        QSPVarsGroup *varsGroup;
        int groupInd = qspSavedVarGroupsCount++;
        if (groupInd >= qspSavedVarGroupsBufSize)
        {
            qspSavedVarGroupsBufSize += QSP_VARGROUPSBATCHSIZE;
            qspSavedVarGroups = (QSPVarsGroup *)realloc(qspSavedVarGroups, qspSavedVarGroupsBufSize * sizeof(QSPVarsGroup));
        }
        varsGroup = qspSavedVarGroups + groupInd;
        varsGroup->Vars = 0;
        varsGroup->Capacity = varsGroup->VarsCount = 0;
        varsGroup->HasSpecialVars = QSP_FALSE;
        return varsGroup;
    }

    INLINE QSPVarsGroup *qspAllocateSavedVarsGroupWithArgs()
    {
        QSPVarsGroup *varsGroup = qspAllocateSavedVarsGroup();
        qspMoveVar(&varsGroup->ArgsVar, qspArgsVar);
        qspMoveVar(&varsGroup->ResultVar, qspResultVar);
        varsGroup->HasSpecialVars = QSP_TRUE;
        return varsGroup;
    }

    INLINE void qspClearLastSavedVarsGroup()
    {
        if (qspSavedVarGroupsCount)
        {
            QSPVarsGroup *varsGroup = &qspSavedVarGroups[--qspSavedVarGroupsCount];
            qspClearSpecialVars(varsGroup);
            qspClearVars(varsGroup->Vars, varsGroup->VarsCount);
        }
    }

    INLINE void qspRestoreLastSavedVarsGroup()
    {
        if (qspSavedVarGroupsCount)
        {
            QSPVarsGroup *varsGroup = &qspSavedVarGroups[--qspSavedVarGroupsCount];
            qspRestoreVars(varsGroup->Vars, varsGroup->VarsCount);
            qspRestoreSpecialVars(varsGroup); /* special vars can override regular vars */
        }
    }

#endif
