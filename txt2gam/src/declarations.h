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
		#define QSP_MBSTOWCSLEN(a) mbstowcs(0, a, 0)
		#define QSP_MBSTOWCS mbstowcs
		#define QSP_FROM_OS_CHAR(a) qspReverseConvertUCS2LE(a, qspCP1251ToUCS2LETable)
		#define QSP_TO_OS_CHAR(a) qspDirectConvertUCS2LE(a, qspCP1251ToUCS2LETable)
		#define QSP_UCS2TOB
		#define QSP_BTOUCS2
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
		#define QSP_MBSTOWCSLEN strlen
		#define QSP_MBSTOWCS strncpy
		#define QSP_FROM_OS_CHAR
		#define QSP_TO_OS_CHAR
		#define QSP_UCS2TOB(a) qspReverseConvertUCS2LE(a, qspCP1251ToUCS2LETable)
		#define QSP_BTOUCS2(a) qspDirectConvertUCS2LE(a, qspCP1251ToUCS2LETable)
	#endif

	#define QSP_VER QSP_FMT("0.1.1")
	#define QSP_LOCALE "russian"

	#define QSP_TRUE 1
	#define QSP_FALSE 0

	#define QSP_BYTE(a) (a & 0xFF)
	#define QSP_DWORD(a, b, c, d) (((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | (((c) & 0xFF) << 8) | ((d) & 0xFF)

	#define QSP_UTF8TOB(a) qspReverseConvertUTF8(a, qspCP1251ToUTF8Table)

	/* Types */
	typedef int QSP_BOOL;
	typedef enum _encoding { CP1251, UTF8, UCS2LE } encoding_t;

#endif
