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
#include "codetools.h"
#include "variant.h"

#ifndef QSP_ACTSDEFINES
	#define QSP_ACTSDEFINES

	#define QSP_MAXACTIONS 50

	typedef struct
	{
		QSP_CHAR *Image;
		QSP_CHAR *Desc;
		QSPLineOfCode *OnPressLines;
		int OnPressLinesCount;
		int Location;
		int ActIndex;
		int StartLine;
		QSP_BOOL IsManageLines;
	} QSPCurAct;

	extern QSPCurAct qspCurActions[QSP_MAXACTIONS];
	extern int qspCurActionsCount;
	extern int qspCurSelAction;
	extern QSP_BOOL qspIsActionsChanged;
	extern QSP_BOOL qspCurIsShowActs;

	/* External functions */
	void qspClearActions(QSP_BOOL);
	void qspAddAction(QSPVariant *, int, QSPLineOfCode *, int, int, QSP_BOOL);
	void qspExecAction(int);
	QSP_CHAR *qspGetAllActionsAsCode();
	/* Statements */
	void qspStatementSinglelineAddAct(QSPLineOfCode *, int, int);
	void qspStatementMultilineAddAct(QSPLineOfCode *, int, int, QSP_BOOL);
	QSP_BOOL qspStatementDelAct(QSPVariant *, int, QSP_CHAR **, int);

#endif
