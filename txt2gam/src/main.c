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

long qspGetLocs(QSP_CHAR *data, QSP_CHAR locStart, QSP_CHAR locEnd, QSP_BOOL isFill)
{
	QSP_CHAR *locCode, *line, *pos, quot = 0;
	long bufSize, codeLen, curLoc = 0;
	QSP_BOOL isInLoc = QSP_FALSE;
	while (*data)
	{
		if (isInLoc)
		{
			if (!quot && qspIsEqual(data, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM)) &&
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
				if (isFill)
				{
					if (++codeLen >= bufSize)
					{
						bufSize <<= 1;
						locCode = (QSP_CHAR *)realloc(locCode, bufSize * sizeof(QSP_CHAR));
					}
					locCode[codeLen - 1] = *data;
				}
				if (quot)
				{
					if (*data == quot)
					{
						if (*(data + 1) == quot)
						{
							if (isFill)
							{
								if (++codeLen >= bufSize)
								{
									bufSize <<= 1;
									locCode = (QSP_CHAR *)realloc(locCode, bufSize * sizeof(QSP_CHAR));
								}
								locCode[codeLen - 1] = *data;
							}
							++data;
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
					codeLen = 0;
					bufSize = 512;
					locCode = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
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

QSP_BOOL qspOpenQuestFromText(char *file, QSP_BOOL isUCS2, QSP_CHAR locStart, QSP_CHAR locEnd)
{
	long fileSize, locsCount;
	char *buf, *resBuf;
	QSP_CHAR *data;
	FILE *f;
	/* Loading file's contents */
	if (!(f = fopen(file, "rb"))) return QSP_FALSE;
	fseek(f, 0, SEEK_END);
	fileSize = ftell(f);
	buf = (char *)malloc(fileSize + 1);
	fseek(f, 0, SEEK_SET);
	fread(buf, 1, fileSize, f);
	fclose(f);
	buf[fileSize] = 0;
	resBuf = buf;
	if (isUCS2 && (unsigned char)resBuf[0] == 0xFF && (unsigned char)resBuf[1] == 0xFE) resBuf += 2;
	data = qspGameToQSPString(resBuf, isUCS2, QSP_FALSE);
	free(buf);
	locsCount = qspGetLocs(data, locStart, locEnd, QSP_FALSE);
	qspCreateWorld(locsCount);
	qspGetLocs(data, locStart, locEnd, QSP_TRUE);
	free(data);
	return QSP_TRUE;
}

QSP_BOOL qspSaveQuest(char *file, QSP_BOOL isOldFormat, QSP_BOOL isUCS2, QSP_CHAR *passwd)
{
	long i, j, len;
	char *buf;
	FILE *f;
	if (!(f = fopen(file, "wb"))) return QSP_FALSE;
	buf = 0;
	if (isOldFormat)
	{
		len = qspGameCodeWriteIntVal(&buf, 0, qspLocsCount, isUCS2, QSP_FALSE);
		len = qspGameCodeWriteVal(&buf, len, passwd, isUCS2, QSP_TRUE);
		len = qspGameCodeWriteVal(&buf, len, QSP_VER, isUCS2, QSP_FALSE);
		for (i = 0; i < 27; ++i) len = qspGameCodeWriteVal(&buf, len, 0, isUCS2, QSP_FALSE);
	}
	else
	{
		len = qspGameCodeWriteVal(&buf, 0, QSP_GAMEID, isUCS2, QSP_FALSE);
		len = qspGameCodeWriteVal(&buf, len, QSP_VER, isUCS2, QSP_FALSE);
		len = qspGameCodeWriteVal(&buf, len, passwd, isUCS2, QSP_TRUE);
		len = qspGameCodeWriteIntVal(&buf, len, qspLocsCount, isUCS2, QSP_TRUE);
	}
	for (i = 0; i < qspLocsCount; ++i)
	{
		len = qspGameCodeWriteVal(&buf, len, qspLocs[i].Name, isUCS2, QSP_TRUE);
		len = qspGameCodeWriteVal(&buf, len, 0, isUCS2, QSP_FALSE);
		len = qspGameCodeWriteVal(&buf, len, qspLocs[i].OnVisit, isUCS2, QSP_TRUE);
		if (isOldFormat)
			for (j = 0; j < 40; ++j) len = qspGameCodeWriteVal(&buf, len, 0, isUCS2, QSP_FALSE);
		else
			len = qspGameCodeWriteIntVal(&buf, len, 0, isUCS2, QSP_TRUE);
	}
	fwrite(buf, isUCS2 ? 2 : 1, len, f);
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
	printf("  txt2gam [txt file] [gam file] [options]\n");
	printf("Options:\n");
	printf("  u, U - Unicode (UCS-2 / UTF-16) mode, default is ANSI mode\n");
	printf("  o, O - Save game in old format, default is new format\n");
	printf("  s[char], S[char] - 'Begin of loc' character, default is '%c'\n", QSP_FROM_OS_CHAR(QSP_STARTLOC[0]));
	printf("  e[char], E[char] - 'End of loc' character, default is '%c'\n", QSP_FROM_OS_CHAR(QSP_ENDLOC[0]));
	temp = qspFromQSPString(QSP_PASSWD);
	printf("  p[pass], P[pass] - Password, default is '%s'\n", temp);
	free(temp);
	printf("Examples:\n");
	printf("  txt2gam file.txt gamefile.gam pMyPassword\n");
	printf("  txt2gam file.txt gamefile.gam\n");
	printf("  txt2gam file.txt gamefile.gam u\n");
	printf("  txt2gam file.txt gamefile.gam o pMyPassword\n");
	printf("  txt2gam file.txt gamefile.gam o e@ pMyPassword\n");
	printf("  txt2gam file.txt gamefile.gam o \"pMy Password\"\n");
	printf("  txt2gam file.txt gamefile.gam u o \"pMy Password\"\n");
	printf("  txt2gam file.txt gamefile.gam o\n");
	printf("  txt2gam file.txt gamefile.gam o e@\n");
	printf("  txt2gam file.txt gamefile.gam s@ e~\n");
	printf("  txt2gam file.txt gamefile.gam s@ E~ o\n");
}

int main(int argc, char **argv)
{
	long i;
	QSP_BOOL isFreePass, isOldFormat, isUCS2, isErr;
	QSP_CHAR *passwd, ch, locStart, locEnd;
	setlocale(LC_ALL, QSP_LOCALE);
	if (argc < 3)
	{
		ShowHelp();
		return 0;
	}
	isOldFormat = QSP_FALSE;
	isUCS2 = QSP_FALSE;
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
		case 'u': case 'U':
			isUCS2 = QSP_TRUE;
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
		}
	}
	qspLocs = 0;
	qspLocsCount = 0;
	if (isErr = !qspOpenQuestFromText(argv[1], isUCS2, locStart, locEnd))
		printf("Loading game failed!\n");
	else
		if (isErr = !qspSaveQuest(argv[2], isOldFormat, isUCS2, passwd))
			printf("Saving game failed!\n");
	qspCreateWorld(0);
	if (isFreePass) free(passwd);
	return (isErr == QSP_TRUE);
}
