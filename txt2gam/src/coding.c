/* Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru) */
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "declarations.h"

wchar_t qspCP1251ToUnicodeTable[] =
{
	0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021,
	0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
	0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x0020, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
	0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7,
	0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
	0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7,
	0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
	0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
	0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
	0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
	0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
	0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
	0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
	0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
	0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F
};

long qspUCS2StrLen(char *str)
{
	unsigned short *ptr = (unsigned short *)str;
	while (*ptr) ++ptr;
	return (long)(ptr - (unsigned short *)str);
}

char *qspUCS2StrStr(char *str, char *subStr)
{
	unsigned short *s1, *s2, *cp = (unsigned short *)str;
	while (*cp)
	{
		s1 = cp;
		s2 = (unsigned short *)subStr;
		while (*s1 && *s2 && !(*s1 - *s2))
			++s1, ++s2;
		if (!(*s2)) return (char *)cp;
		++cp;
	}
	return 0;
}

wchar_t qspDirectConvertUC(char ch, wchar_t *table)
{
	unsigned char ch2 = (unsigned char)ch;
	return ch2 >= 0x80 ? table[ch2 - 0x80] : ch;
}

char qspReverseConvertUC(wchar_t ch, wchar_t *table)
{
	long i;
	if (ch < 0x80) return (char)ch;
	for (i = 127; i >= 0; --i)
		if (table[i] == ch) return (char)(i + 0x80);
	return 0x20;
}

QSP_CHAR *qspCodeReCode(QSP_CHAR *str, QSP_BOOL isCode)
{
	long offset, len = (long)QSP_STRLEN(str);
	QSP_CHAR *buf = (QSP_CHAR *)malloc((len + 1) * sizeof(QSP_CHAR));
	offset = isCode ? -QSP_CODREMOV : QSP_CODREMOV;
	buf[len] = 0;
	while (--len >= 0)
		buf[len] = (QSP_CHAR)(str[len] + offset);
	return buf;
}

char *qspFromQSPString(QSP_CHAR *s)
{
	long len = (long)QSP_WCSTOMBSLEN(s) + 1;
	char *ret = (char *)malloc(len);
	QSP_WCSTOMBS(ret, s, len);
	return ret;
}

QSP_CHAR *qspToQSPString(char *s)
{
	long len = (long)strlen(s);
	QSP_CHAR *ret = (QSP_CHAR *)malloc((len + 1) * sizeof(QSP_CHAR));
	ret[len] = 0;
	while (--len >= 0)
		ret[len] = QSP_TO_OS_CHAR(s[len]);
	return ret;
}

char *qspQSPToGameString(QSP_CHAR *s, QSP_BOOL isUCS2, QSP_BOOL isCode)
{
	unsigned short *ptr;
	long offset, len = (long)QSP_STRLEN(s);
	char *ret = (char *)malloc((len + 1) * (isUCS2 ? 2 : 1));
	offset = isCode ? -QSP_CODREMOV : 0;
	if (isUCS2)
	{
		ptr = (unsigned short *)ret;
		ptr[len] = 0;
		while (--len >= 0)
			ptr[len] = (unsigned short)(QSP_BTOWC(s[len]) + offset);
	}
	else
	{
		ret[len] = 0;
		while (--len >= 0)
			ret[len] = (char)(QSP_FROM_OS_CHAR(s[len]) + offset);
	}
	return ret;
}

QSP_CHAR *qspGameToQSPString(char *s, QSP_BOOL isUCS2, QSP_BOOL isCoded)
{
	unsigned short *ptr;
	long offset, len = isUCS2 ? qspUCS2StrLen(s) : (long)strlen(s);
	QSP_CHAR *ret = (QSP_CHAR *)malloc((len + 1) * sizeof(QSP_CHAR));
	offset = isCoded ? QSP_CODREMOV : 0;
	ret[len] = 0;
	if (isUCS2)
	{
		ptr = (unsigned short *)s;
		while (--len >= 0)
			ret[len] = QSP_WCTOB((wchar_t)(ptr[len] + offset));
	}
	else
	{
		while (--len >= 0)
			ret[len] = QSP_TO_OS_CHAR((char)(s[len] + offset));
	}
	return ret;
}

long qspSplitGameStr(char *str, QSP_BOOL isUCS2, QSP_CHAR *delim, char ***res)
{
	char *delimStr, *newStr, **ret, *found, *curPos = str;
	long charSize, delimSize, allocChars, count = 0, bufSize = 8;
	charSize = isUCS2 ? 2 : 1;
	delimSize = (long)QSP_STRLEN(delim) * charSize;
	delimStr = qspQSPToGameString(delim, isUCS2, QSP_FALSE);
	found = isUCS2 ? qspUCS2StrStr(str, delimStr) : strstr(str, delimStr);
	ret = (char **)malloc(bufSize * sizeof(char *));
	while (found)
	{
		allocChars = (long)(found - curPos);
		newStr = (char *)malloc(allocChars + charSize);
		memcpy(newStr, curPos, allocChars);
		if (isUCS2)
			*((unsigned short *)(newStr + allocChars)) = 0;
		else
			newStr[allocChars] = 0;
		if (++count > bufSize)
		{
			bufSize <<= 1;
			ret = (char **)realloc(ret, bufSize * sizeof(char *));
		}
		ret[count - 1] = newStr;
		curPos = found + delimSize;
		found = isUCS2 ? qspUCS2StrStr(curPos, delimStr) : strstr(curPos, delimStr);
	}
	free(delimStr);
	allocChars = isUCS2 ? (qspUCS2StrLen(curPos) + 1) * charSize : (long)strlen(curPos) + 1;
	newStr = (char *)malloc(allocChars);
	memcpy(newStr, curPos, allocChars);
	if (++count > bufSize)
		ret = (char **)realloc(ret, count * sizeof(char *));
	ret[count - 1] = newStr;
	*res = ret;
	return count;
}

long qspAddGameText(char **dest, char *val, QSP_BOOL isUCS2, long destLen, long valLen, QSP_BOOL isCreate)
{
	char *destPtr;
	unsigned short *destUCS2, *valUCS2;
	long ret, charSize = isUCS2 ? 2 : 1;
	if (valLen < 0) valLen = isUCS2 ? qspUCS2StrLen(val) : (long)strlen(val);
	if (!isCreate && *dest)
	{
		if (destLen < 0) destLen = isUCS2 ? qspUCS2StrLen(*dest) : (long)strlen(*dest);
		ret = destLen + valLen;
		destPtr = (char *)realloc(*dest, (ret + 1) * charSize);
		*dest = destPtr;
		destPtr += destLen * charSize;
	}
	else
	{
		ret = valLen;
		destPtr = (char *)malloc((ret + 1) * charSize);
		*dest = destPtr;
	}
	if (isUCS2)
	{
		valUCS2 = (unsigned short *)val;
		destUCS2 = (unsigned short *)destPtr;
		while (valLen && (*destUCS2++ = *valUCS2++)) --valLen;
		*destUCS2 = 0;
	}
	else
	{
		strncpy(destPtr, val, valLen);
		destPtr[valLen] = 0;
	}
	return ret;
}

long qspGameCodeWriteIntVal(char **s, long len, long val, QSP_BOOL isUCS2, QSP_BOOL isCode)
{
	char *temp;
	QSP_CHAR buf[12];
	qspNumToStr(buf, val);
	temp = qspQSPToGameString(buf, isUCS2, isCode);
	len = qspAddGameText(s, temp, isUCS2, len, -1, QSP_FALSE);
	free(temp);
	temp = qspQSPToGameString(QSP_STRSDELIM, isUCS2, QSP_FALSE);
	len = qspAddGameText(s, temp, isUCS2, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
	free(temp);
	return len;
}

long qspGameCodeWriteVal(char **s, long len, QSP_CHAR *val, QSP_BOOL isUCS2, QSP_BOOL isCode)
{
	char *temp;
	if (val)
	{
		temp = qspQSPToGameString(val, isUCS2, isCode);
		len = qspAddGameText(s, temp, isUCS2, len, -1, QSP_FALSE);
		free(temp);
	}
	temp = qspQSPToGameString(QSP_STRSDELIM, isUCS2, QSP_FALSE);
	len = qspAddGameText(s, temp, isUCS2, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
	free(temp);
	return len;
}
