/* Copyright (C) 2001-2020 Valeriy Argunov (val AT time DOT guru) */
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

#ifndef QSP_MAINDEFINES
    #define QSP_MAINDEFINES

    #define QSP_STARTLOC QSP_FMT("#")
    #define QSP_ENDLOC QSP_FMT("---")
    #define TXT2GAM_BOM "\xFF\xFE"

    enum Mode
    {
        QSP_ENCODE_INTO_GAME,
        QSP_DECODE_INTO_TEXT,
        QSP_EXTRACT_STRINGS,
        QSP_EXTRACT_QSTRINGS
    };

#endif
