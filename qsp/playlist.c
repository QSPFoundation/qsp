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

#include "playlist.h"
#include "callbacks.h"
#include "game.h"
#include "text.h"

QSPString qspPLFiles[QSP_MAXPLFILES];
int qspPLFilesCount = 0;

static void qspPlayFile(QSPString s, int volume, QSP_BOOL isAddToPlayList);
static int qspSearchPlayList(QSPString file);

void qspClearPlayList(QSP_BOOL isFirst)
{
	int i;
	if (!isFirst)
	{
		for (i = 0; i < qspPLFilesCount; ++i)
			qspFreeString(qspPLFiles[i]);
	}
	qspPLFilesCount = 0;
}

static void qspPlayFile(QSPString s, int volume, QSP_BOOL isAddToPlayList)
{
	QSPString file;
	QSP_CHAR buf[4];
	if (!qspIsAnyString(s)) return;
	if (volume < 0)
		volume = 0;
	else if (volume > 100)
		volume = 100;
	file = qspGetAbsFromRelPath(s);
	qspCallPlayFile(file, volume);
	qspFreeString(file);
	if (isAddToPlayList)
	{
		if (qspPLFilesCount == QSP_MAXPLFILES)
		{
			qspRefreshPlayList();
			if (qspPLFilesCount == QSP_MAXPLFILES) return;
		}
		qspAddText(&file, s, QSP_TRUE);
		if (volume != 100)
		{
			qspAddText(&file, QSP_STATIC_STR(QSP_PLVOLUMEDELIM), QSP_FALSE);
			qspAddText(&file, qspNumToStr(buf, volume), QSP_FALSE);
		}
		qspPLFiles[qspPLFilesCount++] = file;
	}
}

static int qspSearchPlayList(QSPString file)
{
	QSPString uName, bufName;
	QSP_CHAR *buf;
	int i, bufSize, itemLen, fileLen;
	if (!qspPLFilesCount) return -1;
	fileLen = qspStrLen(file);
	uName = qspGetNewText(file);
	qspUpperStr(&uName);
	bufSize = 32;
	buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	for (i = 0; i < qspPLFilesCount; ++i)
	{
		itemLen = qspStrLen(qspPLFiles[i]);
		if (itemLen >= bufSize)
		{
			bufSize = itemLen + 8;
			buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
		}
		memcpy(buf, qspPLFiles[i].Str, itemLen * sizeof(QSP_CHAR));
		bufName = qspStringFromLen(buf, itemLen);
		qspUpperStr(&bufName);
		if (!qspStrsNComp(bufName, uName, fileLen))
		{
			if (itemLen == fileLen || qspIsInList(QSP_PLVOLUMEDELIM, bufName.Str[fileLen]))
			{
				qspFreeString(uName);
				free(buf);
				return i;
			}
		}
	}
	qspFreeString(uName);
	free(buf);
	return -1;
}

void qspPlayPLFiles()
{
	int i;
	QSP_CHAR *pos;
	for (i = 0; i < qspPLFilesCount; ++i)
	{
		pos = qspStrChar(qspPLFiles[i], QSP_PLVOLUMEDELIM[0]);
		if (pos)
		{
			qspPlayFile(qspStringFromPair(qspPLFiles[i].Str, pos),
				qspStrToNum(qspStringFromPair(pos + QSP_STATIC_LEN(QSP_PLVOLUMEDELIM), qspPLFiles[i].End), 0),
				QSP_FALSE);
		}
		else
			qspPlayFile(qspPLFiles[i], 100, QSP_FALSE);
	}
}

void qspRefreshPlayList()
{
	QSP_CHAR *pos;
	QSPString *s, fullPath, curFile;
	int count = qspPLFilesCount;
	if (!count) return;
	qspCopyStrs(&s, qspPLFiles, 0, count);
	qspClearPlayList(QSP_FALSE);
	while (--count >= 0)
	{
		pos = qspStrChar(s[count], QSP_PLVOLUMEDELIM[0]);
		if (pos)
			curFile = qspStringFromPair(s[count].Str, pos);
		else
			curFile = s[count];
		if (qspIsAnyString(curFile) && qspSearchPlayList(curFile) < 0)
		{
			fullPath = qspGetAbsFromRelPath(curFile);
			if (qspCallIsPlayingFile(fullPath))
				qspPLFiles[qspPLFilesCount++] = qspGetNewText(s[count]);
			qspFreeString(fullPath);
		}
		qspFreeString(s[count]);
	}
	free(s);
}

QSP_BOOL qspStatementPlayFile(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	int volume = (count == 2 ? QSP_NUM(args[1]) : 100);
	qspPlayFile(QSP_STR(args[0]), volume, QSP_TRUE);
	return QSP_FALSE;
}

QSP_BOOL qspStatementCloseFile(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	int pos;
	QSPString file;
	if (!qspPLFilesCount) return QSP_FALSE;
	if (count)
	{
		if (qspIsAnyString(QSP_STR(args[0])))
		{
			pos = qspSearchPlayList(QSP_STR(args[0]));
			if (pos >= 0)
			{
				file = qspGetAbsFromRelPath(QSP_STR(args[0]));
				qspCallCloseFile(file);
				qspFreeString(file);
				do
				{
					qspFreeString(qspPLFiles[pos]);
					--qspPLFilesCount;
					while (pos < qspPLFilesCount)
					{
						qspPLFiles[pos] = qspPLFiles[pos + 1];
						++pos;
					}
					pos = qspSearchPlayList(QSP_STR(args[0]));
				} while (pos >= 0);
			}
		}
	}
	else
	{
		qspClearPlayList(QSP_FALSE);
		qspCallCloseFile(qspNullString);
	}
	return QSP_FALSE;
}
