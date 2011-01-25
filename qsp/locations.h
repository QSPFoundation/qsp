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
#include "actions.h"
#include "codetools.h"
#include "variant.h"

#ifndef QSP_LOCSDEFINES
	#define QSP_LOCSDEFINES

	typedef struct
	{
		QSP_CHAR *Image;
		QSP_CHAR *Desc;
		QSPLineOfCode *OnPressLines;
		int OnPressLinesCount;
	} QSPLocAct;
	typedef struct
	{
		QSP_CHAR *Name;
		QSP_CHAR *Desc;
		QSPLineOfCode *OnVisitLines;
		int OnVisitLinesCount;
		QSPLocAct Actions[QSP_MAXACTIONS];
	} QSPLocation;
	typedef struct
	{
		int Index;
		QSP_CHAR *Name;
	} QSPLocName;

	extern QSPLocation *qspLocs;
	extern QSPLocName *qspLocsNames;
	extern int qspLocsCount;
	extern int qspCurLoc;
	extern int qspRefreshCount;
	extern int qspFullRefreshCount;

	/* External functions */
	void qspCreateWorld(int, int);
	void qspPrepareLocs();
	int qspLocIndex(QSP_CHAR *);
	void qspExecLocByIndex(int, QSP_BOOL, QSP_BOOL);
	void qspExecLocByName(QSP_CHAR *, QSP_BOOL);
	void qspExecLocByNameWithArgs(QSP_CHAR *, QSPVariant *, int, QSPVariant *);
	void qspExecLocByVarNameWithArgs(QSP_CHAR *, QSPVariant *, int);
	void qspRefreshCurLoc(QSP_BOOL, QSPVariant *, int);

#endif
