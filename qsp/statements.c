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
static void qspAddStatName(int, QSP_CHAR *, int);
static int qspStatsCompare(const void *, const void *);
static int qspStatStringCompare(const void *, const void *);
static int qspGetStatCode(QSP_CHAR *, QSP_CHAR **);
static int qspSearchElse(QSPLineOfCode *, int, int);
static int qspSearchEnd(QSPLineOfCode *, int, int);
static int qspSearchLabel(QSPLineOfCode *, int, int, QSP_CHAR *);
static QSP_BOOL qspExecString(QSPLineOfCode *, int, int, QSP_CHAR **);
static QSP_BOOL qspExecMultilineCode(QSPLineOfCode *, int, int, QSP_CHAR **, int *, int *);
static QSP_BOOL qspExecSinglelineCode(QSPLineOfCode *, int, int, QSP_CHAR **, int *, int *);
static QSP_BOOL qspExecCode(QSPLineOfCode *, int, int, int, QSP_CHAR **);
static QSP_BOOL qspExecCodeBlockWithLocals(QSPLineOfCode *, int, int, int, QSP_CHAR **);
static QSP_BOOL qspExecStringWithLocals(QSPLineOfCode *, int, int, QSP_CHAR **);
static QSP_BOOL qspStatementIf(QSPLineOfCode *, int, int, QSP_CHAR **);
static QSP_CHAR *qspPrepareForLoop(QSP_CHAR *, QSPVar *, QSP_CHAR **, QSP_CHAR **);
static QSP_BOOL qspCheckForLoop(QSP_CHAR *, QSP_CHAR *, QSP_CHAR *, QSP_CHAR *, int *);
static void qspEndForLoop(QSP_CHAR *, int);
static QSP_BOOL qspStatementSinglelineFor(QSPLineOfCode *, int, int, QSP_CHAR **);
static QSP_BOOL qspStatementMultilineFor(QSPLineOfCode *, int, int, int, QSP_CHAR **);
static QSP_BOOL qspStatementAddText(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementClear(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementExit(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementGoSub(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementGoTo(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementJump(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementWait(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementSetTimer(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementShowWin(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementRefInt(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementView(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementMsg(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementExec(QSPVariant *, int, QSP_CHAR **, int);
static QSP_BOOL qspStatementDynamic(QSPVariant *, int, QSP_CHAR **, int);

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

static void qspAddStatName(int statCode, QSP_CHAR *statName, int level)
{
	int count, len = qspStrLen(statName);
	count = qspStatsNamesCounts[level];
	qspStatsNames[level][count].Name = statName;
	qspStatsNames[level][count].NameLen = len;
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
	return qspStrsNComp((QSP_CHAR *)name, ((QSPStatName *)compareTo)->Name, ((QSPStatName *)compareTo)->NameLen);
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
	qspAddStatName(qspStatElse, QSP_STATELSE, 2);
	qspAddStatName(qspStatElseIf, QSP_FMT("ELSEIF"), 1);
	qspAddStatName(qspStatEnd, QSP_FMT("END"), 2);
	qspAddStatName(qspStatLocal, QSP_FMT("LOCAL"), 2);
	qspAddStatName(qspStatSet, QSP_FMT("SET"), 2);
	qspAddStatName(qspStatSet, QSP_FMT("LET"), 2);
	qspAddStatName(qspStatIf, QSP_FMT("IF"), 2);
	qspAddStatName(qspStatAct, QSP_FMT("ACT"), 2);
	qspAddStatName(qspStatFor, QSP_FMT("FOR"), 2);
	qspAddStatName(qspStatAddObj, QSP_FMT("ADDOBJ"), 2);
	qspAddStatName(qspStatAddObj, QSP_FMT("ADD OBJ"), 2);
	qspAddStatName(qspStatClA, QSP_FMT("CLA"), 2);
	qspAddStatName(qspStatCloseAll, QSP_FMT("CLOSE ALL"), 1);
	qspAddStatName(qspStatClose, QSP_FMT("CLOSE"), 2);
	qspAddStatName(qspStatClS, QSP_FMT("CLS"), 2);
	qspAddStatName(qspStatCmdClear, QSP_FMT("CMDCLEAR"), 2);
	qspAddStatName(qspStatCmdClear, QSP_FMT("CMDCLR"), 2);
	qspAddStatName(qspStatCopyArr, QSP_FMT("COPYARR"), 2);
	qspAddStatName(qspStatDelAct, QSP_FMT("DELACT"), 2);
	qspAddStatName(qspStatDelAct, QSP_FMT("DEL ACT"), 2);
	qspAddStatName(qspStatDelObj, QSP_FMT("DELOBJ"), 2);
	qspAddStatName(qspStatDelObj, QSP_FMT("DEL OBJ"), 2);
	qspAddStatName(qspStatDynamic, QSP_FMT("DYNAMIC"), 2);
	qspAddStatName(qspStatExec, QSP_FMT("EXEC"), 2);
	qspAddStatName(qspStatExit, QSP_FMT("EXIT"), 2);
	qspAddStatName(qspStatFreeLib, QSP_FMT("FREELIB"), 2);
	qspAddStatName(qspStatGoSub, QSP_FMT("GOSUB"), 2);
	qspAddStatName(qspStatGoSub, QSP_FMT("GS"), 2);
	qspAddStatName(qspStatGoTo, QSP_FMT("GOTO"), 2);
	qspAddStatName(qspStatGoTo, QSP_FMT("GT"), 2);
	qspAddStatName(qspStatIncLib, QSP_FMT("INCLIB"), 2);
	qspAddStatName(qspStatJump, QSP_FMT("JUMP"), 2);
	qspAddStatName(qspStatKillAll, QSP_FMT("KILLALL"), 2);
	qspAddStatName(qspStatKillObj, QSP_FMT("KILLOBJ"), 2);
	qspAddStatName(qspStatKillVar, QSP_FMT("KILLVAR"), 2);
	qspAddStatName(qspStatMenu, QSP_FMT("MENU"), 2);
	qspAddStatName(qspStatMClear, QSP_FMT("*CLEAR"), 2);
	qspAddStatName(qspStatMClear, QSP_FMT("*CLR"), 2);
	qspAddStatName(qspStatMNL, QSP_FMT("*NL"), 2);
	qspAddStatName(qspStatMPL, QSP_FMT("*PL"), 1);
	qspAddStatName(qspStatMP, QSP_FMT("*P"), 2);
	qspAddStatName(qspStatClear, QSP_FMT("CLEAR"), 2);
	qspAddStatName(qspStatClear, QSP_FMT("CLR"), 2);
	qspAddStatName(qspStatNL, QSP_FMT("NL"), 2);
	qspAddStatName(qspStatPL, QSP_FMT("PL"), 1);
	qspAddStatName(qspStatP, QSP_FMT("P"), 2);
	qspAddStatName(qspStatMsg, QSP_FMT("MSG"), 2);
	qspAddStatName(qspStatOpenGame, QSP_FMT("OPENGAME"), 2);
	qspAddStatName(qspStatOpenQst, QSP_FMT("OPENQST"), 2);
	qspAddStatName(qspStatPlay, QSP_FMT("PLAY"), 0);
	qspAddStatName(qspStatRefInt, QSP_FMT("REFINT"), 2);
	qspAddStatName(qspStatSaveGame, QSP_FMT("SAVEGAME"), 2);
	qspAddStatName(qspStatSetTimer, QSP_FMT("SETTIMER"), 1);
	qspAddStatName(qspStatShowActs, QSP_FMT("SHOWACTS"), 2);
	qspAddStatName(qspStatShowInput, QSP_FMT("SHOWINPUT"), 2);
	qspAddStatName(qspStatShowObjs, QSP_FMT("SHOWOBJS"), 2);
	qspAddStatName(qspStatShowVars, QSP_FMT("SHOWSTAT"), 2);
	qspAddStatName(qspStatUnSelect, QSP_FMT("UNSELECT"), 1);
	qspAddStatName(qspStatUnSelect, QSP_FMT("UNSEL"), 2);
	qspAddStatName(qspStatView, QSP_FMT("VIEW"), 2);
	qspAddStatName(qspStatWait, QSP_FMT("WAIT"), 2);
	qspAddStatName(qspStatXGoTo, QSP_FMT("XGOTO"), 2);
	qspAddStatName(qspStatXGoTo, QSP_FMT("XGT"), 2);
	for (i = 0; i < QSP_STATSLEVELS; ++i)
		qsort(qspStatsNames[i], qspStatsNamesCounts[i], sizeof(QSPStatName), qspStatsCompare);
}

static int qspGetStatCode(QSP_CHAR *s, QSP_CHAR **pos)
{
	int i;
	QSP_CHAR *uStr;
	QSPStatName *name;
	if (!(*s)) return qspStatUnknown;
	if (*s == QSP_LABEL[0]) return qspStatLabel;
	if (*s == QSP_COMMENT[0]) return qspStatComment;
	/* ------------------------------------------------------------------ */
	qspUpperStr(uStr = qspGetNewText(s, qspStatMaxLen));
	for (i = 0; i < QSP_STATSLEVELS; ++i)
	{
		name = (QSPStatName *)bsearch(uStr, qspStatsNames[i], qspStatsNamesCounts[i], sizeof(QSPStatName), qspStatStringCompare);
		if (name && qspIsInListEOL(QSP_DELIMS, s[name->NameLen]))
		{
			*pos = s + name->NameLen;
			free(uStr);
			return name->Code;
		}
	}
	free(uStr);
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

static int qspSearchLabel(QSPLineOfCode *s, int start, int end, QSP_CHAR *str)
{
	s += start;
	while (start < end)
	{
		if (s->Label && !qspStrsComp(s->Label, str)) return start;
		++start;
		++s;
	}
	return -1;
}

int qspGetStatArgs(QSP_CHAR *s, int statCode, QSPVariant *args)
{
	int type;
	int oldRefreshCount, count = 0;
	QSP_CHAR *pos, *brack = 0;
	s = qspSkipSpaces(s);
	if (*s == QSP_LRBRACK[0])
	{
		if (!(brack = qspStrPos(s, QSP_RRBRACK, QSP_FALSE)))
		{
			qspSetError(QSP_ERR_BRACKNOTFOUND);
			return 0;
		}
		if (qspIsAnyString(brack + 1))
			brack = 0;
		else
		{
			*brack = 0;
			s = qspSkipSpaces(s + 1);
		}
	}
	if (*s)
	{
		oldRefreshCount = qspRefreshCount;
		while (1)
		{
			if (count == qspStats[statCode].MaxArgsCount)
			{
				qspSetError(QSP_ERR_ARGSCOUNT);
				break;
			}
			pos = qspStrPos(s, QSP_COMMA, QSP_FALSE);
			args[count] = qspExprValuePartial(s, pos);
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
			s = qspSkipSpaces(pos + QSP_LEN(QSP_COMMA));
			if (!(*s))
			{
				qspSetError(QSP_ERR_SYNTAX);
				break;
			}
		}
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			qspFreeVariants(args, count);
			if (brack) *brack = QSP_RRBRACK[0];
			return 0;
		}
	}
	if (brack) *brack = QSP_RRBRACK[0];
	if (count < qspStats[statCode].MinArgsCount)
	{
		qspSetError(QSP_ERR_ARGSCOUNT);
		qspFreeVariants(args, count);
		return 0;
	}
	return count;
}

static QSP_BOOL qspExecString(QSPLineOfCode *s, int startStat, int endStat, QSP_CHAR **jumpTo)
{
	QSPVariant args[QSP_STATMAXARGS];
	QSP_BOOL isExit;
	QSP_CHAR *pos;
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
			{
				pos = s->Str + s->Stats[i].EndPos;
				*pos = 0;
				qspStatementLocal(s->Str + s->Stats[i].ParamPos);
				*pos = QSP_STATDELIM[0];
			}
			else
				qspStatementLocal(s->Str + s->Stats[i].ParamPos);
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
			break;
		case qspStatSet:
			if (i < s->StatsCount - 1)
			{
				pos = s->Str + s->Stats[i].EndPos;
				*pos = 0;
				qspStatementSetVarValue(s->Str + s->Stats[i].ParamPos);
				*pos = QSP_STATDELIM[0];
			}
			else
				qspStatementSetVarValue(s->Str + s->Stats[i].ParamPos);
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
			break;
		default:
			if (i < s->StatsCount - 1)
			{
				pos = s->Str + s->Stats[i].EndPos;
				*pos = 0;
				count = qspGetStatArgs(s->Str + s->Stats[i].ParamPos, statCode, args);
				*pos = QSP_STATDELIM[0];
			}
			else
				count = qspGetStatArgs(s->Str + s->Stats[i].ParamPos, statCode, args);
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
			isExit = qspStats[statCode].Func(args, count, jumpTo, qspStats[statCode].ExtArg);
			qspFreeVariants(args, count);
			if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum || **jumpTo) return isExit;
			break;
		}
	}
	return QSP_FALSE;
}

static QSP_BOOL qspExecMultilineCode(QSPLineOfCode *s, int endLine, int codeOffset,
	QSP_CHAR **jumpTo, int *lineInd, int *action)
{
	QSPVariant arg;
	QSPLineOfCode *line;
	QSP_CHAR *pos;
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
		pos = line->Str + line->Stats->EndPos;
		oldRefreshCount = qspRefreshCount;
		*pos = 0;
		qspGetStatArgs(line->Str + line->Stats->ParamPos, statCode, &arg);
		*pos = QSP_COLONDELIM[0];
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
	QSP_CHAR **jumpTo, int *lineInd, int *action)
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
		pos = line->Str + line->Stats->EndPos;
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
		*pos = 0;
		qspGetStatArgs(line->Str + line->Stats->ParamPos, qspStatElseIf, &arg);
		*pos = QSP_COLONDELIM[0];
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

static QSP_BOOL qspExecCode(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSP_CHAR **jumpTo)
{
	QSPLineOfCode *line;
	QSP_CHAR *jumpToFake;
	QSP_BOOL uLevel, isExit = QSP_FALSE;
	int i, oldRefreshCount, action = qspFlowExecute;
	oldRefreshCount = qspRefreshCount;
	/* Prepare temporary data */
	if (uLevel = !jumpTo)
	{
		jumpToFake = qspGetNewText(QSP_FMT(""), 0);
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
			if (qspIsDebug && *line->Str)
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
		if (**jumpTo)
		{
			i = qspSearchLabel(s, startLine, endLine, *jumpTo);
			if (i < 0)
			{
				if (uLevel) qspSetError(QSP_ERR_LABELNOTFOUND);
				break;
			}
			**jumpTo = 0;
		}
	}
	if (uLevel) free(jumpToFake);
	return isExit;
}

static QSP_BOOL qspExecCodeBlockWithLocals(QSPLineOfCode *s, int startLine, int endLine, int codeOffset, QSP_CHAR **jumpTo)
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

static QSP_BOOL qspExecStringWithLocals(QSPLineOfCode *s, int startStat, int endStat, QSP_CHAR **jumpTo)
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

void qspExecStringAsCodeWithArgs(QSP_CHAR *s, QSPVariant *args, int count, QSPVariant *res)
{
	QSPLineOfCode *strs;
	QSPVar local, result, *var, *varRes;
	int oldRefreshCount;
	if (!(varRes = qspVarReference(QSP_VARRES, QSP_TRUE))) return;
	if (!(var = qspVarReference(QSP_VARARGS, QSP_TRUE))) return;
	qspMoveVar(&result, varRes);
	qspMoveVar(&local, var);
	qspSetArgs(var, args, count);
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
	if (!(var = qspVarReference(QSP_VARARGS, QSP_TRUE)))
	{
		qspEmptyVar(&local);
		qspEmptyVar(&result);
		return;
	}
	qspEmptyVar(var);
	qspMoveVar(var, &local);
	if (!(varRes = qspVarReference(QSP_VARRES, QSP_TRUE)))
	{
		qspEmptyVar(&result);
		return;
	}
	if (res) qspApplyResult(varRes, res);
	qspEmptyVar(varRes);
	qspMoveVar(varRes, &result);
}

QSP_CHAR *qspGetLineLabel(QSP_CHAR *str)
{
	QSP_CHAR *delimPos;
	str = qspSkipSpaces(str);
	if (*str == QSP_LABEL[0])
	{
		delimPos = qspStrChar(str, QSP_STATDELIM[0]);
		if (delimPos)
		{
			*delimPos = 0;
			str = qspDelSpc(str + 1);
			*delimPos = QSP_STATDELIM[0];
		}
		else
			str = qspDelSpc(str + 1);
		qspUpperStr(str);
		return str;
	}
	return 0;
}

void qspInitLineOfCode(QSPLineOfCode *line, QSP_CHAR *str, int lineNum)
{
	QSP_BOOL isInLoop, isSearchElse;
	int statCode, count = 0;
	QSP_CHAR *uStr, *nextPos, *temp, *buf, *elsePos, *delimPos = 0, *paramPos = 0;
	line->Str = str;
	line->LineNum = lineNum;
	line->StatsCount = 0;
	line->Stats = 0;
	buf = qspSkipSpaces(str);
	statCode = qspGetStatCode(buf, &paramPos);
	if (*buf && statCode != qspStatComment)
	{
		isInLoop = isSearchElse = QSP_TRUE;
		elsePos = 0;
		qspUpperStr(uStr = qspGetNewText(str, -1));
		switch (statCode)
		{
		case qspStatAct:
		case qspStatFor:
		case qspStatIf:
		case qspStatElseIf:
			delimPos = qspStrPos(buf, QSP_COLONDELIM, QSP_FALSE);
			if (delimPos)
			{
				nextPos = delimPos + 1;
				if (!(*nextPos)) isInLoop = QSP_FALSE;
			}
			break;
		case qspStatElse:
			nextPos = qspSkipSpaces(paramPos);
			if (*nextPos == QSP_COLONDELIM[0]) ++nextPos;
			delimPos = (*nextPos ? nextPos : 0);
			break;
		default:
			delimPos = qspStrPos(buf, QSP_STATDELIM, QSP_FALSE);
			if (delimPos) nextPos = delimPos + 1;
			elsePos = qspStrPos(uStr + (buf - str), QSP_STATELSE, QSP_TRUE);
			if (elsePos)
				elsePos = str + (elsePos - uStr);
			else
				isSearchElse = QSP_FALSE;
			if (elsePos && (!delimPos || elsePos < delimPos))
			{
				nextPos = delimPos = elsePos;
				elsePos = 0;
			}
			if (statCode == qspStatUnknown && buf != delimPos)
			{
				temp = qspStrPosPartial(buf, delimPos, QSP_EQUAL, QSP_FALSE);
				statCode = (temp ? qspStatSet : qspStatMPL);
			}
			break;
		}
		while (delimPos && isInLoop)
		{
			line->StatsCount++;
			line->Stats = (QSPCachedStat *)realloc(line->Stats, line->StatsCount * sizeof(QSPCachedStat));
			line->Stats[count].Stat = statCode;
			line->Stats[count].EndPos = (int)(delimPos - str);
			if (paramPos)
				line->Stats[count].ParamPos = (int)(qspSkipSpaces(paramPos) - str);
			else
				line->Stats[count].ParamPos = (int)(buf - str);
			++count;
			buf = qspSkipSpaces(nextPos);
			paramPos = 0;
			statCode = qspGetStatCode(buf, &paramPos);
			if (*buf && statCode != qspStatComment)
			{
				switch (statCode)
				{
				case qspStatAct:
				case qspStatFor:
				case qspStatIf:
				case qspStatElseIf:
					delimPos = qspStrPos(buf, QSP_COLONDELIM, QSP_FALSE);
					if (delimPos)
					{
						nextPos = delimPos + 1;
						if (!(*nextPos)) isInLoop = QSP_FALSE;
					}
					break;
				case qspStatElse:
					nextPos = qspSkipSpaces(paramPos);
					if (*nextPos == QSP_COLONDELIM[0]) ++nextPos;
					delimPos = (*nextPos ? nextPos : 0);
					break;
				default:
					delimPos = qspStrPos(buf, QSP_STATDELIM, QSP_FALSE);
					if (delimPos) nextPos = delimPos + 1;
					if (elsePos && buf >= elsePos) elsePos = 0;
					if (!elsePos && isSearchElse)
					{
						elsePos = qspStrPos(uStr + (buf - str), QSP_STATELSE, QSP_TRUE);
						if (elsePos)
							elsePos = str + (elsePos - uStr);
						else
							isSearchElse = QSP_FALSE;
					}
					if (elsePos && (!delimPos || elsePos < delimPos))
					{
						nextPos = delimPos = elsePos;
						elsePos = 0;
					}
					if (statCode == qspStatUnknown && buf != delimPos)
					{
						temp = qspStrPosPartial(buf, delimPos, QSP_EQUAL, QSP_FALSE);
						statCode = (temp ? qspStatSet : qspStatMPL);
					}
					break;
				}
			}
			else
				delimPos = 0;
		}
		free(uStr);
	}
	/* Check for ELSE IF */
	if (count == 1 && delimPos && line->Stats->Stat == qspStatElse && statCode == qspStatIf &&
		*(str + line->Stats->ParamPos) != QSP_COLONDELIM[0])
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
		line->Stats[count].EndPos = (int)(delimPos - str);
	else
		line->Stats[count].EndPos = (int)(qspStrEnd(buf) - str);
	if (paramPos)
		line->Stats[count].ParamPos = (int)(qspSkipSpaces(paramPos) - str);
	else
		line->Stats[count].ParamPos = (int)(buf - str);
	switch (line->Stats->Stat)
	{
	case qspStatAct:
	case qspStatFor:
	case qspStatIf:
	case qspStatElseIf:
		line->IsMultiline = (line->StatsCount == 1 && *(str + line->Stats->EndPos) == QSP_COLONDELIM[0]);
		break;
	default:
		line->IsMultiline = QSP_FALSE;
		break;
	}
	line->Label = qspGetLineLabel(str);
}

static QSP_BOOL qspStatementIf(QSPLineOfCode *s, int startStat, int endStat, QSP_CHAR **jumpTo)
{
	QSPVariant arg;
	int i, c, elseStat, oldRefreshCount;
	QSP_CHAR *pos = s->Str + s->Stats[startStat].EndPos;
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
	*pos = 0;
	qspGetStatArgs(s->Str + s->Stats[startStat].ParamPos, qspStatIf, &arg);
	*pos = QSP_COLONDELIM[0];
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

static QSP_CHAR *qspPrepareForLoop(QSP_CHAR *paramPos, QSPVar *local, QSP_CHAR **toPosRes, QSP_CHAR **stepPosRes)
{
	QSPVar *var;
	int oldRefreshCount;
	QSPVariant initValue;
	QSP_CHAR *uStr, *eqPos, *toPos, *stepPos, *varName;
	eqPos = qspStrPos(paramPos, QSP_EQUAL, QSP_FALSE);
	if (!eqPos)
	{
		qspSetError(QSP_ERR_EQNOTFOUND);
		return 0;
	}
	*eqPos = 0;
	varName = qspDelSpc(paramPos);
	*eqPos = QSP_EQUAL[0];
	if (qspStrChar(varName, QSP_LSBRACK[0]))
	{
		qspSetError(QSP_ERR_NOTCORRECTNAME);
		free(varName);
		return 0;
	}
	qspUpperStr(uStr = qspGetNewText(eqPos, -1));
	toPos = qspStrPos(uStr + QSP_LEN(QSP_EQUAL), QSP_STATFORTO, QSP_TRUE);
	if (!toPos)
	{
		qspSetError(QSP_ERR_TONOTFOUND);
		free(varName);
		free(uStr);
		return 0;
	}
	toPos = eqPos + (toPos - uStr);
	stepPos = qspStrPos(uStr + (toPos - eqPos) + QSP_LEN(QSP_STATFORTO), QSP_STATFORSTEP, QSP_TRUE);
	if (stepPos) stepPos = eqPos + (stepPos - uStr);
	free(uStr);
	oldRefreshCount = qspRefreshCount;
	*toPos = 0;
	initValue = qspExprValue(eqPos + QSP_LEN(QSP_EQUAL));
	*toPos = QSP_STATFORTO[0];
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
	{
		free(varName);
		return 0;
	}
	if (qspConvertVariantTo(&initValue, QSP_FALSE))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		free(varName);
		free(QSP_STR(initValue));
		return 0;
	}
	if (!(var = qspVarReference(varName, QSP_TRUE)))
	{
		free(varName);
		return 0;
	}
	qspMoveVar(local, var);
	qspConvertVariantTo(&initValue, (*varName == QSP_STRCHAR[0]));
	qspSetVarValueByReference(var, 0, &initValue);
	if (initValue.IsStr) free(QSP_STR(initValue));
	*toPosRes = toPos;
	*stepPosRes = stepPos;
	return varName;
}

static QSP_BOOL qspCheckForLoop(QSP_CHAR *varName, QSP_CHAR *toPos, QSP_CHAR *stepPos, QSP_CHAR *endPos, int *curStep)
{
	QSPVar *var;
	QSPVariant toValue, stepValue, curValue;
	int oldRefreshCount = qspRefreshCount;
	if (stepPos)
	{
		*endPos = 0;
		stepValue = qspExprValue(stepPos + QSP_LEN(QSP_STATFORSTEP));
		*endPos = QSP_COLONDELIM[0];
		if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
		if (qspConvertVariantTo(&stepValue, QSP_FALSE))
		{
			qspSetError(QSP_ERR_TYPEMISMATCH);
			free(QSP_STR(stepValue));
			return QSP_FALSE;
		}
		*curStep = QSP_NUM(stepValue);
		*stepPos = 0;
		toValue = qspExprValue(toPos + QSP_LEN(QSP_STATFORTO));
		*stepPos = QSP_STATFORSTEP[0];
	}
	else
	{
		*endPos = 0;
		toValue = qspExprValue(toPos + QSP_LEN(QSP_STATFORTO));
		*endPos = QSP_COLONDELIM[0];
	}
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
	if (qspConvertVariantTo(&toValue, QSP_FALSE))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		free(QSP_STR(toValue));
		return QSP_FALSE;
	}
	if (!(var = qspVarReference(varName, QSP_TRUE))) return QSP_FALSE;
	curValue = qspGetVarValueByReference(var, 0, (*varName == QSP_STRCHAR[0]));
	if (qspConvertVariantTo(&curValue, QSP_FALSE))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		free(QSP_STR(curValue));
		return QSP_FALSE;
	}
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

static void qspEndForLoop(QSP_CHAR *varName, int curStep)
{
	QSPVar *var;
	QSPVariant curValue;
	if (!(var = qspVarReference(varName, QSP_TRUE))) return;
	curValue = qspGetVarValueByReference(var, 0, (*varName == QSP_STRCHAR[0]));
	if (qspConvertVariantTo(&curValue, QSP_FALSE))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		free(QSP_STR(curValue));
		return;
	}
	QSP_NUM(curValue) += curStep;
	qspConvertVariantTo(&curValue, (*varName == QSP_STRCHAR[0]));
	qspSetVarValueByReference(var, 0, &curValue);
	if (curValue.IsStr) free(QSP_STR(curValue));
}

static QSP_BOOL qspStatementSinglelineFor(QSPLineOfCode *s, int startStat, int endStat, QSP_CHAR **jumpTo)
{
	QSP_BOOL isExit;
	QSPVar local, *var;
	int curStep, oldRefreshCount;
	QSP_CHAR *endPos, *toPos, *stepPos, *varName;
	endPos = s->Str + s->Stats[startStat].EndPos;
	if (*endPos != QSP_COLONDELIM[0])
	{
		qspSetError(QSP_ERR_COLONNOTFOUND);
		return QSP_FALSE;
	}
	if (!(varName = qspPrepareForLoop(s->Str + s->Stats[startStat].ParamPos, &local, &toPos, &stepPos)))
		return QSP_FALSE;
	curStep = 1;
	oldRefreshCount = qspRefreshCount;
	while (1)
	{
		if (qspCheckForLoop(varName, toPos, stepPos, endPos, &curStep)) break;
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			free(varName);
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
		isExit = qspExecStringWithLocals(s, startStat + 1, endStat, jumpTo);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			free(varName);
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
		if (isExit || **jumpTo) break;
		qspEndForLoop(varName, curStep);
		if (qspErrorNum)
		{
			free(varName);
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
	}
	if (!(var = qspVarReference(varName, QSP_TRUE)))
	{
		free(varName);
		qspEmptyVar(&local);
		return QSP_FALSE;
	}
	free(varName);
	qspEmptyVar(var);
	qspMoveVar(var, &local);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementMultilineFor(QSPLineOfCode *s, int endLine, int lineInd,
	int codeOffset, QSP_CHAR **jumpTo)
{
	QSP_BOOL isExit;
	QSPVar local, *var;
	int curStep, oldRefreshCount;
	QSP_CHAR *endPos, *toPos, *stepPos, *varName;
	QSPLineOfCode *line = s + lineInd;
	if (!(varName = qspPrepareForLoop(line->Str + line->Stats->ParamPos, &local, &toPos, &stepPos)))
		return QSP_FALSE;
	endPos = line->Str + line->Stats->EndPos;
	curStep = 1;
	++lineInd;
	oldRefreshCount = qspRefreshCount;
	while (1)
	{
		if (qspCheckForLoop(varName, toPos, stepPos, endPos, &curStep)) break;
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			free(varName);
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
		isExit = qspExecCodeBlockWithLocals(s, lineInd, endLine, codeOffset, jumpTo);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			free(varName);
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
		if (isExit || **jumpTo) break;
		if (codeOffset > 0)
		{
			qspRealLine = line->LineNum + codeOffset;
			if (qspIsDebug)
			{
				qspCallDebug(line->Str);
				if (qspRefreshCount != oldRefreshCount)
				{
					free(varName);
					qspEmptyVar(&local);
					return QSP_FALSE;
				}
			}
		}
		qspEndForLoop(varName, curStep);
		if (qspErrorNum)
		{
			free(varName);
			qspEmptyVar(&local);
			return QSP_FALSE;
		}
	}
	if (!(var = qspVarReference(varName, QSP_TRUE)))
	{
		free(varName);
		qspEmptyVar(&local);
		return QSP_FALSE;
	}
	free(varName);
	qspEmptyVar(var);
	qspMoveVar(var, &local);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementAddText(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	switch (extArg)
	{
	case 0:
		if (*QSP_STR(args[0]))
		{
			qspCurVarsLen = qspAddText(&qspCurVars, QSP_STR(args[0]), qspCurVarsLen, -1, QSP_FALSE);
			qspIsVarsDescChanged = QSP_TRUE;
		}
		break;
	case 1:
		if (*QSP_STR(args[0]))
		{
			qspCurDescLen = qspAddText(&qspCurDesc, QSP_STR(args[0]), qspCurDescLen, -1, QSP_FALSE);
			qspIsMainDescChanged = QSP_TRUE;
		}
		break;
	case 2:
		if (count) qspCurVarsLen = qspAddText(&qspCurVars, QSP_STR(args[0]), qspCurVarsLen, -1, QSP_FALSE);
		qspCurVarsLen = qspAddText(&qspCurVars, QSP_STRSDELIM, qspCurVarsLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 3:
		if (count) qspCurDescLen = qspAddText(&qspCurDesc, QSP_STR(args[0]), qspCurDescLen, -1, QSP_FALSE);
		qspCurDescLen = qspAddText(&qspCurDesc, QSP_STRSDELIM, qspCurDescLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		qspIsMainDescChanged = QSP_TRUE;
		break;
	case 4:
		qspCurVarsLen = qspAddText(&qspCurVars, QSP_STRSDELIM, qspCurVarsLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		if (count) qspCurVarsLen = qspAddText(&qspCurVars, QSP_STR(args[0]), qspCurVarsLen, -1, QSP_FALSE);
		qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 5:
		qspCurDescLen = qspAddText(&qspCurDesc, QSP_STRSDELIM, qspCurDescLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		if (count) qspCurDescLen = qspAddText(&qspCurDesc, QSP_STR(args[0]), qspCurDescLen, -1, QSP_FALSE);
		qspIsMainDescChanged = QSP_TRUE;
		break;
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementClear(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	switch (extArg)
	{
	case 0:
		if (qspClearText(&qspCurVars, &qspCurVarsLen))
			qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 1:
		if (qspClearText(&qspCurDesc, &qspCurDescLen))
			qspIsMainDescChanged = QSP_TRUE;
		break;
	case 2:
		qspClearText(&qspCurInput, &qspCurInputLen);
		qspCallSetInputStrText(0);
		break;
	case 3:
		qspClearActions(QSP_FALSE);
		break;
	case 4:
		if (qspClearText(&qspCurVars, &qspCurVarsLen))
			qspIsVarsDescChanged = QSP_TRUE;
		if (qspClearText(&qspCurDesc, &qspCurDescLen))
			qspIsMainDescChanged = QSP_TRUE;
		qspClearText(&qspCurInput, &qspCurInputLen);
		qspClearActions(QSP_FALSE);
		qspCallSetInputStrText(0);
		break;
	case 5:
		qspClearVars(QSP_FALSE);
		qspClearObjectsWithNotify();
		break;
	case 6:
		qspClearIncludes(QSP_FALSE);
		if (qspCurLoc >= qspLocsCount) qspCurLoc = -1;
		break;
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementExit(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	return QSP_TRUE;
}

static QSP_BOOL qspStatementGoSub(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	qspExecLocByNameWithArgs(QSP_STR(args[0]), args + 1, count - 1, 0);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementGoTo(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
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

static QSP_BOOL qspStatementJump(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	free(*jumpTo);
	qspUpperStr(*jumpTo = qspDelSpc(QSP_STR(args[0])));
	return QSP_FALSE;
}

static QSP_BOOL qspStatementWait(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	int num = QSP_NUM(args[0]);
	qspCallRefreshInt(QSP_TRUE);
	if (num < 0) num = 0;
	qspCallSleep(num);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementSetTimer(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	int num = QSP_NUM(args[0]);
	if (num < 0) num = 0;
	qspTimerInterval = num;
	qspCallSetTimer(num);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementShowWin(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
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

static QSP_BOOL qspStatementRefInt(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	qspCallRefreshInt(QSP_TRUE);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementView(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	QSP_CHAR *file;
	if (count && qspIsAnyString(QSP_STR(args[0])))
	{
		qspViewPath = qspGetAddText(qspViewPath, QSP_STR(args[0]), 0, -1);
		file = qspGetAbsFromRelPath(qspViewPath);
		qspCallShowPicture(file);
		free(file);
	}
	else
	{
		if (qspViewPath)
		{
			free(qspViewPath);
			qspViewPath = 0;
		}
		qspCallShowPicture(0);
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementMsg(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	qspCallShowMessage(QSP_STR(args[0]));
	return QSP_FALSE;
}

static QSP_BOOL qspStatementExec(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	qspCallSystem(QSP_STR(args[0]));
	return QSP_FALSE;
}

static QSP_BOOL qspStatementDynamic(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	qspExecStringAsCodeWithArgs(QSP_STR(args[0]), args + 1, count - 1, 0);
	return QSP_FALSE;
}
