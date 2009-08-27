/* Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru) */
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
#include "errors.h"
#include "locations.h"
#include "mathops.h"
#include "text.h"

QSPVar qspVars[QSP_VARSCOUNT];

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

static void qspRemoveArray(QSP_CHAR *);
static void qspRemoveArrayItem(QSP_CHAR *, long);
static void qspInitVarData(QSPVar *);
static long qspGetVarTextIndex(QSPVar *, QSP_CHAR *, QSP_BOOL);
static QSPVar *qspGetVarData(QSP_CHAR *, QSP_BOOL, long *);
static void qspSetVarValueByReference(QSPVar *, long, QSPVariant *);
static void qspSetVar(QSP_CHAR *, QSPVariant *);
static QSPVariant qspGetVarValueByReference(QSPVar *, long, QSP_BOOL);
static void qspCopyVar(QSPVar *, QSPVar *);

void qspClearVars(QSP_BOOL isFirst)
{
	long i;
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
	if (var->Value) free(var->Value);
	qspFreeStrs(var->TextValue, var->ValsCount, QSP_TRUE);
	qspFreeStrs(var->TextIndex, var->IndsCount, QSP_FALSE);
	qspInitVarData(var);
}

static void qspRemoveArray(QSP_CHAR *name)
{
	QSPVar *var;
	if (!(var = qspVarReferenceWithType(name, QSP_FALSE, 0))) return;
	qspEmptyVar(var);
}

static void qspRemoveArrayItem(QSP_CHAR *name, long index)
{
	QSPVar *var;
	long origIndex;
	if (!(var = qspVarReferenceWithType(name, QSP_FALSE, 0))) return;
	if (!var->ValsCount) return;
	if (index < 0)
		index = 0;
	else if (index >= var->ValsCount)
		index = var->ValsCount - 1;
	origIndex = index;
	if (var->TextValue[index]) free(var->TextValue[index]);
	var->ValsCount--;
	while (index < var->ValsCount)
	{
		var->Value[index] = var->Value[index + 1];
		var->TextValue[index] = var->TextValue[index + 1];
		++index;
	}
	if (origIndex < var->IndsCount)
	{
		free(var->TextIndex[origIndex]);
		var->IndsCount--;
		while (origIndex < var->IndsCount)
		{
			var->TextIndex[origIndex] = var->TextIndex[origIndex + 1];
			++origIndex;
		}
	}
}

static void qspInitVarData(QSPVar *var)
{
	var->Value = 0;
	var->TextValue = 0;
	var->ValsCount = 0;
	var->TextIndex = 0;
	var->IndsCount = 0;
}

QSPVar *qspVarReference(QSP_CHAR *name, QSP_BOOL isCreate)
{
	long i;
	QSPVar *var;
	QSP_CHAR *uName;
	unsigned char bCode;
	if (*name == QSP_STRCHAR[0]) ++name;
	if (!(*name) || qspIsDigit(*name) || QSP_STRPBRK(name, QSP_DELIMS))
	{
		qspSetError(QSP_ERR_NOTCORRECTNAME);
		return 0;
	}
	qspUpperStr(uName = qspGetNewText(name, -1));
	bCode = 0;
	for (i = 0; uName[i]; ++i)
		bCode = qspRand8[bCode ^ QSP_MBTOSB(uName[i])];
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
		if (!QSP_STRCMP(var->Name, uName))
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
	QSP_CHAR *varName = qspDelSpc(name);
	if (isString) *isString = (*varName == QSP_STRCHAR[0]);
	var = qspVarReference(varName, isCreate);
	free(varName);
	return var;
}

static long qspGetVarTextIndex(QSPVar *var, QSP_CHAR *str, QSP_BOOL isCreate)
{
	QSP_CHAR *uStr;
	long i, n = var->IndsCount;
	qspUpperStr(uStr = qspGetNewText(str, -1));
	for (i = 0; i < n; ++i)
		if (!QSP_STRCMP(var->TextIndex[i], uStr))
		{
			free(uStr);
			return i;
		}
	if (isCreate)
	{
		var->IndsCount++;
		var->TextIndex = (QSP_CHAR **)realloc(var->TextIndex, (n + 1) * sizeof(QSP_CHAR *));
		var->TextIndex[n] = uStr;
		return n;
	}
	free(uStr);
	return var->ValsCount;
}

static QSPVar *qspGetVarData(QSP_CHAR *s, QSP_BOOL isCreate, long *index)
{
	QSPVar *var;
	QSPVariant ind;
	long oldRefreshCount;
	QSP_CHAR *rPos, *lPos = QSP_STRCHR(s, QSP_LSBRACK[0]);
	if (lPos)
	{
		*lPos = 0;
		var = qspVarReference(s, isCreate);
		*lPos = QSP_LSBRACK[0];
		if (!var) return 0;
		rPos = qspStrPos(lPos, QSP_RSBRACK, QSP_FALSE);
		if (!rPos)
		{
			qspSetError(QSP_ERR_BRACKNOTFOUND);
			return 0;
		}
		oldRefreshCount = qspRefreshCount;
		*rPos = 0;
		ind = qspExprValue(lPos + 1);
		*rPos = QSP_RSBRACK[0];
		if (qspRefreshCount != oldRefreshCount || qspErrorNum) return 0;
		if (ind.IsStr)
		{
			*index = qspGetVarTextIndex(var, QSP_STR(ind), isCreate);
			free(QSP_STR(ind));
		}
		else
			*index = (QSP_NUM(ind) >= 0 ? QSP_NUM(ind) : 0);
		return var;
	}
	*index = 0;
	return qspVarReference(s, isCreate);
}

static void qspSetVarValueByReference(QSPVar *var, long ind, QSPVariant *val)
{
	long count, oldCount = var->ValsCount;
	if (ind >= oldCount)
	{
		count = ind + 1;
		var->ValsCount = count;
		var->Value = (long *)realloc(var->Value, count * sizeof(long));
		var->TextValue = (QSP_CHAR **)realloc(var->TextValue, count * sizeof(QSP_CHAR *));
		while (oldCount < count)
		{
			var->Value[oldCount] = 0;
			var->TextValue[oldCount] = 0;
			++oldCount;
		}
	}
	if (val->IsStr)
		var->TextValue[ind] = qspGetAddText(var->TextValue[ind], QSP_PSTR(val), 0, -1);
	else
		var->Value[ind] = QSP_PNUM(val);
}

void qspSetVarValueByName(QSP_CHAR *name, QSPVariant *val)
{
	QSPVar *var;
	if (!(var = qspVarReference(name, QSP_TRUE))) return;
	qspSetVarValueByReference(var, 0, val);
}

static void qspSetVar(QSP_CHAR *name, QSPVariant *val)
{
	QSPVar *var;
	long index;
	if (!(var = qspGetVarData(name, QSP_TRUE, &index))) return;
	if (qspConvertVariantTo(val, *name == QSP_STRCHAR[0]))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		return;
	}
	qspSetVarValueByReference(var, index, val);
}

static QSPVariant qspGetVarValueByReference(QSPVar *var, long ind, QSP_BOOL isStringType)
{
	QSPVariant ret;
	QSP_CHAR *text;
	if (ind < var->ValsCount)
	{
		if (ret.IsStr = isStringType)
		{
			text = var->TextValue[ind];
			QSP_STR(ret) = (text ? qspGetNewText(text, -1) : qspGetNewText(QSP_FMT(""), 0));
		}
		else
			QSP_NUM(ret) = var->Value[ind];
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
			text = var->TextValue[0];
			if (text) return text;
		}
	}
	else
		qspResetError(QSP_FALSE);
	return QSP_FMT("");
}

long qspGetVarNumValue(QSP_CHAR *name)
{
	QSPVar *var;
	if (var = qspVarReference(name, QSP_FALSE))
	{
		if (var->ValsCount) return var->Value[0];
	}
	else
		qspResetError(QSP_FALSE);
	return 0;
}

QSPVariant qspGetVar(QSP_CHAR *name)
{
	QSPVar *var;
	long index;
	if (!(var = qspGetVarData(name, QSP_FALSE, &index))) return qspGetEmptyVariant(QSP_FALSE);
	return qspGetVarValueByReference(var, index, *name == QSP_STRCHAR[0]);
}

static void qspCopyVar(QSPVar *dest, QSPVar *src)
{
	QSP_CHAR *str;
	long count = dest->ValsCount = src->ValsCount;
	if (count)
	{
		dest->Value = (long *)malloc(count * sizeof(long));
		dest->TextValue = (QSP_CHAR **)malloc(count * sizeof(QSP_CHAR *));
		while (--count >= 0)
		{
			dest->Value[count] = src->Value[count];
			str = src->TextValue[count];
			dest->TextValue[count] = (str ? qspGetNewText(str, -1) : 0);
		}
	}
	else
	{
		dest->Value = 0;
		dest->TextValue = 0;
	}
	count = dest->IndsCount = src->IndsCount;
	qspCopyStrs(&dest->TextIndex, src->TextIndex, 0, count);
}

long qspArraySize(QSP_CHAR *name)
{
	QSPVar *var;
	if (!(var = qspVarReferenceWithType(name, QSP_FALSE, 0))) return 0;
	return var->ValsCount;
}

long qspArrayPos(QSPVariant *args, long argsCount, QSP_BOOL isRegExp)
{
	long num, count, ind;
	QSPVar *var;
	QSPVariant *val;
	QSP_CHAR emptyStr[1], *str;
	OnigUChar *tempBeg, *tempEnd;
	regex_t *onigExp;
	OnigRegion *onigReg;
	OnigErrorInfo onigInfo;
	QSP_BOOL isString;
	if (qspConvertVariantTo(args, argsCount == 2))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		return -1;
	}
	if (argsCount == 2)
	{
		str = QSP_STR(args[0]);
		ind = 0;
		val = args + 1;
	}
	else
	{
		qspConvertVariantTo(args + 1, QSP_TRUE);
		str = QSP_STR(args[1]);
		ind = QSP_NUM(args[0]);
		val = args + 2;
		if (ind < 0) ind = 0;
	}
	if (!(var = qspVarReferenceWithType(str, QSP_FALSE, &isString))) return -1;
	if (qspConvertVariantTo(val, isRegExp || isString))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		return -1;
	}
	if (isRegExp)
	{
		tempBeg = (OnigUChar *)QSP_PSTR(val);
		tempEnd = (OnigUChar *)qspStrEnd(QSP_PSTR(val));
		if (onig_new(&onigExp, tempBeg, tempEnd, ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL, &onigInfo))
		{
			qspSetError(QSP_ERR_INCORRECTREGEXP);
			return -1;
		}
	}
	*emptyStr = 0;
	count = var->ValsCount;
	while (ind <= count)
	{
		if (val->IsStr)
		{
			if (!(ind < count && (str = var->TextValue[ind]))) str = emptyStr;
			if (isRegExp)
			{
				onigReg = onig_region_new();
				tempBeg = (OnigUChar *)str;
				tempEnd = (OnigUChar *)qspStrEnd(str);
				if (onig_match(onigExp, tempBeg, tempEnd, tempBeg, onigReg, ONIG_OPTION_NONE) >= 0)
				{
					onig_region_free(onigReg, 1);
					onig_free(onigExp);
					return ind;
				}
				onig_region_free(onigReg, 1);
			}
			else if (!QSP_STRCMP(str, QSP_PSTR(val)))
				return ind;
		}
		else
		{
			num = (ind < count ? var->Value[ind] : 0);
			if (num == QSP_PNUM(val)) return ind;
		}
		++ind;
	}
	if (isRegExp) onig_free(onigExp);
	return -1;
}

QSPVariant qspArrayMinMaxItem(QSP_CHAR *name, QSP_BOOL isMin)
{
	QSPVar *var;
	QSP_CHAR *str;
	QSP_BOOL isString;
	long curInd, count;
	QSPVariant res;
	if (!(var = qspVarReferenceWithType(name, QSP_FALSE, &isString)))
		return qspGetEmptyVariant(QSP_FALSE);
	curInd = -1;
	count = var->ValsCount;
	while (--count >= 0)
	{
		if (isString)
		{
			str = var->TextValue[count];
			if (curInd >= 0)
			{
				if (str && *str)
				{
					if (isMin)
					{
						if (QSP_STRCOLL(str, var->TextValue[curInd]) < 0)
							curInd = count;
					}
					else if (QSP_STRCOLL(str, var->TextValue[curInd]) > 0)
						curInd = count;
				}
			}
			else if (str && *str)
				curInd = count;
		}
		else if (curInd >= 0)
		{
			if (isMin)
			{
				if (var->Value[count] < var->Value[curInd])
					curInd = count;
			}
			else if (var->Value[count] > var->Value[curInd])
				curInd = count;
		}
		else
			curInd = count;
	}
	if (curInd < 0) return qspGetEmptyVariant(isString);
	if (res.IsStr = isString)
		QSP_STR(res) = qspGetNewText(var->TextValue[curInd], -1);
	else
		QSP_NUM(res) = var->Value[curInd];
	return res;
}

long qspGetVarsCount()
{
	long i, count = 0;
	for (i = 0; i < QSP_VARSCOUNT; ++i)
		if (qspVars[i].Name) ++count;
	return count;
}

void qspSetArgs(QSPVar *var, QSPVariant *args, long count)
{
	while (--count >= 0)
		qspSetVarValueByReference(var, count, args + count);
}

void qspMoveVar(QSPVar *dest, QSPVar *src)
{
	dest->Value = src->Value;
	dest->TextValue = src->TextValue;
	dest->ValsCount = src->ValsCount;
	dest->TextIndex = src->TextIndex;
	dest->IndsCount = src->IndsCount;
	qspInitVarData(src);
}

void qspStatementSetVarValue(QSP_CHAR *s)
{
	QSPVariant v;
	long oldRefreshCount;
	QSP_CHAR *name, *pos = qspStrPos(s, QSP_EQUAL, QSP_FALSE);
	if (!pos)
	{
		qspSetError(QSP_ERR_EQNOTFOUND);
		return;
	}
	oldRefreshCount = qspRefreshCount;
	v = qspExprValue(pos + QSP_LEN(QSP_EQUAL));
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return;
	*pos = 0;
	name = qspDelSpc(s);
	*pos = QSP_EQUAL[0];
	qspSetVar(name, &v);
	free(name);
	if (v.IsStr) free(QSP_STR(v));
}

QSP_BOOL qspStatementCopyArr(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSPVar *dest, *src;
	if (!(dest = qspVarReferenceWithType(QSP_STR(args[0]), QSP_TRUE, 0))) return QSP_FALSE;
	if (!(src = qspVarReferenceWithType(QSP_STR(args[1]), QSP_FALSE, 0))) return QSP_FALSE;
	if (dest != src)
	{
		qspEmptyVar(dest);
		qspCopyVar(dest, src);
	}
	return QSP_FALSE;
}

QSP_BOOL qspStatementKillVar(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	if (count == 1)
		qspRemoveArray(QSP_STR(args[0]));
	else if (count == 2)
		qspRemoveArrayItem(QSP_STR(args[0]), QSP_NUM(args[1]));
	else
		qspClearVars(QSP_FALSE);
	return QSP_FALSE;
}
