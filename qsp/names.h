/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "text.h"

#ifndef QSP_NAMESDEFINES
    #define QSP_NAMESDEFINES

    #define QSP_NAMESCAPACITY 64 /* has to be a power of 2 */

    typedef struct
    {
        QSPString Name; /* empty for released entry */
        unsigned int NameHash;
        int Generation;
    } QSPName;

    /* External functions */
    int qspGetExactNameIndex(QSPString name, unsigned int nameHash, QSP_BOOL toIntern);
    int qspGetNameIndex(QSPString name, QSP_BOOL toIntern);
    QSPString qspGetNameText(int nameIndex);
    void qspAdvanceNamesGeneration(void);
    void qspReleaseNamesGeneration(void);
    void qspInitNames(void);
    void qspTerminateNames(void);

#endif
