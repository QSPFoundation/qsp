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

#ifndef QSP_VARSDEFINES
	#define QSP_VARSDEFINES

	#define QSP_VARSSEEK 50
	#define QSP_VARSCOUNT 256 * QSP_VARSSEEK

	typedef struct
	{
		QSP_CHAR *Name;
		long *Value;
		QSP_CHAR **TextValue;
		long ValsCount;
		QSP_CHAR **TextIndex;
		long IndsCount;
		long Type;
	} QSPVar;

	enum
	{
		qspVarNormal,
		qspVarQSPVer,
		qspVarRnd,
		qspVarCountObj,
		qspVarMsecsCount,
		qspVarUserText,
		qspVarCurLoc,
		qspVarSelObj,
		qspVarSelAct,
		qspVarMainText,
		qspVarStatText
	};

	extern QSPVar qspVars[QSP_VARSCOUNT];

	/* External functions */
	void qspClearVars(QSP_BOOL);
	void qspInitVars();
	long qspVarIndex(QSP_CHAR *, QSP_BOOL);
	long qspVarIndexWithType(QSP_CHAR *, QSP_BOOL, QSP_BOOL *);
	void qspSetVarValueByName(QSP_CHAR *, QSPVariant);
	QSP_CHAR *qspGetVarStrValue(QSP_CHAR *);
	long qspGetVarNumValue(QSP_CHAR *);
	QSPVariant qspGetVar(QSP_CHAR *);
	long qspArrayPos(QSP_CHAR *, long, QSPVariant, QSP_BOOL);
	long qspGetVarsCount();
	/* Statements */
	void qspStatementSetVarValue(QSP_CHAR *);
	QSP_BOOL qspStatementCopyArr(QSPVariant *, long, QSP_CHAR **, char);

#endif
