/* Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include "text.h"

static QSP_CHAR *qspStrEnd(QSP_CHAR *);

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

QSP_BOOL qspIsInList(QSP_CHAR *list, QSP_CHAR ch)
{
	while (*list)
		if (*list++ == ch) return QSP_TRUE;
	return QSP_FALSE;
}

QSP_CHAR *qspSkipSpaces(QSP_CHAR *s)
{
	while (qspIsInList(QSP_SPACES, *s)) ++s;
	return s;
}

static QSP_CHAR *qspStrEnd(QSP_CHAR *s)
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

QSP_BOOL qspIsEqual(QSP_CHAR *str1, QSP_CHAR *str2, long maxLen)
{
	long delta = 0;
	while (maxLen-- && !(delta = (long)(*str1 - *str2)) && *str2)
		++str1, ++str2;
	return (delta == 0);
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
