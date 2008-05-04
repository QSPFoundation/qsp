/* Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <wchar.h>

/* MEMWATCH */

#ifdef _DEBUG
	#define MEMWATCH
	#define MEMWATCH_STDIO

	#include "memwatch.h"
#endif

#ifndef QSP_DEFINES
	#define QSP_DEFINES

	#ifdef _UNICODE
		typedef wchar_t QSP_CHAR;
		#define QSP_FMT2(x) L##x
		#define QSP_FMT(x) QSP_FMT2(x)
		#define QSP_STRCPY wcscpy
		#define QSP_STRNCPY wcsncpy
		#define QSP_STRLEN wcslen
		#define QSP_STRSTR wcsstr
		#define QSP_STRCHR wcschr
		#define QSP_WCSTOMBSLEN(a) wcstombs(0, a, 0)
		#define QSP_WCSTOMBS wcstombs
		#define QSP_FROM_OS_CHAR(a) qspReverseConvertUC(a, qspCP1251ToUnicodeTable)
		#define QSP_TO_OS_CHAR(a) qspDirectConvertUC(a, qspCP1251ToUnicodeTable)
		#define QSP_WCTOB
		#define QSP_BTOWC
	#else
		typedef char QSP_CHAR;
		#define QSP_FMT(x) x
		#define QSP_STRCPY strcpy
		#define QSP_STRNCPY strncpy
		#define QSP_STRLEN strlen
		#define QSP_STRSTR strstr
		#define QSP_STRCHR strchr
		#define QSP_WCSTOMBSLEN strlen
		#define QSP_WCSTOMBS strncpy
		#define QSP_FROM_OS_CHAR
		#define QSP_TO_OS_CHAR
		#define QSP_WCTOB(a) qspReverseConvertUC(a, qspCP1251ToUnicodeTable)
		#define QSP_BTOWC(a) qspDirectConvertUC(a, qspCP1251ToUnicodeTable)
	#endif

	#define QSP_VER QSP_FMT("0.0.6")
	#define QSP_LOCALE "russian"
	#define QSP_GAMEID QSP_FMT("QSPGAME")
	#define QSP_PASSWD QSP_FMT("No")
	#define QSP_QUOTS QSP_FMT("'\"")
	#define QSP_SPACES QSP_FMT(" \t")
	#define QSP_STRSDELIM QSP_FMT("\r\n")
	#define QSP_STARTLOC QSP_FMT("#")
	#define QSP_ENDLOC QSP_FMT("-")
	#define QSP_CODREMOV 5

	#define QSP_LEN(x) (sizeof(x) / sizeof(QSP_CHAR) - 1)

	#define QSP_TRUE 1
	#define QSP_FALSE 0

	/* Types */
	typedef long QSP_BOOL;
	typedef struct
	{
		QSP_CHAR *Name;
		QSP_CHAR *OnVisit;
	} QSPLocation;

	/* Variables */
	extern QSPLocation *qspLocs;
	extern long qspLocsCount;

	/* Tables */
	extern wchar_t qspCP1251ToUnicodeTable[];

	/* Functions */
	/* ---------------------------------------------------------------- coding.c */
	wchar_t qspDirectConvertUC(char, wchar_t *);
	char qspReverseConvertUC(wchar_t, wchar_t *);
	char *qspFromQSPString(QSP_CHAR *);
	QSP_CHAR *qspToQSPString(char *);
	char *qspQSPToGameString(QSP_CHAR *, QSP_BOOL, QSP_BOOL);
	QSP_CHAR *qspGameToQSPString(char *, QSP_BOOL, QSP_BOOL);
	long qspSplitGameStr(char *, QSP_BOOL, QSP_CHAR *, char ***);
	long qspGameCodeWriteIntVal(char **, long, long, QSP_BOOL, QSP_BOOL);
	long qspGameCodeWriteVal(char **, long, QSP_CHAR *, QSP_BOOL, QSP_BOOL);
	/* ---------------------------------------------------------------- locations.c */
	void qspCreateWorld(long);
	/* ---------------------------------------------------------------- text.c */
	long qspAddText(QSP_CHAR **, QSP_CHAR *, long, long, QSP_BOOL);
	QSP_BOOL qspIsInList(QSP_CHAR *, QSP_CHAR);
	QSP_CHAR *qspSkipSpaces(QSP_CHAR *);
	QSP_CHAR *qspDelSpc(QSP_CHAR *);
	QSP_BOOL qspIsEqual(QSP_CHAR *, QSP_CHAR *, long);
	void qspFreeStrs(void **, long, QSP_BOOL);
	QSP_CHAR *qspNumToStr(QSP_CHAR *, long);

#endif
