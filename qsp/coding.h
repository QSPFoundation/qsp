/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
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

#ifndef QSP_CODINGDEFINES
    #define QSP_CODINGDEFINES

    #define QSP_CODREMOV 5

    extern unsigned char qspCP1251ToUpperTable[];
    extern unsigned char qspCP1251ToLowerTable[];
    extern unsigned char qspKOI8RToUpperTable[];
    extern unsigned char qspKOI8RToLowerTable[];
    extern unsigned char qspCP1251OrderTable[];
    extern unsigned char qspKOI8ROrderTable[];

    /* External functions */
    int qspStrCmpSB(char *, char *, unsigned char *);
    QSPString qspCodeReCode(QSPString str, QSP_BOOL isCode);
    char *qspQSPToGameString(QSPString s, QSP_BOOL isUCS2);
    QSPString qspGameToQSPString(char *s, QSP_BOOL isUCS2);
    int qspReCodeGetIntVal(QSPString val);
    void qspCodeWriteIntVal(QSPString *s, int val, QSP_BOOL isCode);
    void qspCodeWriteVal(QSPString *s, QSPString val, QSP_BOOL isCode);
    char *qspToSysString(QSPString s);

#endif
