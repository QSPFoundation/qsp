/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include "coding.h"
#include "text.h"

wchar_t qspCP1251ToUCS2LETable[] =
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

unsigned int qspCP1251ToUTF8Table[] =
{
	0x00D082, 0x00D083, 0xE2809A, 0x00D193, 0xE2809E, 0xE280A6, 0xE280A0, 0xE280A1,
	0xE282AC, 0xE280B0, 0x00D089, 0xE280B9, 0x00D08A, 0x00D08C, 0x00D08B, 0x00D08F,
	0x00D192, 0xE28098, 0xE28099, 0xE2809C, 0xE2809D, 0xE280A2, 0xE28093, 0xE28094,
	0x000020, 0xE284A2, 0x00D199, 0xE280BA, 0x00D19A, 0x00D19C, 0x00D19B, 0x00D19F,
	0x00C2A0, 0x00D08E, 0x00D19E, 0x00D088, 0x00C2A4, 0x00D290, 0x00C2A6, 0x00C2A7,
	0x00D081, 0x00C2A9, 0x00D084, 0x00C2AB, 0x00C2AC, 0x00C2AD, 0x00C2AE, 0x00D087,
	0x00C2B0, 0x00C2B1, 0x00D086, 0x00D196, 0x00D291, 0x00C2B5, 0x00C2B6, 0x00C2B7,
	0x00D191, 0xE28496, 0x00D194, 0x00C2BB, 0x00D198, 0x00D085, 0x00D195, 0x00D197,
	0x00D090, 0x00D091, 0x00D092, 0x00D093, 0x00D094, 0x00D095, 0x00D096, 0x00D097,
	0x00D098, 0x00D099, 0x00D09A, 0x00D09B, 0x00D09C, 0x00D09D, 0x00D09E, 0x00D09F,
	0x00D0A0, 0x00D0A1, 0x00D0A2, 0x00D0A3, 0x00D0A4, 0x00D0A5, 0x00D0A6, 0x00D0A7,
	0x00D0A8, 0x00D0A9, 0x00D0AA, 0x00D0AB, 0x00D0AC, 0x00D0AD, 0x00D0AE, 0x00D0AF,
	0x00D0B0, 0x00D0B1, 0x00D0B2, 0x00D0B3, 0x00D0B4, 0x00D0B5, 0x00D0B6, 0x00D0B7,
	0x00D0B8, 0x00D0B9, 0x00D0BA, 0x00D0BB, 0x00D0BC, 0x00D0BD, 0x00D0BE, 0x00D0BF,
	0x00D180, 0x00D181, 0x00D182, 0x00D183, 0x00D184, 0x00D185, 0x00D186, 0x00D187,
	0x00D188, 0x00D189, 0x00D18A, 0x00D18B, 0x00D18C, 0x00D18D, 0x00D18E, 0x00D18F
};

static int qspUCS2StrLen(char *);
static char *qspUCS2StrStr(char *, char *);
static int qspAddGameText(char **, char *, QSP_BOOL, int, int, QSP_BOOL);

static int qspUCS2StrLen(char *str)
{
	unsigned short *ptr = (unsigned short *)str;
	while (*ptr) ++ptr;
	return (int)(ptr - (unsigned short *)str);
}

static char *qspUCS2StrStr(char *str, char *subStr)
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

wchar_t qspDirectConvertUCS2LE(char ch, wchar_t *table)
{
	unsigned char ch2 = (unsigned char)ch;
	return (ch2 >= 0x80 ? table[ch2 - 0x80] : ch);
}

char qspReverseConvertUCS2LE(wchar_t ch, wchar_t *table)
{
	int i;
	if (ch < 0x80) return (char)ch;
	for (i = 127; i >= 0; --i)
		if (table[i] == ch) return (char)(i + 0x80);
	return 0x20;
}

char qspReverseConvertUTF8(unsigned int codePoint, unsigned int *table)
{
    int i;
    if (codePoint < 0x80) return (char)codePoint;
    for (i = 127; i >= 0; --i)
        if (table[i] == codePoint) return (char)(i + 0x80);
    return 0x20;
}

char *qspFromQSPString(QSP_CHAR *s)
{
	int len = (int)QSP_WCSTOMBSLEN(s) + 1;
	char *ret = (char *)malloc(len);
	QSP_WCSTOMBS(ret, s, len);
	return ret;
}

QSP_CHAR *qspToQSPString(char *s)
{
	int len = (int)QSP_MBSTOWCSLEN(s) + 1;
	QSP_CHAR *ret = (QSP_CHAR *)malloc(len * sizeof(QSP_CHAR));
	QSP_MBSTOWCS(ret, s, len);
	return ret;
}

char *qspQSPToGameString(QSP_CHAR *s, QSP_BOOL isUCS2, QSP_BOOL isCode)
{
	unsigned short uCh, *ptr;
	int len = (int)QSP_STRLEN(s);
	char ch, *ret = (char *)malloc((len + 1) * (isUCS2 ? 2 : 1));
	if (isUCS2)
	{
		ptr = (unsigned short *)ret;
		ptr[len] = 0;
		if (isCode)
		{
			while (--len >= 0)
			{
				uCh = QSP_BTOUCS2(s[len]);
				if (uCh == QSP_CODREMOV)
					uCh = (unsigned short)-QSP_CODREMOV;
				else
					uCh -= QSP_CODREMOV;
				ptr[len] = uCh;
			}
		}
		else
		{
			while (--len >= 0)
				ptr[len] = QSP_BTOUCS2(s[len]);
		}
	}
	else
	{
		ret[len] = 0;
		if (isCode)
		{
			while (--len >= 0)
			{
				ch = QSP_FROM_OS_CHAR(s[len]);
				if (ch == QSP_CODREMOV)
					ch = -QSP_CODREMOV;
				else
					ch -= QSP_CODREMOV;
				ret[len] = ch;
			}
		}
		else
		{
			while (--len >= 0)
				ret[len] = QSP_FROM_OS_CHAR(s[len]);
		}
	}
	return ret;
}

QSP_CHAR *qspGameToQSPString(char *s, encoding_t encoding)
{
	char ch, *si;
	unsigned short *ptr;
	unsigned int codePoint;
	int len = encoding == UCS2LE ? qspUCS2StrLen(s) : (int)strlen(s);
	QSP_CHAR *reti, *ret = (QSP_CHAR *)malloc((len + 1) * sizeof(QSP_CHAR));
	ret[len] = 0;
	switch (encoding)
	{
	case UCS2LE:
		ptr = (unsigned short *)s;
		while (--len >= 0)
			ret[len] = QSP_UCS2TOB(ptr[len]);
		break;
	case UTF8:
		for (si = s, reti = ret; *si; ++si)
		{
			switch (QSP_BYTE(*si))
			{
			case 0xd0:
			case 0xd1:
			case 0xc2:
				codePoint = QSP_DWORD(0, 0, si[0], si[1]);
				*reti = QSP_TO_OS_CHAR(QSP_UTF8TOB(codePoint));
				si++;
				break;
			case 0xe2:
				codePoint = QSP_DWORD(0, si[0], si[1], si[2]);
				*reti = QSP_TO_OS_CHAR(QSP_UTF8TOB(codePoint));
				si += 2;
				break;
			default:
				*reti = QSP_TO_OS_CHAR(*si);
				break;
			}
			reti++;
		}
		break;
	default:
		while (--len >= 0)
			ret[len] = QSP_TO_OS_CHAR(s[len]);
		break;
	}
	return ret;
}

int qspSplitGameStr(char *str, QSP_BOOL isUCS2, QSP_CHAR *delim, char ***res)
{
	char *delimStr, *newStr, **ret, *found, *curPos = str;
	int charSize, delimSize, allocChars, count = 0, bufSize = 8;
	charSize = (isUCS2 ? 2 : 1);
	delimSize = (int)QSP_STRLEN(delim) * charSize;
	delimStr = qspQSPToGameString(delim, isUCS2, QSP_FALSE);
	found = (isUCS2 ? qspUCS2StrStr(str, delimStr) : strstr(str, delimStr));
	ret = (char **)malloc(bufSize * sizeof(char *));
	while (found)
	{
		allocChars = (int)(found - curPos);
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
		found = (isUCS2 ? qspUCS2StrStr(curPos, delimStr) : strstr(curPos, delimStr));
	}
	free(delimStr);
	allocChars = (isUCS2 ? (qspUCS2StrLen(curPos) + 1) * charSize : (int)strlen(curPos) + 1);
	newStr = (char *)malloc(allocChars);
	memcpy(newStr, curPos, allocChars);
	if (++count > bufSize)
		ret = (char **)realloc(ret, count * sizeof(char *));
	ret[count - 1] = newStr;
	*res = ret;
	return count;
}

static int qspAddGameText(char **dest, char *val, QSP_BOOL isUCS2, int destLen, int valLen, QSP_BOOL isCreate)
{
	char *destPtr;
	unsigned short *destUCS2, *valUCS2;
	int ret, charSize = (isUCS2 ? 2 : 1);
	if (valLen < 0) valLen = (isUCS2 ? qspUCS2StrLen(val) : (int)strlen(val));
	if (!isCreate && *dest)
	{
		if (destLen < 0) destLen = (isUCS2 ? qspUCS2StrLen(*dest) : (int)strlen(*dest));
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

int qspGameCodeWriteIntVal(char **s, int len, int val, QSP_BOOL isUCS2, QSP_BOOL isCode)
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

int qspGameCodeWriteVal(char **s, int len, QSP_CHAR *val, QSP_BOOL isUCS2, QSP_BOOL isCode)
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
