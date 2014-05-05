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

static int qspProcessPreformattedStrings(QSPString data, QSPLineOfCode **strs);
static int qspProcessEOLExtensions(QSPLineOfCode *, int, QSPLineOfCode **);

void qspFreePrepLines(QSPLineOfCode *strs, int count)
{
	if (strs)
	{
		while (--count >= 0)
		{
			free(strs[count].Str.Str);
			if (strs[count].Label.Str) free(strs[count].Label.Str);
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
			line->Str = qspGetNewText(src[start].Str);
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
			if (src[start].Label.Str)
				line->Label = qspGetNewText(src[start].Label);
			else
				line->Label = qspNullString;
			++line;
			++start;
		}
	}
	else
		*dest = 0;
}

QSPString qspJoinPrepLines(QSPLineOfCode *s, int count, QSPString delim)
{
	int i, itemLen, txtLen = 0, txtRealLen = 0, bufSize = 256, delimLen = qspStrLen(delim);
	QSP_CHAR *txt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	for (i = 0; i < count; ++i)
	{
		itemLen = qspStrLen(s[i].Str);
		if ((txtLen += itemLen) >= bufSize)
		{
			bufSize = txtLen + 128;
			txt = (QSP_CHAR *)realloc(txt, bufSize * sizeof(QSP_CHAR));
		}
		memcpy(txt + txtRealLen, s[i].Str.Str, itemLen * sizeof(QSP_CHAR));
		if (i == count - 1) break;
		txtRealLen = txtLen;
		if ((txtLen += delimLen) >= bufSize)
		{
			bufSize = txtLen + 128;
			txt = (QSP_CHAR *)realloc(txt, bufSize * sizeof(QSP_CHAR));
		}
		memcpy(txt + txtRealLen, delim.Str, delimLen * sizeof(QSP_CHAR));
		txtRealLen = txtLen;
	}
	return qspStringFromLen(txt, txtLen);
}

static int qspProcessPreformattedStrings(QSPString data, QSPLineOfCode **strs)
{
	QSPLineOfCode *ret, *line;
	QSP_BOOL isNewLine;
	QSP_CHAR *str, *pos, quot = 0;
	QSPString strsDelim;
	int lineNum = 0, lastLineNum = 0, count = 0, quotsCount = 0, strLen = 0, bufSize = 8, strBufSize = 256;
	str = (QSP_CHAR *)malloc(strBufSize * sizeof(QSP_CHAR));
	ret = (QSPLineOfCode *)malloc(bufSize * sizeof(QSPLineOfCode));
	strsDelim = QSP_STATIC_STR(QSP_STRSDELIM);
	pos = data.Str;
	while (pos < data.End)
	{
		isNewLine = (qspStrsNComp(data, strsDelim, QSP_STATIC_LEN(QSP_STRSDELIM)) == 0);
		if (isNewLine) ++lineNum;
		if (quotsCount || quot || !isNewLine)
		{
			if (++strLen > strBufSize)
			{
				strBufSize += 256;
				str = (QSP_CHAR *)realloc(str, strBufSize * sizeof(QSP_CHAR));
			}
			str[strLen - 1] = *pos;
			if (quot)
			{
				if (*pos == quot)
				{
					if (++pos < data.End && *pos == quot)
					{
						if (++strLen > strBufSize)
						{
							strBufSize += 256;
							str = (QSP_CHAR *)realloc(str, strBufSize * sizeof(QSP_CHAR));
						}
						str[strLen - 1] = *pos++;
					}
					else
						quot = 0;
				}
				else
					++pos;
			}
			else
			{
				if (*pos == QSP_LQUOT[0])
					++quotsCount;
				else if (*pos == QSP_RQUOT[0])
				{
					if (quotsCount) --quotsCount;
				}
				else if (qspIsInList(QSP_QUOTS, *pos))
					quot = *pos;
				++pos;
			}
		}
		else
		{
			if (++count > bufSize)
			{
				bufSize += 16;
				ret = (QSPLineOfCode *)realloc(ret, bufSize * sizeof(QSPLineOfCode));
			}
			line = ret + count - 1;
			line->Str = qspGetNewText(qspDelSpc(qspStringFromLen(str, strLen)));
			line->LineNum = lastLineNum;
			line->Label = qspNullString;
			line->Stats = 0;
			lastLineNum = lineNum;
			strLen = 0;
			pos += QSP_STATIC_LEN(QSP_STRSDELIM);
		}
		data.Str = pos;
	}
	if (++count > bufSize)
		ret = (QSPLineOfCode *)realloc(ret, count * sizeof(QSPLineOfCode));
	line = ret + count - 1;
	line->Str = qspGetNewText(qspDelSpc(qspStringFromLen(str, strLen)));
	line->LineNum = lastLineNum;
	line->Label = qspNullString;
	line->Stats = 0;
	free(str);
	*strs = ret;
	return count;
}

static int qspProcessEOLExtensions(QSPLineOfCode *s, int count, QSPLineOfCode **strs)
{
	QSPLineOfCode *ret;
	QSPString str, eol, eolExt;
	int len, lastNum = 0, i = 0, bufSize = 8, newCount = 0;
	ret = (QSPLineOfCode *)malloc(bufSize * sizeof(QSPLineOfCode));
	eolExt = QSP_STATIC_STR(QSP_PREEOLEXT QSP_EOLEXT);
	while (i < count)
	{
		qspAddText(&str, s[i].Str, QSP_TRUE);
		len = qspStrLen(str);
		if (len >= QSP_STATIC_LEN(QSP_PREEOLEXT QSP_EOLEXT))
		{
			eol = str;
			eol.Str += len - QSP_STATIC_LEN(QSP_PREEOLEXT QSP_EOLEXT);
			while (!qspStrsComp(eol, eolExt))
			{
				if (++i == count) break;
				str.End -= QSP_STATIC_LEN(QSP_EOLEXT);
				qspAddText(&str, s[i].Str, QSP_FALSE);
				len = qspStrLen(str);
				eol = str;
				eol.Str += len - QSP_STATIC_LEN(QSP_PREEOLEXT QSP_EOLEXT);
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

int qspPreprocessData(QSPString data, QSPLineOfCode **strs)
{
	QSPLineOfCode *s;
	int res, count = qspProcessPreformattedStrings(data, &s);
	res = qspProcessEOLExtensions(s, count, strs);
	qspFreePrepLines(s, count);
	return res;
}
