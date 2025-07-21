/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "playlist.h"
#include "callbacks.h"
#include "common.h"
#include "game.h"
#include "locations.h"
#include "statements.h"
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
    int oldLocationState;
    if (!qspIsAnyString(s)) return;
    if (volume < 0)
        volume = 0;
    else if (volume > 100)
        volume = 100;
    oldLocationState = qspLocationState;
    if (qspPLFilesCount == QSP_MAXPLFILES)
    {
        qspRefreshPlayList();
        if (qspLocationState != oldLocationState) return;
        if (qspPLFilesCount == QSP_MAXPLFILES) return;
    }
    qspCallPlayFile(s, volume);
    if (qspLocationState != oldLocationState) return;
    if (toAddToPlayList)
    {
        QSPBufString file = qspNewBufString(8);
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
    if (qspPLFilesCount)
    {
        int i, fileLen;
        QSPBufString buf;
        QSPString bufName;
        file = qspCopyToNewText(file);
        qspUpperStr(&file);
        fileLen = qspStrLen(file);
        buf = qspNewBufString(32);
        for (i = 0; i < qspPLFilesCount; ++i)
        {
            qspUpdateBufString(&buf, qspPLFiles[i]);
            bufName = qspBufTextToString(buf);
            qspUpperStr(&bufName);
            if (!qspStrsPartCompare(bufName, file, fileLen))
            {
                /* The current item is prefixed with the file */
                if (qspStrLen(bufName) == fileLen || qspIsInList(bufName.Str[fileLen], QSP_PLVOLUMEDELIM))
                {
                    qspFreeString(&file);
                    qspFreeBufString(&buf);
                    return i;
                }
            }
        }
        qspFreeString(&file);
        qspFreeBufString(&buf);
    }
    return -1;
}

void qspPlayPLFiles(void)
{
    QSP_CHAR *pos;
    int i, volume, oldLocationState = qspLocationState;
    qspCallCloseFile(qspNullString);
    if (qspLocationState != oldLocationState) return;
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
        if (qspLocationState != oldLocationState) return;
    }
}

void qspRefreshPlayList(void)
{
    int count = qspPLFilesCount;
    if (count)
    {
        int i, oldLocationState;
        QSP_BOOL isPlaying;
        QSP_CHAR *pos;
        QSPString *files, curFile;
        qspCopyStrs(&files, qspPLFiles, 0, count);
        qspClearPlayList(QSP_FALSE);
        oldLocationState = qspLocationState;
        for (i = 0; i < count; ++i)
        {
            pos = qspStrChar(files[i], QSP_PLVOLUMEDELIM[0]);
            if (pos)
                curFile = qspStringFromPair(files[i].Str, pos);
            else
                curFile = files[i];
            if (qspIsAnyString(curFile) && qspSearchPlayList(curFile) < 0)
            {
                isPlaying = qspCallIsPlayingFile(curFile);
                if (qspLocationState != oldLocationState)
                {
                    qspFreeStrs(files, count);
                    return;
                }
                if (isPlaying)
                    qspPLFiles[qspPLFilesCount++] = qspMoveText(files + i);
            }
        }
        qspFreeStrs(files, count);
    }
}

void qspStatementPlayFile(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (count == 2)
    {
        int volume = QSP_TOINT(QSP_NUM(args[1]));
        qspPlayFile(QSP_STR(args[0]), volume, QSP_TRUE);
    }
    else
        qspPlayFile(QSP_STR(args[0]), 100, QSP_TRUE);
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
                qspCallCloseFile(QSP_STR(args[0]));
            }
        }
    }
    else
    {
        qspClearPlayList(QSP_FALSE);
        qspCallCloseFile(qspNullString);
    }
}
