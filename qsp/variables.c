/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
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

INLINE int qspIndStringCompare(const void *, const void *);
INLINE void qspRemoveArray(QSPString name);
INLINE void qspRemoveArrayItem(QSPString name, int index);
INLINE int qspGetVarTextIndex(QSPVar *, QSPString, QSP_BOOL);
INLINE QSPVar *qspGetVarData(QSPString s, QSP_BOOL isSet, int *index);
INLINE void qspSetVar(QSPString name, QSPVariant *val, QSP_CHAR op);
INLINE void qspCopyVar(QSPVar *, QSPVar *, int, int);
INLINE int qspGetVarsNames(QSPString names, QSPString **varNames);
INLINE void qspSetVarsValues(QSPString *varNames, int varsCount, QSPVariant *v, QSP_CHAR op);
INLINE QSPString qspGetVarNameOnly(QSPString s);

INLINE int qspIndStringCompare(const void *name, const void *compareTo)
{
    return qspStrsComp(*(QSPString *)name, ((QSPVarIndex *)compareTo)->Str);
}

QSPVar *qspVarReference(QSPString name, QSP_BOOL isCreate)
{
    int i;
    QSPVar *var;
    QSP_CHAR *pos;
    unsigned char bCode;
    if (qspIsEmpty(name))
    {
        qspSetError(QSP_ERR_NOTCORRECTNAME);
        return 0;
    }
    if (*name.Str == QSP_STRCHAR[0]) ++name.Str;
    if (qspIsEmpty(name) || qspIsDigit(*name.Str) || qspIsAnyInClass(name, QSP_CHAR_DELIM))
    {
        qspSetError(QSP_ERR_NOTCORRECTNAME);
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
            if (isCreate) var->Name = qspGetNewText(name);
            return var;
        }
        if (!qspStrsComp(var->Name, name)) return var;
        ++var;
    }
    qspSetError(QSP_ERR_TOOMANYVARS);
    return 0;
}

void qspClearVars(QSP_BOOL isFirst)
{
    int i;
    QSPVar *var = qspVars;
    for (i = 0; i < QSP_VARSCOUNT; ++i)
    {
        if (isFirst)
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

INLINE void qspRemoveArray(QSPString name)
{
    QSPVar *var;
    if (!(var = qspVarReferenceWithType(name, QSP_FALSE, 0))) return;
    qspEmptyVar(var);
}

INLINE void qspRemoveArrayItem(QSPString name, int index)
{
    QSPVar *var;
    QSP_BOOL isRemoving;
    QSPVarIndex *ind;
    int curIndex;
    if (!(var = qspVarReferenceWithType(name, QSP_FALSE, 0))) return;
    if (index < 0 || index >= var->ValsCount) return;
    curIndex = index;
    qspFreeString(var->Values[curIndex].Str);
    var->ValsCount--;
    while (curIndex < var->ValsCount)
    {
        var->Values[curIndex] = var->Values[curIndex + 1];
        ++curIndex;
    }
    isRemoving = QSP_FALSE;
    for (curIndex = 0; curIndex < var->IndsCount; ++curIndex)
    {
        ind = var->Indices + curIndex;
        if (ind->Index == index)
        {
            qspFreeString(ind->Str);
            var->IndsCount--;
            if (curIndex == var->IndsCount) break;
            isRemoving = QSP_TRUE;
        }
        if (isRemoving) *ind = var->Indices[curIndex + 1];
        if (ind->Index > index) ind->Index--;
    }
}

QSPVar *qspVarReferenceWithType(QSPString name, QSP_BOOL isCreate, QSP_BOOL *isString)
{
    QSPVar *var;
    QSPString uName = qspGetNewText(qspDelSpc(name));
    qspUpperStr(&uName);
    if (isString) *isString = (!qspIsEmpty(uName) && *uName.Str == QSP_STRCHAR[0]);
    var = qspVarReference(uName, isCreate);
    qspFreeString(uName);
    return var;
}

INLINE int qspGetVarTextIndex(QSPVar *var, QSPString str, QSP_BOOL isCreate)
{
    QSPVarIndex *ind;
    int i, n = var->IndsCount;
    QSPString uStr = qspGetNewText(str);
    qspUpperStr(&uStr);
    if (n > 0)
    {
        ind = (QSPVarIndex *)bsearch(&uStr, var->Indices, n, sizeof(QSPVarIndex), qspIndStringCompare);
        if (ind)
        {
            qspFreeString(uStr);
            return ind->Index;
        }
    }
    if (isCreate)
    {
        var->IndsCount++;
        if (n >= var->IndsBufSize)
        {
            var->IndsBufSize = n + 8;
            var->Indices = (QSPVarIndex *)realloc(var->Indices, var->IndsBufSize * sizeof(QSPVarIndex));
        }
        i = n - 1;
        while (i >= 0 && qspStrsComp(var->Indices[i].Str, uStr) > 0)
        {
            var->Indices[i + 1] = var->Indices[i];
            --i;
        }
        ++i;
        n = var->ValsCount;
        var->Indices[i].Str = uStr;
        var->Indices[i].Index = n;
        return n;
    }
    qspFreeString(uStr);
    return -1;
}

INLINE QSPVar *qspGetVarData(QSPString s, QSP_BOOL isSet, int *index)
{
    QSPVar *var;
    QSPVariant ind;
    int oldRefreshCount;
    QSP_CHAR *startPos, *rPos, *lPos = qspStrChar(s, QSP_LSBRACK[0]);
    if (lPos)
    {
        startPos = s.Str;
        s.Str = lPos;
        rPos = qspStrPos(s, QSP_STATIC_STR(QSP_RSBRACK), QSP_FALSE);
        if (!rPos)
        {
            qspSetError(QSP_ERR_BRACKNOTFOUND);
            return 0;
        }
        var = qspVarReference(qspStringFromPair(startPos, lPos), isSet);
        if (!var) return 0;
        s.Str = lPos + QSP_STATIC_LEN(QSP_LSBRACK);
        qspSkipSpaces(&s);
        if (s.Str == rPos)
        {
            if (isSet)
                *index = var->ValsCount;
            else
                *index = (var->ValsCount ? var->ValsCount - 1 : 0);
        }
        else
        {
            oldRefreshCount = qspRefreshCount;
            ind = qspExprValue(qspStringFromPair(s.Str, rPos));
            if (qspRefreshCount != oldRefreshCount || qspErrorNum) return 0;
            if (ind.IsStr)
            {
                *index = qspGetVarTextIndex(var, QSP_STR(ind), isSet);
                qspFreeString(QSP_STR(ind));
            }
            else
                *index = QSP_NUM(ind);
        }
        return var;
    }
    *index = 0;
    return qspVarReference(s, isSet);
}

void qspSetVarValueByReference(QSPVar *var, int ind, QSPVariant *val)
{
    int count, oldCount = var->ValsCount;
    if (ind >= oldCount)
    {
        count = var->ValsCount = ind + 1;
        var->Values = (QSPVarValue *)realloc(var->Values, count * sizeof(QSPVarValue));
        while (oldCount < count)
        {
            var->Values[oldCount].Num = 0;
            var->Values[oldCount].Str = qspNullString;
            ++oldCount;
        }
    }
    if (ind >= 0)
    {
        if (val->IsStr)
            qspUpdateText(&var->Values[ind].Str, QSP_PSTR(val));
        else
            var->Values[ind].Num = QSP_PNUM(val);
    }
}

INLINE void qspSetVar(QSPString name, QSPVariant *val, QSP_CHAR op)
{
    QSPVariant oldVal;
    QSPVar *var;
    int index;
    if (!(var = qspGetVarData(name, QSP_TRUE, &index))) return;
    if (op == QSP_EQUAL[0])
    {
        if (qspConvertVariantTo(val, *name.Str == QSP_STRCHAR[0]))
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            return;
        }
        qspSetVarValueByReference(var, index, val);
    }
    else if (op == QSP_ADD[0])
    {
        oldVal = qspGetVarValueByReference(var, index, *name.Str == QSP_STRCHAR[0]);
        if (oldVal.IsStr && val->IsStr)
            qspAddText(&QSP_STR(oldVal), QSP_PSTR(val), QSP_FALSE);
        else if (qspIsCanConvertToNum(&oldVal) && qspIsCanConvertToNum(val))
        {
            qspConvertVariantTo(&oldVal, QSP_FALSE);
            qspConvertVariantTo(val, QSP_FALSE);
            QSP_NUM(oldVal) += QSP_PNUM(val);
            qspConvertVariantTo(&oldVal, *name.Str == QSP_STRCHAR[0]);
        }
        else
        {
            if (!oldVal.IsStr)
            {
                qspSetError(QSP_ERR_TYPEMISMATCH);
                return;
            }
            qspConvertVariantTo(val, QSP_TRUE);
            qspAddText(&QSP_STR(oldVal), QSP_PSTR(val), QSP_FALSE);
        }
        qspSetVarValueByReference(var, index, &oldVal);
        if (oldVal.IsStr) qspFreeString(QSP_STR(oldVal));
    }
    else if (qspIsInClass(op, QSP_CHAR_SIMPLEOP))
    {
        if (qspConvertVariantTo(val, QSP_FALSE))
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            return;
        }
        oldVal = qspGetVarValueByReference(var, index, *name.Str == QSP_STRCHAR[0]);
        if (qspConvertVariantTo(&oldVal, QSP_FALSE))
        {
            qspSetError(QSP_ERR_TYPEMISMATCH);
            qspFreeString(QSP_STR(oldVal));
            return;
        }
        if (op == QSP_SUB[0])
            QSP_NUM(oldVal) -= QSP_PNUM(val);
        else if (op == QSP_DIV[0])
        {
            if (!QSP_PNUM(val))
            {
                qspSetError(QSP_ERR_DIVBYZERO);
                return;
            }
            QSP_NUM(oldVal) /= QSP_PNUM(val);
        }
        else
            QSP_NUM(oldVal) *= QSP_PNUM(val);
        qspConvertVariantTo(&oldVal, *name.Str == QSP_STRCHAR[0]);
        qspSetVarValueByReference(var, index, &oldVal);
        if (oldVal.IsStr) qspFreeString(QSP_STR(oldVal));
    }
}

QSPVariant qspGetVarValueByReference(QSPVar *var, int ind, QSP_BOOL isStringType)
{
    QSPVariant ret;
    QSPString text;
    if (ind >= 0 && ind < var->ValsCount)
    {
        if (ret.IsStr = isStringType)
        {
            text = var->Values[ind].Str;
            QSP_STR(ret) = (text.Str ? qspGetNewText(text) : qspNewEmptyString());
        }
        else
            QSP_NUM(ret) = var->Values[ind].Num;
        return ret;
    }
    return qspGetEmptyVariant(isStringType);
}

QSPString qspGetVarStrValue(QSPString name)
{
    QSPString text;
    QSPVar *var;
    if (var = qspVarReference(name, QSP_FALSE))
    {
        if (var->ValsCount)
        {
            text = var->Values->Str;
            if (text.Str) return text;
        }
    }
    else
        qspResetError();
    return qspEmptyString;
}

int qspGetVarNumValue(QSPString name)
{
    QSPVar *var;
    if (var = qspVarReference(name, QSP_FALSE))
    {
        if (var->ValsCount) return var->Values->Num;
    }
    else
        qspResetError();
    return 0;
}

QSPVariant qspGetVar(QSPString name)
{
    QSPVar *var;
    int index;
    if (!(var = qspGetVarData(name, QSP_FALSE, &index)))
        return qspGetEmptyVariant(QSP_FALSE);
    return qspGetVarValueByReference(var, index, *name.Str == QSP_STRCHAR[0]);
}

void qspRestoreGlobalVars()
{
    int i, j;
    QSPVar *var;
    if (qspSavedVarGroupsCount)
    {
        for (i = qspSavedVarGroupsCount - 1; i >= 0; --i)
        {
            for (j = qspSavedVarGroups[i].VarsCount - 1; j >= 0; --j)
            {
                if (!(var = qspVarReference(qspSavedVarGroups[i].Vars[j].Name, QSP_TRUE))) return;
                qspEmptyVar(var);
                qspMoveVar(var, &qspSavedVarGroups[i].Vars[j]);
            }
        }
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
        ind = 0;
        for (i = groupsCount - 1; i >= 0; --i)
        {
            for (j = savedGroups[i].VarsCount - 1; j >= 0; --j)
            {
                if (!(var = qspVarReference(savedGroups[i].Vars[j].Name, QSP_TRUE)))
                {
                    while (ind < varsCount)
                    {
                        qspEmptyVar(savedVars + ind);
                        ++ind;
                    }
                    free(savedVars);
                    return;
                }
                qspMoveVar(&savedGroups[i].Vars[j], var);
                qspMoveVar(var, savedVars + ind);
                ++ind;
            }
        }
        free(savedVars);
    }
}

void qspClearLocalVars(QSPVar *savedVars, int varsCount)
{
    int i;
    if (savedVars)
    {
        for (i = 0; i < varsCount; ++i)
            qspEmptyVar(savedVars + i);
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
    int i;
    if (vars)
    {
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
    QSPString str;
    int i, maxCount, newInd;
    if (start < 0) start = 0;
    maxCount = src->ValsCount - start;
    if (count <= 0 || maxCount <= 0)
    {
        qspInitVarData(dest);
        return;
    }
    if (count < maxCount) maxCount = count;
    dest->ValsCount = maxCount;
    dest->Values = (QSPVarValue *)malloc(maxCount * sizeof(QSPVarValue));
    for (i = 0; i < maxCount; ++i)
    {
        dest->Values[i].Num = src->Values[i + start].Num;
        str = src->Values[i + start].Str;
        dest->Values[i].Str = (str.Str ? qspGetNewText(str) : qspNullString);
    }
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
    if (!(var = qspVarReferenceWithType(name, QSP_FALSE, 0))) return 0;
    return var->ValsCount;
}

int qspArrayPos(QSPString varName, QSPVariant *val, int ind, QSP_BOOL isRegExp)
{
    int num, count;
    QSPVar *var;
    QSPString str, emptyStr;
    regex_t *regExp;
    QSP_BOOL isString;
    if (!(var = qspVarReferenceWithType(varName, QSP_FALSE, &isString))) return -1;
    if (qspConvertVariantTo(val, isRegExp || isString))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        return -1;
    }
    if (isRegExp)
    {
        regExp = qspRegExpGetCompiled(QSP_PSTR(val));
        if (!regExp) return -1;
    }
    count = var->ValsCount;
    if (ind < 0)
        ind = 0;
    else if (ind > count)
        ind = count;
    emptyStr = qspEmptyString;
    while (ind <= count)
    {
        if (val->IsStr)
        {
            if (ind >= count)
                str = emptyStr;
            else
            {
                str = var->Values[ind].Str;
                if (!str.Str) str = emptyStr;
            }
            if (isRegExp)
            {
                if (qspRegExpStrMatch(regExp, str)) return ind;
            }
            else if (!qspStrsComp(str, QSP_PSTR(val)))
                return ind;
        }
        else
        {
            num = (ind < count ? var->Values[ind].Num : 0);
            if (num == QSP_PNUM(val)) return ind;
        }
        ++ind;
    }
    return -1;
}

QSPVariant qspArrayMinMaxItem(QSPString name, QSP_BOOL isMin)
{
    QSPVar *var;
    QSPString str;
    QSP_BOOL isString;
    int curInd, count;
    QSPVariant res;
    if (!(var = qspVarReferenceWithType(name, QSP_FALSE, &isString)))
        return qspGetEmptyVariant(QSP_FALSE);
    curInd = -1;
    count = var->ValsCount;
    while (--count >= 0)
    {
        if (isString)
        {
            str = var->Values[count].Str;
            if (!qspIsEmpty(str))
            {
                if (curInd >= 0)
                {
                    if (isMin)
                    {
                        if (qspStrsComp(str, var->Values[curInd].Str) < 0)
                            curInd = count;
                    }
                    else if (qspStrsComp(str, var->Values[curInd].Str) > 0)
                        curInd = count;
                }
                else
                    curInd = count;
            }
        }
        else if (curInd >= 0)
        {
            if (isMin)
            {
                if (var->Values[count].Num < var->Values[curInd].Num)
                    curInd = count;
            }
            else if (var->Values[count].Num > var->Values[curInd].Num)
                curInd = count;
        }
        else
            curInd = count;
    }
    if (curInd < 0) return qspGetEmptyVariant(isString);
    if (res.IsStr = isString)
        QSP_STR(res) = qspGetNewText(var->Values[curInd].Str);
    else
        QSP_NUM(res) = var->Values[curInd].Num;
    return res;
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
    QSPString text;
    if (varRes->ValsCount)
    {
        text = varRes->Values[0].Str;
        if (text.Str)
        {
            res->IsStr = QSP_TRUE;
            QSP_PSTR(res) = qspGetNewText(text);
        }
        else
        {
            res->IsStr = QSP_FALSE;
            QSP_PNUM(res) = varRes->Values[0].Num;
        }
    }
    else
    {
        res->IsStr = QSP_TRUE;
        QSP_PSTR(res) = qspNewEmptyString();
    }
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
            bufSize = count + 2;
            foundNames = (QSPString *)realloc(foundNames, bufSize * sizeof(QSPString));
        }
        pos = qspStrPos(names, QSP_STATIC_STR(QSP_COMMA), QSP_FALSE);
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
    int i, lastVarIndex;
    QSPVariant v2;
    QSPString strVal;
    QSP_CHAR *newValPos;
    int oldRefreshCount;
    if (varsCount == 1)
    {
        qspSetVar(varNames[0], v, op);
        return;
    }
    oldRefreshCount = qspRefreshCount;
    if (v->IsStr)
    {
        strVal = QSP_PSTR(v);
        newValPos = qspStrStr(strVal, QSP_STATIC_STR(QSP_VALSDELIM));
        lastVarIndex = varsCount - 1;
        for (i = 0; i < lastVarIndex && newValPos; ++i)
        {
            v2.IsStr = QSP_TRUE;
            QSP_STR(v2) = qspGetNewText(qspStringFromPair(strVal.Str, newValPos));
            qspSetVar(varNames[i], &v2, op);
            if (v2.IsStr) qspFreeString(QSP_STR(v2));
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                return;
            strVal.Str = newValPos + QSP_STATIC_LEN(QSP_VALSDELIM);
            newValPos = qspStrStr(strVal, QSP_STATIC_STR(QSP_VALSDELIM));
        }
        /* fill the rest with the last value */
        v2.IsStr = QSP_TRUE;
        QSP_STR(v2) = qspGetNewText(strVal);
        while (i < varsCount)
        {
            qspSetVar(varNames[i], &v2, op);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                break;
            ++i;
        }
        if (v2.IsStr) qspFreeString(QSP_STR(v2));
    }
    else
    {
        for (i = 0; i < varsCount; ++i)
        {
            qspSetVar(varNames[i], v, op);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
                break;
        }
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
    if (v.IsStr) qspFreeString(QSP_STR(v));
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
    QSPVariant v;
    QSPVar *var;
    QSP_BOOL isVarFound;
    QSPString *names, varName;
    int i, namesCount, groupInd, varsCount, bufSize, oldRefreshCount;
    if (stat->ErrorCode)
    {
        qspSetError(stat->ErrorCode);
        return;
    }
    if (stat->ArgsCount > 1 && *(s.Str + stat->Args[1].StartPos) != QSP_EQUAL[0])
    {
        qspSetError(QSP_ERR_SYNTAX);
        return;
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
        /* Skip type char */
        if (*varName.Str == QSP_STRCHAR[0]) ++varName.Str;
        varName = qspGetVarNameOnly(varName);
        /* Check for the existence */
        for (i = 0; i < varsCount; ++i)
        {
            if (!qspStrsComp(varName, qspSavedVarGroups[groupInd].Vars[i].Name))
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
        oldRefreshCount = qspRefreshCount;
        v = qspExprValue(qspStringFromPair(s.Str + stat->Args[2].StartPos, s.Str + stat->Args[2].EndPos));
        if (qspRefreshCount != oldRefreshCount || qspErrorNum)
        {
            free(names);
            return;
        }
        qspSetVarsValues(names, namesCount, &v, QSP_EQUAL[0]);
        if (v.IsStr) qspFreeString(QSP_STR(v));
    }
    free(names);
}

QSP_BOOL qspStatementCopyArr(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    int start, num;
    QSPVar *dest, *src;
    if (!(dest = qspVarReferenceWithType(QSP_STR(args[0]), QSP_TRUE, 0))) return QSP_FALSE;
    if (!(src = qspVarReferenceWithType(QSP_STR(args[1]), QSP_FALSE, 0))) return QSP_FALSE;
    if (dest != src)
    {
        start = (count >= 3 ? QSP_NUM(args[2]) : 0);
        num = (count == 4 ? QSP_NUM(args[3]) : src->ValsCount);
        qspEmptyVar(dest);
        qspCopyVar(dest, src, start, num);
    }
    return QSP_FALSE;
}

QSP_BOOL qspStatementKillVar(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    if (count == 1)
        qspRemoveArray(QSP_STR(args[0]));
    else if (count == 2)
        qspRemoveArrayItem(QSP_STR(args[0]), QSP_NUM(args[1]));
    else
        qspClearVars(QSP_FALSE);
    return QSP_FALSE;
}
