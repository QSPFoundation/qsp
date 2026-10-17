/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../../declarations.h"

#ifdef _WASM_BINDING

#include "../../callbacks.h"
#include "../../actions.h"
#include "../../common.h"
#include "../../errors.h"
#include "../../locations.h"
#include "../../objects.h"
#include "../../text.h"

/* Host calls, implemented in qsp-wasm-host.js.
   Strings are passed as UTF-16 buffers with their size in bytes. */
void qspWasmHostDebug(const QSP_CHAR *str, int bytes);
void qspWasmHostSetTimer(int msecs);
void qspWasmHostRefreshInt(int isForced, int isNewLoc);
void qspWasmHostSetInputStrText(const QSP_CHAR *text, int bytes);
void qspWasmHostShowMessage(const QSP_CHAR *text, int bytes);
void qspWasmHostShowPicture(const QSP_CHAR *file, int bytes);
void qspWasmHostShowWindow(int type, int toShow);
void qspWasmHostPlayFile(const QSP_CHAR *file, int bytes, int volume);
int qspWasmHostIsPlayingFile(const QSP_CHAR *file, int bytes);
void qspWasmHostCloseFile(const QSP_CHAR *file, int bytes);
void qspWasmHostSystem(const QSP_CHAR *cmd, int bytes);
void qspWasmHostSleep(int msecs);
int qspWasmHostGetMSCount(void);
void qspWasmHostOpenGame(const QSP_CHAR *file, int bytes, int isNewGame);
void qspWasmHostInitGame(int isNewGame);
void qspWasmHostOpenGameStatus(const QSP_CHAR *file, int bytes);
void qspWasmHostSaveGameStatus(const QSP_CHAR *file, int bytes);
QSP_CHAR *qspWasmHostInputBox(const QSP_CHAR *text, int bytes); /* allocated, 0 if there's no input */
int qspWasmHostShowMenu(const QSPListItem *items, int count);
QSP_CHAR *qspWasmHostVersion(const QSP_CHAR *param, int bytes); /* allocated, 0 if there's no info */

#define QSP_WASM_STRBYTES(s) ((int)(((s).End - (s).Str) * sizeof(QSP_CHAR)))

void qspInitCallbacks(void)
{
    qspIsInCallback = QSP_FALSE;
}

void qspCallDebug(QSPString str)
{
    /* Jump into the debugger */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostDebug(str.Str, QSP_WASM_STRBYTES(str));
    qspFinalizeCallback(&state, QSP_TRUE);
}

void qspCallSetTimer(int msecs)
{
    /* Set timer interval */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostSetTimer(msecs);
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallRefreshInt(QSP_BOOL isForced)
{
    /* Refresh UI to show the latest state */
    static int oldFullRefreshCount = 0;
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    if (qspFullRefreshCount != oldFullRefreshCount)
    {
        oldFullRefreshCount = qspFullRefreshCount;
        qspWasmHostRefreshInt(isForced, QSP_TRUE);
    }
    else
        qspWasmHostRefreshInt(isForced, QSP_FALSE);
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallSetInputStrText(QSPString text)
{
    /* Set value of the text input control */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostSetInputStrText(text.Str, QSP_WASM_STRBYTES(text));
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallSystem(QSPString cmd)
{
    /* Execute system call */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostSystem(cmd.Str, QSP_WASM_STRBYTES(cmd));
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallOpenGame(QSPString file, QSP_BOOL isNewGame)
{
    /* Open game file */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostOpenGame(file.Str, QSP_WASM_STRBYTES(file), isNewGame);
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallInitGame(QSP_BOOL isNewGame)
{
    /* Init game state */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostInitGame(isNewGame);
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallOpenGameStatus(QSPString file)
{
    /* Open game state (showing the dialog to choose a file) */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostOpenGameStatus(file.Str, QSP_WASM_STRBYTES(file));
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallSaveGameStatus(QSPString file)
{
    /* Save game state (showing the dialog to choose a file) */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostSaveGameStatus(file.Str, QSP_WASM_STRBYTES(file));
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallShowMessage(QSPString text)
{
    /* Show a message */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_TRUE);
    qspWasmHostShowMessage(text.Str, QSP_WASM_STRBYTES(text));
    qspFinalizeCallback(&state, QSP_FALSE);
}

int qspCallShowMenu(QSPListItem *items, int count)
{
    /* Show a menu */
    QSPCallState state;
    int index;
    qspPrepareCallback(&state, QSP_TRUE);
    index = qspWasmHostShowMenu(items, count);
    qspFinalizeCallback(&state, QSP_FALSE);
    return index;
}

void qspCallShowPicture(QSPString file)
{
    /* Show an image */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostShowPicture(file.Str, QSP_WASM_STRBYTES(file));
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallShowWindow(int type, QSP_BOOL toShow)
{
    /* Show (hide) a region of the UI */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostShowWindow(type, toShow);
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallPlayFile(QSPString file, int volume)
{
    /* Start playing a music file */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostPlayFile(file.Str, QSP_WASM_STRBYTES(file), volume);
    qspFinalizeCallback(&state, QSP_FALSE);
}

QSP_BOOL qspCallIsPlayingFile(QSPString file)
{
    /* Check whether a file is still playing */
    QSPCallState state;
    QSP_BOOL isPlaying;
    qspPrepareCallback(&state, QSP_FALSE);
    isPlaying = (QSP_BOOL)qspWasmHostIsPlayingFile(file.Str, QSP_WASM_STRBYTES(file));
    qspFinalizeCallback(&state, QSP_FALSE);
    return isPlaying;
}

void qspCallCloseFile(QSPString file)
{
    /* Stop playing a file */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_FALSE);
    qspWasmHostCloseFile(file.Str, QSP_WASM_STRBYTES(file));
    qspFinalizeCallback(&state, QSP_FALSE);
}

void qspCallSleep(int msecs)
{
    /* Wait for the specified number of milliseconds */
    QSPCallState state;
    qspPrepareCallback(&state, QSP_TRUE);
    qspWasmHostSleep(msecs);
    qspFinalizeCallback(&state, QSP_FALSE);
}

int qspCallGetMSCount(void)
{
    /* Get the number of milliseconds since the last call of this function */
    QSPCallState state;
    int count;
    qspPrepareCallback(&state, QSP_FALSE);
    count = qspWasmHostGetMSCount();
    qspFinalizeCallback(&state, QSP_FALSE);
    return count;
}

QSPString qspCallInputBox(QSPString text)
{
    /* Get input from the user */
    QSPCallState state;
    QSP_CHAR *buffer;
    qspPrepareCallback(&state, QSP_TRUE);
    buffer = qspWasmHostInputBox(text.Str, QSP_WASM_STRBYTES(text));
    if (!qspFinalizeCallback(&state, QSP_FALSE))
    {
        if (buffer) free(buffer);
        return qspNullString;
    }
    return (buffer ? qspStringFromC(buffer) : qspNullString);
}

QSPString qspCallVersion(QSPString param)
{
    /* Get info from the player */
    QSPCallState state;
    QSP_CHAR *buffer;
    qspPrepareCallback(&state, QSP_FALSE);
    buffer = qspWasmHostVersion(param.Str, QSP_WASM_STRBYTES(param));
    if (!qspFinalizeCallback(&state, QSP_FALSE))
    {
        if (buffer) free(buffer);
        return qspNullString;
    }
    if (buffer)
    {
        QSPString res = qspStringFromC(buffer);
        if (!(qspIsEmpty(res) && qspIsEmpty(param))) return res;
        free(buffer);
    }
    return qspCopyToNewText(QSP_STATIC_STR(QSP_VER));
}

#endif
