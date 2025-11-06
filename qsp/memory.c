/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "memory.h"

QSPMemoryChunk *qspCurAllocChunk = 0;

void qspInitStackAllocator(void)
{
    qspCurAllocChunk = qspAllocateMemChunk();
}

void qspTerminateStackAllocator(void)
{
    QSPMemoryChunk *curChunk = qspCurAllocChunk;
    if (curChunk)
    {
        QSPMemoryChunk *nextChunk;
        /* Walk back to the first chunk */
        while (curChunk->Prev) curChunk = curChunk->Prev;
        /* Free all chunks forward */
        while (curChunk)
        {
            nextChunk = curChunk->Next;
            free(curChunk);
            curChunk = nextChunk;
        }
        qspCurAllocChunk = 0;
    }
}
