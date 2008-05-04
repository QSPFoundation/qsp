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

long qspProcessPreformattedStrings(QSP_CHAR *data, QSP_CHAR ***strs)
{
	QSP_CHAR **ret, *str, quot = 0;
	long count = 0, strLen = 0, bufSize = 8, strBufSize = 256;
	str = (QSP_CHAR *)malloc(strBufSize * sizeof(QSP_CHAR));
	ret = (QSP_CHAR **)malloc(bufSize * sizeof(QSP_CHAR *));
	while (*data)
	{
		if (!quot && qspIsEqual(data, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM)))
		{
			str[strLen] = 0;
			if (++count > bufSize)
			{
				bufSize <<= 1;
				ret = (QSP_CHAR **)realloc(ret, bufSize * sizeof(QSP_CHAR *));
			}
			ret[count - 1] = qspDelSpc(str);
			strLen = 0;
			data += QSP_LEN(QSP_STRSDELIM);
		}
		else
		{
			if (++strLen >= strBufSize)
			{
				strBufSize <<= 1;
				str = (QSP_CHAR *)realloc(str, strBufSize * sizeof(QSP_CHAR));
			}
			str[strLen - 1] = *data;
			if (quot)
			{
				if (*data == quot)
				{
					if (*(data + 1) == quot)
					{
						if (++strLen >= strBufSize)
						{
							strBufSize <<= 1;
							str = (QSP_CHAR *)realloc(str, strBufSize * sizeof(QSP_CHAR));
						}
						str[strLen - 1] = *data++;
					}
					else
						quot = 0;
				}
			}
			else if (qspIsInList(QSP_QUOTS, *data))
				quot = *data;
			++data;
		}
	}
	str[strLen] = 0;
	if (++count > bufSize)
		ret = (QSP_CHAR **)realloc(ret, count * sizeof(QSP_CHAR *));
	ret[count - 1] = qspDelSpc(str);
	free(str);
	*strs = ret;
	return count;
}

long qspProcessEOLExtensions(QSP_CHAR **s, long count, QSP_CHAR ***strs)
{
	QSP_CHAR **ret, *str;
	long len, i = 0, bufSize = 8, newCount = 0;
	ret = (QSP_CHAR **)malloc(bufSize * sizeof(QSP_CHAR *));
	while (i < count)
	{
		len = qspAddText(&str, s[i], 0, -1, QSP_TRUE);
		if (len >= QSP_LEN(QSP_EOLEXT))
		{
			while (!QSP_STRCMP(str + len - QSP_LEN(QSP_EOLEXT), QSP_EOLEXT))
			{
				if (++i == count) break;
				len = qspAddText(&str, s[i], len - QSP_LEN(QSP_EOLEXT), -1, QSP_FALSE);
			}
		}
		if (++newCount > bufSize)
		{
			bufSize <<= 1;
			ret = (QSP_CHAR **)realloc(ret, bufSize * sizeof(QSP_CHAR *));
		}
		ret[newCount - 1] = str;
		++i;
	}
	*strs = ret;
	return newCount;
}

long qspPreprocessData(QSP_CHAR *data, QSP_CHAR ***strs)
{
	QSP_CHAR **s;
	long res, count = qspProcessPreformattedStrings(data, &s);
	res = qspProcessEOLExtensions(s, count, strs);
	qspFreeStrs(s, count, QSP_FALSE);
	return res;
}
