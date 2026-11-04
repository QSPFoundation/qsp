/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "names.h"
#include "text.h"

QSPName qspNullName;

static QSPName *qspNames = 0; /* entry 0 is the empty name */
static int qspNamesEntriesCount = 0;
static int qspNamesCapacity = 0; /* has to be a power of 2 */
static int *qspNamesSlots = 0; /* hash table by names, QSP_SLOTSCOUNT(qspNamesCapacity) slots */
static int qspNamesFreeIndex = 1; /* search for a free entry starts here */
static int qspNamesGeneration = 0; /* current generation */
static int qspReleasedGeneration = 0; /* generation of the last release */

INLINE int *qspGetNameSlot(QSPString name, unsigned int nameHash);
INLINE int *qspGetEmptyNameSlot(unsigned int nameHash);
INLINE void qspRebuildNamesSlots(void);
INLINE void qspResizeNames(int newCapacity);
INLINE int qspInternExactName(QSPString name, unsigned int nameHash);
INLINE int qspFindExactName(QSPString name, unsigned int nameHash);

INLINE int *qspGetNameSlot(QSPString name, unsigned int nameHash)
{
    QSPName *entry;
    int *slot, mask = QSP_SLOTSMASK(qspNamesCapacity), ind = (int)(nameHash & mask);
    while (*(slot = qspNamesSlots + ind))
    {
        entry = qspNames + *slot;
        if (entry->NameHash == nameHash && qspStrsEqual(entry->Name, name))
            return slot;
        ind = (ind + 1) & mask;
    }
    return slot;
}

INLINE int *qspGetEmptyNameSlot(unsigned int nameHash)
{
    int *slot, mask = QSP_SLOTSMASK(qspNamesCapacity), ind = (int)(nameHash & mask);
    while (*(slot = qspNamesSlots + ind))
        ind = (ind + 1) & mask;
    return slot;
}

INLINE void qspRebuildNamesSlots(void)
{
    int i, *slot;
    memset(qspNamesSlots, 0, QSP_SLOTSCOUNT(qspNamesCapacity) * sizeof(int));
    for (i = 0; i < qspNamesEntriesCount; ++i)
    {
        if (qspNames[i].Name.Str)
        {
            slot = qspGetEmptyNameSlot(qspNames[i].NameHash);
            *slot = i;
        }
    }
}

INLINE void qspResizeNames(int newCapacity)
{
    if (qspNamesCapacity)
        qspNames = (QSPName *)realloc(qspNames, newCapacity * sizeof(QSPName));
    else
    {
        qspNames = (QSPName *)malloc(newCapacity * sizeof(QSPName));
        *qspNames = qspNullName; /* entry 0 is the empty name */
        qspNamesEntriesCount = 1;
    }
    qspNamesCapacity = newCapacity;
    qspNamesSlots = (int *)realloc(qspNamesSlots, QSP_SLOTSCOUNT(newCapacity) * sizeof(int));
    qspRebuildNamesSlots();
}

INLINE int qspInternExactName(QSPString name, unsigned int nameHash)
{
    /* The name has to be uppercased */
    QSPName *entry;
    int *slot, nameIndex;
    if (qspIsEmpty(name)) return 0;
    if (qspNamesEntriesCount >= qspNamesCapacity)
    {
        int newCapacity = (qspNamesCapacity ? qspNamesCapacity * 2 : QSP_NAMESCAPACITY);
        qspResizeNames(newCapacity);
    }
    slot = qspGetNameSlot(name, nameHash);
    if (*slot)
    {
        qspNames[*slot].Generation = qspNamesGeneration; /* used by current generation */
        return *slot;
    }
    while (qspNamesFreeIndex < qspNamesEntriesCount && qspNames[qspNamesFreeIndex].Name.Str)
        ++qspNamesFreeIndex;
    nameIndex = qspNamesFreeIndex++;
    if (nameIndex == qspNamesEntriesCount)
        ++qspNamesEntriesCount;
    *slot = nameIndex;
    entry = qspNames + nameIndex;
    entry->Name = qspCopyToNewText(name);
    entry->NameHash = nameHash;
    entry->Generation = qspNamesGeneration;
    return nameIndex;
}

INLINE int qspFindExactName(QSPString name, unsigned int nameHash)
{
    /* The name has to be uppercased */
    if (qspIsEmpty(name)) return 0;
    if (qspNamesCapacity)
    {
        int *slot = qspGetNameSlot(name, nameHash);
        if (*slot) return *slot;
    }
    return -1;
}

int qspGetNameIndex(QSPString name, QSP_BOOL toIntern)
{
    int nameIndex;
    unsigned int nameHash;
    QSPString upperName = qspUpperText(name);
    nameHash = qspGetTextHash(upperName);
    nameIndex = (toIntern ? qspInternExactName(upperName, nameHash) : qspFindExactName(upperName, nameHash));
    qspFreeNewString(&upperName, &name);
    return nameIndex;
}

QSPString qspGetNameText(int nameIndex)
{
    if (nameIndex > 0 && nameIndex < qspNamesEntriesCount)
        return qspNames[nameIndex].Name;
    return qspNullString;
}

void qspAdvanceNamesGeneration(void)
{
    ++qspNamesGeneration;
}

void qspReleaseNamesGeneration(void)
{
    /* Nothing holds the names of the previous generations once the state of the game is cleared */
    if (qspNamesCapacity && qspReleasedGeneration != qspNamesGeneration)
    {
        int i;
        QSPName *entry;
        for (i = 0, entry = qspNames; i < qspNamesEntriesCount; ++i, ++entry)
        {
            if (entry->Name.Str && entry->Generation != qspNamesGeneration)
            {
                qspFreeString(&entry->Name);
                entry->Name = qspNullString;
            }
        }
        qspNamesFreeIndex = 1;
        qspRebuildNamesSlots();
        qspReleasedGeneration = qspNamesGeneration;
    }
}

void qspClearNames(QSP_BOOL toInit)
{
    if (!toInit && qspNamesCapacity)
    {
        int i;
        for (i = 0; i < qspNamesEntriesCount; ++i)
            qspFreeString(&qspNames[i].Name);
        free(qspNames);
        free(qspNamesSlots);
    }
    qspNames = 0;
    qspNamesEntriesCount = 0;
    qspNamesCapacity = 0;
    qspNamesSlots = 0;
    qspNamesFreeIndex = 1;
    qspNamesGeneration = 0;
    qspReleasedGeneration = 0;
}
