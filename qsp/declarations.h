/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include "bindings/bindings_config.h"
#include "qsp.h"
#include "onig/oniguruma.h"

#ifndef QSP_DEFINES
	#define QSP_DEFINES

	static int qspEndiannessTestValue = 1;

	#ifdef _UNICODE
		#ifdef _MSC_VER
			#define QSP_FOPEN _wfopen
		#else
			#define QSP_FOPEN qspFileOpen
		#endif
		#define QSP_STRCOLL qspStrsComp
		#define QSP_CHRLWR qspToWLower
		#define QSP_CHRUPR qspToWUpper
		#define QSP_ONIG_ENC ((*(char *)&(qspEndiannessTestValue) == 1) ? \
			(sizeof(QSP_CHAR) == 2 ? ONIG_ENCODING_UTF16_LE : ONIG_ENCODING_UTF32_LE) : \
			(sizeof(QSP_CHAR) == 2 ? ONIG_ENCODING_UTF16_BE : ONIG_ENCODING_UTF32_BE))
		#define QSP_FROM_OS_CHAR(a) qspReverseConvertUC(a, qspCP1251ToUnicodeTable)
		#define QSP_TO_OS_CHAR(a) qspDirectConvertUC(a, qspCP1251ToUnicodeTable)
		#define QSP_WCTOB
		#define QSP_BTOWC
	#else
		#define QSP_FOPEN fopen
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
	#define QSP_FIXBYTESORDER(a) ((*(char *)&(qspEndiannessTestValue) == 1) ? \
		(a) : \
		((unsigned short)(((a) << 8) | ((a) >> 8))))
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

	#define QSP_VER QSP_FMT("5.7.0")
	#define QSP_LOCALE "russian"
	#define QSP_STRCHAR QSP_FMT("$")
	#define QSP_LABEL QSP_FMT(":")
	#define QSP_COMMENT QSP_FMT("!")
	#define QSP_QUOTS QSP_FMT("'\"")
	#define QSP_LQUOT QSP_FMT("{")
	#define QSP_RQUOT QSP_FMT("}")
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
	#define QSP_DELIMS QSP_FMT(" \t&'\"()[]=!<>+-/*:,{}")

#endif
