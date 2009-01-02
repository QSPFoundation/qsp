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

#include "declarations.h"
#include "actions.h"

#ifndef QSP_LOCSDEFINES
	#define QSP_LOCSDEFINES

	typedef struct
	{
		QSP_CHAR *Image;
		QSP_CHAR *Desc;
		QSP_CHAR **OnPressLines;
		long OnPressLinesCount;
	} QSPLocAct;
	typedef struct
	{
		QSP_CHAR *Name;
		QSP_CHAR *Desc;
		QSP_CHAR **OnVisitLines;
		long OnVisitLinesCount;
		QSPLocAct Actions[QSP_MAXACTIONS];
	} QSPLocation;

	extern QSPLocation *qspLocs;
	extern long qspLocsCount;
	extern long qspCurLoc;
	extern long qspRefreshCount;
	extern long qspFullRefreshCount;

	/* External functions */
	void qspCreateWorld(long, long);
	long qspLocIndex(QSP_CHAR *);
	void qspExecLocByIndex(long, QSP_BOOL);
	void qspExecLocByName(QSP_CHAR *, QSP_BOOL);
	void qspExecLocByVarName(QSP_CHAR *);
	void qspRefreshCurLoc(QSP_BOOL);

#endif
