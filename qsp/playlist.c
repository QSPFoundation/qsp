/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "playlist.h"
#include "callbacks.h"
#include "common.h"
#include "errors.h"
#include "locations.h"
#include "text.h"

QSPString qspPLFiles[QSP_MAXPLFILES];
int qspPLFilesCount = 0;

INLINE int qspSearchPlayList(QSPString file);
INLINE void qspAddToPlayList(QSPString file, int volume);
INLINE void qspRemoveFromPlayList(QSPString file);
INLINE void qspPlayFile(QSPString file, int volume, QSP_BOOL toAddToPlayList);

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

INLINE int qspSearchPlayList(QSPString file)
{
    int i, fileLen = qspStrLen(file);
    for (i = 0; i < qspPLFilesCount; ++i)
    {
        if (!qspStrsPartCompare(qspPLFiles[i], file))
        {
            /* The current item is prefixed with the file */
            if (qspStrLen(qspPLFiles[i]) == fileLen || qspIsCharAtPos(qspPLFiles[i], qspPLFiles[i].Str + fileLen, QSP_PLVOLUMEDELIM_CHAR))
                return i;
        }
    }
    return -1;
}

INLINE void qspAddToPlayList(QSPString file, int volume)
{
    QSPBufString fileBuf = qspNewBufString(0, 8);
    qspAddBufText(&fileBuf, file);
    if (volume != 100)
    {
        QSP_CHAR buf[QSP_MAX_BIGINT_LEN];
        qspAddBufText(&fileBuf, QSP_STATIC_STR(QSP_PLVOLUMEDELIM));
        qspAddBufText(&fileBuf, qspNumToStr(buf, volume));
    }
    qspPLFiles[qspPLFilesCount++] = qspBufStringToString(fileBuf);
}

INLINE void qspRemoveFromPlayList(QSPString file)
{
    int fileIndex = qspSearchPlayList(file);
    while (fileIndex >= 0)
    {
        qspFreeString(qspPLFiles + fileIndex);
        --qspPLFilesCount;
        memmove(qspPLFiles + fileIndex, qspPLFiles + fileIndex + 1, (qspPLFilesCount - fileIndex) * sizeof(QSPString));
        fileIndex = qspSearchPlayList(file);
    }
}

INLINE void qspPlayFile(QSPString file, int volume, QSP_BOOL toAddToPlayList)
{
    if (!qspIsAnyString(file)) return;
    if (volume < 0 || volume > 100) return;
    if (toAddToPlayList)
    {
        if (qspPLFilesCount == QSP_MAXPLFILES && qspSearchPlayList(file) < 0)
        {
            int oldLocationState = qspLocationState;
            qspRefreshPlayList();
            if (qspLocationState != oldLocationState) return;
            if (qspPLFilesCount == QSP_MAXPLFILES)
            {
                qspSetError(QSP_ERR_CANTPLAYFILE);
                return;
            }
        }
        qspRemoveFromPlayList(file);
        qspAddToPlayList(file, volume);
    }
    qspCallPlayFile(file, volume);
}

void qspPlayPLFiles(void)
{
    QSP_CHAR *pos;
    QSPString volumeStr, *files;
    int i, count, volume, oldLocationState = qspLocationState;
    qspCallCloseFile(qspNullString);
    if (qspLocationState != oldLocationState) return;
    /* Copy files, the host can change the playlist */
    count = qspPLFilesCount;
    qspCopyStrs(&files, qspPLFiles, 0, count);
    for (i = 0; i < count; ++i)
    {
        pos = qspStrLastChar(files[i], QSP_PLVOLUMEDELIM_CHAR);
        if (pos)
        {
            volumeStr = qspStringFromPair(pos + QSP_STATIC_LEN(QSP_PLVOLUMEDELIM), files[i].End);
            volume = qspClampNum(qspStrToNum(volumeStr, 0), 0, 100);
            qspPlayFile(qspStringFromPair(files[i].Str, pos), volume, QSP_FALSE);
        }
        else
            qspPlayFile(files[i], 100, QSP_FALSE);
        if (qspLocationState != oldLocationState) break;
    }
    qspFreeStrs(files, count);
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
        /* Copy files, the host can change the playlist */
        qspCopyStrs(&files, qspPLFiles, 0, count);
        oldLocationState = qspLocationState;
        for (i = 0; i < count; ++i)
        {
            pos = qspStrLastChar(files[i], QSP_PLVOLUMEDELIM_CHAR);
            curFile = (pos ? qspStringFromPair(files[i].Str, pos) : files[i]);
            if (qspIsAnyString(curFile))
            {
                isPlaying = qspCallIsPlayingFile(curFile);
                if (qspLocationState != oldLocationState) break;
                if (!isPlaying) qspRemoveFromPlayList(curFile);
            }
            else
                qspRemoveFromPlayList(curFile);
        }
        qspFreeStrs(files, count);
    }
}

void qspStatementPlayFile(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    int volume;
    if (count == 2)
        volume = qspClampNum(QSP_NUM(args[1]), 0, 100);
    else
        volume = 100;
    qspPlayFile(QSP_STR(args[0]), volume, QSP_TRUE);
}

void qspStatementCloseFile(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (!qspPLFilesCount) return;
    if (count)
    {
        if (qspIsAnyString(QSP_STR(args[0])))
        {
            qspRemoveFromPlayList(QSP_STR(args[0]));
            qspCallCloseFile(QSP_STR(args[0])); /* call the host anyway */
        }
    }
    else
    {
        qspClearPlayList(QSP_FALSE);
        qspCallCloseFile(qspNullString);
    }
}
