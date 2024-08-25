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
#include "errors.h"
#include "locations.h"
#include "mathops.h"
#include "regexp.h"
#include "text.h"

QSPVar qspVars[QSP_VARSCOUNT];
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
INLINE void qspResetVar(QSPString varName);
INLINE void qspSetVar(QSPString varName, QSPVariant *val, QSP_CHAR op);
INLINE void qspCopyVar(QSPVar *dest, QSPVar *src, int start, int count);
INLINE void qspSortArray(QSPVar *var, QSP_TINYINT baseValType, QSP_BOOL isAscending);
INLINE int qspGetVarsNames(QSPString names, QSPString **varNames);
INLINE void qspSetVarsValues(QSPString *varNames, int varsCount, QSPVariant *v, QSP_CHAR op);
INLINE QSPString qspGetVarNameOnly(QSPString s);

void qspInitVarTypes(void)
{
    int i;
    for (i = 0; i < sizeof(qspSpecToBaseTypeTable); ++i)
        qspSpecToBaseTypeTable[i] = QSP_TYPE_NUM;

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
    int i;
    QSPVar *var;
    QSP_CHAR *pos;
    unsigned char bCode;
    if (qspIsEmpty(name))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return 0;
    }

    /* Ignore type specification */
    if (*name.Str == QSP_STRCHAR[0])
        name.Str += QSP_STATIC_LEN(QSP_STRCHAR);
    else if (*name.Str == QSP_TUPLECHAR[0])
        name.Str += QSP_STATIC_LEN(QSP_TUPLECHAR);

    if (qspIsEmpty(name) || qspIsInClass(*name.Str, QSP_CHAR_DIGIT) || qspIsAnyInClass(name, QSP_CHAR_DELIM))
    {
        qspSetError(QSP_ERR_INCORRECTNAME);
        return 0;
    }
    bCode = 7;
    for (pos = name.Str; pos < name.End; ++pos)
        bCode = bCode * 31 + (unsigned char)*pos;
    var = qspVars + QSP_VARSSEEK * bCode;
    for (i = 0; i < QSP_VARSSEEK; ++i)
    {
        if (!var->Name.Str)
        {
            if (toCreate) var->Name = qspCopyToNewText(name);
            return var;
        }
        if (!qspStrsComp(var->Name, name)) return var;
        ++var;
    }
    qspSetError(QSP_ERR_TOOMANYVARS);
    return 0;
}

void qspClearAllVars(QSP_BOOL toInit)
{
    int i;
    QSPVar *var = qspVars;
    for (i = 0; i < QSP_VARSCOUNT; ++i)
    {
        if (toInit)
            qspInitVarData(var);
        else
        {
            qspFreeString(&var->Name);
            qspEmptyVar(var);
        }
        var->Name = qspNullString;
        ++var;
    }
}

INLINE void qspRemoveArrayItem(QSPVar *var, int index)
{
    QSP_BOOL toRemove;
    QSPVarIndex *ind;
    int i;
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
    if (QSP_ISNUM(index.Type)) return QSP_NUM(index);
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
            ind = qspExprValue(qspStringFromPair(s.Str, rPos));
            if (qspLocationState != oldLocationState) return 0;
            *index = qspGetVarIndex(var, ind, isSetOperation);
            qspFreeVariant(&ind);
        }
        return var;
    }
    *index = 0;
    return qspVarReference(s, isSetOperation);
}

void qspSetVarValueByReference(QSPVar *var, int ind, QSPVariant *val)
{
    int oldCount = var->ValsCount;
    if (ind >= oldCount)
    {
        QSPVariant *curValue;
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
        qspCopyToNewVariant(var->Values + ind, val);
    }
    else if (ind >= 0)
    {
        qspFreeVariant(var->Values + ind);
        qspCopyToNewVariant(var->Values + ind, val);
    }
}

INLINE void qspResetVar(QSPString varName)
{
    QSPVar* var;
    int index;
    if (!(var = qspGetVarData(varName, &index, QSP_TRUE))) return;
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

INLINE void qspSetVar(QSPString varName, QSPVariant *val, QSP_CHAR op)
{
    QSPVar *var;
    int index;
    QSP_TINYINT baseVarType;
    if (!(var = qspGetVarData(varName, &index, QSP_TRUE))) return;
    baseVarType = qspGetVarType(varName);
    if (op == QSP_EQUAL[0])
    {
        if (baseVarType != QSP_BASETYPE(val->Type))
        {
            if (!qspConvertVariantTo(val, baseVarType))
            {
                qspSetError(QSP_ERR_TYPEMISMATCH);
                return;
            }
        }
        qspSetVarValueByReference(var, index, val);
    }
    else if (qspIsInClass(op, QSP_CHAR_SIMPLEOP))
    {
        QSPVariant oldVal, res;
        qspGetVarValueByReference(var, index, baseVarType, &oldVal);
        if (!qspAutoConvertCombine(&oldVal, val, op, &res))
        {
            qspFreeVariant(&oldVal);
            return;
        }
        if (!qspConvertVariantTo(&res, baseVarType))
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            qspFreeVariant(&res);
            qspFreeVariant(&oldVal);
            return;
        }
        qspSetVarValueByReference(var, index, &res);
        qspFreeVariant(&res);
        qspFreeVariant(&oldVal);
    }
    else
    {
        qspSetError(QSP_ERR_UNKNOWNACTION);
        return;
    }
}

void qspGetVarValueByReference(QSPVar *var, int ind, QSP_TINYINT baseType, QSPVariant *res)
{
    if (ind >= 0 && ind < var->ValsCount)
    {
        QSP_TINYINT varType = var->Values[ind].Type;
        if (QSP_ISDEF(varType) && QSP_BASETYPE(varType) == baseType)
        {
            qspCopyToNewVariant(res, var->Values + ind);
            return;
        }
    }
    qspInitVariant(res, baseType);
}

QSPTuple qspGetVarTupleValue(QSPString name)
{
    QSPVar *var = qspVarReference(name, QSP_FALSE);
    if (var)
    {
        if (var->ValsCount && QSP_ISTUPLE(var->Values[0].Type))
            return QSP_TUPLE(var->Values[0]);
    }
    else
        qspResetError(QSP_FALSE);
    return qspNullTuple;
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

int qspGetVarNumValue(QSPString name)
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

void qspRestoreGlobalVars(void)
{
    if (qspSavedVarGroupsCount)
    {
        int i;
        QSPVarsGroup *curVarGroup = qspSavedVarGroups;
        for (i = qspSavedVarGroupsCount; i > 0; --i, ++curVarGroup)
            qspRestoreVarsList(curVarGroup->Vars, curVarGroup->VarsCount);
        qspSavedVarGroupsCount = 0;
    }
}

int qspSaveLocalVarsAndRestoreGlobals(QSPVar **vars)
{
    QSPVar *var, *savedVars;
    int i, j, ind, varsCount = 0;
    QSPVarsGroup *curVarGroup = qspSavedVarGroups;
    for (i = qspSavedVarGroupsCount; i > 0; --i, ++curVarGroup)
        varsCount += curVarGroup->VarsCount;
    if (!varsCount)
    {
        *vars = 0;
        return 0;
    }
    savedVars = (QSPVar *)malloc(varsCount * sizeof(QSPVar));
    ind = 0;
    curVarGroup = qspSavedVarGroups;
    for (i = qspSavedVarGroupsCount; i > 0; --i, ++curVarGroup)
    {
        for (j = curVarGroup->VarsCount - 1; j >= 0; --j)
        {
            if (!(var = qspVarReference(curVarGroup->Vars[j].Name, QSP_TRUE)))
            {
                while (--ind >= 0)
                    qspEmptyVar(savedVars + ind);
                free(savedVars);
                return 0;
            }
            /* We keep var names in qspSavedVarGroups, savedVars have empty names */
            qspMoveVar(savedVars + ind, var);
            qspMoveVar(var, &curVarGroup->Vars[j]);
            ++ind;
        }
    }
    *vars = savedVars;
    return varsCount;
}

void qspRestoreLocalVars(QSPVar *savedVars, int varsCount, QSPVarsGroup *savedGroups, int groupsCount)
{
    if (savedVars)
    {
        QSPVar *var;
        int i, j, ind = varsCount - 1;
        QSPVarsGroup *curVarGroup = savedGroups;
        for (i = 0; i < groupsCount; ++i, ++curVarGroup)
        {
            for (j = 0; j < curVarGroup->VarsCount; ++j)
            {
                if (!(var = qspVarReference(curVarGroup->Vars[j].Name, QSP_TRUE)))
                {
                    while (ind >= 0)
                    {
                        /* savedVars don't have names here */
                        qspEmptyVar(savedVars + ind);
                        --ind;
                    }
                    free(savedVars);
                    return;
                }
                /* savedVars don't have names here */
                qspMoveVar(&curVarGroup->Vars[j], var);
                qspMoveVar(var, savedVars + ind);
                --ind;
            }
        }
        free(savedVars);
    }
}

void qspRestoreVarsList(QSPVar *vars, int count)
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

INLINE void qspCopyVar(QSPVar *dest, QSPVar *src, int start, int count)
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
    QSPVar *var;
    if (!(var = qspVarReference(varName, QSP_FALSE))) return 0;
    return var->ValsCount;
}

int qspArrayPos(QSPString varName, QSPVariant *val, int ind, QSP_BOOL isRegExp)
{
    QSPVar *var;
    QSP_TINYINT baseVarType;
    QSPVariant defaultValue, *curValue;
    QSP_BOOL isFound;
    QSPRegExp *regExp;
    if (!(var = qspVarReference(varName, QSP_FALSE))) return -1;
    if (isRegExp)
    {
        baseVarType = QSP_TYPE_STR;
        qspConvertVariantTo(val, baseVarType);
        regExp = qspRegExpGetCompiled(QSP_PSTR(val));
        if (!regExp) return -1;
    }
    else
    {
        baseVarType = qspGetVarType(varName);
        if (!qspConvertVariantTo(val, baseVarType))
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            return -1;
        }
    }
    defaultValue = qspGetEmptyVariant(baseVarType);
    if (ind < 0) ind = 0;
    for (curValue = var->Values + ind; ind < var->ValsCount; ++ind, ++curValue)
    {
        if (!QSP_ISDEF(curValue->Type)) curValue = &defaultValue; /* check undefined values */
        if (QSP_BASETYPE(curValue->Type) == baseVarType)
        {
            switch (baseVarType)
            {
                case QSP_TYPE_TUPLE:
                    isFound = !qspTuplesComp(QSP_PTUPLE(val), QSP_PTUPLE(curValue));
                    break;
                case QSP_TYPE_STR:
                    isFound = isRegExp ?
                              qspRegExpStrMatch(regExp, QSP_PSTR(curValue)) :
                              !qspStrsComp(QSP_PSTR(val), QSP_PSTR(curValue));
                    break;
                case QSP_TYPE_NUM:
                    isFound = (QSP_PNUM(val) == QSP_PNUM(curValue));
                    break;
            }
            if (isFound) return ind;
        }
    }
    return -1;
}

QSPVariant qspArrayMinMaxItem(QSPString varName, QSP_BOOL isMin)
{
    QSPVar *var;
    QSPVariant resultValue, *bestValue, *curValue;
    QSP_TINYINT baseVarType;
    int count;
    if (!(var = qspVarReference(varName, QSP_FALSE)))
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    baseVarType = qspGetVarType(varName);
    resultValue = qspGetEmptyVariant(baseVarType);
    bestValue = 0;
    curValue = var->Values;
    count = var->ValsCount;
    while (--count >= 0)
    {
        if (!QSP_ISDEF(curValue->Type)) curValue = &resultValue; /* check undefined values */
        if (QSP_BASETYPE(curValue->Type) == baseVarType)
        {
            if (bestValue)
            {
                switch (baseVarType)
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
        ++curValue;
    }
    if (bestValue) qspCopyToNewVariant(&resultValue, bestValue);
    return resultValue;
}

int qspGetVarsCount(void)
{
    int i, count = 0;
    for (i = 0; i < QSP_VARSCOUNT; ++i)
        if (qspVars[i].Name.Str) ++count;
    return count;
}

void qspSetArgs(QSPVar *var, QSPVariant *args, int count)
{
    while (--count >= 0) /* iterate from top to bottom to optimize memory allocations */
        qspSetVarValueByReference(var, count, args + count);
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
        qspSetVar(varNames[0], v, op);
        return;
    }
    /* Examples:
     * a,b=2,3
     * a,b=5
     * a,b,c=4,5
     * a,b=5,6,7
     * a,b=%empty_tuple
     * */
    oldLocationState = qspLocationState;
    switch (QSP_BASETYPE(v->Type))
    {
        case QSP_TYPE_TUPLE:
        {
            int lastVarIndex = varsCount - 1, lastValIndex = QSP_PTUPLE(v).Items - 1;
            if (lastVarIndex < lastValIndex)
            {
                /* Assign variables that contain single values */
                QSPVariant v2;
                for (i = 0; i < lastVarIndex; ++i)
                {
                    qspSetVar(varNames[i], QSP_PTUPLE(v).Vals + i, op);
                    if (qspLocationState != oldLocationState)
                        return;
                }
                /* Only 1 variable left, fill it with the tuple containing all the values left */
                v2.Type = QSP_TYPE_TUPLE;
                QSP_TUPLE(v2) = qspMoveToNewTuple(QSP_PTUPLE(v).Vals + i, QSP_PTUPLE(v).Items - i);
                qspSetVar(varNames[lastVarIndex], &v2, op);
                qspFreeVariant(&v2);
            }
            else
            {
                if (lastValIndex >= 0)
                {
                    /* Assign variables that contain single values */
                    for (i = 0; i < lastValIndex; ++i)
                    {
                        qspSetVar(varNames[i], QSP_PTUPLE(v).Vals + i, op);
                        if (qspLocationState != oldLocationState)
                            return;
                    }
                    /* Only 1 value left, fill the rest of vars with the last value */
                    while (i < varsCount)
                    {
                        qspSetVar(varNames[i], QSP_PTUPLE(v).Vals + lastValIndex, op);
                        if (qspLocationState != oldLocationState)
                            return;
                        ++i;
                    }
                }
                else
                {
                    /* No values, update vars with default values */
                    for (i = 0; i < varsCount; ++i)
                    {
                        qspResetVar(varNames[i]);
                        if (qspLocationState != oldLocationState)
                            return;
                    }
                }
            }
            break;
        }
        case QSP_TYPE_NUM:
        case QSP_TYPE_STR:
            for (i = 0; i < varsCount; ++i)
            {
                qspSetVar(varNames[i], v, op);
                if (qspLocationState != oldLocationState)
                    return;
            }
            break;
    }
}

void qspStatementSetVarValue(QSPString s, QSPCachedStat *stat)
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
    v = qspExprValue(qspStringFromPair(s.Str + stat->Args[2].StartPos, s.Str + stat->Args[2].EndPos));
    if (qspLocationState != oldLocationState) return;
    namesCount = qspGetVarsNames(qspStringFromPair(s.Str + stat->Args[0].StartPos, s.Str + stat->Args[0].EndPos), &names);
    if (!namesCount) return;
    op = *(s.Str + stat->Args[1].StartPos);
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

void qspStatementLocal(QSPString s, QSPCachedStat *stat)
{
    QSP_BOOL isVarFound;
    QSPString *names, varName;
    QSPVarsGroup *curVarGroup;
    QSPVariant v;
    int i, j, namesCount, groupInd, varsCount, bufSize;
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
        v = qspExprValue(qspStringFromPair(s.Str + stat->Args[2].StartPos, s.Str + stat->Args[2].EndPos));
        if (qspLocationState != oldLocationState) return;
    }
    namesCount = qspGetVarsNames(qspStringFromPair(s.Str + stat->Args[0].StartPos, s.Str + stat->Args[0].EndPos), &names);
    if (!namesCount) return;
    groupInd = qspSavedVarGroupsCount - 1;
    curVarGroup = qspSavedVarGroups + groupInd;
    varsCount = bufSize = curVarGroup->VarsCount;
    isVarFound = QSP_FALSE;
    for (i = 0; i < namesCount; ++i)
    {
        varName = names[i];
        if (qspIsEmpty(varName))
        {
            qspSetError(QSP_ERR_SYNTAX);
            free(names);
            return;
        }

        /* Ignore type specification */
        if (*varName.Str == QSP_STRCHAR[0])
            varName.Str += QSP_STATIC_LEN(QSP_STRCHAR);
        else if (*varName.Str == QSP_TUPLECHAR[0])
            varName.Str += QSP_STATIC_LEN(QSP_TUPLECHAR);

        varName = qspGetVarNameOnly(varName);
        /* Check for the existence */
        for (j = 0; j < varsCount; ++j)
        {
            if (!qspStrsComp(varName, curVarGroup->Vars[j].Name))
            {
                isVarFound = QSP_TRUE;
                break;
            }
        }
        if (isVarFound)
        {
            /* Already exists */
            isVarFound = QSP_FALSE;
        }
        else
        {
            QSPVar *var;
            /* Add variable to the local group */
            if (!(var = qspVarReference(varName, QSP_FALSE)))
            {
                free(names);
                return;
            }
            if (varsCount >= bufSize)
            {
                bufSize = varsCount + 4;
                curVarGroup->Vars = (QSPVar *)realloc(curVarGroup->Vars, bufSize * sizeof(QSPVar));
            }
            qspMoveVar(curVarGroup->Vars + varsCount, var);
            curVarGroup->Vars[varsCount].Name = qspCopyToNewText(varName);
            curVarGroup->VarsCount = ++varsCount;
        }
    }
    if (stat->ArgsCount > 1)
    {
        qspSetVarsValues(names, namesCount, &v, QSP_EQUAL[0]);
        qspFreeVariant(&v);
    }
    free(names);
}

void qspStatementCopyArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPVar *dest, *src;
    if (!(dest = qspVarReference(QSP_STR(args[0]), QSP_TRUE))) return;
    if (!(src = qspVarReference(QSP_STR(args[1]), QSP_FALSE))) return;
    if (dest != src)
    {
        int start = (count >= 3 ? QSP_NUM(args[2]) : 0);
        int maxCount = (count == 4 ? QSP_NUM(args[3]) : src->ValsCount);
        qspCopyVar(dest, src, start, maxCount);
    }
}

void qspStatementSortArr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPVar *var;
    QSP_TINYINT baseVarType;
    if (!(var = qspVarReference(QSP_STR(args[0]), QSP_FALSE))) return; /* we don't create a new array */
    baseVarType = qspGetVarType(QSP_STR(args[0]));
    if (count == 2)
        qspSortArray(var, baseVarType, QSP_ISFALSE(QSP_NUM(args[1])));
    else
        qspSortArray(var, baseVarType, QSP_TRUE);
}

void qspStatementSplitStr(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPVar *var;
    QSPRegExp *regExp;
    QSP_CHAR *foundPos;
    QSPString text;
    QSPVariant foundString;
    int curInd, groupInd, foundLen;
    if (!(var = qspVarReference(QSP_STR(args[0]), QSP_TRUE))) return;
    regExp = qspRegExpGetCompiled(QSP_STR(args[2]));
    if (!regExp) return;
    text = QSP_STR(args[1]);
    groupInd = (count == 4 ? QSP_NUM(args[3]) : 0);
    qspEmptyVar(var); /* clear the dest array anyway */
    foundString.Type = QSP_TYPE_STR;
    curInd = 0;
    foundPos = qspRegExpStrSearch(regExp, text, groupInd, &foundLen);
    while (foundPos)
    {
        QSP_STR(foundString) = qspStringFromLen(foundPos, foundLen);
        if (curInd >= var->ValsBufSize)
        {
            var->ValsBufSize = curInd + 8;
            var->Values = (QSPVariant *)realloc(var->Values, var->ValsBufSize * sizeof(QSPVariant));
        }
        qspCopyToNewVariant(var->Values + curInd, &foundString);
        ++curInd;
        text.Str = foundPos + foundLen;
        foundPos = qspRegExpStrSearch(regExp, text, groupInd, &foundLen);
    }
    var->ValsCount = curInd;
}

void qspStatementKillVar(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (count == 0)
        qspClearAllVars(QSP_FALSE);
    else
    {
        QSPVar *var;
        if (!(var = qspVarReference(QSP_STR(args[0]), QSP_FALSE))) return; /* we don't create a new array */
        if (count == 1)
            qspEmptyVar(var);
        else
        {
            int arrIndex = qspGetVarIndex(var, args[1], QSP_FALSE);
            qspRemoveArrayItem(var, arrIndex);
        }
    }
}
