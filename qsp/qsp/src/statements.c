/* Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru) */
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
#include "codetools.h"
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
long qspStatsNamesCounts[QSP_STATSLEVELS];
long qspStatMaxLen = 0;

static void qspAddStatement(long, char, QSP_STATEMENT, long, long, ...);
static void qspAddStatName(long, QSP_CHAR *, long);
static int qspStatsCompare(const void *, const void *);
static int qspStatStringCompare(const void *, const void *);
static long qspGetStatCode(QSP_CHAR *, QSP_CHAR **);
static long qspSearchElse(QSP_CHAR **, long, long);
static long qspSearchEnd(QSP_CHAR **, long, long);
static long qspSearchLabel(QSP_CHAR **, long, long, QSP_CHAR *);
static QSP_BOOL qspExecString(QSP_CHAR *, QSP_CHAR **);
static QSP_BOOL qspStatementIf(QSP_CHAR *, QSP_CHAR **);
static QSP_BOOL qspStatementAddText(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementClear(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementExit(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementGoSub(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementGoTo(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementJump(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementWait(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementSetTimer(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementShowWin(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementRefInt(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementView(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementMsg(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementExec(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementDynamic(QSPVariant *, long, QSP_CHAR **, char);

static void qspAddStatement(long statCode, char extArg, QSP_STATEMENT func, long minArgs, long maxArgs, ...)
{
	long i;
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

static void qspAddStatName(long statCode, QSP_CHAR *statName, long level)
{
	long count, len = (long)QSP_STRLEN(statName);
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
	return QSP_STRCMP(((QSPStatName *)statName1)->Name, ((QSPStatName *)statName2)->Name);
}

static int qspStatStringCompare(const void *name, const void *compareTo)
{
	return qspStrsComp((QSP_CHAR *)name, ((QSPStatName *)compareTo)->Name, ((QSPStatName *)compareTo)->NameLen);
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
	long i;
	for (i = 0; i < QSP_STATSLEVELS; ++i) qspStatsNamesCounts[i] = 0;
	qspStatMaxLen = 0;
	qspAddStatement(qspStatElse, 0, 0, 0, 0);
	qspAddStatement(qspStatEnd, 0, 0, 0, 0);
	qspAddStatement(qspStatSet, 0, 0, 0, 0);
	qspAddStatement(qspStatIf, 0, 0, 1, 1, 2);
	qspAddStatement(qspStatAct, 0, 0, 1, 2, 1, 1);
	qspAddStatement(qspStatAddObj, 0, qspStatementAddObject, 1, 2, 1, 1);
	qspAddStatement(qspStatAddQst, 1, qspStatementOpenQst, 1, 1, 1);
	qspAddStatement(qspStatClA, 3, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCloseAll, 1, qspStatementCloseFile, 0, 0);
	qspAddStatement(qspStatClose, 0, qspStatementCloseFile, 0, 1, 1);
	qspAddStatement(qspStatClS, 4, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCmdClear, 2, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCopyArr, 0, qspStatementCopyArr, 2, 2, 1, 1);
	qspAddStatement(qspStatDelAct, 0, qspStatementDelAct, 1, 1, 1);
	qspAddStatement(qspStatDelObj, 0, qspStatementDelObj, 1, 1, 1);
	qspAddStatement(qspStatDynamic, 0, qspStatementDynamic, 1, 1, 1);
	qspAddStatement(qspStatExec, 0, qspStatementExec, 1, 1, 1);
	qspAddStatement(qspStatExit, 0, qspStatementExit, 0, 0);
	qspAddStatement(qspStatGoSub, 0, qspStatementGoSub, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddStatement(qspStatGoTo, 1, qspStatementGoTo, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddStatement(qspStatJump, 0, qspStatementJump, 1, 1, 1);
	qspAddStatement(qspStatKillAll, 5, qspStatementClear, 0, 0);
	qspAddStatement(qspStatKillObj, 1, qspStatementDelObj, 0, 1, 2);
	qspAddStatement(qspStatKillQst, 6, qspStatementClear, 0, 0);
	qspAddStatement(qspStatKillVar, 0, qspStatementKillVar, 0, 2, 1, 2);
	qspAddStatement(qspStatMenu, 0, qspStatementShowMenu, 1, 1, 1);
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
	qspAddStatName(qspStatEnd, QSP_FMT("END"), 2);
	qspAddStatName(qspStatSet, QSP_FMT("SET"), 2);
	qspAddStatName(qspStatSet, QSP_FMT("LET"), 2);
	qspAddStatName(qspStatIf, QSP_FMT("IF"), 2);
	qspAddStatName(qspStatAct, QSP_FMT("ACT"), 2);
	qspAddStatName(qspStatAddObj, QSP_FMT("ADDOBJ"), 2);
	qspAddStatName(qspStatAddObj, QSP_FMT("ADD OBJ"), 2);
	qspAddStatName(qspStatAddQst, QSP_FMT("ADDQST"), 2);
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
	qspAddStatName(qspStatGoSub, QSP_FMT("GOSUB"), 2);
	qspAddStatName(qspStatGoSub, QSP_FMT("GS"), 2);
	qspAddStatName(qspStatGoTo, QSP_FMT("GOTO"), 2);
	qspAddStatName(qspStatGoTo, QSP_FMT("GT"), 2);
	qspAddStatName(qspStatJump, QSP_FMT("JUMP"), 2);
	qspAddStatName(qspStatKillAll, QSP_FMT("KILLALL"), 2);
	qspAddStatName(qspStatKillObj, QSP_FMT("KILLOBJ"), 2);
	qspAddStatName(qspStatKillQst, QSP_FMT("KILLQST"), 2);
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

static long qspGetStatCode(QSP_CHAR *s, QSP_CHAR **pos)
{
	long i;
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
			if (pos) *pos = s + name->NameLen;
			free(uStr);
			return name->Code;
		}
	}
	free(uStr);
	return qspStatUnknown;
}

static long qspSearchElse(QSP_CHAR **s, long start, long end)
{
	long c = 1;
	while (start < end)
	{
		switch (qspGetStatCode(s[start], 0))
		{
		case qspStatAct:
		case qspStatIf:
			if (*(qspStrEnd(s[start]) - 1) == QSP_COLONDELIM[0]) ++c;
			break;
		case qspStatElse:
			if (c == 1) return start;
			break;
		case qspStatEnd:
			if (!(--c)) return -1;
			break;
		}
		++start;
	}
	return -1;
}

static long qspSearchEnd(QSP_CHAR **s, long start, long end)
{
	long c = 1;
	while (start < end)
	{
		switch (qspGetStatCode(s[start], 0))
		{
		case qspStatAct:
		case qspStatIf:
			if (*(qspStrEnd(s[start]) - 1) == QSP_COLONDELIM[0]) ++c;
			break;
		case qspStatEnd:
			if (!(--c)) return start;
			break;
		}
		++start;
	}
	return -1;
}

static long qspSearchLabel(QSP_CHAR **s, long start, long end, QSP_CHAR *str)
{
	QSP_CHAR *buf, *pos;
	while (start < end)
	{
		if (*s[start] == QSP_LABEL[0])
		{
			pos = QSP_STRCHR(s[start], QSP_STATDELIM[0]);
			if (pos)
			{
				*pos = 0;
				buf = qspDelSpc(s[start] + 1);
				*pos = QSP_STATDELIM[0];
			}
			else
				buf = qspDelSpc(s[start] + 1);
			qspUpperStr(buf);
			if (!QSP_STRCMP(buf, str))
			{
				free(buf);
				return start;
			}
			free(buf);
		}
		++start;
	}
	return -1;
}

long qspGetStatArgs(QSP_CHAR *s, long statCode, QSPVariant *args)
{
	char type;
	long oldRefreshCount, count = 0;
	QSP_CHAR *pos, *brack = 0;
	s = qspSkipSpaces(s);
	if (*s == QSP_LRBRACK[0])
	{
		if (!(brack = qspStrPos(s, QSP_RRBRACK, QSP_FALSE)))
		{
			qspSetError(QSP_ERR_BRACKNOTFOUND);
			return 0;
		}
		*brack = 0;
		s = qspSkipSpaces(s + 1);
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
			if (pos)
			{
				*pos = 0;
				args[count] = qspExprValue(s);
				*pos = QSP_COMMA[0];
			}
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

static QSP_BOOL qspExecString(QSP_CHAR *s, QSP_CHAR **jumpTo)
{
	QSPVariant args[QSP_STATMAXARGS];
	long oldRefreshCount, statCode, count;
	QSP_BOOL isExit;
	QSP_CHAR *pos, *paramPos;
	s = qspSkipSpaces(s);
	if (!(*s)) return QSP_FALSE;
	pos = qspStrPos(s, QSP_STATDELIM, QSP_FALSE);
	statCode = qspGetStatCode(s, &paramPos);
	if (pos)
	{
		switch (statCode)
		{
		case qspStatComment:
		case qspStatAct:
		case qspStatIf:
			break;
		default:
			oldRefreshCount = qspRefreshCount;
			*pos = 0;
			isExit = qspExecString(s, jumpTo);
			*pos = QSP_STATDELIM[0];
			if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum || **jumpTo) return isExit;
			return qspExecString(pos + 1, jumpTo);
		}
	}
	switch (statCode)
	{
	case qspStatLabel:
	case qspStatComment:
	case qspStatElse:
	case qspStatEnd:
		return QSP_FALSE;
	case qspStatUnknown:
		statCode = (qspStrPos(s, QSP_EQUAL, QSP_FALSE) ? qspStatSet : qspStatMPL);
		paramPos = s;
	default:
		switch (statCode)
		{
		case qspStatAct:
			qspStatementAddAct(paramPos);
			break;
		case qspStatIf:
			return qspStatementIf(paramPos, jumpTo);
		case qspStatSet:
			qspStatementSetVarValue(paramPos);
			break;
		default:
			oldRefreshCount = qspRefreshCount;
			count = qspGetStatArgs(paramPos, statCode, args);
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
			isExit = qspStats[statCode].Func(args, count, jumpTo, qspStats[statCode].ExtArg);
			qspFreeVariants(args, count);
			return isExit;
		}
		return QSP_FALSE;
	}
}

QSP_BOOL qspExecCode(QSP_CHAR **s, long startLine, long endLine, long codeOffset, QSP_CHAR **jumpTo, QSP_BOOL uLevel)
{
	QSPVariant args[2];
	QSP_CHAR *jumpToFake, *pos, *paramPos;
	long i, statCode, count, endPos, elsePos, oldRefreshCount;
	QSP_BOOL isExit = QSP_FALSE;
	oldRefreshCount = qspRefreshCount;
	/* Prepare temporary data */
	if (uLevel)
	{
		jumpToFake = qspGetNewText(QSP_FMT(""), 0);
		jumpTo = &jumpToFake;
	}
	/* Code execution */
	i = startLine;
	while (i < endLine)
	{
		if (codeOffset > 0) qspRealLine = i + codeOffset;
		statCode = qspGetStatCode(s[i], &paramPos);
		if (statCode == qspStatAct || statCode == qspStatIf)
		{
			pos = qspStrEnd(s[i]) - 1;
			if (*pos == QSP_COLONDELIM[0]) /* Multiline */
			{
				endPos = qspSearchEnd(s, ++i, endLine);
				if (endPos < 0)
				{
					qspSetError(QSP_ERR_ENDNOTFOUND);
					break;
				}
				*pos = 0;
				count = qspGetStatArgs(paramPos, statCode, args);
				*pos = QSP_COLONDELIM[0];
				if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
				if (statCode == qspStatAct)
				{
					qspAddAction(args, count, s, i, endPos, codeOffset > 0);
					qspFreeVariants(args, count);
					if (qspErrorNum) break;
					i = endPos;
				}
				else if (statCode == qspStatIf)
				{
					elsePos = qspSearchElse(s, i, endLine);
					if (QSP_NUM(args[0]))
					{
						if (elsePos >= 0)
						{
							isExit = qspExecCode(s, i, elsePos, codeOffset, jumpTo, QSP_FALSE);
							if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum) break;
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
							else
								i = endPos;
						}
					}
					else
						i = (elsePos < 0 ? endPos : elsePos);
				}
				continue;
			}
		}
		isExit = qspExecString(s[i], jumpTo);
		if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum) break;
		if (**jumpTo)
		{
			i = qspSearchLabel(s, startLine, endLine, *jumpTo);
			if (i < 0)
			{
				if (uLevel) qspSetError(QSP_ERR_LABELNOTFOUND);
				break;
			}
			**jumpTo = 0;
			continue;
		}
		++i;
	}
	if (uLevel) free(jumpToFake);
	return isExit;
}

QSP_BOOL qspExecStringAsCode(QSP_CHAR *s, QSP_CHAR **jumpTo)
{
	QSP_BOOL isExit;
	QSP_CHAR **strs;
	long count = qspPreprocessData(s, &strs);
	isExit = qspExecCode(strs, 0, count, 0, jumpTo, QSP_FALSE);
	qspFreeStrs(strs, count, QSP_FALSE);
	return isExit;
}

static QSP_BOOL qspStatementIf(QSP_CHAR *s, QSP_CHAR **jumpTo)
{
	QSPVariant arg;
	QSP_BOOL isExit;
	long oldRefreshCount;
	QSP_CHAR *uStr, *ePos, *pos = qspStrPos(s, QSP_COLONDELIM, QSP_FALSE);
	if (!pos)
	{
		qspSetError(QSP_ERR_COLONNOTFOUND);
		return QSP_FALSE;
	}
	oldRefreshCount = qspRefreshCount;
	*pos = 0;
	qspGetStatArgs(s, qspStatIf, &arg);
	*pos = QSP_COLONDELIM[0];
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
	qspUpperStr(uStr = qspGetNewText(pos, -1));
	ePos = qspStrPos(uStr, QSP_STATELSE, QSP_TRUE);
	free(uStr);
	if (QSP_NUM(arg))
	{
		if (ePos)
		{
			ePos = ePos - uStr + pos;
			*ePos = 0;
			isExit = qspExecString(pos + 1, jumpTo);
			*ePos = QSP_STATELSE[0];
			return isExit;
		}
		else
			return qspExecString(pos + 1, jumpTo);
	}
	else if (ePos)
		return qspExecString(ePos - uStr + pos + QSP_LEN(QSP_STATELSE), jumpTo);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementAddText(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
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

static QSP_BOOL qspStatementClear(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
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

static QSP_BOOL qspStatementExit(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	return QSP_TRUE;
}

static QSP_BOOL qspStatementGoSub(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	qspExecLocByNameWithArgs(QSP_STR(args[0]), args + 1, count - 1);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementGoTo(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSPVar *var;
	long locInd = qspLocIndex(QSP_STR(args[0]));
	if (locInd < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return QSP_FALSE;
	}
	if (!(var = qspVarReference(QSP_FMT("ARGS"), QSP_TRUE))) return QSP_FALSE;
	qspEmptyVar(var);
	qspSetArgs(var, args + 1, count - 1);
	qspCurLoc = locInd;
	qspRefreshCurLoc(extArg);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementJump(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	free(*jumpTo);
	qspUpperStr(*jumpTo = qspDelSpc(QSP_STR(args[0])));
	return QSP_FALSE;
}

static QSP_BOOL qspStatementWait(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long num = QSP_NUM(args[0]);
	qspCallRefreshInt(QSP_TRUE);
	if (num < 0) num = 0;
	qspCallSleep(num);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementSetTimer(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long num = QSP_NUM(args[0]);
	if (num < 0) num = 0;
	qspCallSetTimer(num);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementShowWin(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
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

static QSP_BOOL qspStatementRefInt(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	qspCallRefreshInt(QSP_TRUE);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementView(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_CHAR *file;
	if (count == 1 && qspIsAnyString(QSP_STR(args[0])))
	{
		file = qspGetAbsFromRelPath(QSP_STR(args[0]));
		qspCallShowPicture(file);
		free(file);
	}
	else
		qspCallShowPicture(0);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementMsg(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	qspCallShowMessage(QSP_STR(args[0]));
	return QSP_FALSE;
}

static QSP_BOOL qspStatementExec(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_CHAR *cmd;
	if (qspIsAnyString(QSP_STR(args[0])))
	{
		cmd = qspGetAbsFromRelPath(QSP_STR(args[0]));
		qspCallSystem(cmd);
		free(cmd);
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementDynamic(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	return qspExecStringAsCode(QSP_STR(args[0]), jumpTo);
}
