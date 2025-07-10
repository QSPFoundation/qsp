/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "text.h"

#ifndef QSP_CODINGDEFINES
    #define QSP_CODINGDEFINES

    #define QSP_CODREMOV 5

    extern const unsigned char qspCP1251ToUpperTable[];
    extern const unsigned char qspCP1251ToLowerTable[];
    extern const unsigned char qspKOI8RToUpperTable[];
    extern const unsigned char qspKOI8RToLowerTable[];

    /* External functions */
    void *qspStringToFileData(QSPString s, QSP_BOOL isUCS2, int *dataSize);
    QSPString qspStringFromFileData(void *data, int dataSize, QSP_BOOL isUCS2);
    QSPString qspEncodeString(QSPString str, QSP_BOOL isUCS2);
    QSPString qspDecodeString(QSPString str, QSP_BOOL isUCS2);
    QSP_BIGINT qspReadEncodedIntVal(QSPString val, QSP_BOOL isUCS2);
    void qspAppendEncodedIntVal(QSPBufString *s, QSP_BIGINT val, QSP_BOOL isUCS2);
    void qspAppendEncodedStrVal(QSPBufString *s, QSPString val, QSP_BOOL isUCS2);
    void qspAppendStrVal(QSPBufString *s, QSPString val);
    void qspAppendEncodedVariant(QSPBufString *s, QSPVariant val, QSP_BOOL isUCS2);
    QSP_BOOL qspReadEncodedVariant(QSPString *strs, int strsCount, int *curIndex, QSP_BOOL isUCS2, QSPVariant *val);

#endif
