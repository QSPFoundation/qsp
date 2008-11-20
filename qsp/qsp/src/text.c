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

#include "text.h"
#include "coding.h"
#include "errors.h"
#include "math.h"
#include "variables.h"
#include "variant.h"

long qspAddText(QSP_CHAR **dest, QSP_CHAR *val, long destLen, long valLen, QSP_BOOL isCreate)
{
	long ret;
	QSP_CHAR *destPtr;
	if (valLen < 0) valLen = (long)QSP_STRLEN(val);
	if (!isCreate && *dest)
	{
		if (destLen < 0) destLen = (long)QSP_STRLEN(*dest);
		ret = destLen + valLen;
		destPtr = (QSP_CHAR *)realloc(*dest, (ret + 1) * sizeof(QSP_CHAR));
		*dest = destPtr;
		destPtr += destLen;
	}
	else
	{
		ret = valLen;
		destPtr = (QSP_CHAR *)malloc((ret + 1) * sizeof(QSP_CHAR));
		*dest = destPtr;
	}
	QSP_STRNCPY(destPtr, val, valLen);
	destPtr[valLen] = 0;
	return ret;
}

QSP_CHAR *qspGetNewText(QSP_CHAR *val, long valLen)
{
	QSP_CHAR *buf;
	qspAddText(&buf, val, 0, valLen, QSP_TRUE);
	return buf;
}

QSP_CHAR *qspGetAddText(QSP_CHAR *dest, QSP_CHAR *val, long destLen, long valLen)
{
	qspAddText(&dest, val, destLen, valLen, QSP_FALSE);
	return dest;
}

QSP_BOOL qspClearText(void **text, long *textLen)
{
	if (*text)
	{
		free(*text);
		*text = 0;
		if (*textLen)
		{
			*textLen = 0;
			return QSP_TRUE;
		}
	}
	return QSP_FALSE;
}

QSP_BOOL qspIsInList(QSP_CHAR *list, QSP_CHAR ch)
{
	while (*list)
		if (*list++ == ch) return QSP_TRUE;
	return QSP_FALSE;
}

QSP_BOOL qspIsInListEOL(QSP_CHAR *list, QSP_CHAR ch)
{
	while (*list && *list != ch) ++list;
	return (*list == ch);
}

QSP_CHAR *qspSkipSpaces(QSP_CHAR *s)
{
	while (qspIsInList(QSP_SPACES, *s)) ++s;
	return s;
}

QSP_CHAR *qspStrEnd(QSP_CHAR *s)
{
	while (*s) ++s;
	return s;
}

QSP_CHAR *qspDelSpc(QSP_CHAR *s)
{
	long len;
	QSP_CHAR *str, *begin = qspSkipSpaces(s), *end = qspStrEnd(begin);
	while (begin < end && qspIsInList(QSP_SPACES, *(end - 1))) --end;
	len = (long)(end - begin);
	str = (QSP_CHAR *)malloc((len + 1) * sizeof(QSP_CHAR));
	QSP_STRNCPY(str, begin, len);
	str[len] = 0;
	return str;
}

QSP_BOOL qspIsAnyString(QSP_CHAR *s)
{
	return (*qspSkipSpaces(s) != 0);
}

void qspLowerStr(QSP_CHAR *str)
{
	while (*str) *str++ = QSP_CHRLWR(*str);
}

void qspUpperStr(QSP_CHAR *str)
{
	while (*str) *str++ = QSP_CHRUPR(*str);
}

QSP_BOOL qspIsEqual(QSP_CHAR *str1, QSP_CHAR *str2, long maxLen)
{
	long delta = 0;
	while (maxLen-- && !(delta = (long)(*str1 - *str2)) && *str2)
		++str1, ++str2;
	return (delta == 0);
}

QSP_CHAR *qspInStrRChar(QSP_CHAR *str, QSP_CHAR ch, QSP_CHAR *end)
{
	if (!end) end = qspStrEnd(str);
	if (end == str) return 0;
	--end;
	while (end != str && *end != ch) --end;
	return (*end == ch ? end : 0);
}

QSP_CHAR *qspJoinStrs(QSP_CHAR **s, long count, QSP_CHAR *delim)
{
	long i, newTxtLen = 0, newTxtRealLen = 0, newTxtBufSize = 256, lastIndex = count - 1, delimLen = (long)QSP_STRLEN(delim);
	QSP_CHAR *newTxt = (QSP_CHAR *)malloc(newTxtBufSize * sizeof(QSP_CHAR));
	for (i = 0; i < count; ++i)
	{
		newTxtLen += (long)QSP_STRLEN(s[i]);
		if (newTxtLen >= newTxtBufSize)
		{
			newTxtBufSize = newTxtLen + 128;
			newTxt = (QSP_CHAR *)realloc(newTxt, newTxtBufSize * sizeof(QSP_CHAR));
		}
		QSP_STRCPY(newTxt + newTxtRealLen, s[i]);
		if (i == lastIndex) break;
		newTxtRealLen = newTxtLen;
		newTxtLen += delimLen;
		if (newTxtLen >= newTxtBufSize)
		{
			newTxtBufSize = newTxtLen + 128;
			newTxt = (QSP_CHAR *)realloc(newTxt, newTxtBufSize * sizeof(QSP_CHAR));
		}
		QSP_STRCPY(newTxt + newTxtRealLen, delim);
		newTxtRealLen = newTxtLen;
	}
	return newTxt;
}

long qspSplitStr(QSP_CHAR *str, QSP_CHAR *delim, QSP_CHAR ***res)
{
	long allocChars, count = 0, bufSize = 8, delimLen = (long)QSP_STRLEN(delim);
	QSP_CHAR *newStr, **ret, *curPos = str, *found = QSP_STRSTR(str, delim);
	ret = (QSP_CHAR **)malloc(bufSize * sizeof(QSP_CHAR *));
	while (found)
	{
		allocChars = (long)(found - curPos);
		newStr = (QSP_CHAR *)malloc((allocChars + 1) * sizeof(QSP_CHAR));
		QSP_STRNCPY(newStr, curPos, allocChars);
		newStr[allocChars] = 0;
		if (++count > bufSize)
		{
			bufSize <<= 1;
			ret = (QSP_CHAR **)realloc(ret, bufSize * sizeof(QSP_CHAR *));
		}
		ret[count - 1] = newStr;
		curPos = found + delimLen;
		found = QSP_STRSTR(curPos, delim);
	}
	newStr = (QSP_CHAR *)malloc((QSP_STRLEN(curPos) + 1) * sizeof(QSP_CHAR));
	QSP_STRCPY(newStr, curPos);
	if (++count > bufSize)
		ret = (QSP_CHAR **)realloc(ret, count * sizeof(QSP_CHAR *));
	ret[count - 1] = newStr;
	*res = ret;
	return count;
}

void qspCopyStrs(QSP_CHAR ***dest, QSP_CHAR **src, long start, long end)
{
	long i, count = end - start;
	if (src && count)
	{
		*dest = (QSP_CHAR **)malloc(count * sizeof(QSP_CHAR *));
		i = 0;
		while (start < end)
			qspAddText(*dest + i++, src[start++], 0, -1, QSP_TRUE);
	}
	else
		*dest = 0;
}

void qspFreeStrs(void **strs, long count, QSP_BOOL isVerify)
{
	if (strs)
	{
		if (isVerify)
		{
			while (--count >= 0)
				if (strs[count]) free(strs[count]);
		}
		else
			while (--count >= 0) free(strs[count]);
		free(strs);
	}
}

long qspStrToNum(QSP_CHAR *s, QSP_CHAR **endChar)
{
	long num;
	s = qspSkipSpaces(s);
	num = QSP_STRTOL(s, endChar, 10);
	if (endChar)
	{
		*endChar = qspSkipSpaces(*endChar);
		if (**endChar) return 0;
	}
	return num;
}

QSP_CHAR *qspNumToStr(QSP_CHAR *buf, long val)
{
	QSP_CHAR temp, *str = buf, *first = str;
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
	*str-- = 0;
	while (first < str)
	{
		temp = *str;
		*str = *first;
		*first = temp;
		--str;
		++first;
	}
	return buf;
}

QSP_CHAR *qspStrPos(QSP_CHAR *txt, QSP_CHAR *str, QSP_BOOL isIsolated)
{
	QSP_BOOL isLastDelim;
	long strLen, c1, c2;
	QSP_CHAR quot, *txtEnd, *pos = QSP_STRSTR(txt, str);
	if (!pos) return 0;
	if (!(isIsolated || QSP_STRPBRK(txt, QSP_QUOTS QSP_LRBRACK QSP_LSBRACK))) return pos;
	strLen = (long)QSP_STRLEN(str);
	txtEnd = qspStrEnd(txt) - strLen + 1;
	c1 = c2 = 0;
	isLastDelim = QSP_TRUE;
	while (txt < txtEnd)
	{
		if (qspIsInList(QSP_QUOTS, *txt))
		{
			quot = *txt;
			while (++txt < txtEnd)
				if (*txt == quot && *(++txt) != quot) break;
			if (txt >= txtEnd) return 0;
			isLastDelim = QSP_TRUE;
		}
		if (*txt == QSP_LRBRACK[0])
			++c1;
		else if (*txt == QSP_RRBRACK[0])
			--c1;
		else if (*txt == QSP_LSBRACK[0])
			++c2;
		else if (*txt == QSP_RSBRACK[0])
			--c2;
		if (!(c1 || c2))
		{
			if (isIsolated)
			{
				if (qspIsInList(QSP_DELIMS, *txt))
					isLastDelim = QSP_TRUE;
				else if (isLastDelim)
				{
					if (qspIsInListEOL(QSP_DELIMS, txt[strLen]) && qspIsEqual(txt, str, strLen)) return txt;
					isLastDelim = QSP_FALSE;
				}
			}
			else if (qspIsEqual(txt, str, strLen))
				return txt;
		}
		++txt;
	}
	return 0;
}

QSP_CHAR *qspFormatText(QSP_CHAR *txt)
{
	QSPVariant val;
	QSP_CHAR *newTxt, *lPos, *rPos;
	long len, txtLen, oldTxtLen, bufSize;
	if (qspGetVarNumValue(QSP_FMT("DISABLESUBEX"))) return qspGetNewText(txt, -1);
	bufSize = 256;
	newTxt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	txtLen = oldTxtLen = 0;
	lPos = QSP_STRSTR(txt, QSP_LSUBEX);
	while (lPos)
	{
		len = (long)(lPos - txt);
		if ((txtLen += len) >= bufSize)
		{
			bufSize = txtLen + 128;
			newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
		}
		QSP_STRNCPY(newTxt + oldTxtLen, txt, len);
		oldTxtLen = txtLen;
		txt = lPos + QSP_LEN(QSP_LSUBEX);
		rPos = qspStrPos(txt, QSP_RSUBEX, QSP_FALSE);
		if (!rPos)
		{
			qspSetError(QSP_ERR_BRACKNOTFOUND);
			free(newTxt);
			return 0;
		}
		*rPos = 0;
		val = qspExprValue(txt);
		*rPos = QSP_RSUBEX[0];
		if (qspErrorNum)
		{
			free(newTxt);
			return 0;
		}
		val = qspConvertVariantTo(val, QSP_TRUE, QSP_TRUE, 0);
		if ((txtLen += (long)QSP_STRLEN(QSP_STR(val))) >= bufSize)
		{
			bufSize = txtLen + 128;
			newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
		}
		QSP_STRCPY(newTxt + oldTxtLen, QSP_STR(val));
		free(QSP_STR(val));
		oldTxtLen = txtLen;
		txt = rPos + QSP_LEN(QSP_RSUBEX);
		lPos = QSP_STRSTR(txt, QSP_LSUBEX);
	}
	return qspGetAddText(newTxt, txt, txtLen, -1);
}
