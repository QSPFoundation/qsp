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
#include "bindings/qsp.h"
#include <oniguruma.h>

#include "qsp_config.h"

#ifndef QSP_DEFINES
    #define QSP_DEFINES

    #if defined(__GNUC__)
        #define INLINE static inline
    #else
        #define INLINE static
    #endif

    #define QSP_STATIC_LEN(x) (sizeof(x) / sizeof(QSP_CHAR) - 1)
    #if defined(__GNUC__)
        #define QSP_STATIC_STR(x) ((QSPString) { (x), (x) + QSP_STATIC_LEN(x) })
    #else
        #define QSP_STATIC_STR(x) (qspStringFromLen(x, QSP_STATIC_LEN(x)))
    #endif

    #define QSP_VER QSP_FMT(QSP_VER_STR)
    #define QSP_LOCALE "russian"
    #define QSP_STRCHAR QSP_FMT("$")
    #define QSP_TUPLECHAR QSP_FMT("%")
    #define QSP_LABEL QSP_FMT(":")
    #define QSP_COMMENT QSP_FMT("!")
    #define QSP_DIGITS QSP_FMT("0123456789")
    #define QSP_QUOTS QSP_FMT("'\"")
    #define QSP_DEFQUOT QSP_FMT("'")
    #define QSP_ESCDEFQUOT QSP_FMT("''")
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
    #define QSP_NEGATION QSP_FMT("-")
    #define QSP_ADD QSP_FMT("+")
    #define QSP_SUB QSP_FMT("-")
    #define QSP_DIV QSP_FMT("/")
    #define QSP_MUL QSP_FMT("*")
    #define QSP_USERFUNC QSP_FMT("@")
    #define QSP_DELIMS QSP_FMT(" \t&'\"()[]=!<>+-/*:,{}")

#endif
