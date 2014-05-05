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

#ifndef QSP_GAMEDEFINES
	#define QSP_GAMEDEFINES

	#define QSP_GAMEID QSP_FMT("QSPGAME")
	#define QSP_SAVEDGAMEID QSP_FMT("QSPSAVEDGAME")
	#define QSP_GAMEMINVER QSP_FMT("5.7.0")
	#define QSP_MAXINCFILES 100
	#define QSP_DEFTIMERINTERVAL 500

	extern QSPString qspQstPath;
	extern QSPString qspQstFullPath;
	extern int qspQstCRC;
	extern int qspCurIncLocsCount;

	/* External functions */
	char *qspSysLoadGameData(QSPString fileName, int *fileSize);
	QSPString qspGetAbsFromRelPath(QSPString path);
	void qspClearIncludes(QSP_BOOL);
	void qspNewGame(QSP_BOOL);
	void qspOpenQuestFromData(char *data, int dataSize, QSPString fileName, QSP_BOOL isNewGame);
	void qspOpenQuestFromFile(QSPString fileName, QSP_BOOL isNewGame);
	QSPString qspSaveGameStatusToString();
	void qspOpenGameStatusFromString(QSPString str);
	/* Statements */
	QSP_BOOL qspStatementOpenQst(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
	QSP_BOOL qspStatementOpenGame(QSPVariant *args, int count, QSPString *jumpTo, int extArg);
	QSP_BOOL qspStatementSaveGame(QSPVariant *args, int count, QSPString *jumpTo, int extArg);

#endif
