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

#include "main.h"
#include "coding.h"
#include "locations.h"
#include "text.h"

static int qspAddCharToBuffer(QSP_CHAR **, QSP_CHAR, int *, int);
static int qspGetLocsStrings(QSP_CHAR *, QSP_CHAR, QSP_CHAR, QSP_BOOL, QSP_CHAR **);
static int qspGetLocs(QSP_CHAR *, QSP_CHAR, QSP_CHAR, QSP_BOOL);
static QSP_BOOL qspLoadFileContents(char *, QSP_CHAR **, QSP_BOOL);
static QSP_BOOL qspExportStrings(char *, char *, QSP_CHAR, QSP_CHAR, QSP_BOOL, QSP_BOOL);
static QSP_BOOL qspOpenQuestFromText(char *, QSP_CHAR, QSP_CHAR, QSP_BOOL);
static QSP_BOOL qspSaveQuest(char *, QSP_BOOL, QSP_BOOL, QSP_CHAR *);

static int qspAddCharToBuffer(QSP_CHAR **curStr, QSP_CHAR ch, int *bufSize, int strLen)
{
	if (++strLen >= *bufSize)
	{
		*bufSize <<= 1;
		*curStr = (QSP_CHAR *)realloc(*curStr, *bufSize * sizeof(QSP_CHAR));
	}
	(*curStr)[strLen - 1] = ch;
	return strLen;
}

static int qspGetLocsStrings(QSP_CHAR *data, QSP_CHAR locStart, QSP_CHAR locEnd, QSP_BOOL isGetQStrings, QSP_CHAR **buf)
{
	QSP_CHAR *name, *curStr, *line, *pos, quot = 0;
	int bufSize = 512, strLen = 0, len = 0, quotsCount = 0;
	QSP_BOOL isAddToString, isInLoc = QSP_FALSE;
	*buf = 0;
	curStr = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	while (*data)
	{
		if (isInLoc)
		{
			if (!quot && !quotsCount && qspIsEqual(data, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM)) &&
				*(line = qspSkipSpaces(data + QSP_LEN(QSP_STRSDELIM))) == locEnd)
			{
				isInLoc = QSP_FALSE;
				pos = QSP_STRSTR(line + 1, QSP_STRSDELIM);
				if (pos)
					data = pos + QSP_LEN(QSP_STRSDELIM);
				else
					break;
			}
			else
			{
				if (quot)
				{
					if (*data == quot)
					{
						if (*(data + 1) == quot)
						{
							if (isGetQStrings && quotsCount)
								strLen = qspAddCharToBuffer(&curStr, *data, &bufSize, strLen);
							++data;
						}
						else
							quot = 0;
					}
					if (quot || (isGetQStrings && quotsCount))
						strLen = qspAddCharToBuffer(&curStr, *data, &bufSize, strLen);
					else
					{
						if (strLen)
						{
							curStr[strLen] = 0;
							len = qspAddText(buf, curStr, len, strLen, QSP_FALSE);
							len = qspAddText(buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
							strLen = 0;
						}
					}
				}
				else
				{
					isAddToString = (quotsCount > 0);
					if (*data == QSP_LQUOT[0])
						++quotsCount;
					else if (*data == QSP_RQUOT[0])
					{
						if (quotsCount)
						{
							--quotsCount;
							if (isGetQStrings && !quotsCount)
							{
								isAddToString = QSP_FALSE;
								if (strLen)
								{
									curStr[strLen] = 0;
									len = qspAddText(buf, curStr, len, strLen, QSP_FALSE);
									len = qspAddText(buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
									strLen = 0;
								}
							}
						}
					}
					else if (qspIsInList(QSP_QUOTS, *data))
						quot = *data;
					if (isGetQStrings && isAddToString)
						strLen = qspAddCharToBuffer(&curStr, *data, &bufSize, strLen);
				}
				++data;
			}
		}
		else
		{
			line = qspSkipSpaces(data);
			pos = QSP_STRSTR(line, QSP_STRSDELIM);
			if (*line == locStart)
			{
				isInLoc = QSP_TRUE;
				++line;
				if (pos)
				{
					*pos = 0;
					name = qspDelSpc(line);
					*pos = QSP_STRSDELIM[0];
				}
				else
					name = qspDelSpc(line);
				len = qspAddText(buf, QSP_FMT("Location: "), len, -1, QSP_FALSE);
				len = qspAddText(buf, name, len, -1, QSP_FALSE);
				free(name);
				len = qspAddText(buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
				if (pos && *qspSkipSpaces(pos + QSP_LEN(QSP_STRSDELIM)) == locEnd)
				{
					data = pos;
					continue;
				}
			}
			if (pos)
				data = pos + QSP_LEN(QSP_STRSDELIM);
			else
				break;
		}
	}
	free(curStr);
	return len;
}

static int qspGetLocs(QSP_CHAR *data, QSP_CHAR locStart, QSP_CHAR locEnd, QSP_BOOL isFill)
{
	char *name;
	QSP_CHAR *locCode, *line, *pos, quot = 0;
	int bufSize, codeLen, quotsCount = 0, curLoc = 0;
	QSP_BOOL isInLoc = QSP_FALSE;
	while (*data)
	{
		if (isInLoc)
		{
			if (!quot && !quotsCount && qspIsEqual(data, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM)) &&
				*(line = qspSkipSpaces(data + QSP_LEN(QSP_STRSDELIM))) == locEnd)
			{
				isInLoc = QSP_FALSE;
				if (isFill)
				{
					locCode[codeLen] = 0;
					qspLocs[curLoc].OnVisit = locCode;
				}
				++curLoc;
				pos = QSP_STRSTR(line + 1, QSP_STRSDELIM);
				if (pos)
					data = pos + QSP_LEN(QSP_STRSDELIM);
				else
					break;
			}
			else
			{
				if (isFill) codeLen = qspAddCharToBuffer(&locCode, *data, &bufSize, codeLen);
				if (quot)
				{
					if (*data == quot)
					{
						if (*(data + 1) == quot)
						{
							if (isFill) codeLen = qspAddCharToBuffer(&locCode, *data, &bufSize, codeLen);
							++data;
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
		}
		else
		{
			line = qspSkipSpaces(data);
			pos = QSP_STRSTR(line, QSP_STRSDELIM);
			if (*line == locStart)
			{
				isInLoc = QSP_TRUE;
				if (isFill)
				{
					++line;
					if (pos)
					{
						*pos = 0;
						qspLocs[curLoc].Name = qspDelSpc(line);
						*pos = QSP_STRSDELIM[0];
					}
					else
						qspLocs[curLoc].Name = qspDelSpc(line);
					name = qspFromQSPString(qspLocs[curLoc].Name);
					printf("Location: %s\n", name);
					free(name);
					codeLen = 0;
					bufSize = 512;
					locCode = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
				}
				if (pos && *qspSkipSpaces(pos + QSP_LEN(QSP_STRSDELIM)) == locEnd)
				{
					data = pos;
					continue;
				}
			}
			if (pos)
				data = pos + QSP_LEN(QSP_STRSDELIM);
			else
				break;
		}
	}
	if (isInLoc)
	{
		if (isFill)
		{
			locCode[codeLen] = 0;
			qspLocs[curLoc].OnVisit = locCode;
		}
		++curLoc;
	}
	return curLoc;
}

static QSP_BOOL qspLoadFileContents(char *file, QSP_CHAR **data, QSP_BOOL isUnicode)
{
	int fileSize;
	char *buf, *resBuf;
	encoding_t encoding;
	FILE *f;
	/* Loading file's contents */
	if (!(f = fopen(file, "rb"))) return QSP_FALSE;
	fseek(f, 0, SEEK_END);
	fileSize = ftell(f);
	buf = (char *)malloc(fileSize + 3);
	fseek(f, 0, SEEK_SET);
	fread(buf, 1, fileSize, f);
	fclose(f);
	buf[fileSize] = buf[fileSize + 1] = buf[fileSize + 2] = 0;
	resBuf = buf;
	if ((unsigned char)resBuf[0] == 0xFF && (unsigned char)resBuf[1] == 0xFE)
	{
		resBuf += 2;
		encoding = UCS2LE;
	}
	else if (resBuf[0] && !resBuf[1])
		encoding = UCS2LE;
	else
		encoding = isUnicode ? UTF8 : CP1251;
	*data = qspGameToQSPString(resBuf, encoding);
	free(buf);
	return QSP_TRUE;
}

static QSP_BOOL qspExportStrings(char *file, char *outFile, QSP_CHAR locStart, QSP_CHAR locEnd, QSP_BOOL isGetQStrings, QSP_BOOL isUnicode)
{
	int len;
	char *buf;
	QSP_CHAR *data, *strs;
	FILE *f;
	if (!qspLoadFileContents(file, &data, isUnicode)) return QSP_FALSE;
	len = qspGetLocsStrings(data, locStart, locEnd, isGetQStrings, &strs);
	free(data);
	if (len)
	{
		if (!(f = fopen(outFile, "wb")))
		{
			free(strs);
			return QSP_FALSE;
		}
		buf = qspQSPToGameString(strs, isUnicode, QSP_FALSE);
		free(strs);
		fwrite(buf, isUnicode ? 2 : 1, len, f);
		free(buf);
		fclose(f);
	}
	return QSP_TRUE;
}

static QSP_BOOL qspOpenQuestFromText(char *file, QSP_CHAR locStart, QSP_CHAR locEnd, QSP_BOOL isUnicode)
{
	int locsCount;
	QSP_CHAR *data;
	if (!qspLoadFileContents(file, &data, isUnicode)) return QSP_FALSE;
	locsCount = qspGetLocs(data, locStart, locEnd, QSP_FALSE);
	qspCreateWorld(locsCount);
	qspGetLocs(data, locStart, locEnd, QSP_TRUE);
	free(data);
	return QSP_TRUE;
}

static QSP_BOOL qspSaveQuest(char *file, QSP_BOOL isOldFormat, QSP_BOOL isUnicode, QSP_CHAR *passwd)
{
	int i, j, len;
	char *buf;
	FILE *f;
	if (!(f = fopen(file, "wb"))) return QSP_FALSE;
	buf = 0;
	if (isOldFormat)
	{
		len = qspGameCodeWriteIntVal(&buf, 0, qspLocsCount, isUnicode, QSP_FALSE);
		len = qspGameCodeWriteVal(&buf, len, passwd, isUnicode, QSP_TRUE);
		len = qspGameCodeWriteVal(&buf, len, QSP_VER, isUnicode, QSP_FALSE);
		for (i = 0; i < 27; ++i) len = qspGameCodeWriteVal(&buf, len, 0, isUnicode, QSP_FALSE);
	}
	else
	{
		len = qspGameCodeWriteVal(&buf, 0, QSP_GAMEID, isUnicode, QSP_FALSE);
		len = qspGameCodeWriteVal(&buf, len, QSP_VER, isUnicode, QSP_FALSE);
		len = qspGameCodeWriteVal(&buf, len, passwd, isUnicode, QSP_TRUE);
		len = qspGameCodeWriteIntVal(&buf, len, qspLocsCount, isUnicode, QSP_TRUE);
	}
	for (i = 0; i < qspLocsCount; ++i)
	{
		len = qspGameCodeWriteVal(&buf, len, qspLocs[i].Name, isUnicode, QSP_TRUE);
		len = qspGameCodeWriteVal(&buf, len, 0, isUnicode, QSP_FALSE);
		len = qspGameCodeWriteVal(&buf, len, qspLocs[i].OnVisit, isUnicode, QSP_TRUE);
		if (isOldFormat)
			for (j = 0; j < 40; ++j) len = qspGameCodeWriteVal(&buf, len, 0, isUnicode, QSP_FALSE);
		else
			len = qspGameCodeWriteIntVal(&buf, len, 0, isUnicode, QSP_TRUE);
	}
	fwrite(buf, isUnicode ? 2 : 1, len, f);
	free(buf);
	fclose(f);
	return QSP_TRUE;
}

void ShowHelp()
{
	char *temp = qspFromQSPString(QSP_VER);
	printf("TXT2GAM utility, ver. %s\n", temp);
	free(temp);
	printf("Using:\n");
	printf("  txt2gam [txt file] [output file] [options]\n");
	printf("Options:\n");
	printf("  a, A - ANSI mode, default is Unicode (UTF-8, UCS-2 / UTF-16) mode\n");
	printf("  o, O - Save game in old format, default is new format\n");
	printf("  s[char], S[char] - 'Begin of loc' character, default is '%c'\n", QSP_FROM_OS_CHAR(QSP_STARTLOC[0]));
	printf("  e[char], E[char] - 'End of loc' character, default is '%c'\n", QSP_FROM_OS_CHAR(QSP_ENDLOC[0]));
	temp = qspFromQSPString(QSP_PASSWD);
	printf("  p[pass], P[pass] - Password, default is '%s'\n", temp);
	free(temp);
	printf("  t, T - Extract strings from text\n");
	printf("  q, Q - Extract q-strings from text\n");
	printf("Examples:\n");
	printf("  txt2gam file.txt gamefile.qsp pMyPassword\n");
	printf("  txt2gam file.txt gamefile.qsp\n");
	printf("  txt2gam file.txt gamefile.qsp u\n");
	printf("  txt2gam file.txt gamefile.qsp o pMyPassword\n");
	printf("  txt2gam file.txt gamefile.qsp o e@ pMyPassword\n");
	printf("  txt2gam file.txt gamefile.qsp o \"pMy Password\"\n");
	printf("  txt2gam file.txt gamefile.qsp u o \"pMy Password\"\n");
	printf("  txt2gam file.txt gamefile.qsp o\n");
	printf("  txt2gam file.txt gamefile.qsp o e@\n");
	printf("  txt2gam file.txt gamefile.qsp s@ e~\n");
	printf("  txt2gam file.txt gamefile.qsp s@ E~ o\n");
	printf("  txt2gam file.txt strsfile.txt t u\n");
}

int main(int argc, char **argv)
{
	int i;
	QSP_BOOL isFreePass, isOldFormat, isUnicode, isErr, isExtractStrs, isGetQStrings;
	QSP_CHAR *passwd, ch, locStart, locEnd;
	setlocale(LC_ALL, QSP_LOCALE);
	if (argc < 3)
	{
		ShowHelp();
		return 0;
	}
	isExtractStrs = QSP_FALSE;
	isGetQStrings = QSP_FALSE;
	isOldFormat = QSP_FALSE;
	isUnicode = QSP_TRUE;
	passwd = QSP_PASSWD;
	isFreePass = QSP_FALSE;
	locStart = QSP_STARTLOC[0];
	locEnd = QSP_ENDLOC[0];
	for (i = 3; i < argc; ++i)
	{
		switch (*argv[i])
		{
		case 'o': case 'O':
			isOldFormat = QSP_TRUE;
			break;
		case 'a': case 'A':
			isUnicode = QSP_FALSE;
			break;
		case 's': case 'S':
		case 'e': case 'E':
		case 'p': case 'P':
			if (argv[i][1])
			{
				switch (*argv[i])
				{
				case 's': case 'S':
					ch = QSP_TO_OS_CHAR(argv[i][1]);
					if (ch != locEnd) locStart = ch;
					break;
				case 'e': case 'E':
					ch = QSP_TO_OS_CHAR(argv[i][1]);
					if (ch != locStart) locEnd = ch;
					break;
				case 'p': case 'P':
					if (isFreePass) free(passwd);
					passwd = qspToQSPString(argv[i] + 1);
					isFreePass = QSP_TRUE;
					break;
				}
			}
			break;
		case 't': case 'T':
			isExtractStrs = QSP_TRUE;
			break;
		case 'q': case 'Q':
			isExtractStrs = QSP_TRUE;
			isGetQStrings = QSP_TRUE;
			break;
		}
	}
	if (isExtractStrs)
	{
		if (isErr = !qspExportStrings(argv[1], argv[2], locStart, locEnd, isGetQStrings, isUnicode))
			printf("Strings extracting failed!\n");
	}
	else
	{
		qspLocs = 0;
		qspLocsCount = 0;
		if (isErr = !qspOpenQuestFromText(argv[1], locStart, locEnd, isUnicode))
			printf("Loading game failed!\n");
		else
			if (isErr = !qspSaveQuest(argv[2], isOldFormat, isUnicode, passwd))
				printf("Saving game failed!\n");
		qspCreateWorld(0);
	}
	if (isFreePass) free(passwd);
	return (isErr == QSP_TRUE);
}
