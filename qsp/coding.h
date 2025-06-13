/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
