/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
#include <time.h>
#include <math.h>
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

#include "qsp_config.h"
#include "bindings/bindings_config.h"
#include "bindings/qsp.h"
#include <oniguruma.h>

#ifndef QSP_DEFINES
    #define QSP_DEFINES

    #if defined(__GNUC__)
        #define INLINE static inline
    #else
        #define INLINE static
    #endif

    #if defined(__GNUC__) || defined(__clang__)
        #define QSP_UNUSED(x) unused_ ## x __attribute__((unused))
    #elif defined(_MSC_VER)
        #define QSP_UNUSED(x) \
            __pragma(warning(push)) \
            __pragma(warning(suppress: 4100)) \
            unused_ ## x \
            __pragma(warning(pop))
    #else
        #define QSP_UNUSED(x) unused_ ## x
    #endif

    #define QSP_STATIC_LEN(x) (sizeof(x) / sizeof(QSP_CHAR) - 1)
    #define QSP_STATIC_STR(x) (qspStringFromLen(x, QSP_STATIC_LEN(x)))
    #define QSP_CHAR_LEN 1

    #define QSP_DEFINE_SPECIAL_CHAR(name, ch) enum { name##_CHAR = ch }; /* use enum to make it compile-time constant */

    #define QSP_VER QSP_FMT(QSP_VER_STR)
    #define QSP_LOCALE "C"

    #define QSP_NUMTYPE      QSP_FMT("#")
    QSP_DEFINE_SPECIAL_CHAR(QSP_NUMTYPE,    QSP_FMT('#'))

    #define QSP_STRTYPE      QSP_FMT("$")
    QSP_DEFINE_SPECIAL_CHAR(QSP_STRTYPE,    QSP_FMT('$'))

    #define QSP_TUPLETYPE    QSP_FMT("%")
    QSP_DEFINE_SPECIAL_CHAR(QSP_TUPLETYPE,  QSP_FMT('%'))

    #define QSP_LABEL        QSP_FMT(":")
    QSP_DEFINE_SPECIAL_CHAR(QSP_LABEL,      QSP_FMT(':'))

    #define QSP_COMMENT      QSP_FMT("!")
    QSP_DEFINE_SPECIAL_CHAR(QSP_COMMENT,    QSP_FMT('!'))

    #define QSP_USERFUNC     QSP_FMT("@")
    QSP_DEFINE_SPECIAL_CHAR(QSP_USERFUNC,   QSP_FMT('@'))

    #define QSP_DIGITS       QSP_FMT("0123456789")
    #define QSP_QUOTS        QSP_FMT("'\"")
    #define QSP_DEFQUOT      QSP_FMT("'")
    #define QSP_ESCDEFQUOT   QSP_FMT("''")
    #define QSP_SPACES       QSP_FMT(" \t\r\n")

    #define QSP_STATDELIM    QSP_FMT("&")
    QSP_DEFINE_SPECIAL_CHAR(QSP_STATDELIM,  QSP_FMT('&'))

    #define QSP_COLONDELIM   QSP_FMT(":")
    QSP_DEFINE_SPECIAL_CHAR(QSP_COLONDELIM, QSP_FMT(':'))

    #define QSP_COMMA        QSP_FMT(",")
    QSP_DEFINE_SPECIAL_CHAR(QSP_COMMA,      QSP_FMT(','))

    #define QSP_EQUAL        QSP_FMT("=")
    QSP_DEFINE_SPECIAL_CHAR(QSP_EQUAL,      QSP_FMT('='))

    #define QSP_NOTEQUAL1    QSP_FMT("!")
    #define QSP_NOTEQUAL2    QSP_FMT("<>")

    #define QSP_LESS         QSP_FMT("<")
    #define QSP_GREAT        QSP_FMT(">")
    #define QSP_LESSEQ1      QSP_FMT("<=")
    #define QSP_LESSEQ2      QSP_FMT("=<")
    #define QSP_GREATEQ1     QSP_FMT(">=")
    #define QSP_GREATEQ2     QSP_FMT("=>")
    #define QSP_APPEND       QSP_FMT("&")

    #define QSP_NEGATION     QSP_FMT("-")
    QSP_DEFINE_SPECIAL_CHAR(QSP_NEGATION,   QSP_FMT('-'))

    #define QSP_ADD          QSP_FMT("+")
    QSP_DEFINE_SPECIAL_CHAR(QSP_ADD,        QSP_FMT('+'))

    #define QSP_SUB          QSP_FMT("-")
    QSP_DEFINE_SPECIAL_CHAR(QSP_SUB,        QSP_FMT('-'))

    #define QSP_DIV          QSP_FMT("/")
    QSP_DEFINE_SPECIAL_CHAR(QSP_DIV,        QSP_FMT('/'))

    #define QSP_MUL          QSP_FMT("*")
    QSP_DEFINE_SPECIAL_CHAR(QSP_MUL,        QSP_FMT('*'))

    #define QSP_LSBRACK      QSP_FMT("[")
    QSP_DEFINE_SPECIAL_CHAR(QSP_LSBRACK,    QSP_FMT('['))

    #define QSP_RSBRACK      QSP_FMT("]")
    QSP_DEFINE_SPECIAL_CHAR(QSP_RSBRACK,    QSP_FMT(']'))

    #define QSP_LRBRACK      QSP_FMT("(")
    QSP_DEFINE_SPECIAL_CHAR(QSP_LRBRACK,    QSP_FMT('('))

    #define QSP_RRBRACK      QSP_FMT(")")
    QSP_DEFINE_SPECIAL_CHAR(QSP_RRBRACK,    QSP_FMT(')'))

    #define QSP_LQUOT        QSP_FMT("{")
    QSP_DEFINE_SPECIAL_CHAR(QSP_LQUOT,      QSP_FMT('{'))

    #define QSP_RQUOT        QSP_FMT("}")
    QSP_DEFINE_SPECIAL_CHAR(QSP_RQUOT,      QSP_FMT('}'))

    #define QSP_DELIMS       QSP_FMT(" \t&'\"()[]=!<>+-/*:,{}\r\n")

    #define QSP_LOC_COUNTER QSP_FMT("COUNTER")
    #define QSP_LOC_USERCOMMAND QSP_FMT("USERCOM")
    #define QSP_LOC_ACTSELECTED QSP_FMT("ONACTSEL")
    #define QSP_LOC_OBJADDED QSP_FMT("ONOBJADD")
    #define QSP_LOC_OBJDELETED QSP_FMT("ONOBJDEL")
    #define QSP_LOC_OBJSELECTED QSP_FMT("ONOBJSEL")
    #define QSP_LOC_NEWLOC QSP_FMT("ONNEWLOC")
    #define QSP_LOC_GAMELOADED QSP_FMT("ONGLOAD")
    #define QSP_LOC_GAMETOBESAVED QSP_FMT("ONGSAVE")

#endif
