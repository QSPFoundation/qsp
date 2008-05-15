/* Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru) */
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
#include "variables.h"

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

void qspSetVarValueByIndex(long, long, QSPVariant, QSP_BOOL);

void qspClearVars(QSP_BOOL isFirst)
{
	long i;
	for (i = 0; i < QSP_VARSCOUNT; ++i)
	{
		if (!isFirst)
		{
			if (qspVars[i].Name) free(qspVars[i].Name);
			if (qspVars[i].Value) free(qspVars[i].Value);
			qspFreeStrs(qspVars[i].TextValue, qspVars[i].ValsCount, QSP_TRUE);
			qspFreeStrs(qspVars[i].TextIndex, qspVars[i].IndsCount, QSP_FALSE);
		}
		qspVars[i].Name = 0;
		qspVars[i].Value = 0;
		qspVars[i].TextValue = 0;
		qspVars[i].ValsCount = 0;
		qspVars[i].TextIndex = 0;
		qspVars[i].IndsCount = 0;
		qspVars[i].Type = qspVarNormal;
	}
}

void qspRefreshVar(long varIndex)
{
	QSPVariant v;
	QSP_CHAR emptyStr[1];
	switch (qspVars[varIndex].Type)
	{
	case qspVarRnd:
		v.IsStr = QSP_FALSE;
		v.Num = rand() % 1000 + 1;
		break;
	case qspVarCountObj:
		v.IsStr = QSP_FALSE;
		v.Num = qspCurObjectsCount;
		break;
	case qspVarMsecsCount:
		if ((qspMSCount += qspCallGetMSCount()) < 0) qspMSCount = 0;
		v.IsStr = QSP_FALSE;
		v.Num = qspMSCount;
		break;
	case qspVarQSPVer:
		v.IsStr = QSP_TRUE;
		v.Str = QSP_VER;
		break;
	case qspVarUserText:
		*emptyStr = 0;
		v.IsStr = QSP_TRUE;
		v.Str = qspCurInput ? qspCurInput : emptyStr;
		break;
	case qspVarCurLoc:
		*emptyStr = 0;
		v.IsStr = QSP_TRUE;
		v.Str = qspCurLoc >= 0 ? qspLocs[qspCurLoc].Name : emptyStr;
		break;
	case qspVarSelObj:
		*emptyStr = 0;
		v.IsStr = QSP_TRUE;
		v.Str = qspCurSelObject >= 0 ? qspCurObjects[qspCurSelObject].Desc : emptyStr;
		break;
	case qspVarSelAct:
		*emptyStr = 0;
		v.IsStr = QSP_TRUE;
		v.Str = qspCurSelAction >= 0 ? qspCurActions[qspCurSelAction].Desc : emptyStr;
		break;
	default:
		return;
	}
	qspSetVarValueByIndex(varIndex, 0, v, v.IsStr);
}

void qspInitSpecialVar(long type, QSP_CHAR *name)
{
	long varIndex = qspVarIndex(name, QSP_TRUE);
	if (varIndex < 0) return;
	qspVars[varIndex].Type = type;
	qspRefreshVar(varIndex);
}

void qspInitVars()
{
	qspInitSpecialVar(qspVarRnd, QSP_FMT("RND"));
	qspInitSpecialVar(qspVarCountObj, QSP_FMT("COUNTOBJ"));
	qspInitSpecialVar(qspVarMsecsCount, QSP_FMT("MSECSCOUNT"));
	qspInitSpecialVar(qspVarQSPVer, QSP_FMT("QSPVER"));
	qspInitSpecialVar(qspVarUserText, QSP_FMT("USER_TEXT"));
	qspInitSpecialVar(qspVarUserText, QSP_FMT("USRTXT"));
	qspInitSpecialVar(qspVarCurLoc, QSP_FMT("CURLOC"));
	qspInitSpecialVar(qspVarSelObj, QSP_FMT("SELOBJ"));
	qspInitSpecialVar(qspVarSelAct, QSP_FMT("SELACT"));
}

long qspVarIndex(QSP_CHAR *name, QSP_BOOL isCreate)
{
	QSP_CHAR *uName;
	unsigned char bCode;
	long i, code;
	if (*name == QSP_STRCHAR[0]) ++name;
	if (!(*name) || QSP_ISDIGIT(*name) || QSP_STRPBRK(name, QSP_DELIMS))
	{
		qspSetError(QSP_ERR_NOTCORRECTNAME);
		return -1;
	}
	qspUpperStr(uName = qspGetNewText(name, -1));
	bCode = 0;
	for (i = 0; uName[i]; ++i)
		bCode = qspRand8[bCode ^ QSP_MBTOSB(uName[i])];
	code = bCode * QSP_VARSSEEK;
	for (i = code; i < code + QSP_VARSSEEK; ++i)
	{
		if (!qspVars[i].Name)
		{
			if (isCreate)
				qspVars[i].Name = uName;
			else
				free(uName);
			return i;
		}
		if (!QSP_STRCMP(qspVars[i].Name, uName))
		{
			free(uName);
			return i;
		}
	}
	free(uName);
	qspSetError(QSP_ERR_TOOMANYVARS);
	return -1;
}

long qspGetVarTextIndex(long varIndex, QSP_CHAR *str, QSP_BOOL isCreate)
{
	QSP_CHAR *uStr;
	long i, n;
	qspUpperStr(uStr = qspGetNewText(str, -1));
	for (i = 0; i < qspVars[varIndex].IndsCount; ++i)
		if (!QSP_STRCMP(qspVars[varIndex].TextIndex[i], uStr))
		{
			free(uStr);
			return i;
		}
	if (isCreate)
	{
		n = qspVars[varIndex].IndsCount++;
		qspVars[varIndex].TextIndex = (QSP_CHAR **)realloc(qspVars[varIndex].TextIndex, (n + 1) * sizeof(QSP_CHAR *));
		qspVars[varIndex].TextIndex[n] = uStr;
		return n;
	}
	free(uStr);
	return qspVars[varIndex].ValsCount;
}

long qspGetVarData(QSP_CHAR *s, QSP_BOOL isCreate, long *index)
{
	QSPVariant ind;
	long varIndex;
	QSP_CHAR *rPos, *lPos = QSP_STRCHR(s, QSP_LSBRACK[0]);
	if (lPos)
	{
		*lPos = 0;
		varIndex = qspVarIndex(s, isCreate);
		*lPos = QSP_LSBRACK[0];
		if (varIndex < 0) return -1;
		rPos = qspStrPos(lPos, QSP_RSBRACK, QSP_FALSE);
		if (!rPos)
		{
			qspSetError(QSP_ERR_BRACKNOTFOUND);
			return -1;
		}
		*rPos = 0;
		ind = qspExprValue(lPos + 1);
		*rPos = QSP_RSBRACK[0];
		if (qspErrorNum) return -1;
		if (ind.IsStr)
		{
			*index = qspGetVarTextIndex(varIndex, ind.Str, isCreate);
			free(ind.Str);
		}
		else
			*index = ind.Num >= 0 ? ind.Num : 0;
		return varIndex;
	}
	*index = 0;
	return qspVarIndex(s, isCreate);
}

void qspSetVarValueByIndex(long varIndex, long ind, QSPVariant val, QSP_BOOL isToString)
{
	QSP_BOOL convErr = QSP_FALSE;
	long count, oldCount = qspVars[varIndex].ValsCount;
	if (ind >= oldCount)
	{
		count = ind + 1;
		qspVars[varIndex].ValsCount = count;
		qspVars[varIndex].Value = (long *)realloc(qspVars[varIndex].Value, count * sizeof(long));
		qspVars[varIndex].TextValue = (QSP_CHAR **)realloc(qspVars[varIndex].TextValue, count * sizeof(QSP_CHAR *));
		while (oldCount < count)
		{
			qspVars[varIndex].Value[oldCount] = 0;
			qspVars[varIndex].TextValue[oldCount] = 0;
			++oldCount;
		}
	}
	val = qspConvertVariantTo(val, isToString, QSP_FALSE, &convErr);
	if (convErr)
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		return;
	}
	if (val.IsStr)
	{
		qspVars[varIndex].TextValue[ind] = qspGetAddText(qspVars[varIndex].TextValue[ind], val.Str, 0, -1);
		free(val.Str);
	}
	else
		qspVars[varIndex].Value[ind] = val.Num;
}

void qspSetVar(QSP_CHAR *name, QSPVariant val)
{
	long index, varIndex = qspGetVarData(name, QSP_TRUE, &index);
	if (varIndex < 0) return;
	qspSetVarValueByIndex(varIndex, index, val, *name == QSP_STRCHAR[0]);
}

QSPVariant qspGetVarValueByIndex(long varIndex, long ind, QSP_BOOL isStringType)
{
	QSPVariant ret;
	QSP_CHAR *text;
	if (ind < qspVars[varIndex].ValsCount)
	{
		qspRefreshVar(varIndex);
		if (ret.IsStr = isStringType)
		{
			text = qspVars[varIndex].TextValue[ind];
			ret.Str = qspGetNewText(text ? text : QSP_FMT(""), -1);
		}
		else
			ret.Num = qspVars[varIndex].Value[ind];
		return ret;
	}
	return qspGetEmptyVariant(isStringType);
}

QSPVariant qspGetVarValueByName(QSP_CHAR *name)
{
	long varIndex = qspVarIndex(name, QSP_FALSE);
	if (varIndex < 0) return qspGetEmptyVariant(QSP_FALSE);
	return qspGetVarValueByIndex(varIndex, 0, *name == QSP_STRCHAR[0]);
}

QSPVariant qspGetVar(QSP_CHAR *name)
{
	long index, varIndex = qspGetVarData(name, QSP_FALSE, &index);
	if (varIndex < 0) return qspGetEmptyVariant(QSP_FALSE);
	return qspGetVarValueByIndex(varIndex, index, *name == QSP_STRCHAR[0]);
}

long qspArrayPos(QSP_CHAR *name, long start, QSPVariant val, QSP_BOOL isRegExp)
{
	long count, varInd, num;
	QSP_CHAR emptyStr[1], *str;
	OnigUChar *tempBeg, *tempEnd;
	regex_t *onigExp;
	OnigRegion *onigReg;
	OnigErrorInfo onigInfo;
	QSP_BOOL convErr = QSP_FALSE;
	val = qspConvertVariantTo(val, isRegExp || *name == QSP_STRCHAR[0], QSP_FALSE, &convErr);
	if (convErr)
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		return -1;
	}
	varInd = qspVarIndex(name, QSP_FALSE);
	if (varInd < 0)
	{
		if (val.IsStr) free(val.Str);
		return -1;
	}
	if (isRegExp)
	{
		tempBeg = (OnigUChar *)val.Str;
		tempEnd = (OnigUChar *)qspStrEnd(val.Str);
		if (onig_new(&onigExp, tempBeg, tempEnd, ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL, &onigInfo))
		{
			qspSetError(QSP_ERR_INCORRECTREGEXP);
			free(val.Str);
			return -1;
		}
	}
	*emptyStr = 0;
	if (start < 0) start = 0;
	count = qspVars[varInd].ValsCount;
	while (start <= count)
	{
		if (val.IsStr)
		{
			if (!(start < count && (str = qspVars[varInd].TextValue[start]))) str = emptyStr;
			if (isRegExp)
			{
				onigReg = onig_region_new();
				tempBeg = (OnigUChar *)str;
				tempEnd = (OnigUChar *)qspStrEnd(str);
				if (onig_match(onigExp, tempBeg, tempEnd, tempBeg, onigReg, ONIG_OPTION_NONE) >= 0)
				{
					free(val.Str);
					onig_region_free(onigReg, 1);
					onig_free(onigExp);
					return start;
				}
				onig_region_free(onigReg, 1);
			}
			else if (!QSP_STRCMP(str, val.Str))
			{
				free(val.Str);
				return start;
			}
		}
		else
		{
			num = (start < count ? qspVars[varInd].Value[start] : 0);
			if (num == val.Num) return start;
		}
		++start;
	}
	if (val.IsStr) free(val.Str);
	if (isRegExp) onig_free(onigExp);
	return -1;
}

long qspGetVarsCount()
{
	long i, count = 0;
	for (i = 0; i < QSP_VARSCOUNT; ++i)
		if (qspVars[i].Name) ++count;
	return count;
}

void qspStatementSetVarValue(QSP_CHAR *s)
{
	QSPVariant v;
	QSP_CHAR *name, *pos = qspStrPos(s, QSP_EQUAL, QSP_FALSE);
	if (!pos)
	{
		qspSetError(QSP_ERR_EQNOTFOUND);
		return;
	}
	v = qspExprValue(pos + QSP_LEN(QSP_EQUAL));
	if (qspErrorNum) return;
	*pos = 0;
	name = qspDelSpc(s);
	*pos = QSP_EQUAL[0];
	qspSetVar(name, v);
	free(name);
	if (v.IsStr) free(v.Str);
}

QSP_BOOL qspStatementCopyArr(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_CHAR *str;
	long arrInd1, arrInd2, itemsCount;
	/* Get index of the first array */
	str = qspDelSpc(args[0].Str);
	arrInd1 = qspVarIndex(str, QSP_TRUE);
	free(str);
	if (arrInd1 < 0) return QSP_FALSE;
	/* Get index of the second array */
	str = qspDelSpc(args[1].Str);
	arrInd2 = qspVarIndex(str, QSP_FALSE);
	free(str);
	if (arrInd2 < 0 || arrInd1 == arrInd2) return QSP_FALSE;
	/* --- */
	if (qspVars[arrInd1].Value) free(qspVars[arrInd1].Value);
	qspFreeStrs(qspVars[arrInd1].TextValue, qspVars[arrInd1].ValsCount, QSP_TRUE);
	itemsCount = qspVars[arrInd1].ValsCount = qspVars[arrInd2].ValsCount;
	if (itemsCount)
	{
		qspVars[arrInd1].Value = (long *)malloc(itemsCount * sizeof(long));
		qspVars[arrInd1].TextValue = (QSP_CHAR **)malloc(itemsCount * sizeof(QSP_CHAR *));
		while (--itemsCount >= 0)
		{
			qspVars[arrInd1].Value[itemsCount] = qspVars[arrInd2].Value[itemsCount];
			str = qspVars[arrInd2].TextValue[itemsCount];
			qspVars[arrInd1].TextValue[itemsCount] = str ? qspGetNewText(str, -1) : 0;
		}
	}
	else
	{
		qspVars[arrInd1].Value = 0;
		qspVars[arrInd1].TextValue = 0;
	}
	qspFreeStrs(qspVars[arrInd1].TextIndex, qspVars[arrInd1].IndsCount, QSP_FALSE);
	itemsCount = qspVars[arrInd1].IndsCount = qspVars[arrInd2].IndsCount;
	qspCopyStrs(&qspVars[arrInd1].TextIndex, qspVars[arrInd2].TextIndex, 0, itemsCount);
	return QSP_FALSE;
}
