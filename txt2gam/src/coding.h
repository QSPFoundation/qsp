/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "declarations.h"

#ifndef QSP_CODINGDEFINES
	#define QSP_CODINGDEFINES

	#define QSP_CODREMOV 5

	extern wchar_t qspCP1251ToUCS2LETable[];
	extern unsigned int qspCP1251ToUTF8Table[];

	/* External functions */
	wchar_t qspDirectConvertUC2LE(char, wchar_t *);
	char qspReverseConvertUCS2LE(wchar_t, wchar_t *);
	char qspReverseConvertUTF8(unsigned int, unsigned int *);
	char *qspFromQSPString(QSP_CHAR *);
	QSP_CHAR *qspToQSPString(char *);
	char *qspQSPToGameString(QSP_CHAR *, QSP_BOOL, QSP_BOOL);
	QSP_CHAR *qspGameToQSPString(char *, encoding_t);
	int qspSplitGameStr(char *, QSP_BOOL, QSP_CHAR *, char ***);
	int qspGameCodeWriteIntVal(char **, int, int, QSP_BOOL, QSP_BOOL);
	int qspGameCodeWriteVal(char **, int, QSP_CHAR *, QSP_BOOL, QSP_BOOL);

#endif
