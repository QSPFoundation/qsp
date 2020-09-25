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

QSPStatement qspStats[qspStatLast_Statement];
QSPStatName qspStatsNames[QSP_STATSLEVELS][QSP_MAXSTATSNAMES];
int qspStatsNamesCounts[QSP_STATSLEVELS];
int qspStatMaxLen = 0;

INLINE void qspAddStatement(int, QSP_STATEMENT, int, int, ...);
INLINE void qspAddStatName(int statCode, QSPString statName, int level);
INLINE int qspStatsCompare(const void *, const void *);
INLINE int qspSearchElse(QSPLineOfCode *, int, int);
INLINE int qspSearchEnd(QSPLineOfCode *, int, int);
INLINE int qspSearchLabel(QSPLineOfCode *s, int start, int end, QSPString str);
INLINE QSP_BOOL qspExecString(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspExecMultilineCode(QSPLineOfCode *s, int endLine, int codeOffset, QSPString *jumpTo, int *lineInd, int *action);
INLINE QSP_BOOL qspExecSinglelineCode(QSPLineOfCode *s, int endLine, int codeOffset, QSPString *jumpTo, int *lineInd, int *action);
INLINE QSP_BOOL qspExecStringWithLocals(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspStatementIf(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspPrepareLoop(QSPString params, QSPString *condition, QSPString *iterator, QSPString *jumpTo);
INLINE QSP_BOOL qspCheckCondition(QSPString expr);
INLINE QSP_BOOL qspStatementSinglelineLoop(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo);
INLINE QSP_BOOL qspStatementMultilineLoop(QSPLineOfCode *s, int endLine, int lineInd, int codeOffset, QSPString *jumpTo);
INLINE QSP_BOOL qspStatementImplicitStatement(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementAddText(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementClear(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementExit(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementGoSub(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementGoTo(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementJump(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementWait(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementSetTimer(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementShowWin(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementRefInt(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementView(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementMsg(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementExec(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
INLINE QSP_BOOL qspStatementDynamic(QSPVariant *args, int count, QSPString *jumpTo, int extArg);

INLINE void qspAddStatement(int statCode, QSP_STATEMENT func, int minArgs, int maxArgs, ...)
{
    int i;
    va_list marker;
    qspStats[statCode].Func = func;
    qspStats[statCode].MinArgsCount = minArgs;
    qspStats[statCode].MaxArgsCount = maxArgs;
    if (maxArgs > 0)
    {
        va_start(marker, maxArgs);
        for (i = 0; i < maxArgs; ++i)
            qspStats[statCode].ArgsTypes[i] = va_arg(marker, int);
        va_end(marker);
    }
}

INLINE void qspAddStatName(int statCode, QSPString statName, int level)
{
    int count, len = qspStrLen(statName);
    count = qspStatsNamesCounts[level];
    qspStatsNames[level][count].Name = statName;
    qspStatsNames[level][count].Code = statCode;
    qspStatsNamesCounts[level] = count + 1;
    /* Max length */
    if (len > qspStatMaxLen) qspStatMaxLen = len;
}

INLINE int qspStatsCompare(const void *statName1, const void *statName2)
{
    return qspStrsComp(((QSPStatName *)statName1)->Name, ((QSPStatName *)statName2)->Name);
}

void qspInitStats()
{
    /*
    Format:
        qspAddStatement(
            Statement,
            Extended Argument,
            Statement's Function,
            Minimum Arguments' Count,
            Maximum Arguments' Count,
            Arguments' Types [optional]
        );

        "Arguments' Types":
        -1 - Unknown / Any
        0 - Number
        1 - String
    */
    int i;
    for (i = 0; i < QSP_STATSLEVELS; ++i) qspStatsNamesCounts[i] = 0;
    qspStatMaxLen = 0;
    qspAddStatement(qspStatImplicitStatement, qspStatementImplicitStatement, 1, 1, -1);
    qspAddStatement(qspStatElse, 0, 0, 0);
    qspAddStatement(qspStatElseIf, 0, 1, 1, 0);
    qspAddStatement(qspStatEnd, 0, 0, 0);
    qspAddStatement(qspStatLocal, 0, 0, 0);
    qspAddStatement(qspStatSet, 0, 0, 0);
    qspAddStatement(qspStatIf, 0, 1, 1, 0);
    qspAddStatement(qspStatAct, 0, 1, 2, 1, 1);
    qspAddStatement(qspStatLoop, 0, 0, 0);
    qspAddStatement(qspStatAddObj, qspStatementAddObject, 1, 3, 1, 1, 0);
    qspAddStatement(qspStatClA, qspStatementClear, 0, 0);
    qspAddStatement(qspStatCloseAll, qspStatementCloseFile, 0, 0);
    qspAddStatement(qspStatClose, qspStatementCloseFile, 0, 1, 1);
    qspAddStatement(qspStatClS, qspStatementClear, 0, 0);
    qspAddStatement(qspStatCmdClear, qspStatementClear, 0, 0);
    qspAddStatement(qspStatCopyArr, qspStatementCopyArr, 2, 4, 1, 1, 0, 0);
    qspAddStatement(qspStatDelAct, qspStatementDelAct, 1, 1, 1);
    qspAddStatement(qspStatDelObj, qspStatementDelObj, 1, 1, 1);
    qspAddStatement(qspStatDynamic, qspStatementDynamic, 1, 20, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    qspAddStatement(qspStatExec, qspStatementExec, 1, 1, 1);
    qspAddStatement(qspStatExit, qspStatementExit, 0, 0);
    qspAddStatement(qspStatFreeLib, qspStatementClear, 0, 0);
    qspAddStatement(qspStatGoSub, qspStatementGoSub, 1, 20, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    qspAddStatement(qspStatGoTo, qspStatementGoTo, 1, 20, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    qspAddStatement(qspStatIncLib, qspStatementOpenQst, 1, 1, 1);
    qspAddStatement(qspStatJump, qspStatementJump, 1, 1, 1);
    qspAddStatement(qspStatKillAll, qspStatementClear, 0, 0);
    qspAddStatement(qspStatKillObj, qspStatementDelObj, 0, 1, 0);
    qspAddStatement(qspStatKillVar, qspStatementKillVar, 0, 2, 1, 0);
    qspAddStatement(qspStatMenu, qspStatementShowMenu, 1, 3, 1, 0, 0);
    qspAddStatement(qspStatMClear, qspStatementClear, 0, 0);
    qspAddStatement(qspStatMNL, qspStatementAddText, 0, 1, 1);
    qspAddStatement(qspStatMPL, qspStatementAddText, 0, 1, 1);
    qspAddStatement(qspStatMP, qspStatementAddText, 1, 1, 1);
    qspAddStatement(qspStatClear, qspStatementClear, 0, 0);
    qspAddStatement(qspStatNL, qspStatementAddText, 0, 1, 1);
    qspAddStatement(qspStatPL, qspStatementAddText, 0, 1, 1);
    qspAddStatement(qspStatP, qspStatementAddText, 1, 1, 1);
    qspAddStatement(qspStatMsg, qspStatementMsg, 1, 1, 1);
    qspAddStatement(qspStatOpenGame, qspStatementOpenGame, 0, 1, 1);
    qspAddStatement(qspStatOpenQst, qspStatementOpenQst, 1, 1, 1);
    qspAddStatement(qspStatPlay, qspStatementPlayFile, 1, 2, 1, 0);
    qspAddStatement(qspStatRefInt, qspStatementRefInt, 0, 0);
    qspAddStatement(qspStatSaveGame, qspStatementSaveGame, 0, 1, 1);
    qspAddStatement(qspStatSetTimer, qspStatementSetTimer, 1, 1, 0);
    qspAddStatement(qspStatShowActs, qspStatementShowWin, 1, 1, 0);
    qspAddStatement(qspStatShowInput, qspStatementShowWin, 1, 1, 0);
    qspAddStatement(qspStatShowObjs, qspStatementShowWin, 1, 1, 0);
    qspAddStatement(qspStatShowVars, qspStatementShowWin, 1, 1, 0);
    qspAddStatement(qspStatUnSelect, qspStatementUnSelect, 0, 0);
    qspAddStatement(qspStatView, qspStatementView, 0, 1, 1);
    qspAddStatement(qspStatWait, qspStatementWait, 1, 1, 0);
    qspAddStatement(qspStatXGoTo, qspStatementGoTo, 1, 20, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    /* Names */
    qspAddStatName(qspStatElse, QSP_STATIC_STR(QSP_STATELSE), 2);
    qspAddStatName(qspStatElseIf, QSP_STATIC_STR(QSP_FMT("ELSEIF")), 1);
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

INLINE int qspSearchElse(QSPLineOfCode *s, int start, int end)
{
    int c = 1;
    s += start;
    while (start < end)
    {
        switch (s->Stats->Stat)
        {
        case qspStatAct:
        case qspStatLoop:
        case qspStatIf:
            if (s->IsMultiline) ++c;
            break;
        case qspStatElse:
        case qspStatElseIf:
            if (c == 1) return start;
            break;
        case qspStatEnd:
            if (!(--c)) return -1;
            break;
        }
        ++start;
        ++s;
    }
    return -1;
}

INLINE int qspSearchEnd(QSPLineOfCode *s, int start, int end)
{
    int c = 1;
    s += start;
    while (start < end)
    {
        switch (s->Stats->Stat)
        {
        case qspStatAct:
        case qspStatLoop:
        case qspStatIf:
            if (s->IsMultiline) ++c;
            break;
        case qspStatEnd:
            if (!(--c)) return start;
            break;
        }
        ++start;
        ++s;
    }
    return -1;
}

INLINE int qspSearchLabel(QSPLineOfCode *s, int start, int end, QSPString str)
{
    s += start;
    while (start < end)
    {
        if (s->Label.Str && !qspStrsComp(s->Label, str)) return start;
        ++start;
        ++s;
    }
    return -1;
}

int qspGetStatArgs(QSPString s, QSPCachedStat *stat, QSPVariant *args)
{
    int type, argIndex, oldRefreshCount;
    if (stat->ErrorCode)
    {
        qspSetError(stat->ErrorCode);
        return 0;
    }
    if (stat->ArgsCount > 0)
    {
        oldRefreshCount = qspRefreshCount;
        for (argIndex = 0; argIndex < stat->ArgsCount; ++argIndex)
        {
            args[argIndex] = qspExprValue(qspStringFromPair(s.Str + stat->Args[argIndex].StartPos, s.Str + stat->Args[argIndex].EndPos));
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
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

INLINE QSP_BOOL qspExecString(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo)
{
    QSPVariant args[QSP_STATMAXARGS];
    QSP_BOOL isExit;
    int i, statCode, count, oldRefreshCount = qspRefreshCount;
    for (i = startStat; i < endStat; ++i)
    {
        statCode = s->Stats[i].Stat;
        switch (statCode)
        {
        case qspStatUnknown:
        case qspStatLabel:
        case qspStatElse:
        case qspStatEnd:
            break;
        case qspStatComment:
        case qspStatElseIf:
            return QSP_FALSE;
        case qspStatAct:
            qspStatementSinglelineAddAct(s, i, endStat);
            return QSP_FALSE;
        case qspStatIf:
            return qspStatementIf(s, i, endStat, jumpTo);
        case qspStatLoop:
            return qspStatementSinglelineLoop(s, i, endStat, jumpTo);
        case qspStatLocal:
            qspStatementLocal(s->Str, s->Stats + i);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
            break;
        case qspStatSet:
            qspStatementSetVarValue(s->Str, s->Stats + i);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
            break;
        default:
            count = qspGetStatArgs(s->Str, s->Stats + i, args);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
            isExit = qspStats[statCode].Func(args, count, jumpTo, statCode);
            qspFreeVariants(args, count);
            if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum) return isExit;
            break;
        }
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspExecMultilineCode(QSPLineOfCode *s, int endLine, int codeOffset,
    QSPString *jumpTo, int *lineInd, int *action)
{
    QSP_BOOL condition;
    QSPLineOfCode *line;
    int ind, statCode, elsePos, oldRefreshCount;
    ind = *lineInd;
    endLine = qspSearchEnd(s, ind + 1, endLine);
    if (endLine < 0)
    {
        qspSetError(QSP_ERR_ENDNOTFOUND);
        return QSP_FALSE;
    }
    line = s + ind;
    statCode = line->Stats->Stat;
    switch (statCode)
    {
    case qspStatIf:
    case qspStatElseIf:
        oldRefreshCount = qspRefreshCount;
        condition = qspCheckCondition(qspStringFromPair(line->Str.Str + line->Stats->ParamPos, line->Str.Str + line->Stats->EndPos));
        if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
        elsePos = qspSearchElse(s, ind + 1, endLine);
        if (condition)
        {
            *lineInd = endLine;
            *action = qspFlowJumpToSpecified;
            if (elsePos >= 0)
                return qspExecCodeBlockWithLocals(s, ind + 1, elsePos, codeOffset, jumpTo);
            return qspExecCodeBlockWithLocals(s, ind + 1, endLine, codeOffset, jumpTo);
        }
        else
        {
            *lineInd = (elsePos >= 0 ? elsePos : endLine);
            *action = qspFlowJumpToSpecified;
        }
        break;
    case qspStatAct:
        *lineInd = endLine;
        *action = qspFlowJumpToSpecified;
        qspStatementMultilineAddAct(s, ind, endLine);
        break;
    case qspStatLoop:
        *lineInd = endLine;
        *action = qspFlowJumpToSpecified;
        return qspStatementMultilineLoop(s, ind, endLine, codeOffset, jumpTo);
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspExecSinglelineCode(QSPLineOfCode *s, int endLine, int codeOffset,
    QSPString *jumpTo, int *lineInd, int *action)
{
    QSP_BOOL condition;
    QSPLineOfCode *line;
    QSP_CHAR *pos;
    int ind, elsePos, oldRefreshCount;
    ind = *lineInd;
    line = s + ind;
    switch (line->Stats->Stat)
    {
    case qspStatElseIf:
        pos = line->Str.Str + line->Stats->EndPos;
        if (pos == line->Str.End || *pos != QSP_COLONDELIM[0])
        {
            qspSetError(QSP_ERR_COLONNOTFOUND);
            break;
        }
        endLine = qspSearchEnd(s, ind + 1, endLine);
        if (endLine < 0)
        {
            qspSetError(QSP_ERR_ENDNOTFOUND);
            break;
        }
        oldRefreshCount = qspRefreshCount;
        condition = qspCheckCondition(qspStringFromPair(line->Str.Str + line->Stats->ParamPos, pos));
        if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
        if (condition)
        {
            *lineInd = endLine;
            *action = qspFlowJumpToSpecified;
            return qspExecStringWithLocals(line, 1, line->StatsCount, jumpTo);
        }
        else
        {
            elsePos = qspSearchElse(s, ind + 1, endLine);
            *lineInd = (elsePos >= 0 ? elsePos : endLine);
            *action = qspFlowJumpToSpecified;
        }
        break;
    case qspStatElse:
        endLine = qspSearchEnd(s, ind + 1, endLine);
        if (endLine < 0)
        {
            qspSetError(QSP_ERR_ENDNOTFOUND);
            break;
        }
        *lineInd = endLine;
        *action = qspFlowJumpToSpecified;
        if (line->StatsCount > 1)
            return qspExecStringWithLocals(line, 1, line->StatsCount, jumpTo);
        else
        {
            elsePos = qspSearchElse(s, ind + 1, endLine);
            if (elsePos >= 0)
                return qspExecCodeBlockWithLocals(s, ind + 1, elsePos, codeOffset, jumpTo);
            return qspExecCodeBlockWithLocals(s, ind + 1, endLine, codeOffset, jumpTo);
        }
        break;
    }
    return QSP_FALSE;
}

QSP_BOOL qspExecCode(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSPString *jumpTo)
{
    QSPLineOfCode *line;
    QSPString jumpToFake;
    QSP_BOOL uLevel, isExit = QSP_FALSE;
    int i, oldRefreshCount, action = qspFlowExecute;
    oldRefreshCount = qspRefreshCount;
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
        if (codeOffset > 0)
        {
            qspRealLine = line->LineNum + codeOffset;
            if (qspIsDebug && !qspIsEmpty(line->Str))
            {
                qspCallDebug(line->Str);
                if (qspRefreshCount != oldRefreshCount) break;
            }
        }
        if (line->IsMultiline)
            isExit = qspExecMultilineCode(s, endLine, codeOffset, jumpTo, &i, &action);
        else
            isExit = qspExecSinglelineCode(s, endLine, codeOffset, jumpTo, &i, &action);
        if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
        if (isExit)
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
            isExit = qspExecString(line, 0, line->StatsCount, jumpTo);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
            if (isExit)
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
        else
            action = qspFlowExecute;
    }
    if (uLevel) qspFreeString(jumpToFake);
    return isExit;
}

QSP_BOOL qspExecCodeBlockWithLocals(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSPString *jumpTo)
{
    QSP_BOOL isExit;
    int oldRefreshCount = qspRefreshCount;
    qspAllocateSavedVarsGroup();
    isExit = qspExecCode(s, startLine, endLine, codeOffset, jumpTo);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum)
    {
        qspReleaseSavedVarsGroup(QSP_TRUE);
        return QSP_FALSE;
    }
    qspReleaseSavedVarsGroup(QSP_FALSE);
    return isExit;
}

INLINE QSP_BOOL qspExecStringWithLocals(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo)
{
    QSP_BOOL isExit;
    int oldRefreshCount = qspRefreshCount;
    qspAllocateSavedVarsGroup();
    isExit = qspExecString(s, startStat, endStat, jumpTo);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum)
    {
        qspReleaseSavedVarsGroup(QSP_TRUE);
        return QSP_FALSE;
    }
    qspReleaseSavedVarsGroup(QSP_FALSE);
    return isExit;
}

void qspExecStringAsCodeWithArgs(QSPString s, QSPVariant *args, int count, QSPVariant *res)
{
    QSPLineOfCode *strs;
    QSPVar *varArgs, *varRes;
    int oldRefreshCount;
    if (!(varArgs = qspVarReference(QSP_STATIC_STR(QSP_VARARGS), QSP_TRUE))) return;
    if (!(varRes = qspVarReference(QSP_STATIC_STR(QSP_VARRES), QSP_TRUE))) return;
    qspAllocateSavedVarsGroupWithArgs(varArgs, varRes);
    qspSetArgs(varArgs, args, count);
    count = qspPreprocessData(s, &strs);
    oldRefreshCount = qspRefreshCount;
    qspExecCode(strs, 0, count, 0, 0);
    qspFreePrepLines(strs, count);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum)
    {
        qspReleaseSavedVarsGroup(QSP_TRUE);
        return;
    }
    if (res)
    {
        if (!(varRes = qspVarReference(QSP_STATIC_STR(QSP_VARRES), QSP_FALSE)))
        {
            qspReleaseSavedVarsGroup(QSP_TRUE);
            return;
        }
        qspApplyResult(varRes, res);
    }
    qspReleaseSavedVarsGroup(QSP_FALSE);
}

INLINE QSP_BOOL qspStatementIf(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo)
{
    QSP_BOOL condition;
    int i, c, elseStat, oldRefreshCount;
    QSP_CHAR *pos = s->Str.Str + s->Stats[startStat].EndPos;
    if (pos == s->Str.End || *pos != QSP_COLONDELIM[0])
    {
        qspSetError(QSP_ERR_COLONNOTFOUND);
        return QSP_FALSE;
    }
    elseStat = 0;
    c = 1;
    for (i = startStat + 1; i < endStat; ++i)
    {
        switch (s->Stats[i].Stat)
        {
        case qspStatIf:
            ++c;
            break;
        case qspStatElse:
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
        if (elseStat == startStat + 1)
        {
            qspSetError(QSP_ERR_CODENOTFOUND);
            return QSP_FALSE;
        }
        if (elseStat == endStat - 1)
        {
            qspSetError(QSP_ERR_CODENOTFOUND);
            return QSP_FALSE;
        }
    }
    else if (startStat == endStat - 1)
    {
        qspSetError(QSP_ERR_CODENOTFOUND);
        return QSP_FALSE;
    }
    oldRefreshCount = qspRefreshCount;
    condition = qspCheckCondition(qspStringFromPair(s->Str.Str + s->Stats[startStat].ParamPos, pos));
    if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
    if (condition)
    {
        if (elseStat)
            return qspExecStringWithLocals(s, startStat + 1, elseStat, jumpTo);
        else
            return qspExecStringWithLocals(s, startStat + 1, endStat, jumpTo);
    }
    else if (elseStat)
        return qspExecStringWithLocals(s, elseStat + 1, endStat, jumpTo);
    return QSP_FALSE;
}

INLINE QSP_BOOL qspPrepareLoop(QSPString params, QSPString *condition, QSPString *iterator, QSPString *jumpTo)
{
    int oldRefreshCount;
    QSPLineOfCode initializatorLine;
    QSP_BOOL isExit;
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
    oldRefreshCount = qspRefreshCount;
    isExit = qspExecString(&initializatorLine, 0, initializatorLine.StatsCount, jumpTo);
    qspFreeLineOfCode(&initializatorLine);
    if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum) return isExit;
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
    int oldRefreshCount = qspRefreshCount;
    QSPVariant condValue = qspExprValue(expr);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
    if (!qspConvertVariantTo(&condValue, QSP_TYPE_NUMBER))
    {
        qspSetError(QSP_ERR_TYPEMISMATCH);
        qspFreeString(QSP_STR(condValue));
        return QSP_FALSE;
    }
    return (QSP_NUM(condValue) != 0);
}

INLINE QSP_BOOL qspStatementSinglelineLoop(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo)
{
    QSP_BOOL isExit, conditionValue;
    int oldRefreshCount;
    QSP_CHAR *endPos;
    QSPLineOfCode iteratorLine;
    QSPString condition, iterator;
    endPos = s->Str.Str + s->Stats[startStat].EndPos;
    if (endPos == s->Str.End || *endPos != QSP_COLONDELIM[0])
    {
        qspSetError(QSP_ERR_COLONNOTFOUND);
        return QSP_FALSE;
    }
    qspAllocateSavedVarsGroup();
    oldRefreshCount = qspRefreshCount;
    isExit = qspPrepareLoop(qspStringFromPair(s->Str.Str + s->Stats[startStat].ParamPos, endPos), &condition, &iterator, jumpTo);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum)
    {
        qspReleaseSavedVarsGroup(QSP_TRUE);
        return QSP_FALSE;
    }
    if (!isExit)
    {
        qspInitLineOfCode(&iteratorLine, iterator, 0);
        while (1)
        {
            /* Check condition */
            conditionValue = qspCheckCondition(condition);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
            {
                qspReleaseSavedVarsGroup(QSP_TRUE);
                qspFreeLineOfCode(&iteratorLine);
                return QSP_FALSE;
            }
            if (!conditionValue) break;
            /* Execute body */
            isExit = qspExecStringWithLocals(s, startStat + 1, endStat, jumpTo);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
            {
                qspReleaseSavedVarsGroup(QSP_TRUE);
                qspFreeLineOfCode(&iteratorLine);
                return QSP_FALSE;
            }
            if (isExit) break;
            /* Execute iterator */
            isExit = qspExecStringWithLocals(&iteratorLine, 0, iteratorLine.StatsCount, jumpTo);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
            {
                qspReleaseSavedVarsGroup(QSP_TRUE);
                qspFreeLineOfCode(&iteratorLine);
                return QSP_FALSE;
            }
            if (isExit) break;
        }
        qspFreeLineOfCode(&iteratorLine);
    }
    qspReleaseSavedVarsGroup(QSP_FALSE);
    return isExit;
}

INLINE QSP_BOOL qspStatementMultilineLoop(QSPLineOfCode *s, int lineInd, int endLine,
    int codeOffset, QSPString *jumpTo)
{
    QSP_BOOL isExit, conditionValue;
    int oldRefreshCount;
    QSP_CHAR *endPos;
    QSPString condition, iterator;
    QSPLineOfCode iteratorLine, *line = s + lineInd;
    endPos = line->Str.Str + line->Stats->EndPos;
    if (endPos == line->Str.End || *endPos != QSP_COLONDELIM[0])
    {
        qspSetError(QSP_ERR_COLONNOTFOUND);
        return QSP_FALSE;
    }
    qspAllocateSavedVarsGroup();
    oldRefreshCount = qspRefreshCount;
    isExit = qspPrepareLoop(qspStringFromPair(line->Str.Str + line->Stats->ParamPos, endPos), &condition, &iterator, jumpTo);
    if (qspRefreshCount != oldRefreshCount || qspErrorNum)
    {
        qspReleaseSavedVarsGroup(QSP_TRUE);
        return QSP_FALSE;
    }
    if (!isExit)
    {
        qspInitLineOfCode(&iteratorLine, iterator, 0);
        ++lineInd;
        while (1)
        {
            /* Check condition */
            if (codeOffset > 0)
            {
                qspRealLine = line->LineNum + codeOffset;
                if (qspIsDebug)
                {
                    qspCallDebug(line->Str);
                    if (qspRefreshCount != oldRefreshCount)
                    {
                        qspReleaseSavedVarsGroup(QSP_TRUE);
                        qspFreeLineOfCode(&iteratorLine);
                        return QSP_FALSE;
                    }
                }
            }
            conditionValue = qspCheckCondition(condition);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
            {
                qspReleaseSavedVarsGroup(QSP_TRUE);
                qspFreeLineOfCode(&iteratorLine);
                return QSP_FALSE;
            }
            if (!conditionValue) break;
            /* Execute body */
            isExit = qspExecCodeBlockWithLocals(s, lineInd, endLine, codeOffset, jumpTo);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
            {
                qspReleaseSavedVarsGroup(QSP_TRUE);
                qspFreeLineOfCode(&iteratorLine);
                return QSP_FALSE;
            }
            if (isExit) break;
            /* Execute iterator */
            if (codeOffset > 0)
            {
                qspRealLine = line->LineNum + codeOffset;
                if (qspIsDebug)
                {
                    qspCallDebug(line->Str);
                    if (qspRefreshCount != oldRefreshCount)
                    {
                        qspReleaseSavedVarsGroup(QSP_TRUE);
                        qspFreeLineOfCode(&iteratorLine);
                        return QSP_FALSE;
                    }
                }
            }
            isExit = qspExecStringWithLocals(&iteratorLine, 0, iteratorLine.StatsCount, jumpTo);
            if (qspRefreshCount != oldRefreshCount || qspErrorNum)
            {
                qspReleaseSavedVarsGroup(QSP_TRUE);
                qspFreeLineOfCode(&iteratorLine);
                return QSP_FALSE;
            }
            if (isExit) break;
        }
        qspFreeLineOfCode(&iteratorLine);
    }
    qspReleaseSavedVarsGroup(QSP_FALSE);
    return isExit;
}

INLINE QSP_BOOL qspStatementImplicitStatement(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    if (QSP_ISDEF(args[0].Type))
    {
        qspConvertVariantTo(args, QSP_TYPE_STRING);
        qspAddText(&qspCurDesc, QSP_STR(args[0]), QSP_FALSE);
        qspAddText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
        qspIsMainDescChanged = QSP_TRUE;
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementAddText(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    switch (extArg)
    {
    case qspStatP:
        if (!qspIsEmpty(QSP_STR(args[0])))
        {
            qspAddText(&qspCurVars, QSP_STR(args[0]), QSP_FALSE);
            qspIsVarsDescChanged = QSP_TRUE;
        }
        break;
    case qspStatMP:
        if (!qspIsEmpty(QSP_STR(args[0])))
        {
            qspAddText(&qspCurDesc, QSP_STR(args[0]), QSP_FALSE);
            qspIsMainDescChanged = QSP_TRUE;
        }
        break;
    case qspStatPL:
        if (count) qspAddText(&qspCurVars, QSP_STR(args[0]), QSP_FALSE);
        qspAddText(&qspCurVars, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
        qspIsVarsDescChanged = QSP_TRUE;
        break;
    case qspStatMPL:
        if (count) qspAddText(&qspCurDesc, QSP_STR(args[0]), QSP_FALSE);
        qspAddText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
        qspIsMainDescChanged = QSP_TRUE;
        break;
    case qspStatNL:
        qspAddText(&qspCurVars, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
        if (count) qspAddText(&qspCurVars, QSP_STR(args[0]), QSP_FALSE);
        qspIsVarsDescChanged = QSP_TRUE;
        break;
    case qspStatMNL:
        qspAddText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
        if (count) qspAddText(&qspCurDesc, QSP_STR(args[0]), QSP_FALSE);
        qspIsMainDescChanged = QSP_TRUE;
        break;
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementClear(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    switch (extArg)
    {
    case qspStatClear:
        if (qspClearText(&qspCurVars))
            qspIsVarsDescChanged = QSP_TRUE;
        break;
    case qspStatMClear:
        if (qspClearText(&qspCurDesc))
            qspIsMainDescChanged = QSP_TRUE;
        break;
    case qspStatCmdClear:
        qspClearText(&qspCurInput);
        qspCallSetInputStrText(qspNullString);
        break;
    case qspStatClA:
        qspClearActions(QSP_FALSE);
        break;
    case qspStatClS:
        if (qspClearText(&qspCurVars))
            qspIsVarsDescChanged = QSP_TRUE;
        if (qspClearText(&qspCurDesc))
            qspIsMainDescChanged = QSP_TRUE;
        qspClearText(&qspCurInput);
        qspClearActions(QSP_FALSE);
        qspCallSetInputStrText(qspNullString);
        break;
    case qspStatKillAll:
        qspClearVars(QSP_FALSE);
        qspClearObjectsWithNotify();
        break;
    case qspStatFreeLib:
        qspClearIncludes(QSP_FALSE);
        if (qspCurLoc >= qspLocsCount) qspCurLoc = -1;
        break;
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementExit(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    return QSP_TRUE;
}

INLINE QSP_BOOL qspStatementGoSub(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    qspExecLocByNameWithArgs(QSP_STR(args[0]), args + 1, count - 1, 0);
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementGoTo(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    int locInd = qspLocIndex(QSP_STR(args[0]));
    if (locInd < 0)
    {
        qspSetError(QSP_ERR_LOCNOTFOUND);
        return QSP_FALSE;
    }
    qspCurLoc = locInd;
    qspRefreshCurLoc(extArg == qspStatGoTo, args + 1, count - 1);
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementJump(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    qspFreeString(*jumpTo);
    *jumpTo = qspGetNewText(qspDelSpc(QSP_STR(args[0])));
    qspUpperStr(jumpTo);
    return QSP_TRUE;
}

INLINE QSP_BOOL qspStatementWait(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    int num = QSP_NUM(args[0]);
    qspCallRefreshInt(QSP_TRUE);
    if (num < 0) num = 0;
    qspCallSleep(num);
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementSetTimer(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    int num = QSP_NUM(args[0]);
    if (num < 0) num = 0;
    qspTimerInterval = num;
    qspCallSetTimer(num);
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementShowWin(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    QSP_BOOL val = QSP_NUM(args[0]) != 0;
    switch (extArg)
    {
    case qspStatShowActs:
        qspCallShowWindow(QSP_WIN_ACTS, qspCurIsShowActs = val);
        break;
    case qspStatShowObjs:
        qspCallShowWindow(QSP_WIN_OBJS, qspCurIsShowObjs = val);
        break;
    case qspStatShowVars:
        qspCallShowWindow(QSP_WIN_VARS, qspCurIsShowVars = val);
        break;
    case qspStatShowInput:
        qspCallShowWindow(QSP_WIN_INPUT, qspCurIsShowInput = val);
        break;
    }
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementRefInt(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    qspCallRefreshInt(QSP_TRUE);
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementView(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
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
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementMsg(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    qspCallShowMessage(QSP_STR(args[0]));
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementExec(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    qspCallSystem(QSP_STR(args[0]));
    return QSP_FALSE;
}

INLINE QSP_BOOL qspStatementDynamic(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
    qspExecStringAsCodeWithArgs(QSP_STR(args[0]), args + 1, count - 1, 0);
    return QSP_FALSE;
}
