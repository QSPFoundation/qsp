/* Copyright (C) 2001-2024 Val Argunov (byte AT qsp DOT org) */
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
    #define QSP_VARSMAXBUCKETSIZE 50
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
        int ValsBufSize;
        QSPVarIndex *Indices;
        int IndsCount;
        int IndsBufSize;
    } QSPVar;

    typedef struct
    {
        QSPVar *Vars;
        int VarsCount;
    } QSPVarsGroup;

    typedef struct {
        QSPVar *Vars;
        int VarsCount;
        int Capacity;
    } QSPVarsBucket;

    extern QSPVar qspNullVar;
    extern QSPVarsBucket qspVars[QSP_VARSBUCKETS];
    extern QSPVarsGroup *qspSavedVarGroups;
    extern int qspSavedVarGroupsCount;
    extern int qspSavedVarGroupsBufSize;

    extern QSP_TINYINT qspSpecToBaseTypeTable[128];

    /* External functions */
    void qspInitVarTypes(void);
    QSPVar *qspVarReference(QSPString name, QSP_BOOL toCreate);
    void qspClearAllVars(QSP_BOOL toInit);
    int qspGetVarIndex(QSPVar *var, QSPVariant index, QSP_BOOL toCreate);
    QSP_BOOL qspGetFirstVarValue(QSPString varName, QSPVariant *res);
    QSP_BOOL qspGetVarValueByIndex(QSPString varName, QSPVariant index, QSPVariant *res);
    QSP_BOOL qspGetLastVarValue(QSPString varName, QSPVariant *res);
    QSPString qspGetVarStrValue(QSPString name);
    QSP_BIGINT qspGetVarNumValue(QSPString name);
    void qspRestoreGlobalVars(void);
    int qspSaveLocalVarsAndRestoreGlobals(QSPVarsGroup **savedVarGroups);
    void qspRestoreSavedLocalVars(QSPVarsGroup *varGroups, int groupsCount);
    void qspRestoreVars(QSPVar *vars, int count);
    void qspClearVars(QSPVar *vars, int count);
    int qspArraySize(QSPString varName);
    int qspArrayPos(QSPString varName, QSPVariant *val, int ind, QSP_BOOL isRegExp);
    QSPVariant qspArrayMinMaxItem(QSPString varName, QSP_BOOL isMin);
    void qspSetArgs(QSPVar *destVar, QSPVariant *args, int count, QSP_BOOL toMove);
    void qspApplyResult(QSPVar *varRes, QSPVariant *res);
    /* Statements */
    void qspStatementSetVarValue(QSPString s, QSPCachedStat *stat);
    void qspStatementLocal(QSPString s, QSPCachedStat *stat);
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
        var->Values = 0;
        var->ValsCount = 0;
        var->ValsBufSize = 0;
        var->Indices = 0;
        var->IndsCount = 0;
        var->IndsBufSize = 0;
    }

    INLINE void qspMoveVar(QSPVar *dest, QSPVar *src)
    {
        dest->Values = src->Values;
        dest->ValsCount = src->ValsCount;
        dest->ValsBufSize = src->ValsBufSize;
        dest->Indices = src->Indices;
        dest->IndsCount = src->IndsCount;
        dest->IndsBufSize = src->IndsBufSize;
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

    INLINE int qspAllocateSavedVarsGroup()
    {
        int ind = qspSavedVarGroupsCount++;
        if (ind >= qspSavedVarGroupsBufSize)
        {
            qspSavedVarGroupsBufSize += QSP_VARGROUPSBATCHSIZE;
            qspSavedVarGroups = (QSPVarsGroup *)realloc(qspSavedVarGroups, qspSavedVarGroupsBufSize * sizeof(QSPVarsGroup));
        }
        qspSavedVarGroups[ind].Vars = 0;
        qspSavedVarGroups[ind].VarsCount = 0;
        return ind;
    }

    INLINE int qspAllocateSavedVarsGroupWithArgs(QSPVar *varArgs, QSPVar *varRes)
    {
        QSPVar *varsList;
        int groupInd = qspAllocateSavedVarsGroup();
        varsList = (QSPVar *)malloc(2 * sizeof(QSPVar)); /* ARGS & RESULT */
        varsList[0].Name = qspCopyToNewText(varArgs->Name);
        qspMoveVar(&varsList[0], varArgs);
        varsList[1].Name = qspCopyToNewText(varRes->Name);
        qspMoveVar(&varsList[1], varRes);
        qspSavedVarGroups[groupInd].Vars = varsList;
        qspSavedVarGroups[groupInd].VarsCount = 2;
        return groupInd;
    }

    INLINE void qspReleaseSavedVarsGroup(QSP_BOOL toKeepLocals)
    {
        if (qspSavedVarGroupsCount)
        {
            int ind = --qspSavedVarGroupsCount;
            if (toKeepLocals)
                qspClearVars(qspSavedVarGroups[ind].Vars, qspSavedVarGroups[ind].VarsCount);
            else
                qspRestoreVars(qspSavedVarGroups[ind].Vars, qspSavedVarGroups[ind].VarsCount);
        }
    }

#endif
