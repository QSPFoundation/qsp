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
#include "variant.h"

#ifndef QSP_ACTSDEFINES
	#define QSP_ACTSDEFINES

	#define QSP_MAXACTIONS 50

	typedef struct
	{
		QSP_CHAR *Image;
		QSP_CHAR *Desc;
		QSP_CHAR **OnPressLines;
		long OnPressLinesCount;
		long Location;
		long Where;
		long StartLine;
	} QSPCurAct;

	extern QSPCurAct qspCurActions[QSP_MAXACTIONS];
	extern long qspCurActionsCount;
	extern long qspCurSelAction;
	extern QSP_BOOL qspIsActionsChanged;
	extern QSP_BOOL qspCurIsShowActs;

	void qspClearActions(QSP_BOOL);
	void qspAddAction(QSPVariant *, long, QSP_CHAR **, long, long, QSP_BOOL);
	void qspExecAction(long);
	/* -- */
	void qspStatementAddAct(QSP_CHAR *);
	QSP_BOOL qspStatementDelAct(QSPVariant *, long, QSP_CHAR **, char);

#endif
