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

#ifndef QSP_OBJSDEFINES
	#define QSP_OBJSDEFINES

	#define QSP_MAXOBJECTS 1000

	typedef struct
	{
		QSP_CHAR *Image;
		QSP_CHAR *Desc;
	} QSPObj;

	extern QSPObj qspCurObjects[QSP_MAXOBJECTS];
	extern int qspCurObjectsCount;
	extern int qspCurSelObject;
	extern QSP_BOOL qspIsObjectsChanged;
	extern QSP_BOOL qspCurIsShowObjs;

	/* External functions */
	void qspClearObjects(QSP_BOOL);
	void qspClearObjectsWithNotify();
	int qspObjIndex(QSP_CHAR *);
	/* Statements */
	QSP_BOOL qspStatementAddObject(QSPVariant *, int, QSP_CHAR **, int);
	QSP_BOOL qspStatementDelObj(QSPVariant *, int, QSP_CHAR **, int);
	QSP_BOOL qspStatementUnSelect(QSPVariant *, int, QSP_CHAR **, int);

#endif
