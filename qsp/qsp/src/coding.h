/* Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru) */
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

	int qspStrCmpSB(char *, char *, unsigned char *);
	char qspDirectConvertSB(char, unsigned char *);
	char qspReverseConvertSB(char, unsigned char *);
	wchar_t qspDirectConvertUC(char, wchar_t *);
	char qspReverseConvertUC(wchar_t, wchar_t *);
	QSP_CHAR *qspCodeReCode(QSP_CHAR *, QSP_BOOL);
	char *qspFromQSPString(QSP_CHAR *);
	QSP_CHAR *qspGameToQSPString(char *, QSP_BOOL, QSP_BOOL);
	long qspSplitGameStr(char *, QSP_BOOL, QSP_CHAR *, char ***);
	long qspReCodeGetIntVal(QSP_CHAR *);
	long qspCodeWriteIntVal(QSP_CHAR **, long, long, QSP_BOOL);
	long qspCodeWriteVal(QSP_CHAR **, long, QSP_CHAR *, QSP_BOOL);

#endif
