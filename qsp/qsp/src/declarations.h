/* Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

/* MEMWATCH */

#ifdef _DEBUG
	#define MEMWATCH
	#define MEMWATCH_STDIO

	#include "memwatch.h"
#endif

/* -------- */

#include "qsp.h"
#include "onig/oniguruma.h"

#ifndef QSP_DEFINES
	#define QSP_DEFINES

	#ifdef _UNICODE
		#ifdef _MSC_VER
			#define QSP_FOPEN _wfopen
		#else
			#define QSP_FOPEN qspFileOpen
		#endif
		#ifdef _INTERNAL_WCS
			#define QSP_STRCPY qspStrCopy
			#define QSP_STRNCPY qspStrNCopy
			#define QSP_STRLEN qspStrLen
			#define QSP_STRSTR qspStrStr
			#define QSP_STRCHR qspStrChar
			#define QSP_STRCMP qspStrsComp
			#define QSP_STRCOLL qspStrsComp
			#define QSP_STRPBRK qspStrPBrk
		#else
			#define QSP_STRCPY wcscpy
			#define QSP_STRNCPY wcsncpy
			#define QSP_STRLEN (long)wcslen
			#define QSP_STRSTR wcsstr
			#define QSP_STRCHR wcschr
			#define QSP_STRCMP wcscmp
			#define QSP_STRCOLL wcscmp
			#define QSP_STRPBRK wcspbrk
		#endif
		#define QSP_CHRLWR towlower
		#define QSP_CHRUPR towupper
		#define QSP_WCSTOMBSLEN(a) (long)wcstombs(0, a, 0)
		#define QSP_WCSTOMBS wcstombs
		#define QSP_MBTOSB(a) ((a) % 256)
		#ifndef _WCHAR_SIZE
			#define QSP_ONIG_ENC (sizeof(wchar_t) == 2 ? ONIG_ENCODING_UTF16_LE : ONIG_ENCODING_UTF32_LE)
		#elif _WCHAR_SIZE == 2
			#define QSP_ONIG_ENC ONIG_ENCODING_UTF16_LE
		#else
			#define QSP_ONIG_ENC ONIG_ENCODING_UTF32_LE
		#endif
		#define QSP_FROM_OS_CHAR(a) qspReverseConvertUC(a, qspCP1251ToUnicodeTable)
		#define QSP_TO_OS_CHAR(a) qspDirectConvertUC(a, qspCP1251ToUnicodeTable)
		#define QSP_WCTOB
		#define QSP_BTOWC
	#else
		#define QSP_FOPEN fopen
		#define QSP_STRCPY strcpy
		#define QSP_STRNCPY strncpy
		#define QSP_STRLEN (long)strlen
		#define QSP_STRSTR strstr
		#define QSP_STRCHR strchr
		#define QSP_STRCMP strcmp
		#define QSP_STRPBRK strpbrk
		#define QSP_WCSTOMBSLEN (long)strlen
		#define QSP_WCSTOMBS strncpy
		#define QSP_MBTOSB(a) ((unsigned char)(a))
		#if defined(_WIN) || defined(_PSP)
			#define QSP_FROM_OS_CHAR
			#define QSP_TO_OS_CHAR
			#define QSP_WCTOB(a) qspReverseConvertUC(a, qspCP1251ToUnicodeTable)
			#define QSP_BTOWC(a) qspDirectConvertUC(a, qspCP1251ToUnicodeTable)
			#define QSP_CHRLWR(a) qspCP1251ToLowerTable[(unsigned char)(a)]
			#define QSP_CHRUPR(a) qspCP1251ToUpperTable[(unsigned char)(a)]
			#define QSP_STRCOLL(a, b) qspStrCmpSB(a, b, qspCP1251OrderTable)
			#define QSP_ONIG_ENC ONIG_ENCODING_CP1251
		#else
			#define QSP_FROM_OS_CHAR(a) qspReverseConvertSB(a, qspCP1251ToKOI8RTable)
			#define QSP_TO_OS_CHAR(a) qspDirectConvertSB(a, qspCP1251ToKOI8RTable)
			#define QSP_WCTOB(a) qspReverseConvertUC(a, qspKOI8RToUnicodeTable)
			#define QSP_BTOWC(a) qspDirectConvertUC(a, qspKOI8RToUnicodeTable)
			#define QSP_CHRLWR(a) qspKOI8RToLowerTable[(unsigned char)(a)]
			#define QSP_CHRUPR(a) qspKOI8RToUpperTable[(unsigned char)(a)]
			#define QSP_STRCOLL(a, b) qspStrCmpSB(a, b, qspKOI8ROrderTable)
			#define QSP_ONIG_ENC ONIG_ENCODING_KOI8_R
		#endif
	#endif
	#ifdef _MSC_VER
		#define QSP_TIME _time64
	#else
		#define QSP_TIME time
	#endif
	#if defined(_WIN) || defined(_PSP)
		#define QSP_PATHDELIMS QSP_FMT("/\\")
	#else
		#define QSP_PATHDELIMS QSP_FMT("/")
	#endif

	#define QSP_VER QSP_FMT("5.6.3")
	#define QSP_LOCALE "russian"
	#define QSP_STRCHAR QSP_FMT("$")
	#define QSP_LABEL QSP_FMT(":")
	#define QSP_COMMENT QSP_FMT("!")
	#define QSP_QUOTS QSP_FMT("'\"")
	#define QSP_STATDELIM QSP_FMT("&")
	#define QSP_COLONDELIM QSP_FMT(":")
	#define QSP_SPACES QSP_FMT(" \t")
	#define QSP_COMMA QSP_FMT(",")
	#define QSP_EQUAL QSP_FMT("=")
	#define QSP_NOTEQUAL1 QSP_FMT("!")
	#define QSP_NOTEQUAL2 QSP_FMT("<>")
	#define QSP_LESS QSP_FMT("<")
	#define QSP_GREAT QSP_FMT(">")
	#define QSP_LESSEQ1 QSP_FMT("<=")
	#define QSP_LESSEQ2 QSP_FMT("=<")
	#define QSP_GREATEQ1 QSP_FMT(">=")
	#define QSP_GREATEQ2 QSP_FMT("=>")
	#define QSP_LSBRACK QSP_FMT("[")
	#define QSP_RSBRACK QSP_FMT("]")
	#define QSP_LRBRACK QSP_FMT("(")
	#define QSP_RRBRACK QSP_FMT(")")
	#define QSP_APPEND QSP_FMT("&")
	#define QSP_UPLUS QSP_FMT("+")
	#define QSP_UMINUS QSP_FMT("-")
	#define QSP_ADD QSP_FMT("+")
	#define QSP_SUB QSP_FMT("-")
	#define QSP_DIV QSP_FMT("/")
	#define QSP_MUL QSP_FMT("*")
	#define QSP_DELIMS QSP_FMT(" \t&'\"()[]=!<>+-/*:,")

	/* Variables */
	extern volatile QSP_BOOL qspIsMustWait;

#endif
