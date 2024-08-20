/* Copyright (C) 2001-2024 Val Argunov (byte AT qsp DOT org) */
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

INLINE void qspPlayFile(QSPString s, int volume, QSP_BOOL toAddToPlayList);
INLINE int qspSearchPlayList(QSPString file);

void qspClearPlayList(QSP_BOOL toInit)
{
    if (!toInit)
    {
        int i;
        for (i = 0; i < qspPLFilesCount; ++i)
            qspFreeString(qspPLFiles + i);
    }
    qspPLFilesCount = 0;
}

INLINE void qspPlayFile(QSPString s, int volume, QSP_BOOL toAddToPlayList)
{
    if (!qspIsAnyString(s)) return;
    if (volume < 0)
        volume = 0;
    else if (volume > 100)
        volume = 100;
    qspCallPlayFile(s, volume);
    if (toAddToPlayList)
    {
        QSPBufString file;
        if (qspPLFilesCount == QSP_MAXPLFILES)
        {
            qspRefreshPlayList();
            if (qspPLFilesCount == QSP_MAXPLFILES) return;
        }
        file = qspNewBufString(8);
        qspAddBufText(&file, s);
        if (volume != 100)
        {
            QSP_CHAR buf[4];
            qspAddBufText(&file, QSP_STATIC_STR(QSP_PLVOLUMEDELIM));
            qspAddBufText(&file, qspNumToStr(buf, volume));
        }
        qspPLFiles[qspPLFilesCount++] = qspBufTextToString(file);
    }
}

INLINE int qspSearchPlayList(QSPString file)
{
    QSPString uName, bufName;
    QSP_CHAR *buf;
    int i, bufSize, itemLen, fileLen;
    if (!qspPLFilesCount) return -1;
    fileLen = qspStrLen(file);
    uName = qspCopyToNewText(file);
    qspUpperStr(&uName);
    bufSize = 32;
    buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
    for (i = 0; i < qspPLFilesCount; ++i)
    {
        itemLen = qspStrLen(qspPLFiles[i]);
        if (itemLen)
        {
            if (itemLen > bufSize)
            {
                bufSize = itemLen + 8;
                buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
            }
            memcpy(buf, qspPLFiles[i].Str, itemLen * sizeof(QSP_CHAR));
        }
        bufName = qspStringFromLen(buf, itemLen);
        qspUpperStr(&bufName);
        if (!qspStrsNComp(bufName, uName, fileLen))
        {
            if (itemLen == fileLen || qspIsInList(QSP_PLVOLUMEDELIM, bufName.Str[fileLen]))
            {
                qspFreeString(&uName);
                free(buf);
                return i;
            }
        }
    }
    qspFreeString(&uName);
    free(buf);
    return -1;
}

void qspPlayPLFiles(void)
{
    int i, volume;
    QSP_CHAR *pos;
    qspCallCloseFile(qspNullString);
    for (i = 0; i < qspPLFilesCount; ++i)
    {
        pos = qspStrChar(qspPLFiles[i], QSP_PLVOLUMEDELIM[0]);
        if (pos)
        {
            volume = qspStrToNum(qspStringFromPair(pos + QSP_STATIC_LEN(QSP_PLVOLUMEDELIM), qspPLFiles[i].End), 0);
            qspPlayFile(qspStringFromPair(qspPLFiles[i].Str, pos), volume, QSP_FALSE);
        }
        else
            qspPlayFile(qspPLFiles[i], 100, QSP_FALSE);
    }
}

void qspRefreshPlayList(void)
{
    QSP_CHAR *pos;
    QSPString *s, curFile;
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
            if (qspCallIsPlayingFile(curFile))
                qspPLFiles[qspPLFilesCount++] = qspCopyToNewText(s[count]);
        }
        qspFreeString(s + count);
    }
    free(s);
}

void qspStatementPlayFile(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    int volume = (count == 2 ? QSP_NUM(args[1]) : 100);
    qspPlayFile(QSP_STR(args[0]), volume, QSP_TRUE);
}

void qspStatementCloseFile(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (!qspPLFilesCount) return;
    if (count)
    {
        if (qspIsAnyString(QSP_STR(args[0])))
        {
            int fileIndex = qspSearchPlayList(QSP_STR(args[0]));
            if (fileIndex >= 0)
            {
                qspCallCloseFile(QSP_STR(args[0]));
                do
                {
                    qspFreeString(qspPLFiles + fileIndex);
                    --qspPLFilesCount;
                    while (fileIndex < qspPLFilesCount)
                    {
                        qspPLFiles[fileIndex] = qspPLFiles[fileIndex + 1];
                        ++fileIndex;
                    }
                    fileIndex = qspSearchPlayList(QSP_STR(args[0]));
                } while (fileIndex >= 0);
            }
        }
    }
    else
    {
        qspClearPlayList(QSP_FALSE);
        qspCallCloseFile(qspNullString);
    }
}
