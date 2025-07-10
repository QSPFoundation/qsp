/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "codetools.h"
#include "variant.h"

#ifndef QSP_STATSDEFINES
    #define QSP_STATSDEFINES

    #define QSP_STATSLEVELS 3
    #define QSP_MAXSTATSNAMES 100
    #define QSP_STATMAXARGS 20
    #define QSP_STATELSE QSP_FMT("ELSE")
    #define QSP_STATELSEIF QSP_FMT("ELSEIF")
    #define QSP_STATLOOPWHILE QSP_FMT("WHILE")
    #define QSP_STATLOOPSTEP QSP_FMT("STEP")

    typedef void (*QSP_STATEMENT)(QSPVariant *args, QSP_TINYINT argsCount, QSP_TINYINT statCode);

    typedef struct
    {
        QSP_TINYINT Code;
        QSPString Name;
    } QSPStatName;

    typedef struct
    {
        QSP_TINYINT MinArgsCount;
        QSP_TINYINT MaxArgsCount;
        QSP_TINYINT ArgsTypes[QSP_STATMAXARGS];
        QSP_STATEMENT Func;
    } QSPStatement;

    enum
    {
        qspFlowExecute,
        qspFlowJumpToSpecified
    };

    enum
    {
        qspStatUnknown,
        qspStatLabel,
        qspStatComment,
        qspStatImplicitStatement,
        qspStatEnd,
        qspStatLoop,
        qspStatIf,
        qspStatElse,
        qspStatElseIf,
        qspStatLocal,
        qspStatSet,
        qspStatExit,
        qspStatJump,
        qspStatGoSub,
        qspStatGoTo,
        qspStatXGoTo,
        qspStatDynamic,
        qspStatExec,
        qspStatSetVar,
        qspStatUnpackArr,
        qspStatCopyArr,
        qspStatSortArr,
        qspStatScanStr,
        qspStatKillVar,
        qspStatKillAll,
        qspStatAddObj,
        qspStatDelObj,
        qspStatKillObj,
        qspStatUnSelect,
        qspStatAct,
        qspStatDelAct,
        qspStatClA,
        qspStatMClear,
        qspStatMNL,
        qspStatMPL,
        qspStatMP,
        qspStatClear,
        qspStatNL,
        qspStatPL,
        qspStatP,
        qspStatClS,
        qspStatCmdClear,
        qspStatShowActs,
        qspStatShowObjs,
        qspStatShowVars,
        qspStatShowInput,
        qspStatRefInt,
        qspStatMenu,
        qspStatMsg,
        qspStatView,
        qspStatWait,
        qspStatPlay,
        qspStatClose,
        qspStatCloseAll,
        qspStatIncLib,
        qspStatFreeLib,
        qspStatOpenGame,
        qspStatSaveGame,
        qspStatOpenQst,
        qspStatSetTimer,

        qspStatLast_Statement
    };

    extern QSPStatement qspStats[qspStatLast_Statement];
    extern QSPStatName qspStatsNames[QSP_STATSLEVELS][QSP_MAXSTATSNAMES];
    extern int qspStatsNamesCounts[QSP_STATSLEVELS];
    extern int qspStatMaxLen;

    /* External functions */
    void qspInitStats(void);
    QSP_TINYINT qspGetStatArgs(QSPString s, QSPCachedStat *stat, QSPVariant *args);
    QSP_BOOL qspExecCode(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSPString *jumpTo);
    QSP_BOOL qspExecCodeBlockWithLocals(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSPString *jumpTo);
    void qspExecStringAsCodeWithArgs(QSPString s, QSPVariant *args, QSP_TINYINT count, int codeOffset, QSPVariant *res);

#endif
