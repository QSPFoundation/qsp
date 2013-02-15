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
QSPVarsGroup *qspSavedVarsGroups = 0;
int qspSavedVarsGroupsCount = 0;

unsigned char qspRand8[256] =
{
	0x0C, 0x6B, 0x35, 0x24, 0x85, 0x05, 0x2B, 0xAC, 0x32, 0x26, 0xF3, 0x4C, 0x8F, 0xF4, 0x8E, 0xBC,
	0xEC, 0x69, 0x4D, 0x95, 0x77, 0x68, 0xC3, 0xDB, 0xC2, 0x71, 0x1F, 0xD1, 0x14, 0xAA, 0x0A, 0x09,
	0x0D, 0x06, 0xD3, 0x51, 0xE9, 0x31, 0x36, 0x9E, 0x9D, 0x80, 0x25, 0xEF, 0xE2, 0x55, 0xEE, 0x90,
	0x5A, 0xB4, 0xE7, 0x29, 0x04, 0xC1, 0x67, 0x00, 0xD4, 0xD2, 0x75, 0xD0, 0xF8, 0x74, 0x84, 0x46,
	0xC8, 0x44, 0xE6, 0x63, 0x3D, 0xD8, 0x9C, 0xDA, 0x07, 0xB5, 0x39, 0x6A, 0xA7, 0xDE, 0x50, 0xF9,
	0x66, 0xA8, 0xBD, 0xC9, 0x19, 0xCE, 0x7D, 0xEB, 0xE4, 0xCD, 0xFD, 0xA5, 0x21, 0x83, 0xA3, 0xD9,
	0x97, 0x10, 0xBF, 0x8B, 0xD5, 0x81, 0x41, 0x1E, 0x6E, 0x11, 0x4E, 0xAE, 0x57, 0x92, 0xC4, 0xA1,
	0x3F, 0x7C, 0x4A, 0x18, 0x23, 0x6D, 0x3B, 0x96, 0xAF, 0xE0, 0x4F, 0xF5, 0x7E, 0x22, 0xB7, 0x30,
	0x59, 0x15, 0x47, 0xDC, 0xE1, 0x65, 0xA6, 0x20, 0x1B, 0x42, 0xCC, 0x1D, 0x94, 0xCF, 0xCA, 0x53,
	0x9A, 0x28, 0x87, 0x3C, 0x8C, 0x78, 0x2D, 0x93, 0x8D, 0x38, 0x03, 0xA2, 0xDD, 0x49, 0x62, 0xF0,
	0xDF, 0xA0, 0xF2, 0x48, 0x72, 0x6F, 0x7F, 0xC6, 0x73, 0x1A, 0x76, 0xAD, 0x0B, 0xFE, 0x82, 0x6C,
	0xBA, 0x0F, 0x3A, 0x60, 0x12, 0x7B, 0x33, 0xBE, 0x9F, 0x5D, 0x01, 0x64, 0xB6, 0x17, 0xD7, 0x98,
	0x02, 0xB9, 0x4B, 0xFF, 0xAB, 0xB0, 0x5B, 0xB3, 0x16, 0xF7, 0xCB, 0xFC, 0xC5, 0x0E, 0x52, 0x5C,
	0xE8, 0x2A, 0x86, 0x61, 0xC7, 0x2E, 0xE5, 0xA4, 0xFA, 0x79, 0x27, 0xFB, 0xC0, 0x7A, 0x8A, 0x37,
	0xB2, 0xED, 0xA9, 0x5F, 0xBB, 0x3E, 0x45, 0x2F, 0x54, 0x58, 0x2C, 0x70, 0x40, 0xE3, 0x56, 0xB8,
	0xEA, 0x91, 0x34, 0xF6, 0x88, 0x43, 0x99, 0xD6, 0x89, 0x9B, 0x08, 0xF1, 0x5E, 0x1C, 0xB1, 0x13
};

static int qspIndStringCompare(const void *, const void *);
static void qspRemoveArray(QSP_CHAR *);
static void qspRemoveArrayItem(QSP_CHAR *, int);
static void qspInitVarData(QSPVar *);
static int qspGetVarTextIndex(QSPVar *, QSP_CHAR *, QSP_BOOL);
static QSPVar *qspGetVarData(QSP_CHAR *, QSP_BOOL, int *);
static void qspSetVar(QSP_CHAR *, QSPVariant *, QSP_CHAR);
static void qspCopyVar(QSPVar *, QSPVar *, int, int);
static void qspSetVarValue(QSP_CHAR *, QSPVariant *, QSP_CHAR);
static QSP_CHAR *qspGetVarNameOnly(QSP_CHAR *);

static int qspIndStringCompare(const void *name, const void *compareTo)
{
	return qspStrsComp((QSP_CHAR *)name, ((QSPVarIndex *)compareTo)->Str);
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
			if (var->Name) free(var->Name);
			qspEmptyVar(var);
		}
		var->Name = 0;
		++var;
	}
}

void qspEmptyVar(QSPVar *var)
{
	int count;
	if (var->Values)
	{
		count = var->ValsCount;
		while (--count >= 0)
			if (var->Values[count].Str) free(var->Values[count].Str);
		free(var->Values);
	}
	if (var->Indices)
	{
		count = var->IndsCount;
		while (--count >= 0)
			free(var->Indices[count].Str);
		free(var->Indices);
	}
	qspInitVarData(var);
}

static void qspRemoveArray(QSP_CHAR *name)
{
	QSPVar *var;
	if (!(var = qspVarReferenceWithType(name, QSP_FALSE, 0))) return;
	qspEmptyVar(var);
}

static void qspRemoveArrayItem(QSP_CHAR *name, int index)
{
	QSPVar *var;
	QSP_BOOL isRemoving;
	QSPVarIndex *ind;
	int origIndex;
	if (!(var = qspVarReferenceWithType(name, QSP_FALSE, 0))) return;
	if (index < 0 || index >= var->ValsCount) return;
	origIndex = index;
	if (var->Values[index].Str) free(var->Values[index].Str);
	var->ValsCount--;
	while (index < var->ValsCount)
	{
		var->Values[index] = var->Values[index + 1];
		++index;
	}
	isRemoving = QSP_FALSE;
	for (index = 0; index < var->IndsCount; ++index)
	{
		ind = var->Indices + index;
		if (ind->Index == origIndex)
		{
			free(ind->Str);
			var->IndsCount--;
			if (index == var->IndsCount) break;
			isRemoving = QSP_TRUE;
		}
		if (isRemoving) *ind = var->Indices[index + 1];
		if (ind->Index > origIndex) ind->Index--;
	}
}

static void qspInitVarData(QSPVar *var)
{
	var->Values = 0;
	var->ValsCount = 0;
	var->Indices = 0;
	var->IndsCount = 0;
	var->IndsBufSize = 0;
}

QSPVar *qspVarReference(QSP_CHAR *name, QSP_BOOL isCreate)
{
	int i;
	QSPVar *var;
	QSP_CHAR *uName;
	unsigned char bCode;
	if (*name == QSP_STRCHAR[0]) ++name;
	if (!(*name) || qspIsDigit(*name) || qspStrPBrk(name, QSP_DELIMS))
	{
		qspSetError(QSP_ERR_NOTCORRECTNAME);
		return 0;
	}
	qspUpperStr(uName = qspGetNewText(name, -1));
	bCode = 0;
	for (i = 0; uName[i]; ++i)
		bCode = qspRand8[bCode ^ (unsigned char)uName[i]];
	var = qspVars + QSP_VARSSEEK * bCode;
	for (i = 0; i < QSP_VARSSEEK; ++i)
	{
		if (!var->Name)
		{
			if (isCreate)
				var->Name = uName;
			else
				free(uName);
			return var;
		}
		if (!qspStrsComp(var->Name, uName))
		{
			free(uName);
			return var;
		}
		++var;
	}
	free(uName);
	qspSetError(QSP_ERR_TOOMANYVARS);
	return 0;
}

QSPVar *qspVarReferenceWithType(QSP_CHAR *name, QSP_BOOL isCreate, QSP_BOOL *isString)
{
	QSPVar *var;
	QSP_CHAR *varName = qspDelSpcCanRetSelf(name);
	if (isString) *isString = (*varName == QSP_STRCHAR[0]);
	var = qspVarReference(varName, isCreate);
	if (varName != name) free(varName);
	return var;
}

static int qspGetVarTextIndex(QSPVar *var, QSP_CHAR *str, QSP_BOOL isCreate)
{
	QSP_CHAR *uStr;
	QSPVarIndex *ind;
	int i, n = var->IndsCount;
	qspUpperStr(uStr = qspGetNewText(str, -1));
	if (n > 0)
	{
		ind = (QSPVarIndex *)bsearch(uStr, var->Indices, n, sizeof(QSPVarIndex), qspIndStringCompare);
		if (ind)
		{
			free(uStr);
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
	free(uStr);
	return -1;
}

static QSPVar *qspGetVarData(QSP_CHAR *s, QSP_BOOL isSet, int *index)
{
	QSPVar *var;
	QSPVariant ind;
	int oldRefreshCount;
	QSP_CHAR *temp, *rPos, *lPos = qspStrChar(s, QSP_LSBRACK[0]);
	if (lPos)
	{
		rPos = qspStrPos(lPos, QSP_RSBRACK, QSP_FALSE);
		if (!rPos)
		{
			qspSetError(QSP_ERR_BRACKNOTFOUND);
			return 0;
		}
		*lPos = 0;
		var = qspVarReference(s, isSet);
		*lPos = QSP_LSBRACK[0];
		if (!var) return 0;
		temp = qspSkipSpaces(lPos + 1);
		if (temp == rPos)
		{
			if (isSet)
				*index = var->ValsCount;
			else
				*index = (var->ValsCount ? var->ValsCount - 1 : 0);
		}
		else
		{
			oldRefreshCount = qspRefreshCount;
			*rPos = 0;
			ind = qspExprValue(temp);
			*rPos = QSP_RSBRACK[0];
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) return 0;
			if (ind.IsStr)
			{
				*index = qspGetVarTextIndex(var, QSP_STR(ind), isSet);
				free(QSP_STR(ind));
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
			var->Values[oldCount].Str = 0;
			++oldCount;
		}
	}
	if (ind >= 0)
	{
		if (val->IsStr)
			var->Values[ind].Str = qspGetAddText(var->Values[ind].Str, QSP_PSTR(val), 0, -1);
		else
			var->Values[ind].Num = QSP_PNUM(val);
	}
}

static void qspSetVar(QSP_CHAR *name, QSPVariant *val, QSP_CHAR op)
{
	QSPVariant oldVal;
	QSPVar *var;
	int index;
	if (!(var = qspGetVarData(name, QSP_TRUE, &index))) return;
	if (op == QSP_EQUAL[0])
	{
		if (qspConvertVariantTo(val, *name == QSP_STRCHAR[0]))
		{
			qspSetError(QSP_ERR_TYPEMISMATCH);
			return;
		}
		qspSetVarValueByReference(var, index, val);
	}
	else if (op == QSP_ADD[0])
	{
		oldVal = qspGetVarValueByReference(var, index, *name == QSP_STRCHAR[0]);
		if (oldVal.IsStr && val->IsStr)
			QSP_STR(oldVal) = qspGetAddText(QSP_STR(oldVal), QSP_PSTR(val), -1, -1);
		else if (qspIsCanConvertToNum(&oldVal) && qspIsCanConvertToNum(val))
		{
			qspConvertVariantTo(&oldVal, QSP_FALSE);
			qspConvertVariantTo(val, QSP_FALSE);
			QSP_NUM(oldVal) += QSP_PNUM(val);
			qspConvertVariantTo(&oldVal, *name == QSP_STRCHAR[0]);
		}
		else
		{
			if (!oldVal.IsStr)
			{
				qspSetError(QSP_ERR_TYPEMISMATCH);
				return;
			}
			qspConvertVariantTo(val, QSP_TRUE);
			QSP_STR(oldVal) = qspGetAddText(QSP_STR(oldVal), QSP_PSTR(val), -1, -1);
		}
		qspSetVarValueByReference(var, index, &oldVal);
		if (oldVal.IsStr) free(QSP_STR(oldVal));
	}
	else if (qspIsInList(QSP_SUB QSP_DIV QSP_MUL, op))
	{
		if (qspConvertVariantTo(val, QSP_FALSE))
		{
			qspSetError(QSP_ERR_TYPEMISMATCH);
			return;
		}
		oldVal = qspGetVarValueByReference(var, index, *name == QSP_STRCHAR[0]);
		if (qspConvertVariantTo(&oldVal, QSP_FALSE))
		{
			qspSetError(QSP_ERR_TYPEMISMATCH);
			free(QSP_STR(oldVal));
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
		qspConvertVariantTo(&oldVal, *name == QSP_STRCHAR[0]);
		qspSetVarValueByReference(var, index, &oldVal);
		if (oldVal.IsStr) free(QSP_STR(oldVal));
	}
}

QSPVariant qspGetVarValueByReference(QSPVar *var, int ind, QSP_BOOL isStringType)
{
	QSPVariant ret;
	QSP_CHAR *text;
	if (ind >= 0 && ind < var->ValsCount)
	{
		if (ret.IsStr = isStringType)
		{
			text = var->Values[ind].Str;
			QSP_STR(ret) = (text ? qspGetNewText(text, -1) : qspGetNewText(QSP_FMT(""), 0));
		}
		else
			QSP_NUM(ret) = var->Values[ind].Num;
		return ret;
	}
	return qspGetEmptyVariant(isStringType);
}

QSP_CHAR *qspGetVarStrValue(QSP_CHAR *name)
{
	QSP_CHAR *text;
	QSPVar *var;
	if (var = qspVarReference(name, QSP_FALSE))
	{
		if (var->ValsCount)
		{
			text = var->Values->Str;
			if (text) return text;
		}
	}
	else
		qspResetError();
	return QSP_FMT("");
}

int qspGetVarNumValue(QSP_CHAR *name)
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

QSPVariant qspGetVar(QSP_CHAR *name)
{
	QSPVar *var;
	int index;
	if (!(var = qspGetVarData(name, QSP_FALSE, &index))) return qspGetEmptyVariant(QSP_FALSE);
	return qspGetVarValueByReference(var, index, *name == QSP_STRCHAR[0]);
}

void qspPrepareGlobalVars()
{
	int i, j;
	QSPVar *var;
	for (i = qspSavedVarsGroupsCount - 1; i >= 0; --i)
	{
		for (j = qspSavedVarsGroups[i].VarsCount - 1; j >= 0; --j)
		{
			if (!(var = qspVarReference(qspSavedVarsGroups[i].Vars[j].Name, QSP_TRUE))) return;
			qspEmptyVar(var);
			qspMoveVar(var, &qspSavedVarsGroups[i].Vars[j]);
		}
	}
}

int qspPrepareLocalVars(QSPVar **vars)
{
	QSPVar *var, *savedVars;
	int i, j, ind, varsCount = 0;
	for (i = qspSavedVarsGroupsCount - 1; i >= 0; --i)
		varsCount += qspSavedVarsGroups[i].VarsCount;
	if (!varsCount)
	{
		*vars = 0;
		return 0;
	}
	savedVars = (QSPVar *)malloc(varsCount * sizeof(QSPVar));
	ind = 0;
	for (i = qspSavedVarsGroupsCount - 1; i >= 0; --i)
	{
		for (j = qspSavedVarsGroups[i].VarsCount - 1; j >= 0; --j)
		{
			if (!(var = qspVarReference(qspSavedVarsGroups[i].Vars[j].Name, QSP_TRUE)))
			{
				while (--ind >= 0)
					qspEmptyVar(savedVars + ind);
				free(savedVars);
				return 0;
			}
			qspMoveVar(savedVars + ind, var);
			qspMoveVar(var, &qspSavedVarsGroups[i].Vars[j]);
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
					free(vars[i].Name);
					qspEmptyVar(vars + i);
					++i;
				}
				free(vars);
				return;
			}
			free(vars[i].Name);
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
			free(vars[i].Name);
			qspEmptyVar(vars + i);
		}
		free(vars);
	}
}

static void qspCopyVar(QSPVar *dest, QSPVar *src, int start, int count)
{
	QSP_CHAR *str;
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
		dest->Values[i].Str = (str ? qspGetNewText(str, -1) : 0);
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
			dest->Indices[count].Str = qspGetNewText(src->Indices[i].Str, -1);
			++count;
		}
	}
	dest->IndsCount = count;
}

int qspArraySize(QSP_CHAR *name)
{
	QSPVar *var;
	if (!(var = qspVarReferenceWithType(name, QSP_FALSE, 0))) return 0;
	return var->ValsCount;
}

int qspArrayPos(QSP_CHAR *varName, QSPVariant *val, int ind, QSP_BOOL isRegExp)
{
	int num, count;
	QSPVar *var;
	QSP_CHAR *str;
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
	while (ind <= count)
	{
		if (val->IsStr)
		{
			if (!(ind < count && (str = var->Values[ind].Str))) str = QSP_FMT("");
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

QSPVariant qspArrayMinMaxItem(QSP_CHAR *name, QSP_BOOL isMin)
{
	QSPVar *var;
	QSP_CHAR *str;
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
			if (str && *str)
			{
				if (curInd >= 0)
				{
					if (isMin)
					{
						if (QSP_STRCOLL(str, var->Values[curInd].Str) < 0)
							curInd = count;
					}
					else if (QSP_STRCOLL(str, var->Values[curInd].Str) > 0)
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
		QSP_STR(res) = qspGetNewText(var->Values[curInd].Str, -1);
	else
		QSP_NUM(res) = var->Values[curInd].Num;
	return res;
}

int qspGetVarsCount()
{
	int i, count = 0;
	for (i = 0; i < QSP_VARSCOUNT; ++i)
		if (qspVars[i].Name) ++count;
	return count;
}

void qspSetArgs(QSPVar *var, QSPVariant *args, int count)
{
	while (--count >= 0)
		qspSetVarValueByReference(var, count, args + count);
}

void qspApplyResult(QSPVar *varRes, QSPVariant *res)
{
	QSP_CHAR *text;
	if (varRes->ValsCount)
	{
		if (text = varRes->Values[0].Str)
		{
			res->IsStr = QSP_TRUE;
			QSP_PSTR(res) = qspGetNewText(text, -1);
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
		QSP_PSTR(res) = qspGetNewText(QSP_FMT(""), 0);
	}
}

void qspMoveVar(QSPVar *dest, QSPVar *src)
{
	dest->Values = src->Values;
	dest->ValsCount = src->ValsCount;
	dest->Indices = src->Indices;
	dest->IndsCount = src->IndsCount;
	dest->IndsBufSize = src->IndsBufSize;
	qspInitVarData(src);
}

static void qspSetVarValue(QSP_CHAR *name, QSPVariant *v, QSP_CHAR op)
{
	QSPVariant v2;
	QSP_BOOL isSingleValue, notFirstValue = QSP_FALSE;
	QSP_CHAR *newValPos, *newCommaPos, *valPos, *commaPos = name;
	int oldRefreshCount = qspRefreshCount;
	if (v->IsStr)
	{
		valPos = QSP_PSTR(v);
		isSingleValue = QSP_FALSE; /* Multiple values by default */
	}
	else
		isSingleValue = QSP_TRUE;
	while (1)
	{
		newCommaPos = qspStrPos(commaPos, QSP_COMMA, QSP_FALSE);
		if (newCommaPos)
		{
			*newCommaPos = 0;
			if (isSingleValue)
			{
				if (notFirstValue)
					qspSetVar(commaPos, &v2, op);
				else
					qspSetVar(commaPos, v, op);
				if (qspRefreshCount != oldRefreshCount || qspErrorNum)
				{
					*newCommaPos = QSP_COMMA[0];
					break;
				}
			}
			else
			{
				newValPos = qspStrStr(valPos, QSP_VALSDELIM);
				if (newValPos)
				{
					notFirstValue = QSP_TRUE;
					v2.IsStr = QSP_TRUE;
					QSP_STR(v2) = qspGetNewText(valPos, (int)(newValPos - valPos));
					qspSetVar(commaPos, &v2, op);
					if (qspRefreshCount != oldRefreshCount || qspErrorNum)
					{
						*newCommaPos = QSP_COMMA[0];
						break;
					}
					if (v2.IsStr) free(QSP_STR(v2));
					valPos = newValPos + QSP_LEN(QSP_VALSDELIM);
				}
				else /* The last value */
				{
					isSingleValue = QSP_TRUE;
					if (notFirstValue)
					{
						v2.IsStr = QSP_TRUE;
						QSP_STR(v2) = qspGetNewText(valPos, -1);
						qspSetVar(commaPos, &v2, op);
					}
					else
						qspSetVar(commaPos, v, op);
					if (qspRefreshCount != oldRefreshCount || qspErrorNum)
					{
						*newCommaPos = QSP_COMMA[0];
						break;
					}
				}
			}
			*newCommaPos = QSP_COMMA[0];
		}
		else /* The last variable */
		{
			if (notFirstValue) /* Not a first value */
			{
				if (!isSingleValue)
				{
					v2.IsStr = QSP_TRUE;
					QSP_STR(v2) = qspGetNewText(valPos, -1);
				}
				qspSetVar(commaPos, &v2, op);
			}
			else
				qspSetVar(commaPos, v, op);
			break;
		}
		commaPos = newCommaPos + QSP_LEN(QSP_COMMA);
	}
	if (notFirstValue && v2.IsStr) free(QSP_STR(v2));
}

void qspStatementSetVarValue(QSP_CHAR *s)
{
	QSPVariant v;
	int oldRefreshCount;
	QSP_CHAR ch, *name, *pos = qspStrPos(s, QSP_EQUAL, QSP_FALSE);
	if (!pos)
	{
		qspSetError(QSP_ERR_EQNOTFOUND);
		return;
	}
	oldRefreshCount = qspRefreshCount;
	v = qspExprValue(pos + QSP_LEN(QSP_EQUAL));
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return;
	if (pos != s && qspIsInList(QSP_ADD QSP_SUB QSP_DIV QSP_MUL, *(pos - 1))) --pos;
	ch = *pos;
	*pos = 0;
	name = qspDelSpcCanRetSelf(s);
	qspSetVarValue(name, &v, ch);
	*pos = ch;
	if (name != s) free(name);
	if (v.IsStr) free(QSP_STR(v));
}

static QSP_CHAR *qspGetVarNameOnly(QSP_CHAR *s)
{
	QSP_CHAR *brackPos = qspStrChar(s, QSP_LSBRACK[0]);
	if (brackPos) return qspGetNewText(s, (int)(brackPos - s));
	return qspDelSpc(s);
}

void qspStatementLocal(QSP_CHAR *s)
{
	QSPVariant v;
	QSPVar *var;
	QSP_BOOL isVarFound;
	QSP_CHAR *curPos, *varName, *commaPos, *eqPos;
	int i, groupInd, count, bufSize, oldRefreshCount;
	s = qspSkipSpaces(s);
	if (!(*s))
	{
		qspSetError(QSP_ERR_SYNTAX);
		return;
	}
	groupInd = qspSavedVarsGroupsCount - 1;
	count = bufSize = qspSavedVarsGroups[groupInd].VarsCount;
	isVarFound = QSP_FALSE;
	curPos = s;
	eqPos = qspStrPos(curPos, QSP_EQUAL, QSP_FALSE);
	if (eqPos) *eqPos = 0;
	while (1)
	{
		/* Skip type char */
		if (*curPos == QSP_STRCHAR[0]) ++curPos;
		/* Get variable's name */
		commaPos = qspStrPos(curPos, QSP_COMMA, QSP_FALSE);
		if (commaPos)
		{
			*commaPos = 0;
			varName = (eqPos ? qspGetVarNameOnly(curPos) : qspDelSpc(curPos));
			*commaPos = QSP_COMMA[0];
		}
		else
			varName = (eqPos ? qspGetVarNameOnly(curPos) : qspDelSpc(curPos));
		qspUpperStr(varName);
		/* Check for the existence */
		for (i = 0; i < count; ++i)
		{
			if (!qspStrsComp(varName, qspSavedVarsGroups[groupInd].Vars[i].Name))
			{
				isVarFound = QSP_TRUE;
				break;
			}
		}
		/* Get variable's data */
		if (isVarFound)
		{
			/* Already exists */
			isVarFound = QSP_FALSE;
			free(varName);
		}
		else
		{
			/* Add variable to the local group */
			if (!(var = qspVarReference(varName, QSP_FALSE)))
			{
				free(varName);
				if (eqPos) *eqPos = QSP_EQUAL[0];
				return;
			}
			if (count >= bufSize)
			{
				bufSize = count + 4;
				qspSavedVarsGroups[groupInd].Vars = (QSPVar *)realloc(qspSavedVarsGroups[groupInd].Vars, bufSize * sizeof(QSPVar));
			}
			qspMoveVar(qspSavedVarsGroups[groupInd].Vars + count, var);
			qspSavedVarsGroups[groupInd].Vars[count].Name = varName;
			qspSavedVarsGroups[groupInd].VarsCount = ++count;
		}
		if (!commaPos) break;
		curPos = qspSkipSpaces(commaPos + QSP_LEN(QSP_COMMA));
		if (!(*curPos))
		{
			qspSetError(QSP_ERR_SYNTAX);
			if (eqPos) *eqPos = QSP_EQUAL[0];
			return;
		}
	}
	if (eqPos)
	{
		oldRefreshCount = qspRefreshCount;
		v = qspExprValue(eqPos + QSP_LEN(QSP_EQUAL));
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			*eqPos = QSP_EQUAL[0];
			return;
		}
		varName = qspDelSpcCanRetSelf(s);
		qspSetVarValue(varName, &v, QSP_EQUAL[0]);
		if (varName != s) free(varName);
		if (v.IsStr) free(QSP_STR(v));
		*eqPos = QSP_EQUAL[0];
	}
}

QSP_BOOL qspStatementCopyArr(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
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

QSP_BOOL qspStatementKillVar(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	if (count == 1)
		qspRemoveArray(QSP_STR(args[0]));
	else if (count == 2)
		qspRemoveArrayItem(QSP_STR(args[0]), QSP_NUM(args[1]));
	else
		qspClearVars(QSP_FALSE);
	return QSP_FALSE;
}
