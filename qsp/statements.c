/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
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

static void qspAddStatement(int, int, QSP_STATEMENT, int, int, ...);
static void qspAddStatName(int statCode, QSPString statName, int level);
static int qspStatsCompare(const void *, const void *);
static int qspStatStringCompare(const void *, const void *);
static int qspGetStatCode(QSPString s, QSP_CHAR **pos);
static int qspSearchElse(QSPLineOfCode *, int, int);
static int qspSearchEnd(QSPLineOfCode *, int, int);
static int qspSearchLabel(QSPLineOfCode *s, int start, int end, QSPString str);
static QSP_BOOL qspExecString(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo);
static QSP_BOOL qspExecMultilineCode(QSPLineOfCode *s, int endLine, int codeOffset, QSPString *jumpTo, int *lineInd, int *action);
static QSP_BOOL qspExecSinglelineCode(QSPLineOfCode *s, int endLine, int codeOffset, QSPString *jumpTo, int *lineInd, int *action);
static QSP_BOOL qspExecCode(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSPString *jumpTo);
static QSP_BOOL qspExecCodeBlockWithLocals(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSPString *jumpTo);
static QSP_BOOL qspExecStringWithLocals(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo);
static QSP_BOOL qspStatementIf(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo);
static QSPString qspPrepareForLoop(QSPString params, QSPVar *local, QSPString *toValuePos, QSPString *stepValuePos);
static QSP_BOOL qspCheckForLoop(QSPString varName, QSPString toValuePos, QSPString stepValuePos, int *curStep);
static void qspEndForLoop(QSPString varName, int curStep);
static QSP_BOOL qspStatementSinglelineFor(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo);
static QSP_BOOL qspStatementMultilineFor(QSPLineOfCode *s, int endLine, int lineInd, int codeOffset, QSPString *jumpTo);
static QSP_BOOL qspStatementAddText(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementClear(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementExit(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementGoSub(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementGoTo(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementJump(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementWait(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementSetTimer(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementShowWin(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementRefInt(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementView(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementMsg(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementExec(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
static QSP_BOOL qspStatementDynamic(QSPVariant *args, int count, QSPString *jumpTo, int extArg);

static void qspAddStatement(int statCode, int extArg, QSP_STATEMENT func, int minArgs, int maxArgs, ...)
{
	int i;
	va_list marker;
	qspStats[statCode].ExtArg = extArg;
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

static void qspAddStatName(int statCode, QSPString statName, int level)
{
	int count, len = qspStrLen(statName);
	count = qspStatsNamesCounts[level];
	qspStatsNames[level][count].Name = statName;
	qspStatsNames[level][count].Code = statCode;
	qspStatsNamesCounts[level] = count + 1;
	/* Max length */
	if (len > qspStatMaxLen) qspStatMaxLen = len;
}

static int qspStatsCompare(const void *statName1, const void *statName2)
{
	return qspStrsComp(((QSPStatName *)statName1)->Name, ((QSPStatName *)statName2)->Name);
}

static int qspStatStringCompare(const void *name, const void *compareTo)
{
	QSPStatName *statName = (QSPStatName *)compareTo;
	return qspStrsNComp(*(QSPString *)name, statName->Name, qspStrLen(statName->Name));
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
		0 - Unknown / Any
		1 - String
		2 - Number
	*/
	int i;
	for (i = 0; i < QSP_STATSLEVELS; ++i) qspStatsNamesCounts[i] = 0;
	qspStatMaxLen = 0;
	qspAddStatement(qspStatElse, 0, 0, 0, 0);
	qspAddStatement(qspStatElseIf, 0, 0, 1, 1, 2);
	qspAddStatement(qspStatEnd, 0, 0, 0, 0);
	qspAddStatement(qspStatLocal, 0, 0, 0, 0);
	qspAddStatement(qspStatSet, 0, 0, 0, 0);
	qspAddStatement(qspStatIf, 0, 0, 1, 1, 2);
	qspAddStatement(qspStatAct, 0, 0, 1, 2, 1, 1);
	qspAddStatement(qspStatFor, 0, 0, 0, 0);
	qspAddStatement(qspStatAddObj, 0, qspStatementAddObject, 1, 3, 1, 1, 2);
	qspAddStatement(qspStatClA, 3, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCloseAll, 1, qspStatementCloseFile, 0, 0);
	qspAddStatement(qspStatClose, 0, qspStatementCloseFile, 0, 1, 1);
	qspAddStatement(qspStatClS, 4, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCmdClear, 2, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCopyArr, 0, qspStatementCopyArr, 2, 4, 1, 1, 2, 2);
	qspAddStatement(qspStatDelAct, 0, qspStatementDelAct, 1, 1, 1);
	qspAddStatement(qspStatDelObj, 0, qspStatementDelObj, 1, 1, 1);
	qspAddStatement(qspStatDynamic, 0, qspStatementDynamic, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddStatement(qspStatExec, 0, qspStatementExec, 1, 1, 1);
	qspAddStatement(qspStatExit, 0, qspStatementExit, 0, 0);
	qspAddStatement(qspStatFreeLib, 6, qspStatementClear, 0, 0);
	qspAddStatement(qspStatGoSub, 0, qspStatementGoSub, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddStatement(qspStatGoTo, 1, qspStatementGoTo, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddStatement(qspStatIncLib, 1, qspStatementOpenQst, 1, 1, 1);
	qspAddStatement(qspStatJump, 0, qspStatementJump, 1, 1, 1);
	qspAddStatement(qspStatKillAll, 5, qspStatementClear, 0, 0);
	qspAddStatement(qspStatKillObj, 1, qspStatementDelObj, 0, 1, 2);
	qspAddStatement(qspStatKillVar, 0, qspStatementKillVar, 0, 2, 1, 2);
	qspAddStatement(qspStatMenu, 0, qspStatementShowMenu, 1, 3, 1, 2, 2);
	qspAddStatement(qspStatMClear, 1, qspStatementClear, 0, 0);
	qspAddStatement(qspStatMNL, 5, qspStatementAddText, 0, 1, 1);
	qspAddStatement(qspStatMPL, 3, qspStatementAddText, 0, 1, 1);
	qspAddStatement(qspStatMP, 1, qspStatementAddText, 1, 1, 1);
	qspAddStatement(qspStatClear, 0, qspStatementClear, 0, 0);
	qspAddStatement(qspStatNL, 4, qspStatementAddText, 0, 1, 1);
	qspAddStatement(qspStatPL, 2, qspStatementAddText, 0, 1, 1);
	qspAddStatement(qspStatP, 0, qspStatementAddText, 1, 1, 1);
	qspAddStatement(qspStatMsg, 0, qspStatementMsg, 1, 1, 1);
	qspAddStatement(qspStatOpenGame, 0, qspStatementOpenGame, 0, 1, 1);
	qspAddStatement(qspStatOpenQst, 0, qspStatementOpenQst, 1, 1, 1);
	qspAddStatement(qspStatPlay, 0, qspStatementPlayFile, 1, 2, 1, 2);
	qspAddStatement(qspStatRefInt, 0, qspStatementRefInt, 0, 0);
	qspAddStatement(qspStatSaveGame, 0, qspStatementSaveGame, 0, 1, 1);
	qspAddStatement(qspStatSetTimer, 0, qspStatementSetTimer, 1, 1, 2);
	qspAddStatement(qspStatShowActs, 0, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatShowInput, 3, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatShowObjs, 1, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatShowVars, 2, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatUnSelect, 0, qspStatementUnSelect, 0, 0);
	qspAddStatement(qspStatView, 0, qspStatementView, 0, 1, 1);
	qspAddStatement(qspStatWait, 0, qspStatementWait, 1, 1, 2);
	qspAddStatement(qspStatXGoTo, 0, qspStatementGoTo, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	/* Names */
	qspAddStatName(qspStatElse, QSP_STATIC_STR(QSP_STATELSE), 2);
	qspAddStatName(qspStatElseIf, QSP_STATIC_STR(QSP_FMT("ELSEIF")), 1);
	qspAddStatName(qspStatEnd, QSP_STATIC_STR(QSP_FMT("END")), 2);
	qspAddStatName(qspStatLocal, QSP_STATIC_STR(QSP_FMT("LOCAL")), 2);
	qspAddStatName(qspStatSet, QSP_STATIC_STR(QSP_FMT("SET")), 2);
	qspAddStatName(qspStatSet, QSP_STATIC_STR(QSP_FMT("LET")), 2);
	qspAddStatName(qspStatIf, QSP_STATIC_STR(QSP_FMT("IF")), 2);
	qspAddStatName(qspStatAct, QSP_STATIC_STR(QSP_FMT("ACT")), 2);
	qspAddStatName(qspStatFor, QSP_STATIC_STR(QSP_FMT("FOR")), 2);
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

static int qspGetStatCode(QSPString s, QSP_CHAR **pos)
{
	int i, nameLen;
	QSPString uStr;
	QSPStatName *name;
	if (qspIsEmpty(s)) return qspStatUnknown;
	if (*s.Str == QSP_LABEL[0]) return qspStatLabel;
	if (*s.Str == QSP_COMMENT[0]) return qspStatComment;
	/* ------------------------------------------------------------------ */
	uStr = qspGetNewText(qspStringFromString(s, qspStatMaxLen));
	qspUpperStr(&uStr);
	for (i = 0; i < QSP_STATSLEVELS; ++i)
	{
		name = (QSPStatName *)bsearch(&uStr, qspStatsNames[i], qspStatsNamesCounts[i], sizeof(QSPStatName), qspStatStringCompare);
		if (name)
		{
			nameLen = qspStrLen(name->Name);
			if (nameLen == qspStrLen(s) || qspIsInList(QSP_DELIMS, s.Str[nameLen]))
			{
				*pos = s.Str + nameLen;
				free(uStr.Str);
				return name->Code;
			}
		}
	}
	free(uStr.Str);
	return qspStatUnknown;
}

static int qspSearchElse(QSPLineOfCode *s, int start, int end)
{
	int c = 1;
	s += start;
	while (start < end)
	{
		switch (s->Stats->Stat)
		{
		case qspStatAct:
		case qspStatFor:
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

static int qspSearchEnd(QSPLineOfCode *s, int start, int end)
{
	int c = 1;
	s += start;
	while (start < end)
	{
		switch (s->Stats->Stat)
		{
		case qspStatAct:
		case qspStatFor:
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

static int qspSearchLabel(QSPLineOfCode *s, int start, int end, QSPString str)
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

int qspGetStatArgs(QSPString s, int statCode, QSPVariant *args)
{
	int type;
	int oldRefreshCount, count = 0;
	QSP_CHAR *pos, *brack;
	qspSkipSpaces(&s);
	if (*s.Str == QSP_LRBRACK[0])
	{
		if (!(brack = qspStrPos(s, QSP_STATIC_STR(QSP_RRBRACK), QSP_FALSE)))
		{
			qspSetError(QSP_ERR_BRACKNOTFOUND);
			return 0;
		}
		if (!qspIsAnyString(qspStringFromPair(brack + QSP_STATIC_LEN(QSP_RRBRACK), s.End)))
		{
			s = qspStringFromPair(s.Str + QSP_STATIC_LEN(QSP_LRBRACK), brack);
			qspSkipSpaces(&s);
		}
	}
	if (!qspIsEmpty(s))
	{
		oldRefreshCount = qspRefreshCount;
		while (1)
		{
			if (count == qspStats[statCode].MaxArgsCount)
			{
				qspSetError(QSP_ERR_ARGSCOUNT);
				break;
			}
			pos = qspStrPos(s, QSP_STATIC_STR(QSP_COMMA), QSP_FALSE);
			if (pos)
				args[count] = qspExprValue(qspStringFromPair(s.Str, pos));
			else
				args[count] = qspExprValue(s);
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
			type = qspStats[statCode].ArgsTypes[count];
			if (type && qspConvertVariantTo(args + count, type == 1))
			{
				qspSetError(QSP_ERR_TYPEMISMATCH);
				++count;
				break;
			}
			++count;
			if (!pos) break;
			s.Str = pos + QSP_STATIC_LEN(QSP_COMMA);
			qspSkipSpaces(&s);
			if (qspIsEmpty(s))
			{
				qspSetError(QSP_ERR_SYNTAX);
				break;
			}
		}
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			qspFreeVariants(args, count);
			return 0;
		}
	}
	if (count < qspStats[statCode].MinArgsCount)
	{
		qspSetError(QSP_ERR_ARGSCOUNT);
		qspFreeVariants(args, count);
		return 0;
	}
	return count;
}

static QSP_BOOL qspExecString(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo)
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
		case qspStatFor:
			return qspStatementSinglelineFor(s, i, endStat, jumpTo);
		case qspStatLocal:
			if (i < s->StatsCount - 1)
				qspStatementLocal(qspStringFromPair(s->Str.Str + s->Stats[i].ParamPos, s->Str.Str + s->Stats[i].EndPos));
			else
				qspStatementLocal(qspStringFromPair(s->Str.Str + s->Stats[i].ParamPos, s->Str.End));
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
			break;
		case qspStatSet:
			if (i < s->StatsCount - 1)
				qspStatementSetVarValue(qspStringFromPair(s->Str.Str + s->Stats[i].ParamPos, s->Str.Str + s->Stats[i].EndPos));
			else
				qspStatementSetVarValue(qspStringFromPair(s->Str.Str + s->Stats[i].ParamPos, s->Str.End));
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
			break;
		default:
			if (i < s->StatsCount - 1)
				count = qspGetStatArgs(qspStringFromPair(s->Str.Str + s->Stats[i].ParamPos, s->Str.Str + s->Stats[i].EndPos), statCode, args);
			else
				count = qspGetStatArgs(qspStringFromPair(s->Str.Str + s->Stats[i].ParamPos, s->Str.End), statCode, args);
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
			isExit = qspStats[statCode].Func(args, count, jumpTo, qspStats[statCode].ExtArg);
			qspFreeVariants(args, count);
			if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum || (jumpTo->Str != jumpTo->End)) return isExit;
			break;
		}
	}
	return QSP_FALSE;
}

static QSP_BOOL qspExecMultilineCode(QSPLineOfCode *s, int endLine, int codeOffset,
	QSPString *jumpTo, int *lineInd, int *action)
{
	QSPVariant arg;
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
		qspGetStatArgs(qspStringFromPair(line->Str.Str + line->Stats->ParamPos, line->Str.Str + line->Stats->EndPos), statCode, &arg);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
		elsePos = qspSearchElse(s, ind + 1, endLine);
		if (QSP_NUM(arg))
		{
			*lineInd = endLine;
			*action = qspFlowSkip;
			if (elsePos >= 0)
				return qspExecCodeBlockWithLocals(s, ind + 1, elsePos, codeOffset, jumpTo);
			return qspExecCodeBlockWithLocals(s, ind + 1, endLine, codeOffset, jumpTo);
		}
		else
		{
			*lineInd = (elsePos >= 0 ? elsePos : endLine);
			*action = qspFlowContinue;
		}
		break;
	case qspStatAct:
		*lineInd = endLine;
		*action = qspFlowContinue;
		qspStatementMultilineAddAct(s, endLine, ind, codeOffset > 0);
		break;
	case qspStatFor:
		*lineInd = endLine;
		*action = qspFlowSkip;
		return qspStatementMultilineFor(s, endLine, ind, codeOffset, jumpTo);
	}
	return QSP_FALSE;
}

static QSP_BOOL qspExecSinglelineCode(QSPLineOfCode *s, int endLine, int codeOffset,
	QSPString *jumpTo, int *lineInd, int *action)
{
	QSPVariant arg;
	QSPLineOfCode *line;
	QSP_CHAR *pos;
	int ind, elsePos, oldRefreshCount;
	ind = *lineInd;
	line = s + ind;
	switch (line->Stats->Stat)
	{
	case qspStatElseIf:
		pos = line->Str.Str + line->Stats->EndPos;
		if (*pos != QSP_COLONDELIM[0])
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
		qspGetStatArgs(qspStringFromPair(line->Str.Str + line->Stats->ParamPos, pos), qspStatElseIf, &arg);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
		if (QSP_NUM(arg))
		{
			*lineInd = endLine;
			*action = qspFlowSkip;
			return qspExecStringWithLocals(line, 1, line->StatsCount, jumpTo);
		}
		else
		{
			elsePos = qspSearchElse(s, ind + 1, endLine);
			*lineInd = (elsePos >= 0 ? elsePos : endLine);
			*action = qspFlowContinue;
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
		*action = qspFlowSkip;
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

static QSP_BOOL qspExecCode(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSPString *jumpTo)
{
	QSPLineOfCode *line;
	QSPString jumpToFake;
	QSP_BOOL uLevel, isExit = QSP_FALSE;
	int i, oldRefreshCount, action = qspFlowExecute;
	oldRefreshCount = qspRefreshCount;
	/* Prepare temporary data */
	if (uLevel = !jumpTo)
	{
		jumpToFake = qspNewEmptyString();
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
			if (qspIsDebug && *line->Str.Str)
			{
				qspCallDebug(line->Str);
				if (qspRefreshCount != oldRefreshCount) break;
			}
		}
		if (line->IsMultiline)
			isExit = qspExecMultilineCode(s, endLine, codeOffset, jumpTo, &i, &action);
		else
			isExit = qspExecSinglelineCode(s, endLine, codeOffset, jumpTo, &i, &action);
		if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum) break;
		if (action == qspFlowContinue)
		{
			action = qspFlowExecute;
			continue;
		}
		if (action == qspFlowSkip)
			action = qspFlowExecute;
		else
		{
			isExit = qspExecString(line, 0, line->StatsCount, jumpTo);
			if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum) break;
			++i;
		}
		if (jumpTo->Str != jumpTo->End)
		{
			i = qspSearchLabel(s, startLine, endLine, *jumpTo);
			if (i < 0)
			{
				if (uLevel) qspSetError(QSP_ERR_LABELNOTFOUND);
				break;
			}
			*jumpTo = qspNullString;
		}
	}
	if (uLevel) qspFreeString(jumpToFake);
	return isExit;
}

static QSP_BOOL qspExecCodeBlockWithLocals(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSPString *jumpTo)
{
	QSP_BOOL isExit;
	int oldRefreshCount, ind = qspSavedVarsGroupsCount;
	qspSavedVarsGroups = (QSPVarsGroup *)realloc(qspSavedVarsGroups, (ind + 1) * sizeof(QSPVarsGroup));
	qspSavedVarsGroups[ind].Vars = 0;
	qspSavedVarsGroups[ind].VarsCount = 0;
	++qspSavedVarsGroupsCount;
	oldRefreshCount = qspRefreshCount;
	isExit = qspExecCode(s, startLine, endLine, codeOffset, jumpTo);
	if (oldRefreshCount != qspRefreshCount || qspErrorNum)
	{
		if (qspSavedVarsGroupsCount)
		{
			--qspSavedVarsGroupsCount;
			qspClearVarsList(qspSavedVarsGroups[ind].Vars, qspSavedVarsGroups[ind].VarsCount);
		}
		return QSP_FALSE;
	}
	--qspSavedVarsGroupsCount;
	qspRestoreVarsList(qspSavedVarsGroups[ind].Vars, qspSavedVarsGroups[ind].VarsCount);
	return isExit;
}

static QSP_BOOL qspExecStringWithLocals(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo)
{
	QSP_BOOL isExit;
	int oldRefreshCount, ind = qspSavedVarsGroupsCount;
	qspSavedVarsGroups = (QSPVarsGroup *)realloc(qspSavedVarsGroups, (ind + 1) * sizeof(QSPVarsGroup));
	qspSavedVarsGroups[ind].Vars = 0;
	qspSavedVarsGroups[ind].VarsCount = 0;
	++qspSavedVarsGroupsCount;
	oldRefreshCount = qspRefreshCount;
	isExit = qspExecString(s, startStat, endStat, jumpTo);
	if (oldRefreshCount != qspRefreshCount || qspErrorNum)
	{
		if (qspSavedVarsGroupsCount)
		{
			--qspSavedVarsGroupsCount;
			qspClearVarsList(qspSavedVarsGroups[ind].Vars, qspSavedVarsGroups[ind].VarsCount);
		}
		return QSP_FALSE;
	}
	--qspSavedVarsGroupsCount;
	qspRestoreVarsList(qspSavedVarsGroups[ind].Vars, qspSavedVarsGroups[ind].VarsCount);
	return isExit;
}

QSP_BOOL qspExecTopCodeWithLocals(QSPLineOfCode *s, int endLine, int codeOffset, QSP_BOOL isNewLoc)
{
	QSP_BOOL isExit;
	QSPVar *savedVars;
	QSPVarsGroup *savedGroups;
	int oldRefreshCount, varsCount, groupsCount;
	if (isNewLoc)
		qspPrepareGlobalVars();
	else
		varsCount = qspPrepareLocalVars(&savedVars);
	if (qspErrorNum) return QSP_FALSE;
	groupsCount = qspSavedVarsGroupsCount;
	savedGroups = qspSavedVarsGroups;
	qspSavedVarsGroupsCount = 1;
	qspSavedVarsGroups = (QSPVarsGroup *)malloc(sizeof(QSPVarsGroup));
	qspSavedVarsGroups[0].Vars = 0;
	qspSavedVarsGroups[0].VarsCount = 0;
	oldRefreshCount = qspRefreshCount;
	isExit = qspExecCode(s, 0, endLine, codeOffset, 0);
	if (oldRefreshCount != qspRefreshCount || qspErrorNum)
	{
		if (qspSavedVarsGroupsCount)
			qspClearVarsList(qspSavedVarsGroups[0].Vars, qspSavedVarsGroups[0].VarsCount);
		if (!isNewLoc)
			qspClearLocalVars(savedVars, varsCount);
	}
	else
	{
		qspRestoreVarsList(qspSavedVarsGroups[0].Vars, qspSavedVarsGroups[0].VarsCount);
		if (!isNewLoc)
		{
			if (qspErrorNum)
				qspClearLocalVars(savedVars, varsCount);
			else
				qspRestoreLocalVars(savedVars, varsCount, savedGroups, groupsCount);
		}
	}
	if (qspSavedVarsGroups) free(qspSavedVarsGroups);
	qspSavedVarsGroups = savedGroups;
	qspSavedVarsGroupsCount = groupsCount;
	return isExit;
}

void qspExecStringAsCodeWithArgs(QSPString s, QSPVariant *args, int count, QSPVariant *res)
{
	QSPLineOfCode *strs;
	QSPString resultName, argsName;
	QSPVar local, result, *varArgs, *varRes;
	int oldRefreshCount;
	resultName = QSP_STATIC_STR(QSP_VARRES);
	if (!(varRes = qspVarReference(resultName, QSP_TRUE))) return;
	argsName = QSP_STATIC_STR(QSP_VARARGS);
	if (!(varArgs = qspVarReference(argsName, QSP_TRUE))) return;
	qspMoveVar(&result, varRes);
	qspMoveVar(&local, varArgs);
	qspSetArgs(varArgs, args, count);
	count = qspPreprocessData(s, &strs);
	oldRefreshCount = qspRefreshCount;
	qspExecCodeBlockWithLocals(strs, 0, count, 0, 0);
	qspFreePrepLines(strs, count);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
	{
		qspEmptyVar(&local);
		qspEmptyVar(&result);
		return;
	}
	if (!(varArgs = qspVarReference(argsName, QSP_TRUE)))
	{
		qspEmptyVar(&local);
		qspEmptyVar(&result);
		return;
	}
	qspEmptyVar(varArgs);
	qspMoveVar(varArgs, &local);
	if (!(varRes = qspVarReference(resultName, QSP_TRUE)))
	{
		qspEmptyVar(&result);
		return;
	}
	if (res) qspApplyResult(varRes, res);
	qspEmptyVar(varRes);
	qspMoveVar(varRes, &result);
}

QSPString qspGetLineLabel(QSPString str)
{
	QSP_CHAR *delimPos;
	qspSkipSpaces(&str);
	if (!qspIsEmpty(str) && *str.Str == QSP_LABEL[0])
	{
		delimPos = qspStrChar(str, QSP_STATDELIM[0]);
		if (delimPos)
			str = qspStringFromPair(str.Str + QSP_STATIC_LEN(QSP_STATDELIM), delimPos);
		else
			str = qspStringFromPair(str.Str + QSP_STATIC_LEN(QSP_STATDELIM), str.End);
		str = qspDelSpc(str);
		qspUpperStr(&str);
		return str;
	}
	return qspNullString;
}

void qspInitLineOfCode(QSPLineOfCode *line, QSPString str, int lineNum)
{
	QSPString uStr, nextPos, tempStr;
	QSP_BOOL isInLoop, isSearchElse;
	int statCode, count = 0;
	QSP_CHAR *temp, *elsePos, *delimPos = 0, *paramPos = 0;
	line->Str = str;
	line->LineNum = lineNum;
	line->StatsCount = 0;
	line->Stats = 0;
	qspSkipSpaces(&str);
	statCode = qspGetStatCode(str, &paramPos);
	if (!qspIsEmpty(str) && statCode != qspStatComment)
	{
		isInLoop = isSearchElse = QSP_TRUE;
		elsePos = 0;
		uStr = qspGetNewText(line->Str);
		qspUpperStr(&uStr);
		switch (statCode)
		{
		case qspStatAct:
		case qspStatFor:
		case qspStatIf:
		case qspStatElseIf:
			delimPos = qspStrPos(str, QSP_STATIC_STR(QSP_COLONDELIM), QSP_FALSE);
			if (delimPos)
			{
				nextPos = qspStringFromPair(delimPos + QSP_STATIC_LEN(QSP_COLONDELIM), str.End);
				if (qspIsEmpty(nextPos)) isInLoop = QSP_FALSE;
			}
			break;
		case qspStatElse:
			nextPos = qspStringFromPair(paramPos, str.End);
			qspSkipSpaces(&nextPos);
			if (!qspIsEmpty(nextPos) && *nextPos.Str == QSP_COLONDELIM[0]) ++nextPos.Str;
			delimPos = (!qspIsEmpty(nextPos) ? nextPos.Str : 0);
			break;
		default:
			delimPos = qspStrPos(str, QSP_STATIC_STR(QSP_STATDELIM), QSP_FALSE);
			if (delimPos) nextPos = qspStringFromPair(delimPos + QSP_STATIC_LEN(QSP_STATDELIM), str.End);
			tempStr = qspStringFromPair(uStr.Str + (int)(str.Str - line->Str.Str), uStr.End);
			elsePos = qspStrPos(tempStr, QSP_STATIC_STR(QSP_STATELSE), QSP_TRUE);
			if (elsePos)
			{
				elsePos = line->Str.Str + (elsePos - uStr.Str);
				if (!delimPos || elsePos < delimPos)
				{
					delimPos = elsePos;
					nextPos = qspStringFromPair(delimPos, str.End);
					elsePos = 0;
				}
			}
			else
				isSearchElse = QSP_FALSE;
			if (statCode == qspStatUnknown && str.Str != delimPos)
			{
				if (delimPos)
					temp = qspStrPos(qspStringFromPair(str.Str, delimPos), QSP_STATIC_STR(QSP_EQUAL), QSP_FALSE);
				else
					temp = qspStrPos(str, QSP_STATIC_STR(QSP_EQUAL), QSP_FALSE);
				statCode = (temp ? qspStatSet : qspStatMPL);
			}
			break;
		}
		while (delimPos && isInLoop)
		{
			line->StatsCount++;
			line->Stats = (QSPCachedStat *)realloc(line->Stats, line->StatsCount * sizeof(QSPCachedStat));
			line->Stats[count].Stat = statCode;
			line->Stats[count].EndPos = (int)(delimPos - line->Str.Str);
			if (paramPos)
			{
				tempStr = qspStringFromPair(paramPos, str.End);
				qspSkipSpaces(&tempStr);
				line->Stats[count].ParamPos = (int)(tempStr.Str - line->Str.Str);
			}
			else
				line->Stats[count].ParamPos = (int)(str.Str - line->Str.Str);
			++count;
			str = nextPos;
			qspSkipSpaces(&str);
			paramPos = 0;
			statCode = qspGetStatCode(str, &paramPos);
			if (!qspIsEmpty(str) && statCode != qspStatComment)
			{
				switch (statCode)
				{
				case qspStatAct:
				case qspStatFor:
				case qspStatIf:
				case qspStatElseIf:
					delimPos = qspStrPos(str, QSP_STATIC_STR(QSP_COLONDELIM), QSP_FALSE);
					if (delimPos)
					{
						nextPos = qspStringFromPair(delimPos + QSP_STATIC_LEN(QSP_COLONDELIM), str.End);
						if (qspIsEmpty(nextPos)) isInLoop = QSP_FALSE;
					}
					break;
				case qspStatElse:
					nextPos = qspStringFromPair(paramPos, str.End);
					qspSkipSpaces(&nextPos);
					if (!qspIsEmpty(nextPos) && *nextPos.Str == QSP_COLONDELIM[0]) ++nextPos.Str;
					delimPos = (!qspIsEmpty(nextPos) ? nextPos.Str : 0);
					break;
				default:
					delimPos = qspStrPos(str, QSP_STATIC_STR(QSP_STATDELIM), QSP_FALSE);
					if (delimPos) nextPos = qspStringFromPair(delimPos + QSP_STATIC_LEN(QSP_STATDELIM), str.End);
					if (elsePos && str.Str >= elsePos) elsePos = 0;
					if (!elsePos && isSearchElse)
					{
						tempStr = qspStringFromPair(uStr.Str + (int)(str.Str - line->Str.Str), uStr.End);
						elsePos = qspStrPos(tempStr, QSP_STATIC_STR(QSP_STATELSE), QSP_TRUE);
						if (elsePos)
							elsePos = line->Str.Str + (elsePos - uStr.Str);
						else
							isSearchElse = QSP_FALSE;
					}
					if (elsePos && (!delimPos || elsePos < delimPos))
					{
						delimPos = elsePos;
						nextPos = qspStringFromPair(delimPos, str.End);
						elsePos = 0;
					}
					if (statCode == qspStatUnknown && str.Str != delimPos)
					{
						temp = qspStrPos(qspStringFromPair(str.Str, delimPos), QSP_STATIC_STR(QSP_EQUAL), QSP_FALSE);
						statCode = (temp ? qspStatSet : qspStatMPL);
					}
					break;
				}
			}
			else
				delimPos = 0;
		}
		qspFreeString(uStr);
	}
	/* Check for ELSE IF */
	if (count == 1 && delimPos && line->Stats->Stat == qspStatElse && statCode == qspStatIf &&
		*(line->Str.Str + line->Stats->ParamPos) != QSP_COLONDELIM[0])
	{
		count = 0;
		statCode = qspStatElseIf;
	}
	else
	{
		line->StatsCount++;
		line->Stats = (QSPCachedStat *)realloc(line->Stats, line->StatsCount * sizeof(QSPCachedStat));
	}
	line->Stats[count].Stat = statCode;
	if (delimPos)
		line->Stats[count].EndPos = (int)(delimPos - line->Str.Str);
	else
		line->Stats[count].EndPos = (int)(str.End - line->Str.Str);
	if (paramPos)
	{
		tempStr = qspStringFromPair(paramPos, str.End);
		qspSkipSpaces(&tempStr);
		line->Stats[count].ParamPos = (int)(tempStr.Str - line->Str.Str);
	}
	else
		line->Stats[count].ParamPos = (int)(str.Str - line->Str.Str);
	switch (line->Stats->Stat)
	{
	case qspStatAct:
	case qspStatFor:
	case qspStatIf:
	case qspStatElseIf:
		line->IsMultiline = (line->StatsCount == 1 && *(line->Str.Str + line->Stats->EndPos) == QSP_COLONDELIM[0]);
		break;
	default:
		line->IsMultiline = QSP_FALSE;
		break;
	}
	line->Label = qspGetLineLabel(line->Str);
}

static QSP_BOOL qspStatementIf(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo)
{
	QSPVariant arg;
	int i, c, elseStat, oldRefreshCount;
	QSP_CHAR *pos = s->Str.Str + s->Stats[startStat].EndPos;
	if (*pos != QSP_COLONDELIM[0])
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
	qspGetStatArgs(qspStringFromPair(s->Str.Str + s->Stats[startStat].ParamPos, pos), qspStatIf, &arg);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
	if (QSP_NUM(arg))
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

static QSPString qspPrepareForLoop(QSPString params, QSPVar *local, QSPString *toValuePos, QSPString *stepValuePos)
{
	QSPVar *var;
	int oldRefreshCount;
	QSPVariant initValue;
	QSPString uStr, varName;
	QSP_CHAR *eqPos, *toPos, *stepPos;
	eqPos = qspStrPos(params, QSP_STATIC_STR(QSP_EQUAL), QSP_FALSE);
	if (!eqPos)
	{
		qspSetError(QSP_ERR_EQNOTFOUND);
		return qspNullString;
	}
	varName = qspDelSpc(qspStringFromPair(params.Str, eqPos));
	if (qspStrChar(varName, QSP_LSBRACK[0]))
	{
		qspSetError(QSP_ERR_NOTCORRECTNAME);
		return qspNullString;
	}
	uStr = qspGetNewText(qspStringFromPair(eqPos, params.End));
	qspUpperStr(&uStr);
	toPos = qspStrPos(qspStringFromPair(uStr.Str + QSP_STATIC_LEN(QSP_EQUAL), uStr.End), QSP_STATIC_STR(QSP_STATFORTO), QSP_TRUE);
	if (!toPos)
	{
		qspSetError(QSP_ERR_TONOTFOUND);
		qspFreeString(uStr);
		return qspNullString;
	}
	stepPos = qspStrPos(qspStringFromPair(toPos + QSP_STATIC_LEN(QSP_STATFORTO), uStr.End), QSP_STATIC_STR(QSP_STATFORSTEP), QSP_TRUE);
	qspFreeString(uStr);
	// Find real positions
	if (stepPos) stepPos = eqPos + (stepPos - uStr.Str);
	toPos = eqPos + (toPos - uStr.Str);
	// Calculate initial value of the counter
	oldRefreshCount = qspRefreshCount;
	initValue = qspExprValue(qspStringFromPair(eqPos + QSP_STATIC_LEN(QSP_EQUAL), toPos));
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		return qspNullString;
	if (qspConvertVariantTo(&initValue, QSP_FALSE))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		qspFreeString(QSP_STR(initValue));
		return qspNullString;
	}
	// Set initial value
	if (!(var = qspVarReference(varName, QSP_TRUE)))
		return qspNullString;
	qspMoveVar(local, var);
	qspConvertVariantTo(&initValue, (*varName.Str == QSP_STRCHAR[0]));
	qspSetVarValueByReference(var, 0, &initValue);
	if (initValue.IsStr) qspFreeString(QSP_STR(initValue));
	// Set positions of the loop conditions
	if (stepPos)
	{
		*toValuePos = qspStringFromPair(toPos + QSP_STATIC_LEN(QSP_STATFORTO), stepPos);
		*stepValuePos = qspStringFromPair(stepPos + QSP_STATIC_LEN(QSP_STATFORSTEP), params.End);
	}
	else
	{
		*toValuePos = qspStringFromPair(toPos + QSP_STATIC_LEN(QSP_STATFORTO), params.End);
		*stepValuePos = qspNullString;
	}
	return varName;
}

static QSP_BOOL qspCheckForLoop(QSPString varName, QSPString toValuePos, QSPString stepValuePos, int *curStep)
{
	QSPVar *var;
	QSPVariant toValue, stepValue, curValue;
	int oldRefreshCount = qspRefreshCount;
	// Evaluate current step
	if (stepValuePos.Str)
	{
		stepValue = qspExprValue(stepValuePos);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
		if (qspConvertVariantTo(&stepValue, QSP_FALSE))
		{
			qspSetError(QSP_ERR_TYPEMISMATCH);
			qspFreeString(QSP_STR(stepValue));
			return QSP_FALSE;
		}
		*curStep = QSP_NUM(stepValue);
	}
	// Evaluate termination value
	toValue = qspExprValue(toValuePos);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
	if (qspConvertVariantTo(&toValue, QSP_FALSE))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		qspFreeString(QSP_STR(toValue));
		return QSP_FALSE;
	}
	// Get value of the counter
	if (!(var = qspVarReference(varName, QSP_TRUE))) return QSP_FALSE;
	curValue = qspGetVarValueByReference(var, 0, (*varName.Str == QSP_STRCHAR[0]));
	if (qspConvertVariantTo(&curValue, QSP_FALSE))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		qspFreeString(QSP_STR(curValue));
		return QSP_FALSE;
	}
	// Get state of the loop
	if (*curStep >= 0)
	{
		if (QSP_NUM(curValue) > QSP_NUM(toValue)) return QSP_TRUE;
	}
	else
	{
		if (QSP_NUM(curValue) < QSP_NUM(toValue)) return QSP_TRUE;
	}
	return QSP_FALSE;
}

static void qspEndForLoop(QSPString varName, int curStep)
{
	QSPVar *var;
	QSPVariant curValue;
	// Get the counter
	if (!(var = qspVarReference(varName, QSP_TRUE))) return;
	curValue = qspGetVarValueByReference(var, 0, (*varName.Str == QSP_STRCHAR[0]));
	if (qspConvertVariantTo(&curValue, QSP_FALSE))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		qspFreeString(QSP_STR(curValue));
		return;
	}
	// Increment the counter
	QSP_NUM(curValue) += curStep;
	// Save new value
	qspConvertVariantTo(&curValue, (*varName.Str == QSP_STRCHAR[0]));
	qspSetVarValueByReference(var, 0, &curValue);
	if (curValue.IsStr) qspFreeString(QSP_STR(curValue));
}

static QSP_BOOL qspStatementSinglelineFor(QSPLineOfCode *s, int startStat, int endStat, QSPString *jumpTo)
{
	QSP_BOOL isExit;
	QSPVar local, *var;
	int curStep, oldRefreshCount;
	QSP_CHAR *endPos;
	QSPString varName, toValuePos, stepValuePos;
	endPos = s->Str.Str + s->Stats[startStat].EndPos;
	if (*endPos != QSP_COLONDELIM[0])
	{
		qspSetError(QSP_ERR_COLONNOTFOUND);
		return QSP_FALSE;
	}
	varName = qspPrepareForLoop(qspStringFromPair(s->Str.Str + s->Stats[startStat].ParamPos, endPos), &local, &toValuePos, &stepValuePos);
	if (!varName.Str) return QSP_FALSE;
	curStep = 1;
	oldRefreshCount = qspRefreshCount;
	while (1)
	{
		if (qspCheckForLoop(varName, toValuePos, stepValuePos, &curStep)) break;
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
		isExit = qspExecStringWithLocals(s, startStat + 1, endStat, jumpTo);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
		if (isExit || (jumpTo->Str != jumpTo->End)) break;
		qspEndForLoop(varName, curStep);
		if (qspErrorNum)
		{
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
	}
	if (!(var = qspVarReference(varName, QSP_TRUE)))
	{
		qspEmptyVar(&local);
		return QSP_FALSE;
	}
	qspEmptyVar(var);
	qspMoveVar(var, &local);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementMultilineFor(QSPLineOfCode *s, int endLine, int lineInd,
	int codeOffset, QSPString *jumpTo)
{
	QSP_BOOL isExit;
	QSPVar local, *var;
	int curStep, oldRefreshCount;
	QSP_CHAR *endPos;
	QSPString varName, toValuePos, stepValuePos;
	QSPLineOfCode *line = s + lineInd;
	endPos = line->Str.Str + line->Stats->EndPos;
	if (*endPos != QSP_COLONDELIM[0])
	{
		qspSetError(QSP_ERR_COLONNOTFOUND);
		return QSP_FALSE;
	}
	varName = qspPrepareForLoop(qspStringFromPair(line->Str.Str + line->Stats->ParamPos, endPos), &local, &toValuePos, &stepValuePos);
	if (!varName.Str) return QSP_FALSE;
	curStep = 1;
	++lineInd;
	oldRefreshCount = qspRefreshCount;
	while (1)
	{
		if (qspCheckForLoop(varName, toValuePos, stepValuePos, &curStep)) break;
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
		isExit = qspExecCodeBlockWithLocals(s, lineInd, endLine, codeOffset, jumpTo);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
		if (isExit || (jumpTo->Str != jumpTo->End)) break;
		if (codeOffset > 0)
		{
			qspRealLine = line->LineNum + codeOffset;
			if (qspIsDebug)
			{
				qspCallDebug(line->Str);
				if (qspRefreshCount != oldRefreshCount)
				{
					qspEmptyVar(&local);
					return QSP_FALSE;
				}
			}
		}
		qspEndForLoop(varName, curStep);
		if (qspErrorNum)
		{
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
	}
	if (!(var = qspVarReference(varName, QSP_TRUE)))
	{
		qspEmptyVar(&local);
		return QSP_FALSE;
	}
	qspEmptyVar(var);
	qspMoveVar(var, &local);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementAddText(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	switch (extArg)
	{
	case 0:
		if (!qspIsEmpty(QSP_STR(args[0])))
		{
			qspAddText(&qspCurVars, QSP_STR(args[0]), QSP_FALSE);
			qspIsVarsDescChanged = QSP_TRUE;
		}
		break;
	case 1:
		if (!qspIsEmpty(QSP_STR(args[0])))
		{
			qspAddText(&qspCurDesc, QSP_STR(args[0]), QSP_FALSE);
			qspIsMainDescChanged = QSP_TRUE;
		}
		break;
	case 2:
		if (count) qspAddText(&qspCurVars, QSP_STR(args[0]), QSP_FALSE);
		qspAddText(&qspCurVars, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
		qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 3:
		if (count) qspAddText(&qspCurDesc, QSP_STR(args[0]), QSP_FALSE);
		qspAddText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
		qspIsMainDescChanged = QSP_TRUE;
		break;
	case 4:
		qspAddText(&qspCurVars, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
		if (count) qspAddText(&qspCurVars, QSP_STR(args[0]), QSP_FALSE);
		qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 5:
		qspAddText(&qspCurDesc, QSP_STATIC_STR(QSP_STRSDELIM), QSP_FALSE);
		if (count) qspAddText(&qspCurDesc, QSP_STR(args[0]), QSP_FALSE);
		qspIsMainDescChanged = QSP_TRUE;
		break;
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementClear(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	switch (extArg)
	{
	case 0: /* qspStatClear */
		if (qspClearText(&qspCurVars))
			qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 1: /* qspStatMClear */
		if (qspClearText(&qspCurDesc))
			qspIsMainDescChanged = QSP_TRUE;
		break;
	case 2: /* qspStatCmdClear */
		qspClearText(&qspCurInput);
		qspCallSetInputStrText(qspNullString);
		break;
	case 3: /* qspStatClA */
		qspClearActions(QSP_FALSE);
		break;
	case 4: /* qspStatClS */
		if (qspClearText(&qspCurVars))
			qspIsVarsDescChanged = QSP_TRUE;
		if (qspClearText(&qspCurDesc))
			qspIsMainDescChanged = QSP_TRUE;
		qspClearText(&qspCurInput);
		qspClearActions(QSP_FALSE);
		qspCallSetInputStrText(qspNullString);
		break;
	case 5: /* qspStatKillAll */
		qspClearVars(QSP_FALSE);
		qspClearObjectsWithNotify();
		break;
	case 6: /* qspStatFreeLib */
		qspClearIncludes(QSP_FALSE);
		if (qspCurLoc >= qspLocsCount) qspCurLoc = -1;
		break;
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementExit(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	return QSP_TRUE;
}

static QSP_BOOL qspStatementGoSub(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	qspExecLocByNameWithArgs(QSP_STR(args[0]), args + 1, count - 1, 0);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementGoTo(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	int locInd = qspLocIndex(QSP_STR(args[0]));
	if (locInd < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return QSP_FALSE;
	}
	qspCurLoc = locInd;
	qspRefreshCurLoc(extArg, args + 1, count - 1);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementJump(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	qspFreeString(*jumpTo);
	*jumpTo = qspGetNewText(qspDelSpc(QSP_STR(args[0])));
	qspUpperStr(jumpTo);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementWait(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	int num = QSP_NUM(args[0]);
	qspCallRefreshInt(QSP_TRUE);
	if (num < 0) num = 0;
	qspCallSleep(num);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementSetTimer(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	int num = QSP_NUM(args[0]);
	if (num < 0) num = 0;
	qspTimerInterval = num;
	qspCallSetTimer(num);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementShowWin(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	QSP_BOOL val = QSP_NUM(args[0]) != 0;
	switch (extArg)
	{
	case 0:
		qspCallShowWindow(QSP_WIN_ACTS, qspCurIsShowActs = val);
		break;
	case 1:
		qspCallShowWindow(QSP_WIN_OBJS, qspCurIsShowObjs = val);
		break;
	case 2:
		qspCallShowWindow(QSP_WIN_VARS, qspCurIsShowVars = val);
		break;
	case 3:
		qspCallShowWindow(QSP_WIN_INPUT, qspCurIsShowInput = val);
		break;
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementRefInt(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	qspCallRefreshInt(QSP_TRUE);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementView(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	QSPString file;
	if (count && qspIsAnyString(QSP_STR(args[0])))
	{
		qspUpdateText(&qspViewPath, QSP_STR(args[0]));
		file = qspGetAbsFromRelPath(qspViewPath);
		qspCallShowPicture(file);
		qspFreeString(file);
	}
	else
	{
		qspClearText(&qspViewPath);
		qspCallShowPicture(qspNullString);
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementMsg(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	qspCallShowMessage(QSP_STR(args[0]));
	return QSP_FALSE;
}

static QSP_BOOL qspStatementExec(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	qspCallSystem(QSP_STR(args[0]));
	return QSP_FALSE;
}

static QSP_BOOL qspStatementDynamic(QSPVariant *args, int count, QSPString *jumpTo, int extArg)
{
	qspExecStringAsCodeWithArgs(QSP_STR(args[0]), args + 1, count - 1, 0);
	return QSP_FALSE;
}
