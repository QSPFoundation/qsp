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

#include "text.h"
#include "coding.h"
#include "errors.h"
#include "locations.h"
#include "mathops.h"
#include "variables.h"
#include "variant.h"

QSPString qspNullString;
QSPString qspEmptyString;

QSP_CHAR *qspStringToC(QSPString s)
{
	QSP_CHAR *string;
	int stringLen = qspStrLen(s);
	string = (QSP_CHAR *)malloc((stringLen + 1) * sizeof(QSP_CHAR));
	memcpy(string, s.Str, stringLen * sizeof(QSP_CHAR));
	string[stringLen] = 0;
	return string;
}

void qspAddText(QSPString *dest, QSPString val, QSP_BOOL isCreate)
{
	int destLen, valLen;
	QSP_CHAR *destPtr;
	valLen = qspStrLen(val);
	if (!isCreate && dest->Str)
	{
		destLen = qspStrLen(*dest);
		destPtr = (QSP_CHAR *)realloc(dest->Str, (destLen + valLen) * sizeof(QSP_CHAR));
		dest->Str = destPtr;
		destPtr += destLen;
	}
	else
	{
		destPtr = (QSP_CHAR *)malloc(valLen * sizeof(QSP_CHAR));
		dest->Str = destPtr;
	}
	dest->End = destPtr + valLen;
	memcpy(destPtr, val.Str, valLen * sizeof(QSP_CHAR));
}

void qspUpdateText(QSPString *dest, QSPString val)
{
	dest->End = dest->Str;
	qspAddText(dest, val, QSP_FALSE);
}

QSPString qspGetNewText(QSPString val)
{
	QSPString string;
	qspAddText(&string, val, QSP_TRUE);
	return string;
}

QSPString qspGetAddText(QSPString dest, QSPString val)
{
	qspAddText(&dest, val, QSP_FALSE);
	return dest;
}

QSPString qspNewEmptyString()
{
	return qspGetNewText(qspEmptyString);
}

QSP_BOOL qspClearText(QSPString *s)
{
	int strLen;
	if (s->Str)
	{
		strLen = (int)(s->End - s->Str);
		free(s->Str);
		s->Str = 0;
		s->End = 0;
		if (strLen) return QSP_TRUE;
	}
	return QSP_FALSE;
}

QSP_BOOL qspIsInList(QSP_CHAR *list, QSP_CHAR ch)
{
	while (*list)
		if (*list++ == ch) return QSP_TRUE;
	return QSP_FALSE;
}

QSP_BOOL qspIsDigit(QSP_CHAR ch)
{
	return (ch >= QSP_FMT('0') && ch <= QSP_FMT('9'));
}

void qspSkipSpaces(QSPString *s)
{
	QSP_CHAR *pos = s->Str;
	while (pos < s->End && qspIsInList(QSP_SPACES, *pos)) ++pos;
	s->Str = pos;
}

/* TODO: rewrite */
QSPString qspDelSpc(QSPString s)
{
	QSP_CHAR *begin, *end = s.End;
	qspSkipSpaces(&s);
	begin = s.Str;
	while (begin < end && qspIsInList(QSP_SPACES, *(end - 1))) --end;
	return qspStringFromPair(begin, end);
}

QSP_BOOL qspIsAnyString(QSPString s)
{
	qspSkipSpaces(&s);
	return (s.Str != s.End);
}

void qspLowerStr(QSPString *str)
{
	QSP_CHAR *pos = str->Str;
	while (pos < str->End) *pos++ = QSP_CHRLWR(*pos);
}

void qspUpperStr(QSPString *str)
{
	QSP_CHAR *pos = str->Str;
	while (pos < str->End) *pos++ = QSP_CHRUPR(*pos);
}

int qspStrsNComp(QSPString str1, QSPString str2, int maxLen)
{
	int delta = 0;
	QSP_CHAR *pos1 = str1.Str, *pos2 = str2.Str;
	while (maxLen-- && pos2 < str2.End && pos1 < str1.End && !(delta = (int)*pos1 - *pos2))
		++pos1, ++pos2;
	return delta;
}

int qspStrsComp(QSPString str1, QSPString str2)
{
	int delta = 0;
	QSP_CHAR *pos1 = str1.Str, *pos2 = str2.Str;
	while (pos2 < str2.End && pos1 < str1.End && !(delta = (int)*pos1 - *pos2))
		++pos1, ++pos2;
	if (delta) return delta;
	return (pos1 == str1.End) ? ((pos2 == str2.End) ? 0 : -1) : 1;
}

QSP_CHAR *qspStrChar(QSPString str, QSP_CHAR ch)
{
	QSP_CHAR *pos = str.Str;
	while (pos < str.End && *pos != ch) ++pos;
	if (*pos == ch) return pos;
	return 0;
}

QSP_CHAR *qspStrStr(QSPString str, QSPString strSearch)
{
	QSP_CHAR *s1, *s2, *pos = str.Str;
	while (pos < str.End)
	{
		s1 = pos;
		s2 = strSearch.Str;
		while (s1 < str.End && s2 < strSearch.End && !((int)*s1 - *s2))
			++s1, ++s2;
		if (s2 == strSearch.End) return pos;
		++pos;
	}
	return 0;
}

QSP_CHAR *qspStrPBrk(QSPString str, QSP_CHAR *strCharSet)
{
	QSP_CHAR *set, *pos = str.Str;
	while (pos < str.End)
	{
		for (set = strCharSet; *set; ++set)
			if (*set == *pos) return pos;
		++pos;
	}
	return 0;
}

QSP_CHAR *qspInStrRChars(QSPString str, QSP_CHAR *chars)
{
	QSP_CHAR *lastPos = 0, *pos = str.Str;
	while (pos < str.End)
	{
		if (qspIsInList(chars, *pos)) lastPos = pos;
		++pos;
	}
	return lastPos;
}

QSPString qspJoinStrs(QSPString *s, int count, QSPString delim)
{
	int i, curLen, txtLen = 0, txtRealLen = 0, bufSize = 256, delimLen = qspStrLen(delim);
	QSP_CHAR *txt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	for (i = 0; i < count; ++i)
	{
		curLen = qspStrLen(s[i]);
		if ((txtLen += curLen) > bufSize)
		{
			bufSize = txtLen + 128;
			txt = (QSP_CHAR *)realloc(txt, bufSize * sizeof(QSP_CHAR));
		}
		memcpy(txt + txtRealLen, s[i].Str, curLen * sizeof(QSP_CHAR));
		if (i == count - 1) break;
		txtRealLen = txtLen;
		if ((txtLen += delimLen) > bufSize)
		{
			bufSize = txtLen + 128;
			txt = (QSP_CHAR *)realloc(txt, bufSize * sizeof(QSP_CHAR));
		}
		memcpy(txt + txtRealLen, delim.Str, delimLen * sizeof(QSP_CHAR));
		txtRealLen = txtLen;
	}
	return qspStringFromLen(txt, txtLen);
}

int qspSplitStr(QSPString str, QSPString delim, QSPString **res)
{
	QSPString *ret;
	int allocChars, count = 0, bufSize = 8, delimLen = qspStrLen(delim);
	QSP_CHAR *newStr, *found = qspStrStr(str, delim);
	ret = (QSPString *)malloc(bufSize * sizeof(QSPString));
	while (found)
	{
		allocChars = (int)(found - str.Str);
		newStr = (QSP_CHAR *)malloc(allocChars * sizeof(QSP_CHAR));
		memcpy(newStr, str.Str, allocChars * sizeof(QSP_CHAR));
		if (++count > bufSize)
		{
			bufSize += 16;
			ret = (QSPString *)realloc(ret, bufSize * sizeof(QSPString));
		}
		ret[count - 1] = qspStringFromLen(newStr, allocChars);
		str.Str = found + delimLen;
		found = qspStrStr(str, delim);
	}
	allocChars = qspStrLen(str);
	newStr = (QSP_CHAR *)malloc(allocChars * sizeof(QSP_CHAR));
	memcpy(newStr, str.Str, allocChars * sizeof(QSP_CHAR));
	if (++count > bufSize)
		ret = (QSPString *)realloc(ret, count * sizeof(QSPString));
	ret[count - 1] = qspStringFromLen(newStr, allocChars);
	*res = ret;
	return count;
}

void qspCopyStrs(QSPString **dest, QSPString *src, int start, int end)
{
	int i, count = end - start;
	if (src && count)
	{
		*dest = (QSPString *)malloc(count * sizeof(QSPString));
		i = 0;
		while (start < end)
			qspAddText(*dest + i++, src[start++], QSP_TRUE);
	}
	else
		*dest = 0;
}

void qspFreeGameStrs(char **strs, int count)
{
	if (strs)
	{
		while (--count >= 0) free(strs[count]);
		free(strs);
	}
}

void qspFreeStrs(QSPString *strs, int count)
{
	if (strs)
	{
		while (--count >= 0) free(strs[count].Str);
		free(strs);
	}
}

QSP_BOOL qspIsNumber(QSPString s)
{
	QSP_CHAR *pos;
	qspSkipSpaces(&s);
	pos = s.Str;
	if (pos < s.End)
	{
		if (*pos == QSP_FMT('-'))
			++pos;
		else if (*pos == QSP_FMT('+'))
			++pos;
	}
	else
		return QSP_FALSE;
	if (pos < s.End && qspIsDigit(*pos))
	{
		do
		{
			++pos;
		} while (pos < s.End && qspIsDigit(*pos));
	}
	else
		return QSP_FALSE;
	s.Str = pos;
	qspSkipSpaces(&s);
	return qspIsEmpty(s);
}

int qspStrToNum(QSPString s, QSP_BOOL *isValid)
{
	int num;
	QSP_CHAR *pos;
	QSP_BOOL isNeg = QSP_FALSE;
	qspSkipSpaces(&s);
	pos = s.Str;
	if (pos < s.End)
	{
		if (*pos == QSP_FMT('-'))
		{
			isNeg = QSP_TRUE;
			++pos;
		}
		else if (*pos == QSP_FMT('+'))
			++pos;
	}
	else /* special case, i.e. empty string must be convertible to 0 */
	{
		if (isValid) *isValid = QSP_TRUE;
		return 0;
	}
	if (pos < s.End && qspIsDigit(*pos))
	{
		num = 0;
		do
		{
			num = num * 10 + (*pos - QSP_FMT('0'));
			++pos;
		} while (pos < s.End && qspIsDigit(*pos));
	}
	else
	{
		if (isValid) *isValid = QSP_FALSE;
		return 0;
	}
	s.Str = pos;
	qspSkipSpaces(&s);
	if (!qspIsEmpty(s))
	{
		if (isValid) *isValid = QSP_FALSE;
		return 0;
	}
	if (isValid) *isValid = QSP_TRUE;
	if (isNeg) return -num;
	return num;
}

QSPString qspNumToStr(QSP_CHAR *buf, int val)
{
	QSP_CHAR temp, *last, *str = buf, *first = str;
	if (val < 0)
	{
		*str++ = QSP_FMT('-');
		val = -val;
		++first;
	}
	do
	{
		*str++ = (QSP_CHAR)(val % 10 + QSP_FMT('0'));
		val /= 10;
	} while (val > 0);
	last = str--;
	while (first < str)
	{
		temp = *str;
		*str = *first;
		*first = temp;
		--str;
		++first;
	}
	return qspStringFromPair(buf, last);
}

QSP_CHAR *qspStrPos(QSPString txt, QSPString str, QSP_BOOL isIsolated)
{
	QSP_BOOL isLastDelim;
	int strLen, c1, c2, c3;
	QSP_CHAR quot, *lastPos, *pos = qspStrStr(txt, str);
	if (!pos) return 0;
	if (!(isIsolated || qspStrPBrk(txt, QSP_QUOTS QSP_LQUOT QSP_LRBRACK QSP_LSBRACK))) return pos;
	strLen = qspStrLen(str);
	lastPos = txt.End - strLen + 1;
	c1 = c2 = c3 = 0;
	isLastDelim = QSP_TRUE;
	pos = txt.Str;
	while (pos < lastPos)
	{
		if (qspIsInList(QSP_QUOTS, *pos))
		{
			quot = *pos;
			while (++pos < lastPos)
				if (*pos == quot && (++pos >= lastPos || *pos != quot)) break;
			if (pos >= lastPos) return 0;
			isLastDelim = QSP_TRUE;
		}
		if (*pos == QSP_LRBRACK[0])
			++c1;
		else if (*pos == QSP_RRBRACK[0])
		{
			if (c1) --c1;
		}
		else if (*pos == QSP_LSBRACK[0])
			++c2;
		else if (*pos == QSP_RSBRACK[0])
		{
			if (c2) --c2;
		}
		else if (*pos == QSP_LQUOT[0])
			++c3;
		else if (*pos == QSP_RQUOT[0])
		{
			if (c3) --c3;
		}
		if (!(c1 || c2 || c3))
		{
			if (isIsolated)
			{
				if (qspIsInList(QSP_DELIMS, *pos))
					isLastDelim = QSP_TRUE;
				else if (isLastDelim)
				{
					if (pos >= lastPos - 1 || qspIsInList(QSP_DELIMS, pos[strLen]))
					{
						txt.Str = pos;
						if (!qspStrsNComp(txt, str, strLen)) return pos;
					}
					isLastDelim = QSP_FALSE;
				}
			}
			else
			{
				txt.Str = pos;
				if (!qspStrsNComp(txt, str, strLen)) return pos;
			}
		}
		++pos;
	}
	return 0;
}

QSPString qspReplaceText(QSPString txt, QSPString searchTxt, QSPString repTxt)
{
	int txtLen, oldTxtLen, bufSize, searchLen, repLen, len;
	QSP_CHAR *newTxt, *pos = qspStrStr(txt, searchTxt);
	if (!pos) return qspGetNewText(txt);
	bufSize = 256;
	txtLen = oldTxtLen = 0;
	searchLen = qspStrLen(searchTxt);
	repLen = qspStrLen(repTxt);
	newTxt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	do
	{
		len = (int)(pos - txt.Str);
		if ((txtLen += len + repLen) > bufSize)
		{
			bufSize = txtLen + 128;
			newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
		}
		memcpy(newTxt + oldTxtLen, txt.Str, len * sizeof(QSP_CHAR));
		memcpy(newTxt + oldTxtLen + len, repTxt.Str, repLen * sizeof(QSP_CHAR));
		oldTxtLen = txtLen;
		txt.Str = pos + searchLen;
		pos = qspStrStr(txt, searchTxt);
	} while (pos);
	return qspGetAddText(qspStringFromLen(newTxt, txtLen), txt);
}

QSPString qspFormatText(QSPString txt, QSP_BOOL canReturnSelf)
{
	QSPVariant val;
	QSPString leftSubEx, rightSubEx;
	QSP_CHAR *newTxt, *lPos, *rPos;
	int oldRefreshCount, len, txtLen, oldTxtLen, bufSize;
	if (qspGetVarNumValue(QSP_STATIC_STR(QSP_FMT("DISABLESUBEX"))))
	{
		if (canReturnSelf) return txt;
		return qspGetNewText(txt);
	}
	leftSubEx = QSP_STATIC_STR(QSP_LSUBEX);
	lPos = qspStrStr(txt, leftSubEx);
	if (!lPos)
	{
		if (canReturnSelf) return txt;
		return qspGetNewText(txt);
	}
	bufSize = 256;
	newTxt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	txtLen = oldTxtLen = 0;
	oldRefreshCount = qspRefreshCount;
	rightSubEx = QSP_STATIC_STR(QSP_RSUBEX);
	do
	{
		len = (int)(lPos - txt.Str);
		if ((txtLen += len) > bufSize)
		{
			bufSize = txtLen + 128;
			newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
		}
		memcpy(newTxt + oldTxtLen, txt.Str, len * sizeof(QSP_CHAR));
		oldTxtLen = txtLen;
		txt.Str = lPos + QSP_STATIC_LEN(QSP_LSUBEX);
		rPos = qspStrPos(txt, rightSubEx, QSP_FALSE);
		if (!rPos)
		{
			qspSetError(QSP_ERR_BRACKNOTFOUND);
			free(newTxt);
			return qspNullString;
		}
		val = qspExprValue(qspStringFromPair(txt.Str, rPos));
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			free(newTxt);
			return qspNullString;
		}
		qspConvertVariantTo(&val, QSP_TRUE);
		len = qspStrLen(QSP_STR(val));
		if ((txtLen += len) > bufSize)
		{
			bufSize = txtLen + 128;
			newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
		}
		memcpy(newTxt + oldTxtLen, QSP_STR(val).Str, len * sizeof(QSP_CHAR));
		free(QSP_STR(val).Str);
		oldTxtLen = txtLen;
		txt.Str = rPos + QSP_STATIC_LEN(QSP_RSUBEX);
		lPos = qspStrStr(txt, leftSubEx);
	} while (lPos);
	return qspGetAddText(qspStringFromLen(newTxt, txtLen), txt);
}
