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

QSPPlayListFile qspPLFiles[QSP_MAXPLFILES];
int qspPLFilesCount = 0;

INLINE int qspSearchPlayList(QSPString file);
INLINE QSP_BOOL qspAddToPlayList(QSPString file, int volume);
INLINE void qspRemoveFromPlayList(QSPString file);
INLINE int qspCopyPlayListFiles(QSPPlayListFile **files);
INLINE void qspFreePlayListFiles(QSPPlayListFile *files, int count);
INLINE void qspPlayFile(QSPString file, int volume, QSP_BOOL toAddToPlayList);

void qspClearPlayList(QSP_BOOL toInit)
{
    if (!toInit)
    {
        int i;
        for (i = 0; i < qspPLFilesCount; ++i)
            qspFreeString(&qspPLFiles[i].Name);
    }
    qspPLFilesCount = 0;
}

INLINE int qspSearchPlayList(QSPString file)
{
    int i;
    for (i = 0; i < qspPLFilesCount; ++i)
    {
        if (qspStrsEqual(qspPLFiles[i].Name, file))
            return i;
    }
    return -1;
}

INLINE QSP_BOOL qspAddToPlayList(QSPString file, int volume)
{
    int fileIndex = qspSearchPlayList(file);
    if (fileIndex < 0)
    {
        if (qspPLFilesCount == QSP_MAXPLFILES) return QSP_FALSE;
        fileIndex = qspPLFilesCount++;
        qspPLFiles[fileIndex].Name = qspCopyToNewText(file);
    }
    qspPLFiles[fileIndex].Volume = volume;
    return QSP_TRUE;
}

INLINE void qspRemoveFromPlayList(QSPString file)
{
    int fileIndex = qspSearchPlayList(file);
    if (fileIndex >= 0)
    {
        qspFreeString(&qspPLFiles[fileIndex].Name);
        --qspPLFilesCount;
        memmove(qspPLFiles + fileIndex, qspPLFiles + fileIndex + 1, (qspPLFilesCount - fileIndex) * sizeof(QSPPlayListFile));
    }
}

INLINE int qspCopyPlayListFiles(QSPPlayListFile **files)
{
    int count = qspPLFilesCount;
    if (count)
    {
        int i;
        QSPPlayListFile *destFiles = (QSPPlayListFile *)malloc(count * sizeof(QSPPlayListFile));
        for (i = 0; i < count; ++i)
        {
            destFiles[i].Name = qspCopyToNewText(qspPLFiles[i].Name);
            destFiles[i].Volume = qspPLFiles[i].Volume;
        }
        *files = destFiles;
    }
    else
        *files = 0;
    return count;
}

INLINE void qspFreePlayListFiles(QSPPlayListFile *files, int count)
{
    if (files)
    {
        QSPPlayListFile *curFile;
        for (curFile = files; count > 0; --count, ++curFile)
            qspFreeString(&curFile->Name);
        free(files);
    }
}

INLINE void qspPlayFile(QSPString file, int volume, QSP_BOOL toAddToPlayList)
{
    if (!qspIsAnyString(file)) return;
    if (volume < 0 || volume > 100) return;
    if (toAddToPlayList && !qspAddToPlayList(file, volume))
    {
        int oldLocationState = qspLocationState;
        qspRefreshPlayList();
        if (qspLocationState != oldLocationState) return;
        if (!qspAddToPlayList(file, volume))
        {
            qspSetError(QSP_ERR_CANTPLAYFILE);
            return;
        }
    }
    qspCallPlayFile(file, volume);
}

void qspPlayPLFiles(void)
{
    QSPPlayListFile *files;
    int i, count, oldLocationState = qspLocationState;
    qspCallCloseFile(qspNullString);
    if (qspLocationState != oldLocationState) return;
    /* Copy files, the host can change the playlist */
    count = qspCopyPlayListFiles(&files);
    for (i = 0; i < count; ++i)
    {
        qspPlayFile(files[i].Name, files[i].Volume, QSP_FALSE);
        if (qspLocationState != oldLocationState) break;
    }
    qspFreePlayListFiles(files, count);
}

void qspRefreshPlayList(void)
{
    QSPPlayListFile *files;
    QSP_BOOL isPlaying;
    int i, count, oldLocationState = qspLocationState;
    /* Copy files, the host can change the playlist */
    count = qspCopyPlayListFiles(&files);
    for (i = 0; i < count; ++i)
    {
        isPlaying = qspCallIsPlayingFile(files[i].Name);
        if (qspLocationState != oldLocationState) break;
        if (!isPlaying) qspRemoveFromPlayList(files[i].Name);
    }
    qspFreePlayListFiles(files, count);
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
