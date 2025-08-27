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

INLINE int qspSearchPlayList(QSPString file);
INLINE void qspAddToPlayList(QSPString file, int volume);
INLINE QSP_BOOL qspRemoveFromPlayList(QSPString file);
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
                if (qspStrLen(bufName) == fileLen || qspIsCharAtPos(bufName, bufName.Str + fileLen, QSP_PLVOLUMEDELIM_CHAR))
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

INLINE void qspAddToPlayList(QSPString file, int volume)
{
    QSPBufString fileBuf = qspNewBufString(8);
    qspAddBufText(&fileBuf, file);
    if (volume != 100)
    {
        QSP_CHAR buf[4];
        qspAddBufText(&fileBuf, QSP_STATIC_STR(QSP_PLVOLUMEDELIM));
        qspAddBufText(&fileBuf, qspNumToStr(buf, volume));
    }
    qspPLFiles[qspPLFilesCount++] = qspBufTextToString(fileBuf);
}

INLINE QSP_BOOL qspRemoveFromPlayList(QSPString file)
{
    int fileIndex = qspSearchPlayList(file);
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
            fileIndex = qspSearchPlayList(file);
        } while (fileIndex >= 0);
        return QSP_TRUE;
    }
    return QSP_FALSE;
}

INLINE void qspPlayFile(QSPString file, int volume, QSP_BOOL toAddToPlayList)
{
    int oldLocationState;
    if (!qspIsAnyString(file)) return;
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
    qspCallPlayFile(file, volume);
    if (qspLocationState != oldLocationState) return;

    if (toAddToPlayList)
        qspAddToPlayList(file, volume);
}

void qspPlayPLFiles(void)
{
    QSP_CHAR *pos;
    int i, volume, oldLocationState = qspLocationState;
    qspCallCloseFile(qspNullString);
    if (qspLocationState != oldLocationState) return;
    for (i = 0; i < qspPLFilesCount; ++i)
    {
        pos = qspStrLastChar(qspPLFiles[i], QSP_PLVOLUMEDELIM_CHAR);
        if (pos)
        {
            volume = qspStrToNum(qspStringFromPair(pos + QSP_CHAR_LEN, qspPLFiles[i].End), 0);
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
        /* Iterate in reverse order to keep only the latest files */
        for (i = count - 1; i >= 0; --i)
        {
            pos = qspStrLastChar(files[i], QSP_PLVOLUMEDELIM_CHAR);
            curFile = (pos ? qspStringFromPair(files[i].Str, pos) : files[i]);
            if (qspIsAnyString(curFile) && qspSearchPlayList(curFile) < 0)
            {
                isPlaying = qspCallIsPlayingFile(curFile);
                if (qspLocationState != oldLocationState)
                {
                    qspFreeStrs(files, count);
                    return;
                }
                if (isPlaying)
                    qspPLFiles[qspPLFilesCount++] = qspMoveToNewText(files + i);
            }
        }
        qspFreeStrs(files, count);
        /* Restore the original order */
        qspReverseStrs(qspPLFiles, qspPLFilesCount);
    }
}

void qspStatementPlayFile(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    int volume = (count == 2 ? QSP_TOINT(QSP_NUM(args[1])) : 100);
    qspPlayFile(QSP_STR(args[0]), volume, QSP_TRUE);
}

void qspStatementCloseFile(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (!qspPLFilesCount) return;
    if (count)
    {
        if (qspIsAnyString(QSP_STR(args[0])) && qspRemoveFromPlayList(QSP_STR(args[0])))
            qspCallCloseFile(QSP_STR(args[0]));
    }
    else
    {
        qspClearPlayList(QSP_FALSE);
        qspCallCloseFile(qspNullString);
    }
}
