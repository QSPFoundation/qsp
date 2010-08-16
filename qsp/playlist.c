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

QSP_CHAR *qspPLFiles[QSP_MAXPLFILES];
int qspPLFilesCount = 0;

static void qspPlayFile(QSP_CHAR *, int, QSP_BOOL);
static int qspSearchPlayList(QSP_CHAR *);

void qspClearPlayList(QSP_BOOL isFirst)
{
	int i;
	if (!isFirst)
	{
		for (i = 0; i < qspPLFilesCount; ++i)
			free(qspPLFiles[i]);
	}
	qspPLFilesCount = 0;
}

static void qspPlayFile(QSP_CHAR *s, int volume, QSP_BOOL isAddToPlayList)
{
	int len;
	QSP_CHAR buf[4], *file;
	if (!qspIsAnyString(s)) return;
	if (volume < 0)
		volume = 0;
	else if (volume > 100)
		volume = 100;
	file = qspGetAbsFromRelPath(s);
	qspCallPlayFile(file, volume);
	free(file);
	if (isAddToPlayList)
	{
		if (qspPLFilesCount == QSP_MAXPLFILES)
		{
			qspRefreshPlayList();
			if (qspPLFilesCount == QSP_MAXPLFILES) return;
		}
		len = qspAddText(&file, s, 0, -1, QSP_TRUE);
		if (volume != 100)
		{
			len = qspAddText(&file, QSP_PLVOLUMEDELIM, len, 1, QSP_FALSE);
			file = qspGetAddText(file, qspNumToStr(buf, volume), len, -1);
		}
		qspPLFiles[qspPLFilesCount++] = file;
	}
}

static int qspSearchPlayList(QSP_CHAR *file)
{
	QSP_CHAR *uName, *buf;
	int i, bufSize, itemLen, len;
	if (!qspPLFilesCount) return -1;
	len = qspStrLen(file);
	qspUpperStr(uName = qspGetNewText(file, len));
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
		qspStrCopy(buf, qspPLFiles[i]);
		qspUpperStr(buf);
		if (!qspStrsNComp(buf, uName, len) && qspIsInListEOL(QSP_PLVOLUMEDELIM, buf[len]))
		{
			free(uName);
			free(buf);
			return i;
		}
	}
	free(uName);
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
			*pos = 0;
			qspPlayFile(qspPLFiles[i], qspStrToNum(pos + 1, 0), QSP_FALSE);
			*pos = QSP_PLVOLUMEDELIM[0];
		}
		else
			qspPlayFile(qspPLFiles[i], 100, QSP_FALSE);
	}
}

void qspRefreshPlayList()
{
	QSP_CHAR **s, *file, *str, *pos;
	int count = qspPLFilesCount;
	if (!count) return;
	qspCopyStrs(&s, qspPLFiles, 0, count);
	qspClearPlayList(QSP_FALSE);
	while (--count >= 0)
	{
		str = s[count];
		pos = qspStrChar(str, QSP_PLVOLUMEDELIM[0]);
		if (pos) *pos = 0;
		if (qspIsAnyString(str))
		{
			file = qspGetAbsFromRelPath(str);
			if (qspSearchPlayList(str) < 0 && qspCallIsPlayingFile(file))
			{
				if (pos) *pos = QSP_PLVOLUMEDELIM[0];
				qspPLFiles[qspPLFilesCount++] = qspGetNewText(str, -1);
			}
			free(file);
		}
		free(str);
	}
	free(s);
}

QSP_BOOL qspStatementPlayFile(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	int volume = (count == 2 ? QSP_NUM(args[1]) : 100);
	qspPlayFile(QSP_STR(args[0]), volume, QSP_TRUE);
	return QSP_FALSE;
}

QSP_BOOL qspStatementCloseFile(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	int pos;
	QSP_CHAR *file;
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
				free(file);
				do
				{
					free(qspPLFiles[pos]);
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
		qspCallCloseFile(0);
	}
	return QSP_FALSE;
}
