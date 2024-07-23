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
INLINE void qspRemoveArrayItem(QSPVar *var, int index);
INLINE QSPString qspTupleToIndexString(QSPTuple tuple);
INLINE QSPVar *qspGetVarData(QSPString s, int *index, QSP_BOOL isSetOperation);
INLINE void qspSetVar(QSPString name, QSPVariant *val, QSP_CHAR op);
INLINE void qspCopyVar(QSPVar *dest, QSPVar *src, int start, int count);
INLINE int qspGetVarsNames(QSPString names, QSPString **varNames);
INLINE void qspSetVarsValues(QSPString *varNames, int varsCount, QSPVariant *v, QSP_CHAR op);
INLINE QSPString qspGetVarNameOnly(QSPString s);

void qspInitVarTypes()
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

    if (qspIsEmpty(name) || qspIsDigit(*name.Str) || qspIsAnyInClass(name, QSP_CHAR_DELIM))
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
            if (toCreate) var->Name = qspGetNewText(name);
            return var;
        }
        if (!qspStrsComp(var->Name, name)) return var;
        ++var;
    }
    qspSetError(QSP_ERR_TOOMANYVARS);
    return 0;
}

void qspClearVars(QSP_BOOL toInit)
{
    int i;
    QSPVar *var = qspVars;
    for (i = 0; i < QSP_VARSCOUNT; ++i)
    {
        if (toInit)
            qspInitVarData(var);
        else
        {
            qspFreeString(var->Name);
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
    qspFreeVariants(var->Values + index, 1);
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
            qspFreeString(ind->Str);
            var->IndsCount--;
            if (i == var->IndsCount) break;
            toRemove = QSP_TRUE;
        }
        if (toRemove) *ind = *(ind + 1);
        if (ind->Index > index) ind->Index--;
        ++ind;
    }
}

INLINE QSPString qspTupleToIndexString(QSPTuple tuple)
{
    QSP_CHAR buf[QSP_NUMTOSTRBUF];
    QSPString res = qspNullString;
    QSPVariant *item = tuple.Vals, *itemsEnd = item + tuple.Items;
    while (item < itemsEnd)
    {
        switch (QSP_BASETYPE(item->Type))
        {
            case QSP_TYPE_TUPLE:
                qspAddText(&res, QSP_STATIC_STR(QSP_TUPLESTART), QSP_FALSE);
                qspAddText(&res, qspTupleToIndexString(QSP_PTUPLE(item)), QSP_FALSE);
                qspAddText(&res, QSP_STATIC_STR(QSP_TUPLEEND), QSP_FALSE);
                break;
            case QSP_TYPE_NUM:
                qspAddText(&res, qspNumToStr(buf, QSP_PNUM(item)), QSP_FALSE);
                break;
            case QSP_TYPE_STR:
                qspAddText(&res, QSP_PSTR(item), QSP_FALSE);
                break;
        }
        if (++item == itemsEnd) break;
        qspAddText(&res, QSP_STATIC_STR(QSP_TUPLEDELIM), QSP_FALSE);
    }
    return res;
}

int qspGetVarIndex(QSPVar *var, QSPVariant index, QSP_BOOL toCreate)
{
    int indsCount;
    QSPString uStr;
    switch (QSP_BASETYPE(index.Type))
    {
        case QSP_TYPE_TUPLE:
            uStr = qspTupleToIndexString(QSP_TUPLE(index));
            break;
        case QSP_TYPE_NUM:
            return QSP_NUM(index);
        case QSP_TYPE_STR:
            uStr = qspGetNewText(QSP_STR(index));
            break;
    }
    qspUpperStr(&uStr);
    indsCount = var->IndsCount;
    if (indsCount > 0)
    {
        QSPVarIndex *ind = (QSPVarIndex *)bsearch(&uStr, var->Indices, indsCount, sizeof(QSPVarIndex), qspIndStringCompare);
        if (ind)
        {
            qspFreeString(uStr);
            return ind->Index;
        }
    }
    if (toCreate)
    {
        int i;
        var->IndsCount++;
        if (indsCount >= var->IndsBufSize)
        {
            var->IndsBufSize = indsCount + 8;
            var->Indices = (QSPVarIndex *)realloc(var->Indices, var->IndsBufSize * sizeof(QSPVarIndex));
        }
        i = indsCount - 1;
        while (i >= 0 && qspStrsComp(var->Indices[i].Str, uStr) > 0)
        {
            var->Indices[i + 1] = var->Indices[i];
            --i;
        }
        ++i;
        indsCount = var->ValsCount; /* point to the new array item */
        var->Indices[i].Str = uStr;
        var->Indices[i].Index = indsCount;
        return indsCount;
    }
    qspFreeString(uStr);
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
                *index = var->ValsCount;
            else
                *index = (var->ValsCount ? var->ValsCount - 1 : 0);
        }
        else
        {
            QSPVariant ind;
            int oldRefreshCount = qspRefreshCount;
            ind = qspExprValue(qspStringFromPair(s.Str, rPos));
            if (qspRefreshCount != oldRefreshCount || qspErrorNum) return 0;
            *index = qspGetVarIndex(var, ind, isSetOperation);
            qspFreeVariants(&ind, 1);
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
        int count = var->ValsCount = ind + 1;
        var->Values = (QSPVariant *)realloc(var->Values, count * sizeof(QSPVariant));
        for (curValue = var->Values + oldCount; oldCount < count; ++curValue, ++oldCount)
            qspInitVariant(curValue, QSP_TYPE_UNDEF);
    }
    if (ind >= 0) qspUpdateVariantValue(var->Values + ind, val);
}

INLINE void qspSetVar(QSPString name, QSPVariant *val, QSP_CHAR op)
{
    QSPVar *var;
    int index;
    QSP_TINYINT baseVarType;
    if (!(var = qspGetVarData(name, &index, QSP_TRUE))) return;
    baseVarType = qspGetVarType(name);
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
            qspFreeVariants(&oldVal, 1);
            return;
        }
        if (!qspConvertVariantTo(&res, baseVarType))
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            qspFreeVariants(&res, 1);
            qspFreeVariants(&oldVal, 1);
            return;
        }
        qspSetVarValueByReference(var, index, &res);
        qspFreeVariants(&res, 1);
        qspFreeVariants(&oldVal, 1);
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
    QSPVar *var;
    if (var = qspVarReference(name, QSP_FALSE))
    {
        if (var->ValsCount && QSP_ISTUPLE(var->Values[0].Type))
            return QSP_TUPLE(var->Values[0]);
    }
    else
        qspResetError();
    return qspNullTuple;
}

QSPString qspGetVarStrValue(QSPString name)
{
    QSPVar *var;
    if (var = qspVarReference(name, QSP_FALSE))
    {
        if (var->ValsCount && QSP_ISSTR(var->Values[0].Type))
            return QSP_STR(var->Values[0]);
    }
    else
        qspResetError();
    return qspNullString;
}

int qspGetVarNumValue(QSPString name)
{
    QSPVar *var;
    if (var = qspVarReference(name, QSP_FALSE))
    {
        if (var->ValsCount && QSP_ISNUM(var->Values[0].Type))
            return QSP_NUM(var->Values[0]);
    }
    else
        qspResetError();
    return 0;
}

void qspRestoreGlobalVars()
{
    int i;
    if (qspSavedVarGroupsCount)
    {
        for (i = qspSavedVarGroupsCount - 1; i >= 0; --i)
            qspRestoreVarsList(qspSavedVarGroups[i].Vars, qspSavedVarGroups[i].VarsCount);
        qspSavedVarGroupsCount = 0;
    }
}

int qspSaveLocalVarsAndRestoreGlobals(QSPVar **vars)
{
    QSPVar *var, *savedVars;
    int i, j, ind, varsCount = 0;
    for (i = qspSavedVarGroupsCount - 1; i >= 0; --i)
        varsCount += qspSavedVarGroups[i].VarsCount;
    if (!varsCount)
    {
        *vars = 0;
        return 0;
    }
    savedVars = (QSPVar *)malloc(varsCount * sizeof(QSPVar));
    ind = 0;
    for (i = qspSavedVarGroupsCount - 1; i >= 0; --i)
    {
        for (j = qspSavedVarGroups[i].VarsCount - 1; j >= 0; --j)
        {
            if (!(var = qspVarReference(qspSavedVarGroups[i].Vars[j].Name, QSP_TRUE)))
            {
                while (--ind >= 0)
                    qspEmptyVar(savedVars + ind);
                free(savedVars);
                return 0;
            }
            /* We keep var names in qspSavedVarGroups, savedVars have empty names */
            qspMoveVar(savedVars + ind, var);
            qspMoveVar(var, &qspSavedVarGroups[i].Vars[j]);
            ++ind;
        }
    }
    *vars = savedVars;
    return varsCount;
}

void qspRestoreLocalVars(QSPVar *savedVars, int varsCount, QSPVarsGroup *savedGroups, int groupsCount)
{
    QSPVar *var;
    int i, j, ind;
    if (savedVars)
    {
        ind = varsCount - 1;
        for (i = 0; i < groupsCount; ++i)
        {
            for (j = 0; j < savedGroups[i].VarsCount; ++j)
            {
                if (!(var = qspVarReference(savedGroups[i].Vars[j].Name, QSP_TRUE)))
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
                qspMoveVar(&savedGroups[i].Vars[j], var);
                qspMoveVar(var, savedVars + ind);
                --ind;
            }
        }
        free(savedVars);
    }
}

void qspRestoreVarsList(QSPVar *vars, int varsCount)
{
    int i;
    QSPVar *var;
    if (vars)
    {
        for (i = 0; i < varsCount; ++i)
        {
            if (!(var = qspVarReference(vars[i].Name, QSP_TRUE)))
            {
                while (i < varsCount)
                {
                    qspFreeString(vars[i].Name);
                    qspEmptyVar(vars + i);
                    ++i;
                }
                free(vars);
                return;
            }
            qspFreeString(vars[i].Name);
            qspEmptyVar(var);
            qspMoveVar(var, vars + i);
        }
        free(vars);
    }
}

void qspClearVarsList(QSPVar *vars, int varsCount)
{
    if (vars)
    {
        int i;
        for (i = 0; i < varsCount; ++i)
        {
            qspFreeString(vars[i].Name);
            qspEmptyVar(vars + i);
        }
        free(vars);
    }
}

INLINE void qspCopyVar(QSPVar *dest, QSPVar *src, int start, int count)
{
    int i, maxCount, newInd;
    qspEmptyVar(dest);
    if (start < 0) start = 0;
    maxCount = src->ValsCount - start;
    if (count <= 0 || maxCount <= 0) return;
    if (count < maxCount) maxCount = count;
    dest->ValsCount = maxCount;
    dest->Values = (QSPVariant *)malloc(maxCount * sizeof(QSPVariant));
    for (i = 0; i < maxCount; ++i)
        qspCopyToNewVariant(dest->Values + i, src->Values + i + start);
    dest->IndsBufSize = 0;
    dest->Indices = 0;
    count = 0;
    for (i = 0; i < src->IndsCount; ++i)
    {
        newInd = src->Indices[i].Index - start;
        if (newInd >= 0 && newInd < maxCount)
        {
            if (count >= dest->IndsBufSize)
            {
                dest->IndsBufSize = count + 16;
                dest->Indices = (QSPVarIndex *)realloc(dest->Indices, dest->IndsBufSize * sizeof(QSPVarIndex));
            }
            dest->Indices[count].Index = newInd;
            dest->Indices[count].Str = qspGetNewText(src->Indices[i].Str);
            ++count;
        }
    }
    dest->IndsCount = count;
}

int qspArraySize(QSPString name)
{
    QSPVar *var;
    if (!(var = qspVarReference(name, QSP_FALSE))) return 0;
    return var->ValsCount;
}

int qspArrayPos(QSPString varName, QSPVariant *val, int ind, QSP_BOOL isRegExp)
{
    int count;
    QSP_TINYINT baseVarType;
    QSPVar *var;
    QSPVariant defaultValue, *curValue;
    QSP_BOOL isFound;
    QSPRegExp *regExp;
    if (!(var = qspVarReference(varName, QSP_FALSE))) return -1;
    baseVarType = qspGetVarType(varName);
    if (!qspConvertVariantTo(val, (isRegExp || QSP_ISSTR(baseVarType) ? QSP_TYPE_STR : QSP_TYPE_NUM)))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return -1;
    }
    if (isRegExp)
    {
        regExp = qspRegExpGetCompiled(QSP_PSTR(val));
        if (!regExp) return -1;
    }
    defaultValue = qspGetEmptyVariant(baseVarType);
    count = var->ValsCount;
    if (ind < 0) ind = 0;
    while (ind < count)
    {
        curValue = var->Values + ind;
        if (!QSP_ISDEF(curValue->Type)) curValue = &defaultValue; /* check undefined values */
        if (QSP_BASETYPE(curValue->Type) == QSP_BASETYPE(val->Type))
        {
            if (QSP_ISSTR(val->Type))
            {
                isFound = isRegExp ?
                          qspRegExpStrMatch(regExp, QSP_PSTR(curValue)) :
                          !qspStrsComp(QSP_PSTR(val), QSP_PSTR(curValue));
            }
            else
            {
                isFound = (QSP_PNUM(val) == QSP_PNUM(curValue));
            }
            if (isFound) return ind;
        }
        ++ind;
    }
    return -1;
}

QSPVariant qspArrayMinMaxItem(QSPString name, QSP_BOOL isMin)
{
    QSPVar *var;
    QSPVariant resultValue, *curValue;
    QSP_TINYINT baseVarType;
    int curInd, count;
    if (!(var = qspVarReference(name, QSP_FALSE)))
        return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    baseVarType = qspGetVarType(name);
    resultValue = qspGetEmptyVariant(baseVarType);
    curInd = -1;
    count = var->ValsCount;
    while (--count >= 0)
    {
        curValue = var->Values + count;
        if (!QSP_ISDEF(curValue->Type)) curValue = &resultValue; /* check undefined values */
        if (QSP_BASETYPE(curValue->Type) == baseVarType)
        {
            if (curInd >= 0)
            {
                if (QSP_ISSTR(baseVarType))
                {
                    if (isMin)
                    {
                        if (qspStrsComp(QSP_PSTR(curValue), QSP_PSTR(var->Values + curInd)) < 0)
                            curInd = count;
                    }
                    else if (qspStrsComp(QSP_PSTR(curValue), QSP_PSTR(var->Values + curInd)) > 0)
                        curInd = count;
                }
                else
                {
                    if (isMin)
                    {
                        if (QSP_PNUM(curValue) < QSP_PNUM(var->Values + curInd))
                            curInd = count;
                    }
                    else if (QSP_PNUM(curValue) > QSP_PNUM(var->Values + curInd))
                        curInd = count;
                }
            }
            else
                curInd = count;
        }
    }
    if (curInd >= 0) qspCopyToNewVariant(&resultValue, var->Values + curInd);
    return resultValue;
}

int qspGetVarsCount()
{
    int i, count = 0;
    for (i = 0; i < QSP_VARSCOUNT; ++i)
        if (qspVars[i].Name.Str) ++count;
    return count;
}

void qspSetArgs(QSPVar *var, QSPVariant *args, int count)
{
    while (--count >= 0)
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
    int i, oldRefreshCount;
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
     * */
    oldRefreshCount = qspRefreshCount;
    switch (QSP_BASETYPE(v->Type))
    {
    case QSP_TYPE_TUPLE:
    {
        int lastVarIndex = varsCount - 1, lastValIndex = QSP_PTUPLE(v).Items - 1;
        int minLastIndex = lastVarIndex < lastValIndex ? lastVarIndex : lastValIndex;
        for (i = 0; i < minLastIndex; ++i)
        {
            qspSetVar(varNames[i], QSP_PTUPLE(v).Vals + i, op);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                return;
        }
        if (i == lastValIndex)
        {
            /* only 1 value left, fill the rest of vars with the last value */
            while (i < varsCount)
            {
                qspSetVar(varNames[i], QSP_PTUPLE(v).Vals + lastValIndex, op);
                if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                    return;
                ++i;
            }
        }
        else
        {
            /* only 1 variable left, fill it with the tuple containing all the values left */
            QSPVariant v2;
            v2.Type = QSP_TYPE_TUPLE;
            QSP_TUPLE(v2) = qspGetNewTuple(QSP_PTUPLE(v).Vals + i, QSP_PTUPLE(v).Items - i);
            qspSetVar(varNames[lastVarIndex], &v2, op);
            qspFreeVariants(&v2, 1);
        }
        break;
    }
    case QSP_TYPE_NUM:
    case QSP_TYPE_STR:
        for (i = 0; i < varsCount; ++i)
        {
            qspSetVar(varNames[i], v, op);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                return;
        }
        break;
    }
}

void qspStatementSetVarValue(QSPString s, QSPCachedStat *stat)
{
    QSPVariant v;
    QSPString *names;
    int namesCount, oldRefreshCount;
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
    oldRefreshCount = qspRefreshCount;
    v = qspExprValue(qspStringFromPair(s.Str + stat->Args[2].StartPos, s.Str + stat->Args[2].EndPos));
    if (qspRefreshCount != oldRefreshCount || qspErrorNum) return;
    namesCount = qspGetVarsNames(qspStringFromPair(s.Str + stat->Args[0].StartPos, s.Str + stat->Args[0].EndPos), &names);
    if (qspErrorNum) return;
    op = *(s.Str + stat->Args[1].StartPos);
    qspSetVarsValues(names, namesCount, &v, op);
    qspFreeVariants(&v, 1);
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
    QSPVariant v;
    int i, j, namesCount, groupInd, varsCount, bufSize;
    if (stat->ErrorCode)
    {
        qspSetError(stat->ErrorCode);
        return;
    }
    if (stat->ArgsCount > 1)
    {
        int oldRefreshCount = qspRefreshCount;
        if (*(s.Str + stat->Args[1].StartPos) != QSP_EQUAL[0])
        {
            qspSetError(QSP_ERR_SYNTAX);
            return;
        }
        /* We have to evaluate expression before allocation of local vars */
        v = qspExprValue(qspStringFromPair(s.Str + stat->Args[2].StartPos, s.Str + stat->Args[2].EndPos));
        if (qspRefreshCount != oldRefreshCount || qspErrorNum) return;
    }
    namesCount = qspGetVarsNames(qspStringFromPair(s.Str + stat->Args[0].StartPos, s.Str + stat->Args[0].EndPos), &names);
    if (qspErrorNum) return;
    groupInd = qspSavedVarGroupsCount - 1;
    varsCount = bufSize = qspSavedVarGroups[groupInd].VarsCount;
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
            if (!qspStrsComp(varName, qspSavedVarGroups[groupInd].Vars[j].Name))
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
                qspSavedVarGroups[groupInd].Vars = (QSPVar *)realloc(qspSavedVarGroups[groupInd].Vars, bufSize * sizeof(QSPVar));
            }
            qspMoveVar(qspSavedVarGroups[groupInd].Vars + varsCount, var);
            qspSavedVarGroups[groupInd].Vars[varsCount].Name = qspGetNewText(varName);
            qspSavedVarGroups[groupInd].VarsCount = ++varsCount;
        }
    }
    if (stat->ArgsCount > 1)
    {
        qspSetVarsValues(names, namesCount, &v, QSP_EQUAL[0]);
        qspFreeVariants(&v, 1);
    }
    free(names);
}

QSP_BOOL qspStatementCopyArr(QSPVariant *args, QSP_TINYINT count, QSPString *jumpTo, QSP_TINYINT extArg)
{
    QSPVar *dest, *src;
    if (!(dest = qspVarReference(QSP_STR(args[0]), QSP_TRUE))) return QSP_FALSE;
    if (!(src = qspVarReference(QSP_STR(args[1]), QSP_FALSE))) return QSP_FALSE;
    if (dest != src)
    {
        int start = (count >= 3 ? QSP_NUM(args[2]) : 0);
        int num = (count == 4 ? QSP_NUM(args[3]) : src->ValsCount);
        qspCopyVar(dest, src, start, num);
    }
    return QSP_FALSE;
}

QSP_BOOL qspStatementKillVar(QSPVariant *args, QSP_TINYINT count, QSPString *jumpTo, QSP_TINYINT extArg)
{
    if (count == 0)
        qspClearVars(QSP_FALSE);
    else
    {
        QSPVar *var;
        if (!(var = qspVarReference(QSP_STR(args[0]), QSP_FALSE))) return QSP_FALSE;
        if (count == 1)
            qspEmptyVar(var);
        else
        {
            int arrIndex = qspGetVarIndex(var, args[1], QSP_FALSE);
            qspRemoveArrayItem(var, arrIndex);
        }
    }
    return QSP_FALSE;
}
