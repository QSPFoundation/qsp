/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
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
#include "variant.h"

#ifndef QSP_VARSDEFINES
    #define QSP_VARSDEFINES

    #define QSP_VARGROUPSBATCHSIZE 256
    #define QSP_VARSSEEK 64
    #define QSP_VARSCOUNT 256 * QSP_VARSSEEK
    #define QSP_VARARGS QSP_FMT("ARGS")
    #define QSP_VARRES QSP_FMT("RESULT")

    #define QSP_VARBASETYPE(name) (*(name).Str == QSP_STRCHAR[0]) /* QSP_TYPE_STRING | QSP_TYPE_NUMBER */

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
        QSPVarIndex *Indices;
        int IndsCount;
        int IndsBufSize;
    } QSPVar;

    typedef struct
    {
        QSPVar *Vars;
        int VarsCount;
    } QSPVarsGroup;

    extern QSPVar qspVars[QSP_VARSCOUNT];
    extern QSPVarsGroup *qspSavedVarGroups;
    extern int qspSavedVarGroupsCount;
    extern int qspSavedVarGroupsBufSize;

    /* External functions */
    QSPVar *qspVarReference(QSPString name, QSP_BOOL isCreate);
    void qspClearVars(QSP_BOOL isFirst);
    QSPVar *qspVarReferenceWithType(QSPString name, QSP_BOOL isCreate, int *baseType);
    void qspSetVarValueByReference(QSPVar *, int, QSPVariant *);
    void qspGetVarValueByReference(QSPVar *var, int ind, int baseType, QSPVariant *res);
    QSPString qspGetVarStrValue(QSPString name);
    int qspGetVarNumValue(QSPString name);
    int qspGetVarTextIndex(QSPVar *, QSPString, QSP_BOOL);
    void qspRestoreGlobalVars();
    int qspSaveLocalVarsAndRestoreGlobals(QSPVar **);
    void qspRestoreLocalVars(QSPVar *, int, QSPVarsGroup *, int);
    void qspClearLocalVars(QSPVar *, int);
    void qspRestoreVarsList(QSPVar *, int);
    void qspClearVarsList(QSPVar *, int);
    int qspArraySize(QSPString name);
    int qspArrayPos(QSPString varName, QSPVariant *val, int ind, QSP_BOOL isRegExp);
    QSPVariant qspArrayMinMaxItem(QSPString name, QSP_BOOL isMin);
    int qspGetVarsCount();
    void qspSetArgs(QSPVar *var, QSPVariant *args, int count);
    void qspApplyResult(QSPVar *varRes, QSPVariant *res);
    /* Statements */
    void qspStatementSetVarValue(QSPString s, QSPCachedStat *stat);
    void qspStatementLocal(QSPString s, QSPCachedStat *stat);
    QSP_BOOL qspStatementCopyArr(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
    QSP_BOOL qspStatementKillVar(QSPVariant *args, int count, QSPString *jumpTo, int extArg);

    INLINE void qspInitVarData(QSPVar *var)
    {
        var->Values = 0;
        var->ValsCount = 0;
        var->Indices = 0;
        var->IndsCount = 0;
        var->IndsBufSize = 0;
    }

    INLINE void qspMoveVar(QSPVar *dest, QSPVar *src)
    {
        dest->Values = src->Values;
        dest->ValsCount = src->ValsCount;
        dest->Indices = src->Indices;
        dest->IndsCount = src->IndsCount;
        dest->IndsBufSize = src->IndsBufSize;
        qspInitVarData(src);
    }

    INLINE void qspEmptyVar(QSPVar *var)
    {
        int count;
        if (var->Values)
        {
            count = var->ValsCount;
            while (--count >= 0)
                if (QSP_ISSTR(var->Values[count].Type)) qspFreeString(QSP_STR(var->Values[count]));
            free(var->Values);
        }
        if (var->Indices)
        {
            count = var->IndsCount;
            while (--count >= 0)
                qspFreeString(var->Indices[count].Str);
            free(var->Indices);
        }
        qspInitVarData(var);
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
        varsList = (QSPVar *)malloc(2 * sizeof(QSPVar));
        varsList[0].Name = qspGetNewText(varArgs->Name);
        qspMoveVar(&varsList[0], varArgs);
        varsList[1].Name = qspGetNewText(varRes->Name);
        qspMoveVar(&varsList[1], varRes);
        qspSavedVarGroups[groupInd].Vars = varsList;
        qspSavedVarGroups[groupInd].VarsCount = 2;
        return groupInd;
    }

    INLINE void qspReleaseSavedVarsGroup(QSP_BOOL isKeepLocals)
    {
        int ind;
        if (qspSavedVarGroupsCount)
        {
            ind = --qspSavedVarGroupsCount;
            if (isKeepLocals)
                qspClearVarsList(qspSavedVarGroups[ind].Vars, qspSavedVarGroups[ind].VarsCount);
            else
                qspRestoreVarsList(qspSavedVarGroups[ind].Vars, qspSavedVarGroups[ind].VarsCount);
        }
    }

#endif
