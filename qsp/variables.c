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

#include "variables.h"
#include "coding.h"
#include "common.h"
#include "errors.h"
#include "locations.h"
#include "mathops.h"
#include "regexp.h"

QSPVar qspNullVar;
QSPVarsGroup qspVars[QSP_VARSBUCKETS];
QSPVarsGroup *qspSavedVarGroups = 0;
int qspSavedVarGroupsCount = 0;
int qspSavedVarGroupsBufSize = 0;

QSP_TINYINT qspSpecToBaseTypeTable[128];

INLINE int qspIndStringCompare(const void *name, const void *compareTo);
INLINE int qspIndStringFloorCompare(const void *name, const void *compareTo);
INLINE int qspValuePositionsAscCompare(const void *arg1, const void *arg2);
INLINE int qspValuePositionsDescCompare(const void *arg1, const void *arg2);
INLINE void qspRemoveArrayItem(QSPVar *var, int index);
INLINE QSPVar *qspGetVarData(QSPString s, int *index, QSP_BOOL isSetOperation);
INLINE QSP_BOOL qspGetVarValueByReference(QSPVar *var, int ind, QSP_TINYINT baseType, QSPVariant *res);
INLINE void qspResetVar(QSPString varName);
INLINE void qspSetVarValueByReference(QSPVar *var, int ind, QSP_TINYINT baseType, QSPVariant *val);
INLINE void qspSetVarValueByIndex(QSPString varName, QSPVariant index, QSPVariant *val);
INLINE void qspSetFirstVarValue(QSPString varName, QSPVariant *val);
INLINE void qspSetVarValue(QSPString varName, QSPVariant *val, QSP_CHAR op);
INLINE void qspClearSavedVars();
INLINE void qspUnpackTupleToArray(QSPVar *dest, QSPTuple src, int start, int count);
INLINE void qspCopyArray(QSPVar *dest, QSPVar *src, int start, int count);
INLINE void qspSortArray(QSPVar *var, QSP_TINYINT baseValType, QSP_BOOL isAscending);
INLINE int qspGetVarsNames(QSPString names, QSPString **varNames);
INLINE void qspSetVarsValues(QSPString *varNames, int varsCount, QSPVariant *v, QSP_CHAR op);
INLINE QSPString qspGetVarNameOnly(QSPString s);
INLINE QSP_BOOL qspSaveVarToLocalGroup(QSPVarsGroup *varGroup, QSPString varName);

void qspInitVarTypes(void)
{
    int i;
    for (i = 0; i < sizeof(qspSpecToBaseTypeTable); ++i)
        qspSpecToBaseTypeTable[i] = QSP_TYPE_NUM;

    qspSpecToBaseTypeTable[QSP_NUMCHAR[0]] = QSP_TYPE_NUM;
    qspSpecToBaseTypeTable[QSP_STRCHAR[0]] = QSP_TYPE_STR;
    qspSpecToBaseTypeTable[QSP_TUPLECHAR[0]] = QSP_TYPE_TUPLE;
}

INLINE int qspIndStringCompare(const void *name, const void *compareTo)
{
    return qspStrsComp(*(QSPString *)name, ((QSPVarIndex *)compareTo)->Str);
}

INLINE int qspIndStringFloorCompare(const void *name, const void *compareTo)
{
    QSPString key = *(QSPString *)name;
    QSPVarIndex *item = (QSPVarIndex *)compareTo;

    /* It's safe to check (item + 1) because the item never points to the last array item */
    if (qspStrsComp(key, (item + 1)->Str) < 0 && qspStrsComp(key, item->Str) >= 0)
        return 0;

    return qspStrsComp(key, item->Str);
}

INLINE int qspValuePositionsAscCompare(const void *arg1, const void *arg2)
{
    return qspAutoConvertCompare(*(QSPVariant **)arg1, *(QSPVariant **)arg2); /* base types of values should be the same */
}

INLINE int qspValuePositionsDescCompare(const void *arg1, const void *arg2)
{
    return qspAutoConvertCompare(*(QSPVariant **)arg2, *(QSPVariant **)arg1); /* base types of values should be the same */
}

QSPVar *qspVarReference(QSPString name, QSP_BOOL toCreate)
{
    int i, varsCount;
    QSPVarsGroup *bucket;
    QSPVar *var;
    QSP_CHAR *pos;
    unsigned int bCode;
    if (qspIsEmpty(name))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return 0;
    }

    /* Ignore type prefix */
    if (qspIsInClass(*name.Str, QSP_CHAR_TYPEPREFIX))
        name.Str++;

    if (qspIsEmpty(name) || qspIsInClass(*name.Str, QSP_CHAR_DIGIT) || qspIsAnyInClass(name, QSP_CHAR_DELIM))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return 0;
    }
    bCode = 7;
    for (pos = name.Str; pos < name.End; ++pos)
        bCode = bCode * 31 + (unsigned char)*pos;
    bucket = qspVars + bCode % QSP_VARSBUCKETS;
    var = bucket->Vars;
    varsCount = bucket->VarsCount;
    for (i = varsCount; i > 0; --i)
    {
        if (!qspStrsComp(var->Name, name)) return var;
        ++var;
    }
    if (toCreate)
    {
        if (varsCount >= QSP_VARSMAXBUCKETSIZE)
        {
            qspSetError(QSP_ERR_TOOMANYVARS);
            return 0;
        }
        if (varsCount >= bucket->Capacity)
        {
            bucket->Capacity = varsCount + 16;
            bucket->Vars = (QSPVar *)realloc(bucket->Vars, bucket->Capacity * sizeof(QSPVar));
        }

        var = bucket->Vars + varsCount;

        var->Name = qspCopyToNewText(name);
        qspInitVarData(var);

        bucket->VarsCount++;
        return var;
    }
    return &qspNullVar;
}

void qspClearAllVars(QSP_BOOL toInit)
{
    int i, j;
    QSPVar *var;
    QSPVarsGroup *bucket = qspVars;
    for (i = 0; i < QSP_VARSBUCKETS; ++i)
    {
        if (!toInit && bucket->Vars)
        {
            var = bucket->Vars;
            for (j = bucket->VarsCount; j > 0; --j)
            {
                qspFreeString(&var->Name);
                qspEmptyVar(var);
                ++var;
            }
            free(bucket->Vars);
        }
        bucket->Capacity = bucket->VarsCount = 0;
        bucket->Vars = 0;
        ++bucket;
    }
}

INLINE void qspRemoveArrayItem(QSPVar *var, int index)
{
    int i;
    QSP_BOOL toRemove;
    QSPVarIndex *ind;
    if (index < 0 || index >= var->ValsCount) return;
    qspFreeVariant(var->Values + index);
    var->ValsCount--;
    i = index;
    while (i < var->ValsCount)
    {
        var->Values[i] = var->Values[i + 1];
        ++i;
    }
    toRemove = QSP_FALSE;
    ind = var->Indices;
    for (i = 0; i < var->IndsCount; ++i)
    {
        if (ind->Index == index)
        {
            qspFreeString(&ind->Str);
            var->IndsCount--;
            if (i == var->IndsCount) break;
            toRemove = QSP_TRUE;
        }
        if (toRemove) *ind = *(ind + 1);
        if (ind->Index > index) ind->Index--;
        ++ind;
    }
}

int qspGetVarIndex(QSPVar *var, QSPVariant index, QSP_BOOL toCreate)
{
    int indsCount;
    QSPString uStr;
    if (QSP_ISNUM(index.Type)) return QSP_TOINT(QSP_NUM(index));
    uStr = qspGetVariantAsIndexString(&index);
    qspUpperStr(&uStr);
    indsCount = var->IndsCount;
    if (indsCount > 0)
    {
        QSPVarIndex *ind = (QSPVarIndex *)bsearch(&uStr, var->Indices, indsCount, sizeof(QSPVarIndex), qspIndStringCompare);
        if (ind)
        {
            qspFreeString(&uStr);
            return ind->Index;
        }
    }
    if (toCreate)
    {
        /* Find the first item that's smaller than uStr */
        int floorItem = indsCount - 1;
        if (indsCount > 0)
        {
            QSPVarIndex *lastItem = var->Indices + floorItem;
            if (qspStrsComp(uStr, lastItem->Str) < 0)
            {
                QSPVarIndex *ind = (QSPVarIndex *)bsearch(&uStr, var->Indices, floorItem, sizeof(QSPVarIndex), qspIndStringFloorCompare);
                floorItem = (ind ? (int)(ind - var->Indices) : -1);
            }
        }
        /* Prepare buffer & shift existing items to allocate extra space */
        if (indsCount >= var->IndsBufSize)
        {
            var->IndsBufSize = indsCount + 8;
            var->Indices = (QSPVarIndex *)realloc(var->Indices, var->IndsBufSize * sizeof(QSPVarIndex));
        }
        ++floorItem;
        while (indsCount > floorItem)
        {
            var->Indices[indsCount] = var->Indices[indsCount - 1];
            --indsCount;
        }
        /* Add new index item */
        indsCount = var->ValsCount; /* point to the new array item */
        var->Indices[floorItem].Str = uStr;
        var->Indices[floorItem].Index = indsCount;
        var->IndsCount++;
        return indsCount;
    }
    qspFreeString(&uStr);
    return -1;
}

INLINE QSPVar *qspGetVarData(QSPString s, int *index, QSP_BOOL isSetOperation)
{
    QSP_CHAR *lPos = qspStrChar(s, QSP_LSBRACK[0]);
    if (lPos)
    {
        QSPVar *var;
        QSP_CHAR *rPos, *startPos = s.Str;
        s.Str = lPos;
        rPos = qspDelimPos(s, QSP_RSBRACK[0]);
        if (!rPos)
        {
            qspSetError(QSP_ERR_BRACKNOTFOUND);
            return 0;
        }
        var = qspVarReference(qspStringFromPair(startPos, lPos), isSetOperation);
        if (!var) return 0;
        s.Str = lPos + QSP_STATIC_LEN(QSP_LSBRACK);
        qspSkipSpaces(&s);
        if (s.Str == rPos)
        {
            if (isSetOperation)
                *index = var->ValsCount; /* new item */
            else
                *index = (var->ValsCount ? var->ValsCount - 1 : 0); /* last item */
        }
        else
        {
            QSPVariant ind;
            int oldLocationState = qspLocationState;
            ind = qspCalculateExprValue(qspStringFromPair(s.Str, rPos));
            if (qspLocationState != oldLocationState) return 0;
            *index = qspGetVarIndex(var, ind, isSetOperation);
            qspFreeVariant(&ind);
        }
        return var;
    }
    *index = 0;
    return qspVarReference(s, isSetOperation);
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
    QSPVar *var = qspVarReference(name, QSP_FALSE);
    if (var)
    {
        if (var->ValsCount && QSP_ISSTR(var->Values[0].Type))
            return QSP_STR(var->Values[0]);
    }
    else
        qspResetError(QSP_FALSE);
    return qspNullString;
}

QSP_BIGINT qspGetVarNumValue(QSPString name)
{
    QSPVar *var = qspVarReference(name, QSP_FALSE);
    if (var)
    {
        if (var->ValsCount && QSP_ISNUM(var->Values[0].Type))
            return QSP_NUM(var->Values[0]);
    }
    else
        qspResetError(QSP_FALSE);
    return 0;
}

INLINE void qspResetVar(QSPString varName)
{
    int index;
    QSPVar *var = qspGetVarData(varName, &index, QSP_TRUE);
    if (!var) return;
    if (index >= 0 && index < var->ValsCount)
    {
        QSPVariant *curValue = var->Values + index;
        if (QSP_ISDEF(curValue->Type))
        {
            qspFreeVariant(curValue);
            qspInitVariant(curValue, QSP_TYPE_UNDEF);
        }
    }
}

INLINE void qspSetVarValueByReference(QSPVar *var, int ind, QSP_TINYINT baseType, QSPVariant *val)
{
    int oldCount = var->ValsCount;
    if (ind >= oldCount)
    {
        QSPVariant *curValue;
        if (baseType != QSP_BASETYPE(val->Type))
        {
            if (!qspConvertVariantTo(val, baseType))
            {
                qspSetError(QSP_ERR_TYPEMISMATCH);
                return;
            }
        }
        if (ind >= var->ValsBufSize)
        {
            if (ind > 0)
                var->ValsBufSize = ind + 4;
            else
                var->ValsBufSize = 1; /* allocate only 1 item for the first value */
            var->Values = (QSPVariant *)realloc(var->Values, var->ValsBufSize * sizeof(QSPVariant));
        }
        var->ValsCount = ind + 1;
        /* Init new values */
        for (curValue = var->Values + oldCount; oldCount < ind; ++curValue, ++oldCount)
            qspInitVariant(curValue, QSP_TYPE_UNDEF);
        qspMoveToNewVariant(var->Values + ind, val);
    }
    else if (ind >= 0)
    {
        if (baseType != QSP_BASETYPE(val->Type))
        {
            if (!qspConvertVariantTo(val, baseType))
            {
                qspSetError(QSP_ERR_TYPEMISMATCH);
                return;
            }
        }
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
    arrIndex = qspGetVarIndex(var, index, QSP_TRUE);
    qspSetVarValueByReference(var, arrIndex, varType, val);
}

INLINE void qspSetFirstVarValue(QSPString varName, QSPVariant *val)
{
    QSP_TINYINT varType;
    QSPVar *var = qspVarReference(varName, QSP_TRUE);
    if (!var) return;
    varType = qspGetVarType(varName);
    qspSetVarValueByReference(var, 0, varType, val);
}

INLINE void qspSetVarValue(QSPString varName, QSPVariant *val, QSP_CHAR op)
{
    int index;
    QSP_TINYINT varType;
    QSPVar *var = qspGetVarData(varName, &index, QSP_TRUE);
    if (!var) return;
    varType = qspGetVarType(varName);
    if (op == QSP_EQUAL[0])
        qspSetVarValueByReference(var, index, varType, val);
    else if (qspIsInClass(op, QSP_CHAR_SIMPLEOP))
    {
        QSPVariant oldVal, res;
        qspGetVarValueByReference(var, index, varType, &oldVal);
        if (!qspAutoConvertCombine(&oldVal, val, op, &res))
        {
            qspFreeVariant(&oldVal);
            return;
        }
        qspSetVarValueByReference(var, index, varType, &res);
        qspFreeVariant(&res);
        qspFreeVariant(&oldVal);
    }
    else
    {
        qspSetError(QSP_ERR_UNKNOWNACTION);
        return;
    }
}

INLINE void qspClearSavedVars()
{
    if (qspSavedVarGroupsCount)
    {
        int i;
        for (i = 0; i < qspSavedVarGroupsCount; ++i)
            qspClearVars(qspSavedVarGroups[i].Vars, qspSavedVarGroups[i].VarsCount);
        qspSavedVarGroupsCount = 0;
    }
}

void qspRestoreGlobalVars(void)
{
    if (qspSavedVarGroupsCount)
    {
        int i;
        /* Iterate backwards to properly restore hierarchy of local variables */
        for (i = qspSavedVarGroupsCount - 1; i >= 0; --i)
            qspRestoreVars(qspSavedVarGroups[i].Vars, qspSavedVarGroups[i].VarsCount);
        qspSavedVarGroupsCount = 0;
    }
}

int qspSaveLocalVarsAndRestoreGlobals(QSPVarsGroup **savedVarGroups)
{
    QSPVar *var;
    QSPVarsGroup *curSavedVarGroup, *curVarGroup, *varGroups;
    int i, j, groupsCount = qspSavedVarGroupsCount;
    if (!groupsCount)
    {
        *savedVarGroups = 0;
        return 0;
    }
    varGroups = (QSPVarsGroup *)malloc(groupsCount * sizeof(QSPVarsGroup));
    /* Iterate backwards to properly restore hierarchy of local variables */
    curVarGroup = varGroups + groupsCount - 1;
    curSavedVarGroup = qspSavedVarGroups + groupsCount - 1;
    for (i = groupsCount - 1; i >= 0; --i, --curVarGroup, --curSavedVarGroup)
    {
        --qspSavedVarGroupsCount; /* we always remove this group */
        if (curSavedVarGroup->Vars)
        {
            curVarGroup->Vars = (QSPVar *)malloc(curSavedVarGroup->VarsCount * sizeof(QSPVar));
            curVarGroup->Capacity = curVarGroup->VarsCount = curSavedVarGroup->VarsCount;
            for (j = 0; j < curSavedVarGroup->VarsCount; ++j)
            {
                if (!(var = qspVarReference(curSavedVarGroup->Vars[j].Name, QSP_TRUE)))
                {
                    qspClearVars(curSavedVarGroup->Vars, curSavedVarGroup->VarsCount);
                    qspClearVars(curVarGroup->Vars, j);
                    while (++i < groupsCount)
                        qspClearVars(varGroups[i].Vars, varGroups[i].VarsCount);
                    free(varGroups);
                    *savedVarGroups = 0;
                    return 0;
                }
                curVarGroup->Vars[j].Name = qspMoveText(&curSavedVarGroup->Vars[j].Name);
                qspMoveVar(curVarGroup->Vars + j, var);
                qspMoveVar(var, curSavedVarGroup->Vars + j);
            }
            free(curSavedVarGroup->Vars);
        }
        else
        {
            curVarGroup->Vars = 0;
            curVarGroup->Capacity = curVarGroup->VarsCount = 0;
        }
    }
    *savedVarGroups = varGroups;
    return groupsCount;
}

void qspClearSavedLocalVars(QSPVarsGroup *varGroups, int groupsCount)
{
    /* Clear saved vars */
    if (varGroups)
    {
        int i;
        for (i = 0; i < groupsCount; ++i)
            qspClearVars(varGroups[i].Vars, varGroups[i].VarsCount);
        free(varGroups);
    }
}

void qspRestoreSavedLocalVars(QSPVarsGroup *varGroups, int groupsCount)
{
    /* Clear current saved vars if they exist */
    qspClearSavedVars();
    /* Restore saved vars */
    if (varGroups)
    {
        int i, j;
        QSPVar *var;
        QSPVarsGroup *curSavedVarGroup, *curVarGroup = varGroups;
        qspSavedVarGroupsBufSize = groupsCount;
        qspSavedVarGroups = (QSPVarsGroup *)realloc(qspSavedVarGroups, qspSavedVarGroupsBufSize * sizeof(QSPVarsGroup));
        curSavedVarGroup = qspSavedVarGroups;
        for (i = 0; i < groupsCount; ++i, ++curVarGroup, ++curSavedVarGroup)
        {
            if (curVarGroup->Vars)
            {
                curSavedVarGroup->Vars = (QSPVar *)malloc(curVarGroup->VarsCount * sizeof(QSPVar));
                curSavedVarGroup->Capacity = curSavedVarGroup->VarsCount = curVarGroup->VarsCount;
                for (j = 0; j < curVarGroup->VarsCount; ++j)
                {
                    if (!(var = qspVarReference(curVarGroup->Vars[j].Name, QSP_TRUE)))
                    {
                        qspClearVars(curVarGroup->Vars, curVarGroup->VarsCount);
                        qspClearVars(curSavedVarGroup->Vars, j);
                        while (++i < groupsCount)
                            qspClearVars(varGroups[i].Vars, varGroups[i].VarsCount);
                        free(varGroups);
                        return;
                    }
                    curSavedVarGroup->Vars[j].Name = qspMoveText(&curVarGroup->Vars[j].Name);
                    qspMoveVar(curSavedVarGroup->Vars + j, var);
                    qspMoveVar(var, curVarGroup->Vars + j);
                }
                free(curVarGroup->Vars);
            }
            else
            {
                curSavedVarGroup->Vars = 0;
                curSavedVarGroup->Capacity = curSavedVarGroup->VarsCount = 0;
            }
            ++qspSavedVarGroupsCount; /* add this group if everything goes well */
        }
        free(varGroups);
    }
}

void qspRestoreVars(QSPVar *vars, int count)
{
    if (vars)
    {
        int i;
        QSPVar *destVar;
        for (i = 0; i < count; ++i)
        {
            if (!(destVar = qspVarReference(vars[i].Name, QSP_TRUE)))
            {
                while (i < count)
                {
                    qspFreeString(&vars[i].Name);
                    qspEmptyVar(vars + i);
                    ++i;
                }
                free(vars);
                return;
            }
            qspFreeString(&vars[i].Name);
            qspEmptyVar(destVar);
            qspMoveVar(destVar, vars + i);
        }
        free(vars);
    }
}

void qspClearVars(QSPVar *vars, int count)
{
    if (vars)
    {
        int i;
        for (i = 0; i < count; ++i)
        {
            qspFreeString(&vars[i].Name);
            qspEmptyVar(vars + i);
        }
        free(vars);
    }
}

INLINE void qspUnpackTupleToArray(QSPVar *dest, QSPTuple src, int start, int count)
{
    int i, itemsToCopy;
    /* Clear the dest array anyway */
    qspEmptyVar(dest);
    /* Validate parameters */
    if (count <= 0) return;
    if (start < 0) start = 0;
    itemsToCopy = src.Items - start;
    if (itemsToCopy <= 0) return;
    if (count < itemsToCopy) itemsToCopy = count;
    /* Copy tuple items */
    dest->ValsBufSize = dest->ValsCount = itemsToCopy;
    dest->Values = (QSPVariant *)malloc(itemsToCopy * sizeof(QSPVariant));
    for (i = 0; i < itemsToCopy; ++i)
        qspCopyToNewVariant(dest->Values + i, src.Vals + start + i);
}

INLINE void qspCopyArray(QSPVar *dest, QSPVar *src, int start, int count)
{
    int i, itemsToCopy, newInd;
    /* Clear the dest array anyway */
    qspEmptyVar(dest);
    /* Validate parameters */
    if (count <= 0) return;
    if (start < 0) start = 0;
    itemsToCopy = src->ValsCount - start;
    if (itemsToCopy <= 0) return;
    if (count < itemsToCopy) itemsToCopy = count;
    /* Copy array values */
    dest->ValsBufSize = dest->ValsCount = itemsToCopy;
    dest->Values = (QSPVariant *)malloc(itemsToCopy * sizeof(QSPVariant));
    for (i = 0; i < itemsToCopy; ++i)
        qspCopyToNewVariant(dest->Values + i, src->Values + start + i);
    /* Copy array indices */
    dest->IndsBufSize = 0;
    dest->Indices = 0;
    count = 0;
    for (i = 0; i < src->IndsCount; ++i)
    {
        newInd = src->Indices[i].Index - start;
        if (newInd >= 0 && newInd < itemsToCopy)
        {
            if (count >= dest->IndsBufSize)
            {
                dest->IndsBufSize = count + 16;
                dest->Indices = (QSPVarIndex *)realloc(dest->Indices, dest->IndsBufSize * sizeof(QSPVarIndex));
            }
            dest->Indices[count].Index = newInd;
            dest->Indices[count].Str = qspCopyToNewText(src->Indices[i].Str);
            ++count;
        }
    }
    dest->IndsCount = count;
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
}

int qspArraySize(QSPString varName)
{
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    if (!var) return 0;
    return var->ValsCount;
}

int qspArrayPos(QSPString varName, QSPVariant *val, int ind, QSP_BOOL isRegExp)
{
    QSP_TINYINT varType;
    QSPVariant defaultValue, *curValue;
    QSPRegExp *regExp;
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    if (!var) return -1;
    if (isRegExp)
    {
        varType = QSP_TYPE_STR;
        qspConvertVariantTo(val, varType);
        regExp = qspRegExpGetCompiled(QSP_PSTR(val));
        if (!regExp) return -1;
    }
    else
    {
        varType = qspGetVarType(varName);
        if (!qspConvertVariantTo(val, varType))
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            return -1;
        }
    }
    defaultValue = qspGetEmptyVariant(varType);
    if (ind < 0) ind = 0;
    while (ind < var->ValsCount)
    {
        curValue = var->Values + ind;
        if (!QSP_ISDEF(curValue->Type)) curValue = &defaultValue; /* check undefined values */
        if (QSP_BASETYPE(curValue->Type) == varType)
        {
            switch (varType)
            {
            case QSP_TYPE_TUPLE:
                if (!qspTuplesComp(QSP_PTUPLE(val), QSP_PTUPLE(curValue))) return ind;
                break;
            case QSP_TYPE_STR:
                if (isRegExp)
                {
                    if (qspRegExpStrMatch(regExp, QSP_PSTR(curValue))) return ind;
                }
                else
                {
                    if (!qspStrsComp(QSP_PSTR(val), QSP_PSTR(curValue))) return ind;
                }
                break;
            case QSP_TYPE_NUM:
                if (QSP_PNUM(val) == QSP_PNUM(curValue)) return ind;
                break;
            }
        }
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
                        if (qspTuplesComp(QSP_PTUPLE(curValue), QSP_PTUPLE(bestValue)) < 0)
                            bestValue = curValue;
                    }
                    else if (qspTuplesComp(QSP_PTUPLE(curValue), QSP_PTUPLE(bestValue)) > 0)
                        bestValue = curValue;
                    break;
                case QSP_TYPE_STR:
                    if (isMin)
                    {
                        if (qspStrsComp(QSP_PSTR(curValue), QSP_PSTR(bestValue)) < 0)
                            bestValue = curValue;
                    }
                    else if (qspStrsComp(QSP_PSTR(curValue), QSP_PSTR(bestValue)) > 0)
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

void qspSetArgs(QSPVar *destVar, QSPVariant *args, int count, QSP_BOOL toMove)
{
    qspEmptyVar(destVar);
    if (count)
    {
        int i;
        destVar->ValsBufSize = destVar->ValsCount = count;
        destVar->Values = (QSPVariant *)malloc(count * sizeof(QSPVariant));
        if (toMove)
        {
            for (i = 0; i < count; ++i)
                qspMoveToNewVariant(destVar->Values + i, args + i);
        }
        else
        {
            for (i = 0; i < count; ++i)
                qspCopyToNewVariant(destVar->Values + i, args + i);
        }
    }
}

void qspApplyResult(QSPVar *varRes, QSPVariant *res)
{
    if (varRes->ValsCount)
        qspCopyToNewVariant(res, varRes->Values);
    else
        qspInitVariant(res, QSP_TYPE_UNDEF);
}

INLINE int qspGetVarsNames(QSPString names, QSPString **varNames)
{
    QSP_CHAR *pos;
    int count = 0, bufSize = 1;
    QSPString *foundNames = (QSPString *)malloc(bufSize * sizeof(QSPString));
    while (1)
    {
        qspSkipSpaces(&names);
        if (qspIsEmpty(names))
        {
            qspSetError(QSP_ERR_SYNTAX);
            free(foundNames);
            return 0;
        }
        if (count >= bufSize)
        {
            bufSize = count + 4;
            foundNames = (QSPString *)realloc(foundNames, bufSize * sizeof(QSPString));
        }
        pos = qspDelimPos(names, QSP_COMMA[0]);
        if (pos)
        {
            foundNames[count] = qspDelSpc(qspStringFromPair(names.Str, pos));
            ++count;
        }
        else
        {
            foundNames[count] = qspDelSpc(names);
            ++count;
            break;
        }
        names.Str = pos + QSP_STATIC_LEN(QSP_COMMA);
    }
    *varNames = foundNames;
    return count;
}

INLINE void qspSetVarsValues(QSPString *varNames, int varsCount, QSPVariant *v, QSP_CHAR op)
{
    int i, oldLocationState;
    if (varsCount == 1)
    {
        qspSetVarValue(varNames[0], v, op);
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
            int valuesCount = QSP_PTUPLE(v).Items;
            if (varsCount < valuesCount)
            {
                /* Assign variables that contain single values */
                QSPVariant v2;
                int lastVarIndex = varsCount - 1;
                for (i = 0; i < lastVarIndex; ++i)
                {
                    qspSetVarValue(varNames[i], QSP_PTUPLE(v).Vals + i, op);
                    if (qspLocationState != oldLocationState)
                        return;
                }
                /* Only 1 variable left, fill it with a tuple containing all the values left */
                v2 = qspTupleVariant(qspMoveToNewTuple(QSP_PTUPLE(v).Vals + i, QSP_PTUPLE(v).Items - i));
                qspSetVarValue(varNames[lastVarIndex], &v2, op);
                qspFreeVariant(&v2);
            }
            else
            {
                /* Assign all values to the variables */
                for (i = 0; i < valuesCount; ++i)
                {
                    qspSetVarValue(varNames[i], QSP_PTUPLE(v).Vals + i, op);
                    if (qspLocationState != oldLocationState)
                        return;
                }
                /* No values left, reset the rest of vars with default values */
                while (i < varsCount)
                {
                    qspResetVar(varNames[i]);
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
        qspSetVarValue(varNames[0], v, op);
        if (qspLocationState != oldLocationState)
            return;
        for (i = 1; i < varsCount; ++i)
        {
            qspResetVar(varNames[i]);
            if (qspLocationState != oldLocationState)
                return;
        }
        break;
    }
}

void qspStatementSetVarsValues(QSPString s, QSPCachedStat *stat)
{
    QSPVariant v;
    QSPString *names;
    int namesCount, oldLocationState;
    QSP_CHAR op;
    if (stat->ErrorCode)
    {
        qspSetError(stat->ErrorCode);
        return;
    }
    if (stat->ArgsCount < 3)
    {
        qspSetError(QSP_ERR_EQNOTFOUND);
        return;
    }
    oldLocationState = qspLocationState;
    v = qspCalculateExprValue(qspStringFromPair(s.Str + stat->Args[2].StartPos, s.Str + stat->Args[2].EndPos));
    if (qspLocationState != oldLocationState) return;
    namesCount = qspGetVarsNames(qspStringFromPair(s.Str + stat->Args[0].StartPos, s.Str + stat->Args[0].EndPos), &names);
    if (!namesCount)
    {
        qspFreeVariant(&v);
        return;
    }
    op = *(s.Str + stat->Args[1].StartPos); /* contains one of QSP_CHAR_SIMPLEOP characters */
    qspSetVarsValues(names, namesCount, &v, op);
    qspFreeVariant(&v);
    free(names);
}

INLINE QSPString qspGetVarNameOnly(QSPString s)
{
    QSP_CHAR *brackPos = qspStrChar(s, QSP_LSBRACK[0]);
    if (brackPos) s.End = brackPos;
    return s;
}

INLINE QSP_BOOL qspSaveVarToLocalGroup(QSPVarsGroup *varGroup, QSPString varName)
{
    QSPVar *var;
    int i, varsCount;
    QSPString plainVarName;
    if (qspIsEmpty(varName))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return QSP_FALSE;
    }

    /* Ignore type prefix */
    plainVarName = varName;
    if (qspIsInClass(*plainVarName.Str, QSP_CHAR_TYPEPREFIX))
        plainVarName.Str++;

    varsCount = varGroup->VarsCount;
    /* Check if the var exists, variable names are preformatted during code preprocessing */
    for (i = 0; i < varsCount; ++i)
    {
        if (!qspStrsComp(plainVarName, varGroup->Vars[i].Name))
        {
            /* Already exists, so the value is saved already */
            return QSP_TRUE;
        }
    }
    /* Get variable to add it to the local group */
    var = qspVarReference(varName, QSP_FALSE); /* use initial name */
    if (!var) return QSP_FALSE;
    if (varsCount >= varGroup->Capacity)
    {
        varGroup->Capacity += 4;
        varGroup->Vars = (QSPVar *)realloc(varGroup->Vars, varGroup->Capacity * sizeof(QSPVar));
    }
    /* Save & reset the variable */
    qspMoveVar(varGroup->Vars + varsCount, var);
    varGroup->Vars[varsCount].Name = qspCopyToNewText(plainVarName);
    varGroup->VarsCount = varsCount + 1;
    return QSP_TRUE;
}

void qspStatementLocal(QSPString s, QSPCachedStat *stat)
{
    QSPVariant v;
    QSPString *names, varName;
    QSPVarsGroup *curVarGroup;
    int i, namesCount, groupInd;
    if (stat->ErrorCode)
    {
        qspSetError(stat->ErrorCode);
        return;
    }
    if (stat->ArgsCount > 1)
    {
        int oldLocationState = qspLocationState;
        if (*(s.Str + stat->Args[1].StartPos) != QSP_EQUAL[0])
        {
            qspSetError(QSP_ERR_SYNTAX);
            return;
        }
        /* We have to evaluate expression before allocation of local vars */
        v = qspCalculateExprValue(qspStringFromPair(s.Str + stat->Args[2].StartPos, s.Str + stat->Args[2].EndPos));
        if (qspLocationState != oldLocationState) return;
    }
    else
        v = qspGetEmptyVariant(QSP_TYPE_UNDEF);
    namesCount = qspGetVarsNames(qspStringFromPair(s.Str + stat->Args[0].StartPos, s.Str + stat->Args[0].EndPos), &names);
    if (!namesCount)
    {
        qspFreeVariant(&v);
        return;
    }
    groupInd = qspSavedVarGroupsCount - 1;
    curVarGroup = qspSavedVarGroups + groupInd;
    for (i = 0; i < namesCount; ++i)
    {
        varName = qspGetVarNameOnly(names[i]);
        if (!qspSaveVarToLocalGroup(curVarGroup, varName))
        {
            qspFreeVariant(&v);
            free(names);
            return;
        }
    }
    if (stat->ArgsCount > 1)
    {
        qspSetVarsValues(names, namesCount, &v, QSP_EQUAL[0]);
        qspFreeVariant(&v);
    }
    free(names);
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
    QSPVar *dest;
    QSPTuple src;
    int startInd, maxCount;
    if (!(dest = qspVarReference(QSP_STR(args[0]), QSP_TRUE))) return;
    src = QSP_TUPLE(args[1]);
    startInd = (count >= 3 ? QSP_TOINT(QSP_NUM(args[2])) : 0);
    maxCount = (count == 4 ? QSP_TOINT(QSP_NUM(args[3])) : src.Items);
    qspUnpackTupleToArray(dest, src, startInd, maxCount);
}

void qspStatementCopyArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPVar *dest, *src;
    if (!(dest = qspVarReference(QSP_STR(args[0]), QSP_TRUE))) return;
    if (!(src = qspVarReference(QSP_STR(args[1]), QSP_FALSE))) return;
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
        if (curInd >= var->ValsBufSize)
        {
            var->ValsBufSize = curInd + 8;
            var->Values = (QSPVariant *)realloc(var->Values, var->ValsBufSize * sizeof(QSPVariant));
        }
        qspCopyToNewVariant(var->Values + curInd, &foundString);
        ++curInd;
        foundPos = qspRegExpStrSearch(regExp, text, foundPos + foundLen, groupInd, &foundLen);
    }
    var->ValsCount = curInd;
}

void qspStatementKillVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (count == 0)
        qspClearAllVars(QSP_FALSE);
    else
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
}
