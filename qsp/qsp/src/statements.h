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

#include "declarations.h"
#include "variant.h"

#ifndef QSP_STATSDEFINES
	#define QSP_STATSDEFINES

	#define QSP_STATSLEVELS 3
	#define QSP_MAXSTATSNAMES 100
	#define QSP_STATMAXARGS 10
	#define QSP_STATELSE QSP_FMT("ELSE")

	typedef QSP_BOOL (*QSP_STATEMENT)(QSPVariant *, int, QSP_CHAR **, int);

	typedef struct
	{
		int Code;
		QSP_CHAR *Name;
		int NameLen;
	} QSPStatName;

	typedef struct
	{
		int MinArgsCount;
		int MaxArgsCount;
		int ArgsTypes[QSP_STATMAXARGS];
		int ExtArg;
		QSP_STATEMENT Func;
	} QSPStatement;

	enum
	{
		qspStatUnknown,
		qspStatLabel,
		qspStatComment,
		qspStatAct,
		qspStatIf,
		qspStatElse,
		qspStatEnd,
		qspStatAddObj,
		qspStatAddQst,
		qspStatClA,
		qspStatClear,
		qspStatCloseAll,
		qspStatClose,
		qspStatClS,
		qspStatCmdClear,
		qspStatCopyArr,
		qspStatDelAct,
		qspStatDelObj,
		qspStatDynamic,
		qspStatExec,
		qspStatExit,
		qspStatGoSub,
		qspStatGoTo,
		qspStatJump,
		qspStatKillAll,
		qspStatKillObj,
		qspStatKillQst,
		qspStatKillVar,
		qspStatMClear,
		qspStatMenu,
		qspStatMNL,
		qspStatMPL,
		qspStatMP,
		qspStatMsg,
		qspStatNL,
		qspStatOpenGame,
		qspStatOpenQst,
		qspStatPlay,
		qspStatPL,
		qspStatP,
		qspStatRefInt,
		qspStatSaveGame,
		qspStatSetTimer,
		qspStatSet,
		qspStatShowActs,
		qspStatShowInput,
		qspStatShowObjs,
		qspStatShowVars,
		qspStatUnSelect,
		qspStatView,
		qspStatWait,
		qspStatXGoTo,

		qspStatLast_Statement
	};

	/* External functions */
	void qspInitStats();
	int qspGetStatArgs(QSP_CHAR *, int, QSPVariant *);
	QSP_BOOL qspExecCode(QSP_CHAR **, int, int, int, QSP_CHAR **);
	void qspExecStringAsCode(QSP_CHAR *);

#endif
