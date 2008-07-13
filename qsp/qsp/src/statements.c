/* Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include "declarations.h"
#include "statements.h"

QSPStatement qspStats[qspStatLast_Statement];

QSP_BOOL qspStatementIf(QSP_CHAR *, QSP_CHAR **);
QSP_BOOL qspStatementAddText(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementAddNewLine(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementClear(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementExit(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementGoSub(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementGoTo(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementJump(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementWait(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementSetTimer(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementShowWin(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementRefInt(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementShowMenu(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementView(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementMsg(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementExec(QSPVariant *, long, QSP_CHAR **, char);
QSP_BOOL qspStatementDynamic(QSPVariant *, long, QSP_CHAR **, char);

void qspAddStatement(long statCode,
					 QSP_CHAR *statName,
					 QSP_CHAR *statAltName,
					 char extArg,
					 QSP_STATEMENT func,
					 long minArgs,
					 long maxArgs,
					 ...)
{
	long i;
	va_list marker;
	qspStats[statCode].Names[0] = statName;
	qspStats[statCode].Names[1] = statAltName;
	qspStats[statCode].NamesLens[0] = (long)QSP_STRLEN(statName);
	qspStats[statCode].NamesLens[1] = (statAltName ? (long)QSP_STRLEN(statAltName) : 0);
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

void qspInitStats()
{
	/*
	Format:
		qspAddStatement(
			Statement,
			Name,
			Alternative Name,
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
	qspAddStatement(qspStatElse, QSP_FMT("ELSE"), 0, 0, 0, 0, 0);
	qspAddStatement(qspStatEnd, QSP_FMT("END"), 0, 0, 0, 0, 0);
	qspAddStatement(qspStatSet, QSP_FMT("SET"), QSP_FMT("LET"), 0, 0, 0, 0);
	qspAddStatement(qspStatIf, QSP_FMT("IF"), 0, 0, 0, 1, 1, 2);
	qspAddStatement(qspStatAct, QSP_FMT("ACT"), 0, 0, 0, 1, 2, 1, 1);
	qspAddStatement(qspStatAddObj, QSP_FMT("ADDOBJ"), QSP_FMT("ADD OBJ"), 0, qspStatementAddObject, 1, 2, 1, 1);
	qspAddStatement(qspStatAddQst, QSP_FMT("ADDQST"), 0, 1, qspStatementOpenQst, 1, 1, 1);
	qspAddStatement(qspStatClA, QSP_FMT("CLA"), 0, 3, qspStatementClear, 0, 0);
	qspAddStatement(qspStatClear, QSP_FMT("CLEAR"), QSP_FMT("CLR"), 0, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCloseAll, QSP_FMT("CLOSE ALL"), 0, 1, qspStatementCloseFile, 0, 0);
	qspAddStatement(qspStatClose, QSP_FMT("CLOSE"), 0, 0, qspStatementCloseFile, 0, 1, 1);
	qspAddStatement(qspStatClS, QSP_FMT("CLS"), 0, 4, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCmdClear, QSP_FMT("CMDCLEAR"), QSP_FMT("CMDCLR"), 2, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCopyArr, QSP_FMT("COPYARR"), 0, 0, qspStatementCopyArr, 2, 2, 1, 1);
	qspAddStatement(qspStatDelAct, QSP_FMT("DELACT"), QSP_FMT("DEL ACT"), 0, qspStatementDelAct, 1, 1, 1);
	qspAddStatement(qspStatDelObj, QSP_FMT("DELOBJ"), QSP_FMT("DEL OBJ"), 0, qspStatementDelObj, 1, 1, 1);
	qspAddStatement(qspStatDynamic, QSP_FMT("DYNAMIC"), 0, 0, qspStatementDynamic, 1, 1, 1);
	qspAddStatement(qspStatExec, QSP_FMT("EXEC"), 0, 0, qspStatementExec, 1, 1, 1);
	qspAddStatement(qspStatExit, QSP_FMT("EXIT"), 0, 0, qspStatementExit, 0, 0);
	qspAddStatement(qspStatGoSub, QSP_FMT("GOSUB"), QSP_FMT("GS"), 0, qspStatementGoSub, 1, 1, 1);
	qspAddStatement(qspStatGoTo, QSP_FMT("GOTO"), QSP_FMT("GT"), 1, qspStatementGoTo, 1, 1, 1);
	qspAddStatement(qspStatJump, QSP_FMT("JUMP"), 0, 0, qspStatementJump, 1, 1, 1);
	qspAddStatement(qspStatKillAll, QSP_FMT("KILLALL"), 0, 7, qspStatementClear, 0, 0);
	qspAddStatement(qspStatKillObj, QSP_FMT("KILLOBJ"), 0, 5, qspStatementClear, 0, 0);
	qspAddStatement(qspStatKillQst, QSP_FMT("KILLQST"), 0, 8, qspStatementClear, 0, 0);
	qspAddStatement(qspStatKillVar, QSP_FMT("KILLVAR"), 0, 6, qspStatementClear, 0, 0);
	qspAddStatement(qspStatMClear, QSP_FMT("*CLEAR"), QSP_FMT("*CLR"), 1, qspStatementClear, 0, 0);
	qspAddStatement(qspStatMenu, QSP_FMT("MENU"), 0, 0, qspStatementShowMenu, 1, 1, 1);
	qspAddStatement(qspStatMNL, QSP_FMT("*NL"), 0, 1, qspStatementAddNewLine, 0, 0);
	qspAddStatement(qspStatMPL, QSP_FMT("*PL"), 0, 3, qspStatementAddText, 1, 1, 1);
	qspAddStatement(qspStatMP, QSP_FMT("*P"), 0, 2, qspStatementAddText, 1, 1, 1);
	qspAddStatement(qspStatMsg, QSP_FMT("MSG"), 0, 0, qspStatementMsg, 1, 1, 1);
	qspAddStatement(qspStatNL, QSP_FMT("NL"), 0, 0, qspStatementAddNewLine, 0, 0);
	qspAddStatement(qspStatOpenGame, QSP_FMT("OPENGAME"), 0, 0, qspStatementOpenGame, 0, 1, 1);
	qspAddStatement(qspStatOpenQst, QSP_FMT("OPENQST"), 0, 0, qspStatementOpenQst, 1, 1, 1);
	qspAddStatement(qspStatPlay, QSP_FMT("PLAY"), 0, 0, qspStatementPlayFile, 1, 2, 1, 2);
	qspAddStatement(qspStatPL, QSP_FMT("PL"), 0, 1, qspStatementAddText, 1, 1, 1);
	qspAddStatement(qspStatP, QSP_FMT("P"), 0, 0, qspStatementAddText, 1, 1, 1);
	qspAddStatement(qspStatRefInt, QSP_FMT("REFINT"), 0, 0, qspStatementRefInt, 0, 0);
	qspAddStatement(qspStatSaveGame, QSP_FMT("SAVEGAME"), 0, 0, qspStatementSaveGame, 0, 1, 1);
	qspAddStatement(qspStatSetTimer, QSP_FMT("SETTIMER"), 0, 0, qspStatementSetTimer, 1, 1, 2);
	qspAddStatement(qspStatShowActs, QSP_FMT("SHOWACTS"), 0, 0, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatShowInput, QSP_FMT("SHOWINPUT"), 0, 3, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatShowObjs, QSP_FMT("SHOWOBJS"), 0, 1, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatShowVars, QSP_FMT("SHOWSTAT"), 0, 2, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatUnSelect, QSP_FMT("UNSELECT"), QSP_FMT("UNSEL"), 0, qspStatementUnSelect, 0, 0);
	qspAddStatement(qspStatView, QSP_FMT("VIEW"), 0, 0, qspStatementView, 0, 1, 1);
	qspAddStatement(qspStatWait, QSP_FMT("WAIT"), 0, 0, qspStatementWait, 1, 1, 2);
	qspAddStatement(qspStatXGoTo, QSP_FMT("XGOTO"), QSP_FMT("XGT"), 0, qspStatementGoTo, 1, 1, 1);
}

long qspGetStatCode(QSP_CHAR *s, QSP_CHAR **pos)
{
	long i, j, len;
	QSP_CHAR *uStr;
	if (!(*s)) return qspStatUnknown;
	if (*s == QSP_LABEL[0]) return qspStatLabel;
	if (*s == QSP_COMMENT[0]) return qspStatComment;
	/* ------------------------------------------------------------------ */
	qspUpperStr(uStr = qspGetNewText(s, -1));
	for (i = qspStatFirst_Statement; i < qspStatLast_Statement; ++i)
		for (j = 0; j < 2; ++j)
			if (qspStats[i].Names[j])
			{
				len = qspStats[i].NamesLens[j];
				if (qspIsEqual(uStr, qspStats[i].Names[j], len) && qspIsInListEOL(QSP_DELIMS, s[len]))
				{
					if (pos) *pos = s + len;
					free(uStr);
					return i;
				}
			}
	free(uStr);
	return qspStatUnknown;
}

long qspSearchElse(QSP_CHAR **s, long start, long end)
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

long qspSearchEnd(QSP_CHAR **s, long start, long end)
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

long qspSearchLabel(QSP_CHAR **s, long start, long end, QSP_CHAR *str)
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
	QSP_CHAR *pos;
	char type;
	long count = 0;
	QSP_BOOL convErr = QSP_FALSE;
	while (1)
	{
		s = qspSkipSpaces(s);
		if (!(*s))
		{
			if (count) qspSetError(QSP_ERR_SYNTAX);
			break;
		}
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
		if (qspErrorNum) break;
		type = qspStats[statCode].ArgsTypes[count];
		if (type) args[count] = qspConvertVariantTo(args[count], type == 1, QSP_TRUE, &convErr);
		++count;
		if (convErr)
		{
			qspSetError(QSP_ERR_TYPEMISMATCH);
			break;
		}
		if (!pos) break;
		s = pos + QSP_LEN(QSP_COMMA);
	}
	if (count < qspStats[statCode].MinArgsCount)
		qspSetError(QSP_ERR_ARGSCOUNT);
	if (qspErrorNum)
	{
		qspFreeVariants(args, count);
		return 0;
	}
	return count;
}

QSP_BOOL qspExecString(QSP_CHAR *s, QSP_CHAR **jumpTo)
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
			count = qspGetStatArgs(paramPos, statCode, args);
			if (qspErrorNum) break;
			isExit = qspStats[statCode].Func(args, count, jumpTo, qspStats[statCode].ExtArg);
			qspFreeVariants(args, count);
			return isExit;
		}
		return QSP_FALSE;
	}
}

QSP_BOOL qspExecCode(QSP_CHAR **s, long startLine, long endLine, long codeOffset, QSP_CHAR **jumpTo, QSP_BOOL uLevel)
{
	QSPVariant args[QSP_STATMAXARGS];
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
		qspRealLine = i + codeOffset;
		statCode = qspGetStatCode(s[i], &paramPos);
		switch (statCode)
		{
		case qspStatAct:
		case qspStatIf:
			pos = qspStrEnd(s[i]) - 1;
			if (*pos == QSP_COLONDELIM[0]) /* Multiline */
			{
				endPos = qspSearchEnd(s, i + 1, endLine);
				if (endPos < 0)
				{
					qspSetError(QSP_ERR_ENDNOTFOUND);
					break;
				}
				*pos = 0;
				count = qspGetStatArgs(paramPos, statCode, args);
				*pos = QSP_COLONDELIM[0];
				if (qspErrorNum) break;
				switch (statCode)
				{
				case qspStatAct:
					qspAddAction(args, count, s, i + 1, endPos, QSP_TRUE);
					qspFreeVariants(args, count);
					i = endPos;
					break;
				case qspStatIf:
					elsePos = qspSearchElse(s, ++i, endLine);
					if (args[0].Num)
					{
						if (elsePos >= 0)
						{
							isExit = qspExecCode(s, i, elsePos, 1, jumpTo, QSP_FALSE);
							if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum)
								i = endLine;
							else if (**jumpTo)
							{
								i = qspSearchLabel(s, startLine, endLine, *jumpTo);
								if (i < 0)
								{
									if (uLevel) qspSetError(QSP_ERR_LABELNOTFOUND);
									i = endLine;
								}
							}
							else
								i = endPos;
						}
					}
					else
						i = (elsePos < 0 ? endPos : elsePos);
					break;
				}
				continue;
			}
			break;
		}
		if (qspErrorNum) break;
		**jumpTo = 0;
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
			continue;
		}
		++i;
	}
	if (uLevel) free(jumpToFake);
	return isExit;
}

QSP_BOOL qspStatementIf(QSP_CHAR *s, QSP_CHAR **jumpTo)
{
	QSPVariant arg;
	QSP_BOOL isExit;
	QSP_CHAR *uStr, *ePos, *pos = qspStrPos(s, QSP_COLONDELIM, QSP_FALSE);
	if (!pos)
	{
		qspSetError(QSP_ERR_COLONNOTFOUND);
		return QSP_FALSE;
	}
	*pos = 0;
	qspGetStatArgs(s, qspStatIf, &arg);
	*pos = QSP_COLONDELIM[0];
	if (qspErrorNum) return QSP_FALSE;
	qspUpperStr(uStr = qspGetNewText(pos, -1));
	ePos = qspStrPos(uStr, qspStats[qspStatElse].Names[0], QSP_TRUE);
	free(uStr);
	if (arg.Num)
	{
		if (ePos)
		{
			ePos = ePos - uStr + pos;
			*ePos = 0;
			isExit = qspExecString(pos + 1, jumpTo);
			*ePos = qspStats[qspStatElse].Names[0][0];
			return isExit;
		}
		else
			return qspExecString(pos + 1, jumpTo);
	}
	else if (ePos)
		return qspExecString(ePos - uStr + pos + qspStats[qspStatElse].NamesLens[0], jumpTo);
	return QSP_FALSE;
}

QSP_BOOL qspStatementAddText(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_CHAR *s = args[0].Str;
	switch (extArg)
	{
	case 0:
		qspCurVarsLen = qspAddText(&qspCurVars, s, qspCurVarsLen, -1, QSP_FALSE);
		if (*s) qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 1:
		qspCurVarsLen = qspAddText(&qspCurVars, s, qspCurVarsLen, -1, QSP_FALSE);
		qspCurVarsLen = qspAddText(&qspCurVars, QSP_STRSDELIM, qspCurVarsLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 2:
		qspCurDescLen = qspAddText(&qspCurDesc, s, qspCurDescLen, -1, QSP_FALSE);
		if (*s) qspIsMainDescChanged = QSP_TRUE;
		break;
	case 3:
		qspCurDescLen = qspAddText(&qspCurDesc, s, qspCurDescLen, -1, QSP_FALSE);
		qspCurDescLen = qspAddText(&qspCurDesc, QSP_STRSDELIM, qspCurDescLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		qspIsMainDescChanged = QSP_TRUE;
		break;
	}
	return QSP_FALSE;
}

QSP_BOOL qspStatementAddNewLine(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	switch (extArg)
	{
	case 0:
		qspCurVarsLen = qspAddText(&qspCurVars, QSP_STRSDELIM, qspCurVarsLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 1:
		qspCurDescLen = qspAddText(&qspCurDesc, QSP_STRSDELIM, qspCurDescLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		qspIsMainDescChanged = QSP_TRUE;
		break;
	}
	return QSP_FALSE;
}

QSP_BOOL qspStatementClear(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
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
		qspClearObjectsWithNotify();
		break;
	case 6:
		qspClearVars(QSP_FALSE);
		qspInitVars();
		break;
	case 7:
		qspClearVars(QSP_FALSE);
		qspInitVars();
		qspClearObjectsWithNotify();
		break;
	case 8:
		qspClearIncludes(QSP_FALSE);
		if (qspCurLoc >= qspLocsCount) qspCurLoc = -1;
		if (qspRealCurLoc >= qspLocsCount) qspRealCurLoc = -1;
		break;
	}
	return QSP_FALSE;
}

QSP_BOOL qspStatementExit(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	return QSP_TRUE;
}

QSP_BOOL qspStatementGoSub(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	qspExecLocByName(args[0].Str, QSP_FALSE);
	return QSP_FALSE;
}

QSP_BOOL qspStatementGoTo(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long locInd = qspLocIndex(args[0].Str);
	if (locInd < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return QSP_FALSE;
	}
	qspCurLoc = locInd;
	qspRefresh(extArg);
	return QSP_FALSE;
}

QSP_BOOL qspStatementJump(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	free(*jumpTo);
	qspUpperStr(*jumpTo = qspDelSpc(args[0].Str));
	return QSP_FALSE;
}

QSP_BOOL qspStatementWait(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_BOOL prevIsMustWait = qspIsMustWait;
	long num = args[0].Num;
	qspIsMustWait = QSP_FALSE;
	qspCallRefreshInt(QSP_TRUE);
	qspIsMustWait = prevIsMustWait;
	if (num < 0) num = 0;
	qspCallSleep(num);
	return QSP_FALSE;
}

QSP_BOOL qspStatementSetTimer(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long num = args[0].Num;
	if (num < 0) num = 0;
	qspCallSetTimer(num);
	return QSP_FALSE;
}

QSP_BOOL qspStatementShowWin(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_BOOL val = args[0].Num != 0;
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

QSP_BOOL qspStatementRefInt(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_BOOL prevIsMustWait = qspIsMustWait;
	qspIsMustWait = QSP_FALSE;
	qspCallRefreshInt(QSP_TRUE);
	qspIsMustWait = prevIsMustWait;
	return QSP_FALSE;
}

QSP_BOOL qspStatementShowMenu(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_CHAR *imgPath, *str, *pos, *pos2, *endPos;
	long i, varInd = qspVarIndexWithSpaces(args[0].Str, QSP_FALSE, 0);
	if (varInd < 0) return QSP_FALSE;
	qspClearMenu(QSP_FALSE);
	qspCallDeleteMenu();
	for (i = 0; i < qspVars[varInd].ValsCount; ++i)
	{
		if (!((str = qspVars[varInd].TextValue[i]) && qspIsAnyString(str))) break;
		pos2 = qspInStrRChar(str, QSP_MENUDELIM[0], 0);
		if (!pos2)
		{
			qspSetError(QSP_ERR_COLONNOTFOUND);
			return QSP_FALSE;
		}
		if (qspCurMenuItems == QSP_MAXMENUITEMS)
		{
			qspSetError(QSP_ERR_CANTADDMENUITEM);
			return QSP_FALSE;
		}
		endPos = qspStrEnd(str);
		pos = qspInStrRChar(str, QSP_MENUDELIM[0], pos2);
		if (!pos)
		{
			pos = pos2;
			pos2 = endPos;
		}
		qspCurMenuLocs[qspCurMenuItems++] = qspGetNewText(pos + 1, (long)(pos2 - pos) - 1);
		if (pos2 < endPos && qspIsAnyString(++pos2))
		{
			imgPath = qspGetNewText(qspQstPath, qspQstPathLen);
			imgPath = qspGetAddText(imgPath, pos2, qspQstPathLen, (long)(endPos - pos2));
		}
		else
			imgPath = 0;
		*pos = 0;
		qspCallAddMenuItem(str, imgPath);
		*pos = QSP_MENUDELIM[0];
		if (imgPath) free(imgPath);
	}
	qspCallShowMenu();
	return QSP_FALSE;
}

QSP_BOOL qspStatementView(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_CHAR *file;
	if (count == 1 && qspIsAnyString(args[0].Str))
	{
		file = qspGetNewText(qspQstPath, qspQstPathLen);
		file = qspGetAddText(file, args[0].Str, qspQstPathLen, -1);
		qspCallShowPicture(file);
		free(file);
	}
	else
		qspCallShowPicture(0);
	return QSP_FALSE;
}

QSP_BOOL qspStatementMsg(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	qspCallShowMessage(args[0].Str);
	return QSP_FALSE;
}

QSP_BOOL qspStatementExec(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_CHAR *cmd;
	if (qspIsAnyString(args[0].Str))
	{
		cmd = qspGetNewText(qspQstPath, qspQstPathLen);
		cmd = qspGetAddText(cmd, args[0].Str, qspQstPathLen, -1);
		qspCallSystem(cmd);
		free(cmd);
	}
	return QSP_FALSE;
}

QSP_BOOL qspStatementDynamic(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	return qspExecString(args[0].Str, jumpTo);
}
