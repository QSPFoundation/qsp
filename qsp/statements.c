/* Copyright (C) 2001-2024 Val Argunov (byte AT qsp DOT org) */
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

#include "statements.h"
#include "actions.h"
#include "callbacks.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "mathops.h"
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

INLINE void qspAddStatement(QSP_TINYINT statCode, QSP_STATEMENT func, QSP_TINYINT minArgs, QSP_TINYINT maxArgs, ...);
INLINE void qspAddStatName(QSP_TINYINT statCode, QSPString statName, int level);
INLINE int qspStatsCompare(const void *statName1, const void *statName2);
INLINE int qspSearchElse(QSPLineOfCode *lines, int start, int end);
INLINE int qspSearchEnd(QSPLineOfCode *lines, int start, int end);
INLINE int qspSearchLabel(QSPLineOfCode *lines, int start, int end, QSPString str);
INLINE QSP_BOOL qspExecString(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspExecMultilineCode(QSPLineOfCode *lines, int endLine, int codeOffset, QSPString *jumpTo, int *lineInd, int *action);
INLINE QSP_BOOL qspExecSinglelineCode(QSPLineOfCode *lines, int endLine, QSPString *jumpTo, int *lineInd, int *action);
INLINE QSP_BOOL qspExecStringWithLocals(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspStatementIf(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspPrepareLoop(QSPString params, QSPString *condition, QSPString *iterator, QSPString *jumpTo);
INLINE QSP_BOOL qspCheckCondition(QSPString expr);
INLINE QSP_BOOL qspCheckCompiledCondition(QSPMathExpression *expression);
INLINE QSP_BOOL qspStatementSinglelineLoop(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspStatementMultilineLoop(QSPLineOfCode *lines, int endLine, int lineInd, int codeOffset, QSPString *jumpTo);
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

INLINE void qspAddStatement(QSP_TINYINT statCode, QSP_STATEMENT func, QSP_TINYINT minArgs, QSP_TINYINT maxArgs, ...)
{
    qspStats[statCode].Func = func;
    qspStats[statCode].MinArgsCount = minArgs;
    qspStats[statCode].MaxArgsCount = maxArgs;
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
                if (curType >= 0)
                    lastType = curType;
                else
                    isFinished = QSP_TRUE; /* use lastType for the rest of arguments */
            }
            qspStats[statCode].ArgsTypes[i] = lastType;
        }
        va_end(marker);
    }
}

INLINE void qspAddStatName(QSP_TINYINT statCode, QSPString statName, int level)
{
    int count, len = qspStrLen(statName);
    count = qspStatsNamesCounts[level]++;
    qspStatsNames[level][count].Name = statName;
    qspStatsNames[level][count].Code = statCode;
    /* Max length */
    if (len > qspStatMaxLen) qspStatMaxLen = len;
}

INLINE int qspStatsCompare(const void *statName1, const void *statName2)
{
    return qspStrsComp(((QSPStatName *)statName1)->Name, ((QSPStatName *)statName2)->Name);
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
            Arguments' types [optional, -1 to use the last known type for the rest of arguments]
        );
    */
    int i;
    for (i = 0; i < QSP_STATSLEVELS; ++i) qspStatsNamesCounts[i] = 0;
    qspStatMaxLen = 0;
    qspAddStatement(qspStatImplicitStatement, qspStatementImplicitStatement, 1, 1, QSP_TYPE_UNDEF);
    qspAddStatement(qspStatElse, 0, 0, 0);
    qspAddStatement(qspStatElseIf, 0, 1, 1, QSP_TYPE_NUM);
    qspAddStatement(qspStatEnd, 0, 0, 0);
    qspAddStatement(qspStatLocal, 0, 0, 0);
    qspAddStatement(qspStatSet, 0, 0, 0);
    qspAddStatement(qspStatIf, 0, 1, 1, QSP_TYPE_NUM);
    qspAddStatement(qspStatAct, 0, 1, 2, QSP_TYPE_STR, QSP_TYPE_STR);
    qspAddStatement(qspStatLoop, 0, 0, 0);
    qspAddStatement(qspStatJump, 0, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatExit, 0, 0, 0);
    qspAddStatement(qspStatAddObj, qspStatementAddObject, 1, 3, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddStatement(qspStatClA, qspStatementClear, 0, 0);
    qspAddStatement(qspStatCloseAll, qspStatementCloseFile, 0, 0);
    qspAddStatement(qspStatClose, qspStatementCloseFile, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatClS, qspStatementClear, 0, 0);
    qspAddStatement(qspStatCmdClear, qspStatementClear, 0, 0);
    qspAddStatement(qspStatCopyArr, qspStatementCopyArr, 2, 4, QSP_TYPE_VARREF, QSP_TYPE_VARREF, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddStatement(qspStatSortArr, qspStatementSortArr, 1, 2, QSP_TYPE_VARREF, QSP_TYPE_NUM);
    qspAddStatement(qspStatScanStr, qspStatementScanStr, 3, 4, QSP_TYPE_VARREF, QSP_TYPE_STR, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddStatement(qspStatDelAct, qspStatementDelAct, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatDelObj, qspStatementDelObj, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatDynamic, qspStatementDynamic, 1, QSP_STATMAXARGS, QSP_TYPE_CODE, QSP_TYPE_UNDEF, -1);
    qspAddStatement(qspStatExec, qspStatementExec, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatFreeLib, qspStatementClear, 0, 0);
    qspAddStatement(qspStatGoSub, qspStatementGoSub, 1, QSP_STATMAXARGS, QSP_TYPE_STR, QSP_TYPE_UNDEF, -1);
    qspAddStatement(qspStatGoTo, qspStatementGoTo, 1, QSP_STATMAXARGS, QSP_TYPE_STR, QSP_TYPE_UNDEF, -1);
    qspAddStatement(qspStatIncLib, qspStatementOpenQst, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatKillAll, qspStatementClear, 0, 0);
    qspAddStatement(qspStatKillObj, qspStatementDelObj, 0, 1, QSP_TYPE_NUM);
    qspAddStatement(qspStatKillVar, qspStatementKillVar, 0, 2, QSP_TYPE_VARREF, QSP_TYPE_UNDEF);
    qspAddStatement(qspStatMenu, qspStatementShowMenu, 1, 3, QSP_TYPE_VARREF, QSP_TYPE_NUM, QSP_TYPE_NUM);
    qspAddStatement(qspStatMClear, qspStatementClear, 0, 0);
    qspAddStatement(qspStatMNL, qspStatementAddText, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatMPL, qspStatementAddText, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatMP, qspStatementAddText, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatClear, qspStatementClear, 0, 0);
    qspAddStatement(qspStatNL, qspStatementAddText, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatPL, qspStatementAddText, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatP, qspStatementAddText, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatMsg, qspStatementMsg, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatOpenGame, qspStatementOpenGame, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatOpenQst, qspStatementOpenQst, 1, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatPlay, qspStatementPlayFile, 1, 2, QSP_TYPE_STR, QSP_TYPE_NUM);
    qspAddStatement(qspStatRefInt, qspStatementRefInt, 0, 0);
    qspAddStatement(qspStatSaveGame, qspStatementSaveGame, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatSetTimer, qspStatementSetTimer, 1, 1, QSP_TYPE_NUM);
    qspAddStatement(qspStatShowActs, qspStatementShowWin, 1, 1, QSP_TYPE_NUM);
    qspAddStatement(qspStatShowInput, qspStatementShowWin, 1, 1, QSP_TYPE_NUM);
    qspAddStatement(qspStatShowObjs, qspStatementShowWin, 1, 1, QSP_TYPE_NUM);
    qspAddStatement(qspStatShowVars, qspStatementShowWin, 1, 1, QSP_TYPE_NUM);
    qspAddStatement(qspStatUnSelect, qspStatementUnSelect, 0, 0);
    qspAddStatement(qspStatView, qspStatementView, 0, 1, QSP_TYPE_STR);
    qspAddStatement(qspStatWait, qspStatementWait, 1, 1, QSP_TYPE_NUM);
    qspAddStatement(qspStatXGoTo, qspStatementGoTo, 1, QSP_STATMAXARGS, QSP_TYPE_STR, QSP_TYPE_UNDEF, -1);
    /* Names */
    qspAddStatName(qspStatElse, QSP_STATIC_STR(QSP_STATELSE), 2);
    qspAddStatName(qspStatElseIf, QSP_STATIC_STR(QSP_STATELSEIF), 1);
    qspAddStatName(qspStatEnd, QSP_STATIC_STR(QSP_FMT("END")), 2);
    qspAddStatName(qspStatLocal, QSP_STATIC_STR(QSP_FMT("LOCAL")), 2);
    qspAddStatName(qspStatSet, QSP_STATIC_STR(QSP_FMT("SET")), 2);
    qspAddStatName(qspStatSet, QSP_STATIC_STR(QSP_FMT("LET")), 2);
    qspAddStatName(qspStatIf, QSP_STATIC_STR(QSP_FMT("IF")), 2);
    qspAddStatName(qspStatAct, QSP_STATIC_STR(QSP_FMT("ACT")), 2);
    qspAddStatName(qspStatLoop, QSP_STATIC_STR(QSP_FMT("LOOP")), 2);
    qspAddStatName(qspStatAddObj, QSP_STATIC_STR(QSP_FMT("ADDOBJ")), 2);
    qspAddStatName(qspStatAddObj, QSP_STATIC_STR(QSP_FMT("ADD OBJ")), 2);
    qspAddStatName(qspStatClA, QSP_STATIC_STR(QSP_FMT("CLA")), 2);
    qspAddStatName(qspStatCloseAll, QSP_STATIC_STR(QSP_FMT("CLOSE ALL")), 1);
    qspAddStatName(qspStatClose, QSP_STATIC_STR(QSP_FMT("CLOSE")), 2);
    qspAddStatName(qspStatClS, QSP_STATIC_STR(QSP_FMT("CLS")), 2);
    qspAddStatName(qspStatCmdClear, QSP_STATIC_STR(QSP_FMT("CMDCLEAR")), 2);
    qspAddStatName(qspStatCmdClear, QSP_STATIC_STR(QSP_FMT("CMDCLR")), 2);
    qspAddStatName(qspStatCopyArr, QSP_STATIC_STR(QSP_FMT("COPYARR")), 2);
    qspAddStatName(qspStatSortArr, QSP_STATIC_STR(QSP_FMT("SORTARR")), 2);
    qspAddStatName(qspStatScanStr, QSP_STATIC_STR(QSP_FMT("SCANSTR")), 2);
    qspAddStatName(qspStatDelAct, QSP_STATIC_STR(QSP_FMT("DELACT")), 2);
    qspAddStatName(qspStatDelAct, QSP_STATIC_STR(QSP_FMT("DEL ACT")), 2);
    qspAddStatName(qspStatDelObj, QSP_STATIC_STR(QSP_FMT("DELOBJ")), 2);
    qspAddStatName(qspStatDelObj, QSP_STATIC_STR(QSP_FMT("DEL OBJ")), 2);
    qspAddStatName(qspStatDynamic, QSP_STATIC_STR(QSP_FMT("DYNAMIC")), 2);
    qspAddStatName(qspStatExec, QSP_STATIC_STR(QSP_FMT("EXEC")), 2);
    qspAddStatName(qspStatExit, QSP_STATIC_STR(QSP_FMT("EXIT")), 2);
    qspAddStatName(qspStatFreeLib, QSP_STATIC_STR(QSP_FMT("FREELIB")), 2);
    qspAddStatName(qspStatGoSub, QSP_STATIC_STR(QSP_FMT("GOSUB")), 2);
    qspAddStatName(qspStatGoSub, QSP_STATIC_STR(QSP_FMT("GS")), 2);
    qspAddStatName(qspStatGoTo, QSP_STATIC_STR(QSP_FMT("GOTO")), 2);
    qspAddStatName(qspStatGoTo, QSP_STATIC_STR(QSP_FMT("GT")), 2);
    qspAddStatName(qspStatIncLib, QSP_STATIC_STR(QSP_FMT("INCLIB")), 2);
    qspAddStatName(qspStatJump, QSP_STATIC_STR(QSP_FMT("JUMP")), 2);
    qspAddStatName(qspStatKillAll, QSP_STATIC_STR(QSP_FMT("KILLALL")), 2);
    qspAddStatName(qspStatKillObj, QSP_STATIC_STR(QSP_FMT("KILLOBJ")), 2);
    qspAddStatName(qspStatKillVar, QSP_STATIC_STR(QSP_FMT("KILLVAR")), 2);
    qspAddStatName(qspStatMenu, QSP_STATIC_STR(QSP_FMT("MENU")), 2);
    qspAddStatName(qspStatMClear, QSP_STATIC_STR(QSP_FMT("*CLEAR")), 2);
    qspAddStatName(qspStatMClear, QSP_STATIC_STR(QSP_FMT("*CLR")), 2);
    qspAddStatName(qspStatMNL, QSP_STATIC_STR(QSP_FMT("*NL")), 2);
    qspAddStatName(qspStatMPL, QSP_STATIC_STR(QSP_FMT("*PL")), 1);
    qspAddStatName(qspStatMP, QSP_STATIC_STR(QSP_FMT("*P")), 2);
    qspAddStatName(qspStatClear, QSP_STATIC_STR(QSP_FMT("CLEAR")), 2);
    qspAddStatName(qspStatClear, QSP_STATIC_STR(QSP_FMT("CLR")), 2);
    qspAddStatName(qspStatNL, QSP_STATIC_STR(QSP_FMT("NL")), 2);
    qspAddStatName(qspStatPL, QSP_STATIC_STR(QSP_FMT("PL")), 1);
    qspAddStatName(qspStatP, QSP_STATIC_STR(QSP_FMT("P")), 2);
    qspAddStatName(qspStatMsg, QSP_STATIC_STR(QSP_FMT("MSG")), 2);
    qspAddStatName(qspStatOpenGame, QSP_STATIC_STR(QSP_FMT("OPENGAME")), 2);
    qspAddStatName(qspStatOpenQst, QSP_STATIC_STR(QSP_FMT("OPENQST")), 2);
    qspAddStatName(qspStatPlay, QSP_STATIC_STR(QSP_FMT("PLAY")), 0);
    qspAddStatName(qspStatRefInt, QSP_STATIC_STR(QSP_FMT("REFINT")), 2);
    qspAddStatName(qspStatSaveGame, QSP_STATIC_STR(QSP_FMT("SAVEGAME")), 2);
    qspAddStatName(qspStatSetTimer, QSP_STATIC_STR(QSP_FMT("SETTIMER")), 1);
    qspAddStatName(qspStatShowActs, QSP_STATIC_STR(QSP_FMT("SHOWACTS")), 2);
    qspAddStatName(qspStatShowInput, QSP_STATIC_STR(QSP_FMT("SHOWINPUT")), 2);
    qspAddStatName(qspStatShowObjs, QSP_STATIC_STR(QSP_FMT("SHOWOBJS")), 2);
    qspAddStatName(qspStatShowVars, QSP_STATIC_STR(QSP_FMT("SHOWSTAT")), 2);
    qspAddStatName(qspStatUnSelect, QSP_STATIC_STR(QSP_FMT("UNSELECT")), 1);
    qspAddStatName(qspStatUnSelect, QSP_STATIC_STR(QSP_FMT("UNSEL")), 2);
    qspAddStatName(qspStatView, QSP_STATIC_STR(QSP_FMT("VIEW")), 2);
    qspAddStatName(qspStatWait, QSP_STATIC_STR(QSP_FMT("WAIT")), 2);
    qspAddStatName(qspStatXGoTo, QSP_STATIC_STR(QSP_FMT("XGOTO")), 2);
    qspAddStatName(qspStatXGoTo, QSP_STATIC_STR(QSP_FMT("XGT")), 2);
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
                    ++c;
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
                if (!(--c)) return -1;
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
                    ++c;
                    start += lines->LinesToEnd;
                    lines += lines->LinesToEnd;
                    continue;
                }
                break;
            case qspStatEnd:
                if (!(--c))
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

INLINE int qspSearchLabel(QSPLineOfCode *lines, int start, int end, QSPString str)
{
    lines += start;
    while (start < end)
    {
        if (lines->Label.Str && !qspStrsComp(lines->Label, str)) return start;
        ++start;
        ++lines;
    }
    return -1;
}

QSP_TINYINT qspGetStatArgs(QSPString s, QSPCachedStat *stat, QSPVariant *args)
{
    if (stat->ErrorCode)
    {
        qspSetError(stat->ErrorCode);
        return 0;
    }
    if (stat->ArgsCount > 0)
    {
        QSP_TINYINT argIndex, type;
        int oldLocationState = qspLocationState;
        for (argIndex = 0; argIndex < stat->ArgsCount; ++argIndex)
        {
            args[argIndex] = qspCalculateExprValue(qspStringFromPair(s.Str + stat->Args[argIndex].StartPos, s.Str + stat->Args[argIndex].EndPos));
            if (qspLocationState != oldLocationState)
            {
                qspFreeVariants(args, argIndex);
                return 0;
            }
            type = qspStats[stat->Stat].ArgsTypes[argIndex];
            if (QSP_ISDEF(type) && !qspConvertVariantTo(args + argIndex, type))
            {
                qspSetError(QSP_ERR_TYPEMISMATCH);
                qspFreeVariants(args, argIndex);
                return 0;
            }
        }
    }
    return stat->ArgsCount;
}

INLINE QSP_BOOL qspExecString(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo)
{
    QSP_TINYINT statCode;
    int i, oldLocationState = qspLocationState;
    for (i = startStat; i < endStat; ++i)
    {
        statCode = line->Stats[i].Stat;
        switch (statCode)
        {
        case qspStatUnknown:
        case qspStatLabel:
        case qspStatElse:
        case qspStatEnd:
            break;
        case qspStatComment:
            return QSP_FALSE;
        case qspStatAct:
            qspStatementSinglelineAddAct(line, i, endStat);
            return QSP_FALSE;
        case qspStatIf:
        case qspStatElseIf:
            return qspStatementIf(line, i, endStat, jumpTo);
        case qspStatLoop:
            return qspStatementSinglelineLoop(line, i, endStat, jumpTo);
        case qspStatLocal:
            qspStatementLocal(line->Str, line->Stats + i);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            break;
        case qspStatSet:
            qspStatementSetVarValue(line->Str, line->Stats + i);
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            break;
        case qspStatExit:
            return QSP_TRUE;
        case qspStatJump:
            {
                QSPVariant arg; /* 1 argument only */
                qspGetStatArgs(line->Str, line->Stats + i, &arg);
                if (qspLocationState != oldLocationState) return QSP_FALSE;
                qspUpdateText(jumpTo, qspDelSpc(QSP_STR(arg)));
                qspUpperStr(jumpTo);
                qspFreeVariant(&arg);
                return QSP_TRUE;
            }
        default:
            {
                QSPVariant args[QSP_STATMAXARGS];
                QSP_TINYINT argsCount = qspGetStatArgs(line->Str, line->Stats + i, args);
                if (qspLocationState != oldLocationState) return QSP_FALSE;
                qspStats[statCode].Func(args, argsCount, statCode);
                qspFreeVariants(args, argsCount);
                if (qspLocationState != oldLocationState) return QSP_FALSE;
                break;
            }
        }
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspExecMultilineCode(QSPLineOfCode *lines, int endLine, int codeOffset,
    QSPString *jumpTo, int *lineInd, int *action)
{
    QSPLineOfCode *line;
    int ind = *lineInd;
    endLine = qspSearchEnd(lines, ind, endLine);
    if (endLine < 0)
    {
        qspSetError(QSP_ERR_ENDNOTFOUND);
        return QSP_FALSE;
    }
    line = lines + ind;
    switch (line->Stats->Stat)
    {
    case qspStatIf:
    case qspStatElseIf:
        {
            int elsePos, oldLocationState = qspLocationState;
            QSP_BOOL condition = qspCheckCondition(qspStringFromPair(line->Str.Str + line->Stats->ParamPos, line->Str.Str + line->Stats->EndPos));
            if (qspLocationState != oldLocationState) return QSP_FALSE;
            elsePos = qspSearchElse(lines, ind, endLine);
            if (condition)
            {
                *lineInd = endLine;
                *action = qspFlowJumpToSpecified;
                if (elsePos >= 0)
                    return qspExecCodeBlockWithLocals(lines, ind + 1, elsePos, codeOffset, jumpTo);
                return qspExecCodeBlockWithLocals(lines, ind + 1, endLine, codeOffset, jumpTo);
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
                return qspExecCodeBlockWithLocals(lines, ind + 1, elsePos, codeOffset, jumpTo);
            return qspExecCodeBlockWithLocals(lines, ind + 1, endLine, codeOffset, jumpTo);
        }
    case qspStatAct:
        *lineInd = endLine;
        *action = qspFlowJumpToSpecified;
        qspStatementMultilineAddAct(lines, ind, endLine);
        break;
    case qspStatLoop:
        *lineInd = endLine;
        *action = qspFlowJumpToSpecified;
        return qspStatementMultilineLoop(lines, ind, endLine, codeOffset, jumpTo);
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspExecSinglelineCode(QSPLineOfCode *lines, int endLine,
    QSPString *jumpTo, int *lineInd, int *action)
{
    int ind = *lineInd;
    QSPLineOfCode *line = lines + ind;
    switch (line->Stats->Stat)
    {
    case qspStatElseIf:
        {
            int oldLocationState;
            QSP_BOOL condition;
            QSP_CHAR *endPos = line->Str.Str + line->Stats->EndPos;
            if (!qspIsCharAtPos(line->Str, endPos, QSP_COLONDELIM[0]))
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
            condition = qspCheckCondition(qspStringFromPair(line->Str.Str + line->Stats->ParamPos, endPos));
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

QSP_BOOL qspExecCode(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSPString *jumpTo)
{
    QSPLineOfCode *line;
    QSPString jumpToFake;
    QSP_BOOL uLevel, toExit = QSP_FALSE;
    int i, oldLocationState = qspLocationState, action = qspFlowExecute;
    /* Prepare temporary data */
    if (uLevel = !jumpTo)
    {
        jumpToFake = qspNullString;
        jumpTo = &jumpToFake;
    }
    /* Code execution */
    i = startLine;
    while (i < endLine)
    {
        line = s + i;

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
            toExit = qspExecMultilineCode(s, endLine, codeOffset, jumpTo, &i, &action);
        else
            toExit = qspExecSinglelineCode(s, endLine, jumpTo, &i, &action);
        if (qspLocationState != oldLocationState) break;
        if (toExit)
        {
            if (!qspIsEmpty(*jumpTo))
            {
                i = qspSearchLabel(s, startLine, endLine, *jumpTo);
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
                    i = qspSearchLabel(s, startLine, endLine, *jumpTo);
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

QSP_BOOL qspExecCodeBlockWithLocals(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSPString *jumpTo)
{
    QSP_BOOL toExit;
    int oldLocationState = qspLocationState;
    qspAllocateSavedVarsGroup();
    toExit = qspExecCode(s, startLine, endLine, codeOffset, jumpTo);
    if (qspLocationState != oldLocationState)
    {
        qspClearLastSavedVarsGroup();
        return QSP_FALSE;
    }
    qspRestoreLastSavedVarsGroup();
    return toExit;
}

INLINE QSP_BOOL qspExecStringWithLocals(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo)
{
    QSP_BOOL toExit;
    int oldLocationState = qspLocationState;
    qspAllocateSavedVarsGroup();
    toExit = qspExecString(line, startStat, endStat, jumpTo);
    if (qspLocationState != oldLocationState)
    {
        qspClearLastSavedVarsGroup();
        return QSP_FALSE;
    }
    qspRestoreLastSavedVarsGroup();
    return toExit;
}

void qspExecStringAsCodeWithArgs(QSPString s, QSPVariant *args, QSP_TINYINT count, int codeOffset, QSPVariant *res)
{
    QSPLineOfCode *strs;
    QSPVar *varArgs, *varRes;
    int oldLocationState, linesCount;
    if (!(varArgs = qspVarReference(QSP_STATIC_STR(QSP_VARARGS), QSP_TRUE))) return;
    if (!(varRes = qspVarReference(QSP_STATIC_STR(QSP_VARRES), QSP_TRUE))) return;
    qspAllocateSavedVarsGroupWithArgs(varArgs, varRes);
    qspSetArgs(varArgs, args, count, QSP_TRUE);
    linesCount = qspPreprocessData(s, &strs);
    oldLocationState = qspLocationState;
    qspExecCode(strs, 0, linesCount, codeOffset, 0);
    qspFreePrepLines(strs, linesCount);
    if (qspLocationState != oldLocationState)
    {
        qspClearLastSavedVarsGroup();
        return;
    }
    if (res)
    {
        if (!(varRes = qspVarReference(QSP_STATIC_STR(QSP_VARRES), QSP_FALSE)))
        {
            qspClearLastSavedVarsGroup();
            return;
        }
        qspApplyResult(varRes, res);
    }
    qspRestoreLastSavedVarsGroup();
}

INLINE QSP_BOOL qspStatementIf(QSPLineOfCode *line, int startStat, int endStat, QSPString *jumpTo)
{
    QSP_BOOL condition;
    int i, c, elseStat, oldLocationState;
    QSP_CHAR *endPos = line->Str.Str + line->Stats[startStat].EndPos;
    if (!qspIsCharAtPos(line->Str, endPos, QSP_COLONDELIM[0]))
    {
        qspSetError(QSP_ERR_COLONNOTFOUND);
        return QSP_FALSE;
    }
    elseStat = 0;
    c = 1;
    for (i = startStat + 1; i < endStat; ++i)
    {
        switch (line->Stats[i].Stat)
        {
        case qspStatIf:
            ++c;
            break;
        case qspStatElse:
        case qspStatElseIf:
            --c;
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
    condition = qspCheckCondition(qspStringFromPair(line->Str.Str + line->Stats[startStat].ParamPos, endPos));
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

INLINE QSP_BOOL qspPrepareLoop(QSPString params, QSPString *condition, QSPString *iterator, QSPString *jumpTo)
{
    QSPLineOfCode initializatorLine;
    QSP_CHAR *whilePos, *stepPos;
    whilePos = qspStrPos(params, QSP_STATIC_STR(QSP_STATLOOPWHILE), QSP_TRUE);
    if (!whilePos)
    {
        qspSetError(QSP_ERR_LOOPWHILENOTFOUND);
        return QSP_FALSE;
    }
    stepPos = qspStrPos(qspStringFromPair(whilePos + QSP_STATIC_LEN(QSP_STATLOOPWHILE), params.End), QSP_STATIC_STR(QSP_STATLOOPSTEP), QSP_TRUE);
    /* Execute loop initialization */
    qspInitLineOfCode(&initializatorLine, qspStringFromPair(params.Str, whilePos), 0);
    if (initializatorLine.StatsCount)
    {
        int oldLocationState = qspLocationState;
        QSP_BOOL toExit = qspExecString(&initializatorLine, 0, initializatorLine.StatsCount, jumpTo);
        if (toExit || qspLocationState != oldLocationState)
        {
            qspFreeLineOfCode(&initializatorLine);
            return toExit;
        }
    }
    qspFreeLineOfCode(&initializatorLine);
    /* Set positions of the loop conditions */
    if (stepPos)
    {
        *condition = qspStringFromPair(whilePos + QSP_STATIC_LEN(QSP_STATLOOPWHILE), stepPos);
        *iterator = qspStringFromPair(stepPos + QSP_STATIC_LEN(QSP_STATLOOPSTEP), params.End);
    }
    else
    {
        *condition = qspStringFromPair(whilePos + QSP_STATIC_LEN(QSP_STATLOOPWHILE), params.End);
        *iterator = qspNullString;
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspCheckCondition(QSPString expr)
{
    int oldLocationState = qspLocationState;
    QSPVariant condValue = qspCalculateExprValue(expr);
    if (qspLocationState != oldLocationState) return QSP_FALSE;
    if (!qspConvertVariantTo(&condValue, QSP_TYPE_NUM))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        qspFreeVariant(&condValue);
        return QSP_FALSE;
    }
    return QSP_ISTRUE(QSP_NUM(condValue));
}

INLINE QSP_BOOL qspCheckCompiledCondition(QSPMathExpression *expression)
{
    int oldLocationState = qspLocationState;
    QSPVariant condValue = qspCalculateValue(expression, expression->ItemsCount - 1);
    if (qspLocationState != oldLocationState) return QSP_FALSE;
    if (!qspConvertVariantTo(&condValue, QSP_TYPE_NUM))
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
    QSPString condition, iterator;
    QSP_CHAR *endPos = line->Str.Str + line->Stats[startStat].EndPos;
    if (!qspIsCharAtPos(line->Str, endPos, QSP_COLONDELIM[0]))
    {
        qspSetError(QSP_ERR_COLONNOTFOUND);
        return QSP_FALSE;
    }
    if (startStat == endStat - 1)
    {
        qspSetError(QSP_ERR_CODENOTFOUND);
        return QSP_FALSE;
    }
    qspAllocateSavedVarsGroup();
    oldLocationState = qspLocationState;
    toExit = qspPrepareLoop(qspStringFromPair(line->Str.Str + line->Stats[startStat].ParamPos, endPos), &condition, &iterator, jumpTo);
    if (qspLocationState != oldLocationState)
    {
        qspClearLastSavedVarsGroup();
        return QSP_FALSE;
    }
    if (!toExit)
    {
        QSP_BOOL conditionValue;
        QSPLineOfCode iteratorLine;
        QSPMathExpression expression;
        if (!qspCompileMathExpression(condition, QSP_TRUE, &expression))
        {
            qspClearLastSavedVarsGroup();
            return QSP_FALSE;
        }
        qspInitLineOfCode(&iteratorLine, iterator, 0);
        while (1)
        {
            /* Check condition */
            conditionValue = qspCheckCompiledCondition(&expression);
            if (qspLocationState != oldLocationState)
            {
                qspFreeMathValue(&expression, expression.ItemsCount - 1);
                qspClearLastSavedVarsGroup();
                qspFreeLineOfCode(&iteratorLine);
                return QSP_FALSE;
            }
            if (!conditionValue) break;
            /* Execute body */
            toExit = qspExecStringWithLocals(line, startStat + 1, endStat, jumpTo);
            if (qspLocationState != oldLocationState)
            {
                qspFreeMathValue(&expression, expression.ItemsCount - 1);
                qspClearLastSavedVarsGroup();
                qspFreeLineOfCode(&iteratorLine);
                return QSP_FALSE;
            }
            if (toExit) break;
            /* Execute iterator */
            if (iteratorLine.StatsCount)
            {
                toExit = qspExecStringWithLocals(&iteratorLine, 0, iteratorLine.StatsCount, jumpTo);
                if (qspLocationState != oldLocationState)
                {
                    qspFreeMathValue(&expression, expression.ItemsCount - 1);
                    qspClearLastSavedVarsGroup();
                    qspFreeLineOfCode(&iteratorLine);
                    return QSP_FALSE;
                }
                if (toExit) break;
            }
        }
        qspFreeMathValue(&expression, expression.ItemsCount - 1);
        qspFreeLineOfCode(&iteratorLine);
    }
    qspRestoreLastSavedVarsGroup();
    return toExit;
}

INLINE QSP_BOOL qspStatementMultilineLoop(QSPLineOfCode *lines, int lineInd, int endLine,
    int codeOffset, QSPString *jumpTo)
{
    QSP_BOOL toExit;
    int oldLocationState;
    QSPString condition, iterator;
    QSPLineOfCode *line = lines + lineInd;
    qspAllocateSavedVarsGroup();
    oldLocationState = qspLocationState;
    toExit = qspPrepareLoop(qspStringFromPair(line->Str.Str + line->Stats->ParamPos, line->Str.Str + line->Stats->EndPos), &condition, &iterator, jumpTo);
    if (qspLocationState != oldLocationState)
    {
        qspClearLastSavedVarsGroup();
        return QSP_FALSE;
    }
    if (!toExit)
    {
        QSP_BOOL conditionValue;
        QSPLineOfCode iteratorLine;
        QSPMathExpression expression;
        if (!qspCompileMathExpression(condition, QSP_TRUE, &expression))
        {
            qspClearLastSavedVarsGroup();
            return QSP_FALSE;
        }
        qspInitLineOfCode(&iteratorLine, iterator, 0);
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
                    if (qspLocationState != oldLocationState)
                    {
                        qspFreeMathValue(&expression, expression.ItemsCount - 1);
                        qspClearLastSavedVarsGroup();
                        qspFreeLineOfCode(&iteratorLine);
                        return QSP_FALSE;
                    }
                }
            }
            /* Check condition */
            conditionValue = qspCheckCompiledCondition(&expression);
            if (qspLocationState != oldLocationState)
            {
                qspFreeMathValue(&expression, expression.ItemsCount - 1);
                qspClearLastSavedVarsGroup();
                qspFreeLineOfCode(&iteratorLine);
                return QSP_FALSE;
            }
            if (!conditionValue) break;
            /* Execute body */
            toExit = qspExecCodeBlockWithLocals(lines, lineInd, endLine, codeOffset, jumpTo);
            if (qspLocationState != oldLocationState)
            {
                qspFreeMathValue(&expression, expression.ItemsCount - 1);
                qspClearLastSavedVarsGroup();
                qspFreeLineOfCode(&iteratorLine);
                return QSP_FALSE;
            }
            if (toExit) break;
            /* Execute iterator */
            if (iteratorLine.StatsCount)
            {
                qspRealLine = line;
                if (codeOffset > 0)
                {
                    qspRealLineNum = line->LineNum + codeOffset;
                    if (qspIsDebug)
                    {
                        qspCallDebug(line->Str);
                        if (qspLocationState != oldLocationState)
                        {
                            qspFreeMathValue(&expression, expression.ItemsCount - 1);
                            qspClearLastSavedVarsGroup();
                            qspFreeLineOfCode(&iteratorLine);
                            return QSP_FALSE;
                        }
                    }
                }
                toExit = qspExecStringWithLocals(&iteratorLine, 0, iteratorLine.StatsCount, jumpTo);
                if (qspLocationState != oldLocationState)
                {
                    qspFreeMathValue(&expression, expression.ItemsCount - 1);
                    qspClearLastSavedVarsGroup();
                    qspFreeLineOfCode(&iteratorLine);
                    return QSP_FALSE;
                }
                if (toExit) break;
            }
        }
        qspFreeMathValue(&expression, expression.ItemsCount - 1);
        qspFreeLineOfCode(&iteratorLine);
    }
    qspRestoreLastSavedVarsGroup();
    return toExit;
}

INLINE void qspStatementImplicitStatement(QSPVariant *args, QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    if (QSP_ISDEF(args[0].Type))
    {
        qspConvertVariantTo(args, QSP_TYPE_STR);
        qspAddBufText(&qspCurDesc, QSP_STR(args[0]));
        qspAddBufText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM));
        qspIsMainDescChanged = QSP_TRUE;
    }
}

INLINE void qspStatementAddText(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT extArg)
{
    switch (extArg)
    {
    case qspStatP:
        if (qspAddBufText(&qspCurVars, QSP_STR(args[0])))
            qspIsVarsDescChanged = QSP_TRUE;
        break;
    case qspStatMP:
        if (qspAddBufText(&qspCurDesc, QSP_STR(args[0])))
            qspIsMainDescChanged = QSP_TRUE;
        break;
    case qspStatPL:
        if (count) qspAddBufText(&qspCurVars, QSP_STR(args[0]));
        qspAddBufText(&qspCurVars, QSP_STATIC_STR(QSP_STRSDELIM));
        qspIsVarsDescChanged = QSP_TRUE;
        break;
    case qspStatMPL:
        if (count) qspAddBufText(&qspCurDesc, QSP_STR(args[0]));
        qspAddBufText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM));
        qspIsMainDescChanged = QSP_TRUE;
        break;
    case qspStatNL:
        qspAddBufText(&qspCurVars, QSP_STATIC_STR(QSP_STRSDELIM));
        if (count) qspAddBufText(&qspCurVars, QSP_STR(args[0]));
        qspIsVarsDescChanged = QSP_TRUE;
        break;
    case qspStatMNL:
        qspAddBufText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM));
        if (count) qspAddBufText(&qspCurDesc, QSP_STR(args[0]));
        qspIsMainDescChanged = QSP_TRUE;
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
            qspIsVarsDescChanged = QSP_TRUE;
        }
        break;
    case qspStatMClear:
        if (qspCurDesc.Len > 0)
        {
            qspClearBufString(&qspCurDesc);
            qspIsMainDescChanged = QSP_TRUE;
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
            qspIsVarsDescChanged = QSP_TRUE;
        }
        if (qspCurDesc.Len > 0)
        {
            qspClearBufString(&qspCurDesc);
            qspIsMainDescChanged = QSP_TRUE;
        }
        qspClearText(&qspCurInput);
        qspClearAllActions(QSP_FALSE);
        qspCallSetInputStrText(qspNullString);
        break;
    case qspStatKillAll:
        qspClearAllVars(QSP_FALSE);
        qspClearAllObjectsWithNotify();
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
    int locInd = qspLocIndex(QSP_STR(args[0]));
    if (locInd < 0)
    {
        qspSetError(QSP_ERR_LOCNOTFOUND);
        return;
    }
    qspNavigateToLocation(locInd, extArg == qspStatGoTo, args + 1, count - 1);
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
    case qspStatShowActs:
        qspCallShowWindow(QSP_WIN_ACTS, qspCurToShowActs = val);
        break;
    case qspStatShowObjs:
        qspCallShowWindow(QSP_WIN_OBJS, qspCurToShowObjs = val);
        break;
    case qspStatShowVars:
        qspCallShowWindow(QSP_WIN_VARS, qspCurToShowVars = val);
        break;
    case qspStatShowInput:
        qspCallShowWindow(QSP_WIN_INPUT, qspCurToShowInput = val);
        break;
    }
}

INLINE void qspStatementRefInt(QSPVariant *QSP_UNUSED(args), QSP_TINYINT QSP_UNUSED(count), QSP_TINYINT QSP_UNUSED(extArg))
{
    qspCallRefreshInt(QSP_TRUE);
}

INLINE void qspStatementView(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    if (count && qspIsAnyString(QSP_STR(args[0])))
    {
        qspUpdateText(&qspViewPath, QSP_STR(args[0]));
        qspCallShowPicture(qspViewPath);
    }
    else
    {
        qspClearText(&qspViewPath);
        qspCallShowPicture(qspNullString);
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
    qspExecStringAsCodeWithArgs(QSP_STR(args[0]), args + 1, count - 1, 0, 0);
}
