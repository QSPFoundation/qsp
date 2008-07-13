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

QSP_CHAR *qspSearchPlayList(QSP_CHAR *file)
{
	QSP_CHAR *uName, *playList, *pos;
	long len = (long)QSP_STRLEN(file);
	qspUpperStr(uName = qspGetNewText(file, len));
	qspUpperStr(playList = qspGetNewText(qspPlayList, qspPlayListLen));
	pos = QSP_STRSTR(playList, uName);
	while (pos)
	{
		if ((pos == playList || *(pos - 1) == QSP_PLFILEDELIM[0]) && pos[len] == QSP_PLVOLUMEDELIM[0])
			break;
		pos = QSP_STRSTR(QSP_STRCHR(pos + 1, QSP_PLFILEDELIM[0]) + 1, uName);
	}
	free(playList);
	free(uName);
	return (pos ? (pos - playList + qspPlayList) : 0);
}

void qspPlayPLFiles()
{
	long i, count;
	QSP_CHAR **s, *pos;
	if (!qspPlayListLen) return;
	count = qspSplitStr(qspPlayList, QSP_PLFILEDELIM, &s);
	for (i = 0; i < count; ++i)
	{
		pos = QSP_STRCHR(s[i], QSP_PLVOLUMEDELIM[0]);
		if (pos)
		{
			*pos = 0;
			qspPlayFile(s[i], qspStrToNum(pos + 1, 0), QSP_FALSE);
			*pos = QSP_PLVOLUMEDELIM[0];
		}
		else
			qspPlayFile(s[i], 100, QSP_FALSE);
		free(s[i]);
	}
	free(s);
}

void qspRefreshPlayList()
{
	long count, len;
	QSP_CHAR **s, *file, *str, *pos;
	if (!qspPlayListLen) return;
	count = qspSplitStr(qspPlayList, QSP_PLFILEDELIM, &s);
	qspClearText(&qspPlayList, &qspPlayListLen);
	while (--count >= 0)
	{
		pos = QSP_STRCHR(s[count], QSP_PLVOLUMEDELIM[0]);
		len = (long)(pos ? (pos - s[count]) : QSP_STRLEN(s[count]));
		str = qspGetNewText(s[count], len);
		if (qspIsAnyString(str))
		{
			file = qspGetNewText(qspQstPath, qspQstPathLen);
			file = qspGetAddText(file, str, qspQstPathLen, len);
			if (!qspSearchPlayList(str) && qspCallIsPlayingFile(file))
			{
				qspPlayListLen = qspAddText(&qspPlayList, s[count], qspPlayListLen, -1, QSP_FALSE);
				qspPlayListLen = qspAddText(&qspPlayList, QSP_PLFILEDELIM, qspPlayListLen, 1, QSP_FALSE);
			}
			free(file);
		}
		free(str);
		free(s[count]);
	}
	free(s);
}

QSP_BOOL qspStatementPlayFile(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long volume = (count == 2 ? args[1].Num : 100);
	qspPlayFile(args[0].Str, volume, QSP_TRUE);
	return QSP_FALSE;
}

QSP_BOOL qspStatementCloseFile(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long len;
	QSP_CHAR *temp, *end, *pos;
	if (!qspPlayListLen) return QSP_FALSE;
	if (count == 1 && qspIsAnyString(args[0].Str))
	{
		pos = qspSearchPlayList(args[0].Str);
		if (pos)
		{
			temp = qspGetNewText(qspQstPath, qspQstPathLen);
			temp = qspGetAddText(temp, args[0].Str, qspQstPathLen, -1);
			qspCallCloseFile(temp);
			free(temp);
			do
			{
				end = QSP_STRCHR(pos + 1, QSP_PLFILEDELIM[0]);
				len = (long)(pos - qspPlayList);
				temp = qspGetNewText(qspPlayList, len);
				qspPlayListLen = qspAddText(&temp, end + 1, len, qspPlayListLen - (long)(end - qspPlayList) - 1, QSP_FALSE);
				free(qspPlayList);
				qspPlayList = temp;
				pos = qspSearchPlayList(args[0].Str);
			} while (pos);
		}
	}
	else
	{
		qspClearText(&qspPlayList, &qspPlayListLen);
		qspCallCloseFile(0);
	}
	return QSP_FALSE;
}
