/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"

#ifndef QSP_MEMORYDEFINES
    #define QSP_MEMORYDEFINES

    #define QSP_ALLOCCHUNKSIZE 16384 /* defines max allowed allocation size */

    typedef struct QSPMemoryChunk_s QSPMemoryChunk;

    typedef struct QSPMemoryChunk_s
    {
        unsigned char Data[QSP_ALLOCCHUNKSIZE];
        int Used;
        QSPMemoryChunk *Next;
        QSPMemoryChunk *Prev;
    } QSPMemoryChunk;

    extern QSPMemoryChunk *qspCurAllocChunk;

    /* External functions */
    void qspInitStackAllocator(void);
    void qspTerminateStackAllocator(void);

    INLINE QSPMemoryChunk *qspAllocateMemChunk(void)
    {
        QSPMemoryChunk *chunk = (QSPMemoryChunk *)malloc(sizeof(QSPMemoryChunk));
        chunk->Used = 0;
        chunk->Prev = chunk->Next = 0;
        return chunk;
    }

    INLINE void *qspAllocateMemory(int size)
    {
        unsigned char *ptr;
        QSPMemoryChunk *curChunk = qspCurAllocChunk;
        if (curChunk->Used + size > QSP_ALLOCCHUNKSIZE)
        {
            /* Allocate a new chunk if the next one doesn't exist */
            if (!curChunk->Next)
            {
                QSPMemoryChunk *newChunk = qspAllocateMemChunk();
                newChunk->Prev = curChunk;
                curChunk->Next = newChunk;
            }
            curChunk = curChunk->Next;
            curChunk->Used = 0;
            qspCurAllocChunk = curChunk;
        }
        ptr = curChunk->Data + curChunk->Used;
        curChunk->Used += size;
        return ptr;
    }

    INLINE void qspReleaseMemory(void *ptr)
    {
        QSPMemoryChunk *curChunk = qspCurAllocChunk;
        /* We always release data from the current chunk */
        curChunk->Used = (int)((unsigned char *)ptr - curChunk->Data);
        /* Move back if empty */
        if (!curChunk->Used && curChunk->Prev)
            qspCurAllocChunk = curChunk->Prev;
    }

#endif
