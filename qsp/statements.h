/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "codetools.h"

#ifndef QSP_STATSDEFINES
    #define QSP_STATSDEFINES

    #define QSP_STATSLEVELS 3
    #define QSP_MAXSTATSNAMES 100
    #define QSP_MAXSTATARGS 20

    #define QSP_KEYWORD_ELSE QSP_FMT("ELSE")
    #define QSP_KEYWORD_ELSEIF QSP_FMT("ELSEIF")
    #define QSP_KEYWORD_WHILE QSP_FMT("WHILE")
    #define QSP_KEYWORD_STEP QSP_FMT("STEP")

    typedef void (*QSP_STATEMENT)(QSPVariant *args, QSP_TINYINT argsCount, QSP_TINYINT statCode);

    typedef struct
    {
        QSP_TINYINT Code;
        QSPString Name;
        QSP_BOOL IsIsolated; /* indicates whether the statement name has to be separated from arguments */
    } QSPStatName;

    typedef struct
    {
        QSP_TINYINT MinArgsCount;
        QSP_TINYINT MaxArgsCount;
        QSP_TINYINT ArgsTypes[QSP_MAXSTATARGS];
        QSP_STATEMENT Func;
    } QSPStatement;

    enum
    {
        QSP_FLOW_EXECUTE,
        QSP_FLOW_JUMPTOSPECIFIED
    };

    enum
    {
        QSP_STAT_UNKNOWN,
        QSP_STAT_LABEL,
        QSP_STAT_COMMENT,
        QSP_STAT_IMPLICITSTATEMENT,
        QSP_STAT_USERCALL,
        QSP_STAT_END,
        QSP_STAT_LOOP,
        QSP_STAT_IF,
        QSP_STAT_ELSE,
        QSP_STAT_ELSEIF,
        QSP_STAT_LOCAL,
        QSP_STAT_SET,
        QSP_STAT_EXIT,
        QSP_STAT_JUMP,
        QSP_STAT_GOSUB,
        QSP_STAT_GOTO,
        QSP_STAT_XGOTO,
        QSP_STAT_DYNAMIC,
        QSP_STAT_EXEC,
        QSP_STAT_SETVAR,
        QSP_STAT_INSVAR,
        QSP_STAT_UNPACKARR,
        QSP_STAT_COPYARR,
        QSP_STAT_SORTARR,
        QSP_STAT_SCANSTR,
        QSP_STAT_KILLVAR,
        QSP_STAT_KILLALL,
        QSP_STAT_ADDOBJ,
        QSP_STAT_DELOBJ,
        QSP_STAT_MODOBJ,
        QSP_STAT_RESETOBJ,
        QSP_STAT_KILLOBJ,
        QSP_STAT_UNSELECT,
        QSP_STAT_ACT,
        QSP_STAT_DELACT,
        QSP_STAT_CLA,
        QSP_STAT_MCLEAR,
        QSP_STAT_MNL,
        QSP_STAT_MPL,
        QSP_STAT_MP,
        QSP_STAT_CLEAR,
        QSP_STAT_NL,
        QSP_STAT_PL,
        QSP_STAT_P,
        QSP_STAT_CLS,
        QSP_STAT_CMDCLEAR,
        QSP_STAT_SHOWACTS,
        QSP_STAT_SHOWOBJS,
        QSP_STAT_SHOWVARS,
        QSP_STAT_SHOWINPUT,
        QSP_STAT_REFINT,
        QSP_STAT_MENU,
        QSP_STAT_MSG,
        QSP_STAT_VIEW,
        QSP_STAT_WAIT,
        QSP_STAT_PLAY,
        QSP_STAT_CLOSE,
        QSP_STAT_CLOSEALL,
        QSP_STAT_INCLIB,
        QSP_STAT_FREELIB,
        QSP_STAT_OPENGAME,
        QSP_STAT_SAVEGAME,
        QSP_STAT_OPENQST,
        QSP_STAT_SETTIMER,

        QSP_STAT_LASTSTATEMENT
    };

    extern QSPStatement qspStats[QSP_STAT_LASTSTATEMENT];
    extern QSPStatName qspStatsNames[QSP_STATSLEVELS][QSP_MAXSTATSNAMES];
    extern int qspStatsNamesCounts[QSP_STATSLEVELS];
    extern int qspStatMaxLen;

    /* External functions */
    void qspInitStats(void);
    QSP_TINYINT qspGetStatArgs(QSPCachedStat *stat, QSPCachedArg *statArgs, QSPVariant *args);
    QSP_BOOL qspExecCode(QSPCodeBlock *code, int startLine, int endLine, QSP_BOOL toTrackLines, QSPString *jumpTo);
    QSP_BOOL qspExecCodeBlockWithLocals(QSPCodeBlock *code, int startLine, int endLine, QSP_BOOL toTrackLines, QSPString *jumpTo);
    void qspExecStringAsCodeWithArgs(QSPString s, QSPVariant *args, QSP_TINYINT count, QSPVariant *res);
    void qspExecStringAsCode(QSPString s);

#endif
