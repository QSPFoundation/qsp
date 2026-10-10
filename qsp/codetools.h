/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "mathops.h"

#ifndef QSP_CODETOOLSDEFINES
    #define QSP_CODETOOLSDEFINES

    #define QSP_EOLEXT QSP_FMT("_")
    #define QSP_CACHEDCODEBUCKETS 64
    #define QSP_MAXCACHEDCODEBUCKETSIZE 8
    #define QSP_MAXCACHEDCODELEN 8192

    enum
    {
        qspArgNone, /* no argument */
        qspArgEmpty, /* empty text */
        qspArgExpression, /* compiled on the first evaluation */
        qspArgCompiled, /* the compiled expression */
        qspArgNumber,
        qspArgString, /* the text between the quotes, without doubled quotes & subexpressions */
        qspArgCode /* the text between the brackets */
    };

    typedef struct
    {
        union
        {
            QSPString Text; /* qspArgEmpty, qspArgExpression, qspArgString, qspArgCode */
            QSPMathExpression *Expression; /* qspArgCompiled */
            QSP_BIGINT Number; /* qspArgNumber */
        } Data;
        QSP_TINYINT Type; /* selects the member of Data */
    } QSPCachedArg;

    typedef struct
    {
        QSPString Name;
        QSPCachedArg Index;
    } QSPCachedTarget;

    typedef struct
    {
        QSPCachedTarget *Targets;
        int TargetsCount;
        QSP_CHAR Operation; /* 0 for LOCAL without a value */
        QSPCachedArg Value;
    } QSPCachedAssignment;

    typedef struct QSPCachedLoop_s QSPCachedLoop;
    typedef struct QSPCachedAct_s QSPCachedAct;

    typedef struct
    {
        QSP_TINYINT Stat; /* selects the member of Data */
        QSP_TINYINT ErrorCode;
        QSP_TINYINT ArgsCount; /* of Data.Args or Data.Act->Args */
        int EndPos;
        union
        {
            QSPCachedArg *Args; /* regular statements */
            QSPCachedLoop *Loop; /* LOOP */
            QSPCachedAssignment *Assignment; /* SET, LOCAL */
            QSPCachedAct *Act; /* ACT */
        } Data; /* 0 for invalid statement, ErrorCode stores the error */
    } QSPCachedStat;

    typedef struct
    {
        QSPString Str;
        int LineNum;
        int LinesToEnd; /* lines to skip to reach the end of multiline block */
        int LinesToElse; /* lines to skip to reach the next ELSE branch within multiline block */
        QSPCachedStat *Stats;
        int StatsCount;
        QSP_TINYINT IsMultiline;
    } QSPLineOfCode;

    typedef struct QSPCachedLoop_s
    {
        QSPCachedArg Condition;
        QSPLineOfCode Initializer;
        QSPLineOfCode Iterator;
    } QSPCachedLoop;

    typedef struct
    {
        QSPString Name;
        int LineIndex;
    } QSPCodeLabel;

    typedef struct
    {
        QSPLineOfCode *Lines;
        int LinesCount;
        QSPCodeLabel *Labels;
        int LabelsCount;
        int RefsCount;
    } QSPCodeBlock;

    typedef struct QSPCachedAct_s
    {
        QSPCachedArg Args[2]; /* the name & the image */
        QSPCodeBlock *OnPressCode; /* single-line ACT only, prepared on the first execution */
    } QSPCachedAct;

    typedef struct
    {
        QSPString Text;
        QSPCodeBlock *Code;
    } QSPCachedCodeBlock;

    typedef struct
    {
        QSPCachedCodeBlock Blocks[QSP_MAXCACHEDCODEBUCKETSIZE];
        int BlocksCount;
        int BlockToEvict;
    } QSPCachedCodeBlocksBucket;

    /* External functions */
    void qspInitLineOfCode(QSPLineOfCode *line, QSPString str, int lineNum);
    void qspFreePrepLines(QSPLineOfCode *lines, int count);
    QSPString qspJoinPrepLines(QSPLineOfCode *s, int count, QSPString delim);
    int qspCollectLabels(QSPLineOfCode *lines, int linesCount, QSPCodeLabel **labels);
    void qspFreeLabels(QSPCodeLabel *labels, int count);
    QSPCodeBlock *qspGetSinglelineActCode(QSPLineOfCode *line, int statPos, int endPos);
    QSP_CHAR *qspDelimPos(QSPString txt, QSP_CHAR ch);
    QSP_CHAR *qspKeywordPos(QSPString txt, QSPString str, QSP_BOOL isIsolated);
    void qspPrepareStringToExecution(QSPString *str);
    QSPCodeBlock *qspPreprocessData(QSPString data);
    void qspClearAllCodeBlocks(QSP_BOOL toInit);
    QSPCodeBlock *qspGetCachedCodeBlock(QSPString s);
    QSPVariant qspCalculateArgValue(QSPCachedArg *arg);

    INLINE QSPCodeBlock *qspNewCodeBlock(QSPLineOfCode *lines, int linesCount)
    {
        QSPCodeBlock *code = (QSPCodeBlock *)malloc(sizeof(QSPCodeBlock));
        code->Lines = lines;
        code->LinesCount = linesCount;
        code->LabelsCount = qspCollectLabels(lines, linesCount, &code->Labels);
        code->RefsCount = 1;
        return code;
    }

    INLINE void qspAcquireCodeBlock(QSPCodeBlock *code)
    {
        ++code->RefsCount;
    }

    INLINE void qspReleaseCodeBlock(QSPCodeBlock *code)
    {
        if (--code->RefsCount == 0)
        {
            qspFreePrepLines(code->Lines, code->LinesCount);
            qspFreeLabels(code->Labels, code->LabelsCount);
            free(code);
        }
    }

#endif
