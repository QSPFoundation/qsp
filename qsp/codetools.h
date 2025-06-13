/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
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

#ifndef QSP_CODETOOLSDEFINES
    #define QSP_CODETOOLSDEFINES

    #define QSP_EOLEXT QSP_FMT("_")
    #define QSP_PREEOLEXT QSP_FMT(" ")

    typedef struct
    {
        int StartPos;
        int EndPos;
    } QSPCachedArg;

    typedef struct
    {
        QSP_TINYINT Stat;
        QSP_TINYINT ErrorCode;
        int ParamPos;
        int EndPos;
        QSP_TINYINT ArgsCount;
        QSPCachedArg *Args;
    } QSPCachedStat;

    typedef struct
    {
        QSPString Str;
        int LineNum;
        QSPString Label;
        int LinesToEnd; /* lines to skip to reach the end of multiline block */
        int LinesToElse; /* lines to skip to reach the next ELSE branch within multiline block */
        QSPCachedStat *Stats;
        int StatsCount;
        QSP_TINYINT IsMultiline;
    } QSPLineOfCode;

    /* External functions */
    QSPString qspGetLineLabel(QSPString str);
    void qspInitLineOfCode(QSPLineOfCode *line, QSPString str, int lineNum);
    void qspFreeLineOfCode(QSPLineOfCode *line);
    void qspFreePrepLines(QSPLineOfCode *strs, int count);
    void qspCopyPrepStatements(QSPCachedStat **dest, QSPCachedStat *src, int start, int end, int codeOffset);
    void qspCopyPrepLines(QSPLineOfCode **dest, QSPLineOfCode *src, int start, int end);
    QSPString qspJoinPrepLines(QSPLineOfCode *s, int count, QSPString delim);
    void qspPrepareStringToExecution(QSPString *str);
    int qspPreprocessData(QSPString data, QSPLineOfCode **strs);

#endif
