/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
    QSP_CHAR *qspDelimPos(QSPString txt, QSP_CHAR ch);
    QSP_CHAR *qspStrPos(QSPString txt, QSPString str, QSP_BOOL isIsolated);
    void qspPrepareStringToExecution(QSPString *str);
    int qspPreprocessData(QSPString data, QSPLineOfCode **strs);

#endif
