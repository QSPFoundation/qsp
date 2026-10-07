/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "statements.h"
#include "actions.h"
#include "callbacks.h"
#include "codetools.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "mathops.h"
#include "memory.h"
#include "menu.h"
#include "objects.h"
#include "playlist.h"
#include "text.h"
#include "variables.h"
#include "variant.h"

QSPStatement qspStats[qspStatLast_Statement];
QSPStatName qspStatsNames[QSP_STATSLEVELS][QSP_MAXSTATSNAMES];
int qspStatsNamesCounts[QSP_STATSLEVELS];
int qspStatMaxLen = 0;

INLINE void qspAddStatement(QSP_TINYINT statCode, QSP_STATEMENT func, QSP_TINYINT minArgs, int maxArgs, ...);
INLINE void qspAddStatName(QSP_TINYINT statCode, QSPString statName, QSP_BOOL isIsolated, int level);
INLINE int qspStatsCompare(const void *statName1, const void *statName2);
INLINE int qspSearchElse(QSPLineOfCode *lines, int start, int end);
INLINE int qspSearchEnd(QSPLineOfCode *lines, int start, int end);
INLINE int qspSearchLabel(QSPCodeBlock *code, int startLine, int endLine, QSPString name);
INLINE QSP_BOOL qspExecString(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspExecMultilineCode(QSPCodeBlock *code, int endLine, int codeOffset, QSPString *jumpTo, int *lineInd, int *action);
INLINE QSP_BOOL qspExecSinglelineCode(QSPCodeBlock *code, int endLine, QSPString *jumpTo, int *lineInd, int *action);
INLINE QSP_BOOL qspExecStringWithLocals(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspStatementIf(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspCheckCondition(QSPCachedArg *condition);
INLINE QSP_BOOL qspStatementSinglelineLoop(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspStatementMultilineLoop(QSPCodeBlock *code, int lineInd, int endLine, int codeOffset, QSPString *jumpTo);
INLINE void qspStatementUserCall(QSPCachedStat *stat);
INLINE void qspStatementImplicitStatement(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementAddText(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementClear(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementGoSub(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementGoTo(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementWait(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementSetTimer(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementShowWin(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementRefInt(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementView(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementMsg(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementExec(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);
INLINE void qspStatementDynamic(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg);

INLINE void qspAddStatement(QSP_TINYINT statCode, QSP_STATEMENT func, QSP_TINYINT minArgs, int maxArgs, ...)
{
    qspStats[statCode].Func = func;
    qspStats[statCode].MinArgsCount = minArgs;
    qspStats[statCode].MaxArgsCount = (QSP_TINYINT)maxArgs;
    if (maxArgs > 0)
    {
        int i;
        va_list marker;
        QSP_BOOL isFinished = QSP_FALSE;
        QSP_TINYINT lastType = QSP_TYPE_UNDEF;
        va_start(marker, maxArgs);
        for (i = 0; i < maxArgs; ++i)
        {
            if (!isFinished)
            {
                QSP_TINYINT curType = (QSP_TINYINT)va_arg(marker, int);
                if (curType == QSP_TYPE_TERM)
                    isFinished = QSP_TRUE; /* use lastType for the rest of arguments */
                else
                    lastType = curType;
            }
            qspStats[statCode].ArgsTypes[i] = lastType;
        }
        va_end(marker);
    }
}

INLINE void qspAddStatName(QSP_TINYINT statCode, QSPString statName, QSP_BOOL isIsolated, int level)
{
    int count, len = qspStrLen(statName);
    count = qspStatsNamesCounts[level]++;
    qspStatsNames[level][count].Name = statName;
    qspStatsNames[level][count].Code = statCode;
    qspStatsNames[level][count].IsIsolated = isIsolated;
    /* Max length */
    if (len > qspStatMaxLen) qspStatMaxLen = len;
}

INLINE int qspStatsCompare(const void *statName1, const void *statName2)
{
    return qspStrsCompare(((QSPStatName *)statName1)->Name, ((QSPStatName *)statName2)->Name);
}

void qspInitStats(void)
{
    /*
    Format:
        qspAddStatement(
            Statement,
            Handler,
            Minimum arguments' count,
            Maximum arguments' count,
            Arguments' types [optional, QSP_TYPE_TERM to use the last known type for the rest of arguments]
        );
    */
    int i;
    for (i = 0; i < QSP_STATSLEVELS; ++i) qspStatsNamesCounts[i] = 0;
    qspStatMaxLen = 0;
    qspAddStatement(qspStatImplicitStatement, qspStatementImplicitStatement, 1, 1, QSP_TYPE_UNDEF);
    qspAddStatement(qspStatLabel, 0, 0, 0);
    qspAddStatement(qspStatComment, 0, 0, 0);
    qspAddStatement(qspStatUserCall, 0, 1, QSP_MAXSTATARGS, QSP_TYPE_STR, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddStatement(qspStatEnd, 0, 0, 0);

    qspAddStatement(qspStatLoop, 0, 0, 0);
    qspAddStatement(qspStatIf, 0, 1, 1, QSP_TYPE_BOOL);
    qspAddStatement(qspStatElse, 0, 0, 0);
    qspAddStatement(qspStatElseIf, 0, 1, 1, QSP_TYPE_BOOL);

    qspAddStatement(qspStatLocal, 0, 0, 0);
    qspAddStatement(qspStatSet, 0, 0, 0);

    qspAddStatement(qspStatExit, 0, 0, 0);
    qspAddStatement(qspStatJump, 0, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatGoSub, qspStatementGoSub, 1, QSP_MAXSTATARGS, QSP_TYPE_STR, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddStatement(qspStatGoTo, qspStatementGoTo, 1, QSP_MAXSTATARGS, QSP_TYPE_STR, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddStatement(qspStatXGoTo, qspStatementGoTo, 1, QSP_MAXSTATARGS, QSP_TYPE_STR, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddStatement(qspStatDynamic, qspStatementDynamic, 1, QSP_MAXSTATARGS, QSP_TYPE_CODE, QSP_TYPE_UNDEF, QSP_TYPE_TERM);
    qspAddStatement(qspStatExec, qspStatementExec, 1, 1, QSP_TYPE_STR);

    qspAddStatement(qspStatSetVar, qspStatementSetVar, 2, 3, QSP_TYPE_VARREF, QSP_TYPE_UNDEF, QSP_TYPE_UNDEF);
    qspAddStatement(qspStatUnpackArr, qspStatementUnpackArr, 2, 4, QSP_TYPE_VARREF, QSP_TYPE_TUPLE, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddStatement(qspStatCopyArr, qspStatementCopyArr, 2, 4, QSP_TYPE_VARREF, QSP_TYPE_VARREF, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddStatement(qspStatSortArr, qspStatementSortArr, 1, 2, QSP_TYPE_VARREF, QSP_TYPE_BOOL);
    qspAddStatement(qspStatScanStr, qspStatementScanStr, 3, 4, QSP_TYPE_VARREF, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddStatement(qspStatKillVar, qspStatementKillVar, 0, 2, QSP_TYPE_VARREF, QSP_TYPE_UNDEF);
    qspAddStatement(qspStatKillAll, qspStatementClear, 0, 0);

    qspAddStatement(qspStatAddObj, qspStatementAddObject, 1, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddStatement(qspStatDelObj, qspStatementDelObj, 1, 2, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddStatement(qspStatModObj, qspStatementModObj, 2, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_STR);
    qspAddStatement(qspStatResetObj, qspStatementResetObj, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatKillObj, qspStatementDelObj, 0, 1, QSP_TYPE_NUM);
    qspAddStatement(qspStatUnSelect, qspStatementUnSelect, 0, 0);

    qspAddStatement(qspStatAct, 0, 1, 2, QSP_TYPE_STR, QSP_TYPE_STR);
    qspAddStatement(qspStatDelAct, qspStatementDelAct, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatClA, qspStatementClear, 0, 0);

    qspAddStatement(qspStatMClear, qspStatementClear, 0, 0);
    qspAddStatement(qspStatMNL, qspStatementAddText, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatMPL, qspStatementAddText, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatMP, qspStatementAddText, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatClear, qspStatementClear, 0, 0);
    qspAddStatement(qspStatNL, qspStatementAddText, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatPL, qspStatementAddText, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatP, qspStatementAddText, 1, 1, QSP_TYPE_STR);

    qspAddStatement(qspStatClS, qspStatementClear, 0, 0);
    qspAddStatement(qspStatCmdClear, qspStatementClear, 0, 0);
    qspAddStatement(qspStatShowActs, qspStatementShowWin, 1, 1, QSP_TYPE_BOOL);
    qspAddStatement(qspStatShowObjs, qspStatementShowWin, 1, 1, QSP_TYPE_BOOL);
    qspAddStatement(qspStatShowVars, qspStatementShowWin, 1, 1, QSP_TYPE_BOOL);
    qspAddStatement(qspStatShowInput, qspStatementShowWin, 1, 1, QSP_TYPE_BOOL);
    qspAddStatement(qspStatRefInt, qspStatementRefInt, 0, 0);

    qspAddStatement(qspStatMenu, qspStatementShowMenu, 1, 3, QSP_TYPE_VARREF, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddStatement(qspStatMsg, qspStatementMsg, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatView, qspStatementView, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatWait, qspStatementWait, 1, 1, QSP_TYPE_NUM);

    qspAddStatement(qspStatPlay, qspStatementPlayFile, 1, 2, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddStatement(qspStatClose, qspStatementCloseFile, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatCloseAll, qspStatementCloseFile, 0, 0);

    qspAddStatement(qspStatIncLib, qspStatementOpenQst, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatFreeLib, qspStatementClear, 0, 0);

    qspAddStatement(qspStatOpenGame, qspStatementOpenGame, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatSaveGame, qspStatementSaveGame, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatOpenQst, qspStatementOpenQst, 1, 1, QSP_TYPE_STR);

    qspAddStatement(qspStatSetTimer, qspStatementSetTimer, 1, 1, QSP_TYPE_NUM);

    /* Names */
    qspAddStatName(qspStatLabel, QSP_STATIC_STR(QSP_LABEL), QSP_FALSE, 0);
    qspAddStatName(qspStatComment, QSP_STATIC_STR(QSP_COMMENT), QSP_FALSE, 0);
    qspAddStatName(qspStatUserCall, QSP_STATIC_STR(QSP_USERSTAT), QSP_FALSE, 0);
    qspAddStatName(qspStatEnd, QSP_STATIC_STR(QSP_FMT("END")), QSP_TRUE, 2);

    qspAddStatName(qspStatLoop, QSP_STATIC_STR(QSP_FMT("LOOP")), QSP_TRUE, 2);
    qspAddStatName(qspStatIf, QSP_STATIC_STR(QSP_FMT("IF")), QSP_TRUE, 2);
    qspAddStatName(qspStatElse, QSP_STATIC_STR(QSP_STATELSE), QSP_TRUE, 2);
    qspAddStatName(qspStatElseIf, QSP_STATIC_STR(QSP_STATELSEIF), QSP_TRUE, 1);

    qspAddStatName(qspStatLocal, QSP_STATIC_STR(QSP_FMT("LOCAL")), QSP_TRUE, 2);
    qspAddStatName(qspStatSet, QSP_STATIC_STR(QSP_FMT("SET")), QSP_TRUE, 2);
    qspAddStatName(qspStatSet, QSP_STATIC_STR(QSP_FMT("LET")), QSP_TRUE, 2);

    qspAddStatName(qspStatExit, QSP_STATIC_STR(QSP_FMT("EXIT")), QSP_TRUE, 2);
    qspAddStatName(qspStatJump, QSP_STATIC_STR(QSP_FMT("JUMP")), QSP_TRUE, 2);
    qspAddStatName(qspStatGoSub, QSP_STATIC_STR(QSP_FMT("GOSUB")), QSP_TRUE, 2);
    qspAddStatName(qspStatGoSub, QSP_STATIC_STR(QSP_FMT("GS")), QSP_TRUE, 2);
    qspAddStatName(qspStatGoTo, QSP_STATIC_STR(QSP_FMT("GOTO")), QSP_TRUE, 2);
    qspAddStatName(qspStatGoTo, QSP_STATIC_STR(QSP_FMT("GT")), QSP_TRUE, 2);
    qspAddStatName(qspStatXGoTo, QSP_STATIC_STR(QSP_FMT("XGOTO")), QSP_TRUE, 2);
    qspAddStatName(qspStatXGoTo, QSP_STATIC_STR(QSP_FMT("XGT")), QSP_TRUE, 2);
    qspAddStatName(qspStatDynamic, QSP_STATIC_STR(QSP_FMT("DYNAMIC")), QSP_TRUE, 2);
    qspAddStatName(qspStatExec, QSP_STATIC_STR(QSP_FMT("EXEC")), QSP_TRUE, 2);

    qspAddStatName(qspStatSetVar, QSP_STATIC_STR(QSP_FMT("SETVAR")), QSP_TRUE, 1);
    qspAddStatName(qspStatUnpackArr, QSP_STATIC_STR(QSP_FMT("UNPACKARR")), QSP_TRUE, 2);
    qspAddStatName(qspStatCopyArr, QSP_STATIC_STR(QSP_FMT("COPYARR")), QSP_TRUE, 2);
    qspAddStatName(qspStatSortArr, QSP_STATIC_STR(QSP_FMT("SORTARR")), QSP_TRUE, 2);
    qspAddStatName(qspStatScanStr, QSP_STATIC_STR(QSP_FMT("SCANSTR")), QSP_TRUE, 2);
    qspAddStatName(qspStatKillVar, QSP_STATIC_STR(QSP_FMT("KILLVAR")), QSP_TRUE, 2);
    qspAddStatName(qspStatKillAll, QSP_STATIC_STR(QSP_FMT("KILLALL")), QSP_TRUE, 2);

    qspAddStatName(qspStatAddObj, QSP_STATIC_STR(QSP_FMT("ADDOBJ")), QSP_TRUE, 2);
    qspAddStatName(qspStatAddObj, QSP_STATIC_STR(QSP_FMT("ADD OBJ")), QSP_TRUE, 2);
    qspAddStatName(qspStatDelObj, QSP_STATIC_STR(QSP_FMT("DELOBJ")), QSP_TRUE, 2);
    qspAddStatName(qspStatDelObj, QSP_STATIC_STR(QSP_FMT("DEL OBJ")), QSP_TRUE, 2);
    qspAddStatName(qspStatModObj, QSP_STATIC_STR(QSP_FMT("MODOBJ")), QSP_TRUE, 2);
    qspAddStatName(qspStatModObj, QSP_STATIC_STR(QSP_FMT("MOD OBJ")), QSP_TRUE, 2);
    qspAddStatName(qspStatResetObj, QSP_STATIC_STR(QSP_FMT("RESETOBJ")), QSP_TRUE, 2);
    qspAddStatName(qspStatKillObj, QSP_STATIC_STR(QSP_FMT("KILLOBJ")), QSP_TRUE, 2);
    qspAddStatName(qspStatUnSelect, QSP_STATIC_STR(QSP_FMT("UNSELECT")), QSP_TRUE, 1);
    qspAddStatName(qspStatUnSelect, QSP_STATIC_STR(QSP_FMT("UNSEL")), QSP_TRUE, 2);

    qspAddStatName(qspStatAct, QSP_STATIC_STR(QSP_FMT("ACT")), QSP_TRUE, 2);
    qspAddStatName(qspStatDelAct, QSP_STATIC_STR(QSP_FMT("DELACT")), QSP_TRUE, 2);
    qspAddStatName(qspStatDelAct, QSP_STATIC_STR(QSP_FMT("DEL ACT")), QSP_TRUE, 2);
    qspAddStatName(qspStatClA, QSP_STATIC_STR(QSP_FMT("CLA")), QSP_TRUE, 2);

    qspAddStatName(qspStatMClear, QSP_STATIC_STR(QSP_FMT("*CLEAR")), QSP_TRUE, 2);
    qspAddStatName(qspStatMClear, QSP_STATIC_STR(QSP_FMT("*CLR")), QSP_TRUE, 2);
    qspAddStatName(qspStatMNL, QSP_STATIC_STR(QSP_FMT("*NL")), QSP_TRUE, 2);
    qspAddStatName(qspStatMPL, QSP_STATIC_STR(QSP_FMT("*PL")), QSP_TRUE, 1);
    qspAddStatName(qspStatMP, QSP_STATIC_STR(QSP_FMT("*P")), QSP_TRUE, 2);
    qspAddStatName(qspStatClear, QSP_STATIC_STR(QSP_FMT("CLEAR")), QSP_TRUE, 2);
    qspAddStatName(qspStatClear, QSP_STATIC_STR(QSP_FMT("CLR")), QSP_TRUE, 2);
    qspAddStatName(qspStatNL, QSP_STATIC_STR(QSP_FMT("NL")), QSP_TRUE, 2);
    qspAddStatName(qspStatPL, QSP_STATIC_STR(QSP_FMT("PL")), QSP_TRUE, 1);
    qspAddStatName(qspStatP, QSP_STATIC_STR(QSP_FMT("P")), QSP_TRUE, 2);

    qspAddStatName(qspStatClS, QSP_STATIC_STR(QSP_FMT("CLS")), QSP_TRUE, 2);
    qspAddStatName(qspStatCmdClear, QSP_STATIC_STR(QSP_FMT("CMDCLEAR")), QSP_TRUE, 2);
    qspAddStatName(qspStatCmdClear, QSP_STATIC_STR(QSP_FMT("CMDCLR")), QSP_TRUE, 2);
    qspAddStatName(qspStatShowActs, QSP_STATIC_STR(QSP_FMT("SHOWACTS")), QSP_TRUE, 2);
    qspAddStatName(qspStatShowObjs, QSP_STATIC_STR(QSP_FMT("SHOWOBJS")), QSP_TRUE, 2);
    qspAddStatName(qspStatShowVars, QSP_STATIC_STR(QSP_FMT("SHOWSTAT")), QSP_TRUE, 2);
    qspAddStatName(qspStatShowInput, QSP_STATIC_STR(QSP_FMT("SHOWINPUT")), QSP_TRUE, 2);
    qspAddStatName(qspStatRefInt, QSP_STATIC_STR(QSP_FMT("REFINT")), QSP_TRUE, 2);

    qspAddStatName(qspStatMenu, QSP_STATIC_STR(QSP_FMT("MENU")), QSP_TRUE, 2);
    qspAddStatName(qspStatMsg, QSP_STATIC_STR(QSP_FMT("MSG")), QSP_TRUE, 2);
    qspAddStatName(qspStatView, QSP_STATIC_STR(QSP_FMT("VIEW")), QSP_TRUE, 2);
    qspAddStatName(qspStatWait, QSP_STATIC_STR(QSP_FMT("WAIT")), QSP_TRUE, 2);

    qspAddStatName(qspStatPlay, QSP_STATIC_STR(QSP_FMT("PLAY")), QSP_TRUE, 0);
    qspAddStatName(qspStatClose, QSP_STATIC_STR(QSP_FMT("CLOSE")), QSP_TRUE, 2);
    qspAddStatName(qspStatCloseAll, QSP_STATIC_STR(QSP_FMT("CLOSE ALL")), QSP_TRUE, 1);

    qspAddStatName(qspStatIncLib, QSP_STATIC_STR(QSP_FMT("INCLIB")), QSP_TRUE, 2);
    qspAddStatName(qspStatFreeLib, QSP_STATIC_STR(QSP_FMT("FREELIB")), QSP_TRUE, 2);

    qspAddStatName(qspStatOpenGame, QSP_STATIC_STR(QSP_FMT("OPENGAME")), QSP_TRUE, 2);
    qspAddStatName(qspStatSaveGame, QSP_STATIC_STR(QSP_FMT("SAVEGAME")), QSP_TRUE, 2);
    qspAddStatName(qspStatOpenQst, QSP_STATIC_STR(QSP_FMT("OPENQST")), QSP_TRUE, 2);

    qspAddStatName(qspStatSetTimer, QSP_STATIC_STR(QSP_FMT("SETTIMER")), QSP_TRUE, 1);

    for (i = 0; i < QSP_STATSLEVELS; ++i)
        qsort(qspStatsNames[i], qspStatsNamesCounts[i], sizeof(QSPStatName), qspStatsCompare);
}

INLINE int qspSearchElse(QSPLineOfCode *lines, int start, int end)
{
    int c = 1;
    QSPLineOfCode *startLine = lines + start;
    /* Skip some lines before we start searching for the right one */
    start += startLine->LinesToElse;
    lines += start;
    while (start < end)
    {
        if (lines->Stats)
        {
            switch (lines->Stats->Stat)
            {
            case qspStatAct:
            case qspStatLoop:
            case qspStatIf:
                if (lines->IsMultiline) /* skip internal multiline statements */
                {
                    QSP_INC_POSITIVE(c);
                    start += lines->LinesToEnd;
                    lines += lines->LinesToEnd;
                    continue;
                }
                break;
            case qspStatElse:
            case qspStatElseIf:
                if (c == 1)
                {
                    /* Update the number of lines to skip next time */
                    startLine->LinesToElse = lines - startLine;
                    return start;
                }
                break;
            case qspStatEnd:
                if (!QSP_DEC_POSITIVE(c)) return -1;
                break;
            }
        }
        ++start;
        ++lines;
    }
    return -1;
}

INLINE int qspSearchEnd(QSPLineOfCode *lines, int start, int end)
{
    int c = 1;
    QSPLineOfCode *startLine = lines + start;
    /* Skip some lines before we start searching for the right one */
    start += startLine->LinesToEnd;
    lines += start;
    while (start < end)
    {
        if (lines->Stats)
        {
            switch (lines->Stats->Stat)
            {
            case qspStatAct:
            case qspStatLoop:
            case qspStatIf:
                if (lines->IsMultiline) /* skip internal multiline statements */
                {
                    QSP_INC_POSITIVE(c);
                    start += lines->LinesToEnd;
                    lines += lines->LinesToEnd;
                    continue;
                }
                break;
            case qspStatEnd:
                if (!QSP_DEC_POSITIVE(c))
                {
                    /* Update the number of lines to skip next time */
                    startLine->LinesToEnd = lines - startLine;
                    return start;
                }
                break;
            }
        }
        ++start;
        ++lines;
    }
    return -1;
}

INLINE int qspSearchLabel(QSPCodeBlock *code, int startLine, int endLine, QSPString name)
{
    QSPCodeLabel *label = code->Labels;
    int count = code->LabelsCount;
    while (--count >= 0)
    {
        if (label->LineIndex >= endLine) break;
        if (label->LineIndex >= startLine && qspStrsEqual(label->Name, name)) return label->LineIndex;
        ++label;
    }
    return -1;
}

QSP_TINYINT qspGetStatArgs(QSPCachedStat *stat, QSPCachedArg *statArgs, QSPVariant *args)
{
    QSP_TINYINT argsCount = stat->ArgsCount;
    if (argsCount > 0)
    {
        QSP_TINYINT argIndex, type, *statArgsTypes;
        int oldLocationState = qspLocationState;
        statArgsTypes = qspStats[stat->Stat].ArgsTypes;
        for (argIndex = 0; argIndex < argsCount; ++argIndex)
        {
            type = statArgsTypes[argIndex];
            args[argIndex] = qspCalculateArgValue(statArgs + argIndex);
            if (qspLocationState != oldLocationState)
            {
                qspFreeVariants(args, argIndex);
                return 0;
            }
            if (QSP_ISDEF(type) && !qspConvertVariantTo(args + argIndex, type))
            {
                qspSetError(QSP_ERR_TYPEMISMATCH);
                qspFreeVariants(args, argIndex + 1); /* include the current item */
                return 0;
            }
        }
    }
    return argsCount;
}

INLINE QSP_BOOL qspExecString(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo)
{
    QSP_TINYINT statCode;
    QSPCachedStat *stat;
    int i, oldLocationState = qspLocationState;
    for (i = startStat, stat = line->Stats + startStat; i < endStat; ++i, ++stat)
    {
        if (stat->ErrorCode)
        {
            qspSetError(stat->ErrorCode);
            return QSP_FALSE;
        }
        statCode = stat->Stat;
        switch (statCode)
        {
        case qspStatUnknown:
        case qspStatLabel:
        case qspStatElse:
        case qspStatEnd:
            break;
        case qspStatComment:
            return QSP_FALSE; /* skip the rest */
        case qspStatIf:
        case qspStatElseIf:
            return qspStatementIf(line, i, endStat, jumpTo);
        case qspStatLoop:
            return qspStatementSinglelineLoop(line, i, endStat, jumpTo);
        case qspStatSet:
            qspStatementSetVarsValues(stat);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            break;
        case qspStatLocal:
            qspStatementLocal(stat);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            break;
        case qspStatExit:
            return QSP_TRUE;
        case qspStatJump:
            {
                QSPVariant arg; /* 1 argument only */
                qspGetStatArgs(stat, stat->Data.Args, &arg);
                if (qspLocationState != oldLocationState) return QSP_FALSE;
                qspUpdateText(jumpTo, qspDelSpc(QSP_STR(arg)));
                qspUpperStr(jumpTo);
                qspFreeVariant(&arg);
                return QSP_TRUE;
            }
        case qspStatUserCall:
            qspStatementUserCall(stat);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            break;
        case qspStatAct:
            qspStatementSinglelineAddAct(line, i, endStat);
            return QSP_FALSE;
        default:
            {
                QSPVariant *args = (QSPVariant *)qspAllocateMemory(stat->ArgsCount * sizeof(QSPVariant));
                QSP_TINYINT argsCount = qspGetStatArgs(stat, stat->Data.Args, args);
                if (qspLocationState != oldLocationState)
                {
                    qspReleaseMemory(args);
                    return QSP_FALSE;
                }
                qspStats[statCode].Func(args, argsCount, statCode);
                qspFreeVariants(args, argsCount);
                qspReleaseMemory(args);
                if (qspLocationState != oldLocationState) return QSP_FALSE;
                break;
            }
        }
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspExecMultilineCode(QSPCodeBlock *code, int endLine, int codeOffset,
    QSPString *jumpTo, int *lineInd, int *action)
{
    QSPLineOfCode *line, *lines = code->Lines;
    int ind = *lineInd;
    endLine = qspSearchEnd(lines, ind, endLine);
    if (endLine < 0)
    {
        qspSetError(QSP_ERR_ENDNOTFOUND);
        return QSP_FALSE;
    }
    line = lines + ind;
    if (line->Stats->ErrorCode)
    {
        qspSetError(line->Stats->ErrorCode);
        return QSP_FALSE;
    }
    switch (line->Stats->Stat)
    {
    case qspStatIf:
    case qspStatElseIf:
        {
            int elsePos, oldLocationState = qspLocationState;
            QSP_BOOL condition = qspCheckCondition(line->Stats->Data.Args);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            elsePos = qspSearchElse(lines, ind, endLine);
            if (condition)
            {
                *lineInd = endLine;
                *action = qspFlowJumpToSpecified;
                if (elsePos >= 0)
                    return qspExecCodeBlockWithLocals(code, ind + 1, elsePos, codeOffset, jumpTo);
                return qspExecCodeBlockWithLocals(code, ind + 1, endLine, codeOffset, jumpTo);
            }
            else
            {
                *lineInd = (elsePos >= 0 ? elsePos : endLine);
                *action = qspFlowJumpToSpecified;
            }
            break;
        }
    case qspStatElse:
        {
            int elsePos = qspSearchElse(lines, ind, endLine);
            *lineInd = endLine;
            *action = qspFlowJumpToSpecified;
            if (elsePos >= 0)
                return qspExecCodeBlockWithLocals(code, ind + 1, elsePos, codeOffset, jumpTo);
            return qspExecCodeBlockWithLocals(code, ind + 1, endLine, codeOffset, jumpTo);
        }
    case qspStatLoop:
        *lineInd = endLine;
        *action = qspFlowJumpToSpecified;
        return qspStatementMultilineLoop(code, ind, endLine, codeOffset, jumpTo);
    case qspStatAct:
        *lineInd = endLine;
        *action = qspFlowJumpToSpecified;
        qspStatementMultilineAddAct(code, ind, endLine);
        break;
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspExecSinglelineCode(QSPCodeBlock *code, int endLine,
    QSPString *jumpTo, int *lineInd, int *action)
{
    int ind = *lineInd;
    QSPLineOfCode *lines = code->Lines, *line = lines + ind;
    switch (line->Stats->Stat)
    {
    case qspStatElseIf:
        {
            int oldLocationState;
            QSP_BOOL condition;
            QSP_CHAR *endPos = line->Str.Str + line->Stats->EndPos;
            if (!qspIsCharAtPos(line->Str, endPos, QSP_COLONDELIM_CHAR))
            {
                qspSetError(QSP_ERR_COLONNOTFOUND);
                break;
            }
            endLine = qspSearchEnd(lines, ind, endLine);
            if (endLine < 0)
            {
                qspSetError(QSP_ERR_ENDNOTFOUND);
                break;
            }
            oldLocationState = qspLocationState;
            condition = qspCheckCondition(line->Stats->Data.Args);
            if (qspLocationState != oldLocationState) break;
            if (condition)
            {
                *lineInd = endLine;
                *action = qspFlowJumpToSpecified;
                return qspExecStringWithLocals(line, 1, line->StatsCount, jumpTo);
            }
            else
            {
                int elsePos = qspSearchElse(lines, ind, endLine);
                *lineInd = (elsePos >= 0 ? elsePos : endLine);
                *action = qspFlowJumpToSpecified;
            }
            break;
        }
    case qspStatElse:
        endLine = qspSearchEnd(lines, ind, endLine);
        if (endLine < 0)
        {
            qspSetError(QSP_ERR_ENDNOTFOUND);
            break;
        }
        *lineInd = endLine;
        *action = qspFlowJumpToSpecified;
        return qspExecStringWithLocals(line, 1, line->StatsCount, jumpTo);
    }
    return QSP_FALSE;
}

QSP_BOOL qspExecCode(QSPCodeBlock *code, int startLine, int endLine, int codeOffset, QSPString *jumpTo)
{
    QSPString jumpToFake;
    QSP_BOOL uLevel, toExit = QSP_FALSE;
    QSPLineOfCode *line, *lines = code->Lines;
    int i, oldLocationState = qspLocationState, action = qspFlowExecute;
    /* Prepare temporary data */
    if ((uLevel = !jumpTo))
    {
        jumpToFake = qspNullString;
        jumpTo = &jumpToFake;
    }
    /* Code execution */
    i = startLine;
    while (i < endLine)
    {
        line = lines + i;

        /* Skip empty lines */
        if (!line->Stats)
        {
            ++i;
            continue;
        }

        qspRealLine = line;
        if (codeOffset > 0)
        {
            qspRealLineNum = line->LineNum + codeOffset;
            if (qspIsDebug)
            {
                qspCallDebug(line->Str);
                if (qspLocationState != oldLocationState) break;
            }
        }
        if (line->IsMultiline)
            toExit = qspExecMultilineCode(code, endLine, codeOffset, jumpTo, &i, &action);
        else
            toExit = qspExecSinglelineCode(code, endLine, jumpTo, &i, &action);
        if (qspLocationState != oldLocationState) break;
        if (toExit)
        {
            if (!qspIsEmpty(*jumpTo))
            {
                i = qspSearchLabel(code, startLine, endLine, *jumpTo);
                if (i >= 0)
                {
                    jumpTo->End = jumpTo->Str;
                    continue;
                }
                if (uLevel) qspSetError(QSP_ERR_LABELNOTFOUND);
            }
            break;
        }
        if (action == qspFlowExecute)
        {
            toExit = qspExecString(line, 0, line->StatsCount, jumpTo);
            if (qspLocationState != oldLocationState) break;
            if (toExit)
            {
                if (!qspIsEmpty(*jumpTo))
                {
                    i = qspSearchLabel(code, startLine, endLine, *jumpTo);
                    if (i >= 0)
                    {
                        jumpTo->End = jumpTo->Str;
                        continue;
                    }
                    if (uLevel) qspSetError(QSP_ERR_LABELNOTFOUND);
                }
                break;
            }
            ++i;
        }
        else /* jump to the specified line & try to execute it */
            action = qspFlowExecute;
    }
    if (uLevel) qspFreeString(&jumpToFake);
    return toExit;
}

QSP_BOOL qspExecCodeBlockWithLocals(QSPCodeBlock *code, int startLine, int endLine, int codeOffset, QSPString *jumpTo)
{
    QSP_BOOL toExit;
    int oldLocationState = qspLocationState;
    qspAllocateLocalScope();

    toExit = qspExecCode(code, startLine, endLine, codeOffset, jumpTo);
    if (qspLocationState != oldLocationState) return QSP_FALSE;

    qspReleaseLastLocalScope();
    return toExit;
}

INLINE QSP_BOOL qspExecStringWithLocals(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo)
{
    QSP_BOOL toExit;
    int oldLocationState = qspLocationState;
    qspAllocateLocalScope();

    toExit = qspExecString(line, startStat, endStat, jumpTo);
    if (qspLocationState != oldLocationState) return QSP_FALSE;

    qspReleaseLastLocalScope();
    return toExit;
}

void qspExecStringAsCodeWithArgs(QSPString s, QSPVariant *args, QSP_TINYINT count, QSPVariant *res)
{
    QSPCodeBlock *code;
    qspAllocateLocalScopeWithArgs(args, count, QSP_TRUE);

    code = qspGetCachedCodeBlock(s);
    if (code)
    {
        int oldLocationState = qspLocationState;
        QSPLineOfCode *oldLine = qspRealLine;
        qspExecCode(code, 0, code->LinesCount, 0, 0);
        qspRealLine = oldLine; /* the executed lines can be released */
        qspReleaseCodeBlock(code);
        if (qspLocationState != oldLocationState) return;
    }

    if (res && !qspApplyResult(res)) return;
    qspReleaseLastLocalScope();
}

void qspExecStringAsCode(QSPString s)
{
    /* Keep the current location context here (don't reset special vars) */
    QSPCodeBlock *code = qspPreprocessData(s);
    if (code)
    {
        QSPLineOfCode *oldLine = qspRealLine;
        qspExecCodeBlockWithLocals(code, 0, code->LinesCount, 0, 0);
        qspRealLine = oldLine; /* the executed lines can be released */
        qspReleaseCodeBlock(code);
    }
}

INLINE QSP_BOOL qspStatementIf(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo)
{
    QSP_BOOL condition;
    int i, c, elseStat, oldLocationState;
    QSPCachedStat *statements = line->Stats;
    QSP_CHAR *endPos = line->Str.Str + statements[startStat].EndPos;
    if (!qspIsCharAtPos(line->Str, endPos, QSP_COLONDELIM_CHAR))
    {
        qspSetError(QSP_ERR_COLONNOTFOUND);
        return QSP_FALSE;
    }
    elseStat = 0;
    c = 1;
    for (i = startStat + 1; i < endStat; ++i)
    {
        switch (statements[i].Stat)
        {
        case qspStatIf:
            QSP_INC_POSITIVE(c);
            break;
        case qspStatElse:
        case qspStatElseIf:
            QSP_DEC_POSITIVE(c);
            break;
        }
        if (!c)
        {
            elseStat = i;
            break;
        }
    }
    if (elseStat)
    {
        if (elseStat == startStat + 1) /* no code between IF and ELSE */
        {
            qspSetError(QSP_ERR_CODENOTFOUND);
            return QSP_FALSE;
        }
        if (elseStat == endStat - 1) /* no code after ELSE */
        {
            qspSetError(QSP_ERR_CODENOTFOUND);
            return QSP_FALSE;
        }
    }
    else if (startStat == endStat - 1) /* no code after IF */
    {
        qspSetError(QSP_ERR_CODENOTFOUND);
        return QSP_FALSE;
    }
    oldLocationState = qspLocationState;
    condition = qspCheckCondition(statements[startStat].Data.Args);
    if (qspLocationState != oldLocationState) return QSP_FALSE;
    if (condition)
    {
        if (elseStat)
            return qspExecStringWithLocals(line, startStat + 1, elseStat, jumpTo);
        else
            return qspExecStringWithLocals(line, startStat + 1, endStat, jumpTo);
    }
    else if (elseStat)
        return qspExecStringWithLocals(line, elseStat, endStat, jumpTo);
    return QSP_FALSE;
}

INLINE QSP_BOOL qspCheckCondition(QSPCachedArg *condition)
{
    int oldLocationState = qspLocationState;
    QSPVariant condValue = qspCalculateArgValue(condition);
    if (qspLocationState != oldLocationState) return QSP_FALSE;
    if (!qspConvertVariantTo(&condValue, QSP_TYPE_BOOL))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        qspFreeVariant(&condValue);
        return QSP_FALSE;
    }
    return QSP_ISTRUE(QSP_NUM(condValue));
}

INLINE QSP_BOOL qspStatementSinglelineLoop(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo)
{
    QSP_BOOL toExit;
    int oldLocationState;
    QSPCachedLoop *loop;
    QSPCachedStat *stat = line->Stats + startStat;
    if (startStat == endStat - 1)
    {
        qspSetError(QSP_ERR_CODENOTFOUND);
        return QSP_FALSE;
    }
    qspAllocateLocalScope();

    /* Execute loop initialization */
    loop = stat->Data.Loop;
    oldLocationState = qspLocationState;
    toExit = qspExecString(&loop->Initializer, 0, loop->Initializer.StatsCount, jumpTo);
    if (qspLocationState != oldLocationState) return QSP_FALSE;
    if (!toExit)
    {
        QSP_BOOL conditionValue;
        while (1)
        {
            /* Check condition */
            conditionValue = qspCheckCondition(&loop->Condition);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            if (!conditionValue) break;
            /* Execute body */
            toExit = qspExecStringWithLocals(line, startStat + 1, endStat, jumpTo);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            if (toExit) break;
            /* Execute iterator */
            if (loop->Iterator.StatsCount)
            {
                toExit = qspExecStringWithLocals(&loop->Iterator, 0, loop->Iterator.StatsCount, jumpTo);
                if (qspLocationState != oldLocationState) return QSP_FALSE;
                if (toExit) break;
            }
        }
    }
    qspReleaseLastLocalScope();
    return toExit;
}

INLINE QSP_BOOL qspStatementMultilineLoop(QSPCodeBlock *code, int lineInd, int endLine,
    int codeOffset, QSPString *jumpTo)
{
    QSP_BOOL toExit;
    int oldLocationState;
    QSPCachedLoop *loop;
    QSPLineOfCode *line = code->Lines + lineInd;
    qspAllocateLocalScope();

    /* Execute loop initialization */
    loop = line->Stats->Data.Loop;
    oldLocationState = qspLocationState;
    toExit = qspExecString(&loop->Initializer, 0, loop->Initializer.StatsCount, jumpTo);
    if (qspLocationState != oldLocationState) return QSP_FALSE;
    if (!toExit)
    {
        QSP_BOOL conditionValue;
        ++lineInd;
        while (1)
        {
            qspRealLine = line;
            if (codeOffset > 0)
            {
                qspRealLineNum = line->LineNum + codeOffset;
                if (qspIsDebug)
                {
                    qspCallDebug(line->Str);
                    if (qspLocationState != oldLocationState) return QSP_FALSE;
                }
            }
            /* Check condition */
            conditionValue = qspCheckCondition(&loop->Condition);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            if (!conditionValue) break;
            /* Execute body */
            toExit = qspExecCodeBlockWithLocals(code, lineInd, endLine, codeOffset, jumpTo);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            if (toExit) break;
            /* Execute iterator */
            if (loop->Iterator.StatsCount)
            {
                qspRealLine = line;
                if (codeOffset > 0)
                {
                    qspRealLineNum = line->LineNum + codeOffset;
                    if (qspIsDebug)
                    {
                        qspCallDebug(line->Str);
                        if (qspLocationState != oldLocationState) return QSP_FALSE;
                    }
                }
                toExit = qspExecStringWithLocals(&loop->Iterator, 0, loop->Iterator.StatsCount, jumpTo);
                if (qspLocationState != oldLocationState) return QSP_FALSE;
                if (toExit) break;
            }
        }
    }
    qspReleaseLastLocalScope();
    return toExit;
}

INLINE void qspStatementUserCall(QSPCachedStat *stat)
{
    int oldLocationState = qspLocationState;
    QSPVariant *args = (QSPVariant *)qspAllocateMemory(stat->ArgsCount * sizeof(QSPVariant));
    QSP_TINYINT argsCount = qspGetStatArgs(stat, stat->Data.Args, args);
    if (qspLocationState != oldLocationState)
    {
        qspReleaseMemory(args);
        return;
    }
    qspExecLocByNameWithArgs(QSP_STR(args[0]), args + 1, argsCount - 1, QSP_TRUE, 0);
    qspFreeVariants(args, argsCount);
    qspReleaseMemory(args);
}

INLINE void qspStatementImplicitStatement(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    if (QSP_ISDEF(args[0].Type))
    {
        qspAppendVariantToString(args, &qspCurDesc);
        qspAddBufText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM));
        qspCurWindowsChangedState |= QSP_WIN_MAIN;
    }
}

INLINE void qspStatementAddText(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg)
{
    switch (extArg)
    {
    case qspStatP:
        if (qspAddBufText(&qspCurVars, QSP_STR(args[0])))
            qspCurWindowsChangedState |= QSP_WIN_VARS;
        break;
    case qspStatMP:
        if (qspAddBufText(&qspCurDesc, QSP_STR(args[0])))
            qspCurWindowsChangedState |= QSP_WIN_MAIN;
        break;
    case qspStatPL:
        if (count) qspAddBufText(&qspCurVars, QSP_STR(args[0]));
        qspAddBufText(&qspCurVars, QSP_STATIC_STR(QSP_STRSDELIM));
        qspCurWindowsChangedState |= QSP_WIN_VARS;
        break;
    case qspStatMPL:
        if (count) qspAddBufText(&qspCurDesc, QSP_STR(args[0]));
        qspAddBufText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM));
        qspCurWindowsChangedState |= QSP_WIN_MAIN;
        break;
    case qspStatNL:
        qspAddBufText(&qspCurVars, QSP_STATIC_STR(QSP_STRSDELIM));
        if (count) qspAddBufText(&qspCurVars, QSP_STR(args[0]));
        qspCurWindowsChangedState |= QSP_WIN_VARS;
        break;
    case qspStatMNL:
        qspAddBufText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM));
        if (count) qspAddBufText(&qspCurDesc, QSP_STR(args[0]));
        qspCurWindowsChangedState |= QSP_WIN_MAIN;
        break;
    }
}

INLINE void qspStatementClear(QSPVariant *QSP_UNUSED(args), QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT extArg)
{
    switch (extArg)
    {
    case qspStatClear:
        if (qspCurVars.Len > 0)
        {
            qspClearBufString(&qspCurVars);
            qspCurWindowsChangedState |= QSP_WIN_VARS;
        }
        break;
    case qspStatMClear:
        if (qspCurDesc.Len > 0)
        {
            qspClearBufString(&qspCurDesc);
            qspCurWindowsChangedState |= QSP_WIN_MAIN;
        }
        break;
    case qspStatCmdClear:
        qspClearText(&qspCurInput);
        qspCallSetInputStrText(qspNullString);
        break;
    case qspStatClA:
        qspClearAllActions(QSP_FALSE);
        break;
    case qspStatClS:
        if (qspCurVars.Len > 0)
        {
            qspClearBufString(&qspCurVars);
            qspCurWindowsChangedState |= QSP_WIN_VARS;
        }
        if (qspCurDesc.Len > 0)
        {
            qspClearBufString(&qspCurDesc);
            qspCurWindowsChangedState |= QSP_WIN_MAIN;
        }
        qspClearText(&qspCurInput);
        qspClearAllActions(QSP_FALSE);
        qspCallSetInputStrText(qspNullString);
        break;
    case qspStatKillAll:
        /* The order is important here */
        qspClearAllVars(QSP_FALSE);
        qspClearAllObjectsWithEvents();
        break;
    case qspStatFreeLib:
        qspClearAllIncludes(QSP_FALSE);
        break;
    }
}

INLINE void qspStatementGoSub(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    qspExecLocByNameWithArgs(QSP_STR(args[0]), args + 1, count - 1, QSP_TRUE, 0);
}

INLINE void qspStatementGoTo(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg)
{
    QSPLocation *loc = qspLocByName(QSP_STR(args[0]));
    if (!loc)
    {
        qspSetError(QSP_ERR_LOCNOTFOUND);
        return;
    }
    qspNavigateToLocation(loc, extArg == qspStatGoTo, args + 1, count - 1);
}

INLINE void qspStatementWait(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    int delayInMsecs = QSP_TOINT(QSP_NUM(args[0]));
    if (delayInMsecs < 0) delayInMsecs = 0;
    qspCallSleep(delayInMsecs);
}

INLINE void qspStatementSetTimer(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    int delayInMsecs = QSP_TOINT(QSP_NUM(args[0]));
    if (delayInMsecs < 0) delayInMsecs = 0;
    qspTimerInterval = delayInMsecs;
    qspCallSetTimer(delayInMsecs);
}

INLINE void qspStatementShowWin(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT extArg)
{
    QSP_BOOL val = QSP_ISTRUE(QSP_NUM(args[0]));
    switch (extArg)
    {
    case qspStatShowVars:
        qspSetWindowState(QSP_WIN_VARS, val);
        qspCallShowWindow(QSP_WIN_VARS, val);
        break;
    case qspStatShowActs:
        qspSetWindowState(QSP_WIN_ACTS, val);
        qspCallShowWindow(QSP_WIN_ACTS, val);
        break;
    case qspStatShowObjs:
        qspSetWindowState(QSP_WIN_OBJS, val);
        qspCallShowWindow(QSP_WIN_OBJS, val);
        break;
    case qspStatShowInput:
        qspSetWindowState(QSP_WIN_INPUT, val);
        qspCallShowWindow(QSP_WIN_INPUT, val);
        break;
    }
}

INLINE void qspStatementRefInt(QSPVariant *QSP_UNUSED(args), QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    qspCallRefreshInt(QSP_TRUE);
}

INLINE void qspStatementView(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    int oldLocationState = qspLocationState;
    if (count && qspIsAnyString(QSP_STR(args[0])))
    {
        qspUpdateText(&qspViewPath, QSP_STR(args[0]));
        qspCallShowPicture(qspViewPath);
        if (qspLocationState != oldLocationState) return;
        qspSetWindowState(QSP_WIN_VIEW, QSP_TRUE);
        qspCallShowWindow(QSP_WIN_VIEW, QSP_TRUE);
    }
    else
    {
        qspClearText(&qspViewPath);
        qspCallShowPicture(qspNullString);
        if (qspLocationState != oldLocationState) return;
        qspSetWindowState(QSP_WIN_VIEW, QSP_FALSE);
        qspCallShowWindow(QSP_WIN_VIEW, QSP_FALSE);
    }
}

INLINE void qspStatementMsg(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    qspCallShowMessage(QSP_STR(args[0]));
}

INLINE void qspStatementExec(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    qspCallSystem(QSP_STR(args[0]));
}

INLINE void qspStatementDynamic(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    qspExecStringAsCodeWithArgs(QSP_STR(args[0]), args + 1, count - 1, 0);
}
