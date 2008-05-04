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

void qspPlayFile(QSP_CHAR *s, long volume, QSP_BOOL isAddToPlayList)
{
	QSP_CHAR buf[4], *file;
	if (qspIsAnyString(s))
	{
		if (volume < 0) volume = 0;
		if (volume > 100) volume = 100;
		file = qspGetNewText(qspQstPath, qspQstPathLen);
		file = qspGetAddText(file, s, qspQstPathLen, -1);
		qspCallPlayFile(file, volume);
		free(file);
		if (isAddToPlayList)
		{
			qspPlayListLen = qspAddText(&qspPlayList, s, qspPlayListLen, -1, QSP_FALSE);
			qspPlayListLen = qspAddText(&qspPlayList, QSP_PLVOLUMEDELIM, qspPlayListLen, 1, QSP_FALSE);
			qspPlayListLen = qspAddText(&qspPlayList, qspNumToStr(buf, volume), qspPlayListLen, -1, QSP_FALSE);
			qspPlayListLen = qspAddText(&qspPlayList, QSP_PLFILEDELIM, qspPlayListLen, 1, QSP_FALSE);
		}
	}
}

long qspSearchPlayList(QSP_CHAR *file)
{
	QSP_CHAR *uName, *playList, *pos;
	long length;
	if (!qspPlayListLen) return -1;
	length = (long)QSP_STRLEN(file);
	qspUpperStr(uName = qspGetNewText(file, length));
	qspUpperStr(playList = qspGetNewText(qspPlayList, qspPlayListLen));
	pos = QSP_STRSTR(playList, uName);
	while (pos)
	{
		if ((pos == playList || *(pos - 1) == QSP_PLFILEDELIM[0]) && pos[length] == QSP_PLVOLUMEDELIM[0])
			break;
		pos = QSP_STRSTR(QSP_STRCHR(pos + 1, QSP_PLFILEDELIM[0]) + 1, uName);
	}
	free(playList);
	free(uName);
	return pos ? (long)(pos - playList) : -1;
}

void qspPlayPLFiles()
{
	QSP_CHAR **s, *str, *pos;
	long i, count, volume;
	if (!qspPlayListLen) return;
	count = qspSplitStr(qspPlayList, QSP_PLFILEDELIM, &s);
	for (i = 0; i < count; ++i)
	{
		pos = QSP_STRCHR(s[i], QSP_PLVOLUMEDELIM[0]);
		if (pos)
		{
			volume = qspStrToNum(pos + 1, 0);
			str = qspGetNewText(s[i], (long)(pos - s[i]));
		}
		else
		{
			volume = 100;
			str = qspGetNewText(s[i], -1);
		}
		qspPlayFile(str, volume, QSP_FALSE);
		free(str);
		free(s[i]);
	}
	free(s);
}

void qspRefreshPlayList()
{
	long i, count, len;
	QSP_CHAR **s, *file, *str, *pos;
	if (!qspPlayListLen) return;
	count = qspSplitStr(qspPlayList, QSP_PLFILEDELIM, &s);
	qspClearText(&qspPlayList, &qspPlayListLen);
	for (i = count - 1; i >= 0; --i)
	{
		pos = QSP_STRCHR(s[i], QSP_PLVOLUMEDELIM[0]);
		len = (long)(pos ? (pos - s[i]) : QSP_STRLEN(s[i]));
		str = qspGetNewText(s[i], len);
		if (qspIsAnyString(str))
		{
			file = qspGetNewText(qspQstPath, qspQstPathLen);
			file = qspGetAddText(file, str, qspQstPathLen, len);
			if (qspSearchPlayList(str) < 0 && qspCallIsPlayingFile(file))
			{
				qspPlayListLen = qspAddText(&qspPlayList, s[i], qspPlayListLen, -1, QSP_FALSE);
				qspPlayListLen = qspAddText(&qspPlayList, QSP_PLFILEDELIM, qspPlayListLen, 1, QSP_FALSE);
			}
			free(file);
		}
		free(str);
		free(s[i]);
	}
	free(s);
}

QSP_BOOL qspStatementPlayFile(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long volume = count == 2 ? args[1].Num : 100;
	qspPlayFile(args[0].Str, volume, QSP_TRUE);
	return QSP_FALSE;
}

QSP_BOOL qspStatementCloseFile(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long pos;
	QSP_CHAR *temp, *end;
	if (!qspPlayListLen) return QSP_FALSE;
	if (count == 1 && qspIsAnyString(args[0].Str))
	{
		pos = qspSearchPlayList(args[0].Str);
		if (pos >= 0)
		{
			temp = qspGetNewText(qspQstPath, qspQstPathLen);
			temp = qspGetAddText(temp, args[0].Str, qspQstPathLen, -1);
			qspCallCloseFile(temp);
			free(temp);
			do
			{
				end = QSP_STRCHR(qspPlayList + pos + 1, QSP_PLFILEDELIM[0]);
				temp = qspGetNewText(qspPlayList, pos);
				qspPlayListLen = qspAddText(&temp, end + 1, pos, qspPlayListLen - (long)(end - qspPlayList) - 1, QSP_FALSE);
				free(qspPlayList);
				qspPlayList = temp;
				pos = qspSearchPlayList(args[0].Str);
			} while (pos >= 0);
		}
	}
	else
	{
		qspClearText(&qspPlayList, &qspPlayListLen);
		qspCallCloseFile(0);
	}
	return QSP_FALSE;
}
