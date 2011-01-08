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

#include "codetools.h"
#include "statements.h"
#include "text.h"

static int qspProcessPreformattedStrings(QSP_CHAR *, QSPLineOfCode **);
static int qspProcessEOLExtensions(QSPLineOfCode *, int, QSPLineOfCode **);

void qspFreePrepLines(QSPLineOfCode *strs, int count)
{
	if (strs)
	{
		while (--count >= 0)
		{
			free(strs[count].Str);
			if (strs[count].Label) free(strs[count].Label);
			if (strs[count].Stats) free(strs[count].Stats);
		}
		free(strs);
	}
}

void qspCopyPrepLines(QSPLineOfCode **dest, QSPLineOfCode *src, int start, int end)
{
	QSPLineOfCode *line;
	int i, count = end - start;
	if (src && count)
	{
		*dest = (QSPLineOfCode *)malloc(count * sizeof(QSPLineOfCode));
		line = *dest;
		while (start < end)
		{
			line->Str = qspGetNewText(src[start].Str, -1);
			line->LineNum = src[start].LineNum;
			count = line->StatsCount = src[start].StatsCount;
			if (count)
			{
				line->Stats = (QSPCachedStat *)malloc(count * sizeof(QSPCachedStat));
				for (i = 0; i < count; ++i)
				{
					line->Stats[i].Stat = src[start].Stats[i].Stat;
					line->Stats[i].EndPos = src[start].Stats[i].EndPos;
					line->Stats[i].ParamPos = src[start].Stats[i].ParamPos;
				}
			}
			else
				line->Stats = 0;
			line->IsMultiline = src[start].IsMultiline;
			if (src[start].Label)
				line->Label = qspGetNewText(src[start].Label, -1);
			else
				line->Label = 0;
			++line;
			++start;
		}
	}
	else
		*dest = 0;
}

QSP_CHAR *qspJoinPrepLines(QSPLineOfCode *s, int count, QSP_CHAR *delim)
{
	int i, txtLen = 0, txtRealLen = 0, bufSize = 256, lastIndex = count - 1, delimLen = qspStrLen(delim);
	QSP_CHAR *txt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	*txt = 0;
	for (i = 0; i < count; ++i)
	{
		if ((txtLen += qspStrLen(s[i].Str)) >= bufSize)
		{
			bufSize = txtLen + 128;
			txt = (QSP_CHAR *)realloc(txt, bufSize * sizeof(QSP_CHAR));
		}
		qspStrCopy(txt + txtRealLen, s[i].Str);
		if (i == lastIndex) break;
		txtRealLen = txtLen;
		if ((txtLen += delimLen) >= bufSize)
		{
			bufSize = txtLen + 128;
			txt = (QSP_CHAR *)realloc(txt, bufSize * sizeof(QSP_CHAR));
		}
		qspStrCopy(txt + txtRealLen, delim);
		txtRealLen = txtLen;
	}
	return txt;
}

static int qspProcessPreformattedStrings(QSP_CHAR *data, QSPLineOfCode **strs)
{
	QSPLineOfCode *ret, *line;
	QSP_BOOL isNewLine;
	QSP_CHAR *str, quot = 0;
	int lineNum = 0, lastLineNum = 0, count = 0, quotsCount = 0, strLen = 0, bufSize = 8, strBufSize = 256;
	str = (QSP_CHAR *)malloc(strBufSize * sizeof(QSP_CHAR));
	ret = (QSPLineOfCode *)malloc(bufSize * sizeof(QSPLineOfCode));
	while (*data)
	{
		isNewLine = (qspStrsNComp(data, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM)) == 0);
		if (isNewLine) ++lineNum;
		if (quotsCount || quot || !isNewLine)
		{
			if (++strLen >= strBufSize)
			{
				strBufSize += 256;
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
							strBufSize += 256;
							str = (QSP_CHAR *)realloc(str, strBufSize * sizeof(QSP_CHAR));
						}
						str[strLen - 1] = *data++;
					}
					else
						quot = 0;
				}
			}
			else
			{
				if (*data == QSP_LQUOT[0])
					++quotsCount;
				else if (*data == QSP_RQUOT[0])
				{
					if (quotsCount) --quotsCount;
				}
				else if (qspIsInList(QSP_QUOTS, *data))
					quot = *data;
			}
			++data;
		}
		else
		{
			str[strLen] = 0;
			if (++count > bufSize)
			{
				bufSize += 16;
				ret = (QSPLineOfCode *)realloc(ret, bufSize * sizeof(QSPLineOfCode));
			}
			line = ret + count - 1;
			line->Str = qspDelSpc(str);
			line->LineNum = lastLineNum;
			line->Label = 0;
			line->Stats = 0;
			lastLineNum = lineNum;
			strLen = 0;
			data += QSP_LEN(QSP_STRSDELIM);
		}
	}
	str[strLen] = 0;
	if (++count > bufSize)
		ret = (QSPLineOfCode *)realloc(ret, count * sizeof(QSPLineOfCode));
	line = ret + count - 1;
	line->Str = qspDelSpc(str);
	line->LineNum = lastLineNum;
	line->Label = 0;
	line->Stats = 0;
	free(str);
	*strs = ret;
	return count;
}

static int qspProcessEOLExtensions(QSPLineOfCode *s, int count, QSPLineOfCode **strs)
{
	QSPLineOfCode *ret;
	QSP_CHAR *str;
	int len, lastNum = 0, i = 0, bufSize = 8, newCount = 0;
	ret = (QSPLineOfCode *)malloc(bufSize * sizeof(QSPLineOfCode));
	while (i < count)
	{
		len = qspAddText(&str, s[i].Str, 0, -1, QSP_TRUE);
		if (len >= QSP_LEN(QSP_PREEOLEXT QSP_EOLEXT))
		{
			while (!qspStrsComp(str + len - QSP_LEN(QSP_PREEOLEXT QSP_EOLEXT), QSP_PREEOLEXT QSP_EOLEXT))
			{
				if (++i == count) break;
				len = qspAddText(&str, s[i].Str, len - QSP_LEN(QSP_EOLEXT), -1, QSP_FALSE);
			}
		}
		if (++newCount > bufSize)
		{
			bufSize += 16;
			ret = (QSPLineOfCode *)realloc(ret, bufSize * sizeof(QSPLineOfCode));
		}
		qspInitLineOfCode(ret + newCount - 1, str, lastNum);
		++i;
		lastNum = s[i].LineNum;
	}
	*strs = ret;
	return newCount;
}

int qspPreprocessData(QSP_CHAR *data, QSPLineOfCode **strs)
{
	QSPLineOfCode *s;
	int res, count = qspProcessPreformattedStrings(data, &s);
	res = qspProcessEOLExtensions(s, count, strs);
	qspFreePrepLines(s, count);
	return res;
}
