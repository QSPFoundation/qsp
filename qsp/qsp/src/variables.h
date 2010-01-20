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
#include "variant.h"

#ifndef QSP_VARSDEFINES
	#define QSP_VARSDEFINES

	#define QSP_VARSSEEK 50
	#define QSP_VARSCOUNT 256 * QSP_VARSSEEK
	#define QSP_VARARGS QSP_FMT("ARGS")
	#define QSP_VARRES QSP_FMT("RESULT")

	typedef struct
	{
		int Num;
		QSP_CHAR *Str;
	} QSPVarValue;

	typedef struct
	{
		QSP_CHAR *Name;
		QSPVarValue *Values;
		int ValsCount;
		QSP_CHAR **Indices;
		int IndsCount;
	} QSPVar;

	extern QSPVar qspVars[QSP_VARSCOUNT];

	/* External functions */
	void qspClearVars(QSP_BOOL);
	void qspEmptyVar(QSPVar *);
	QSPVar *qspVarReference(QSP_CHAR *, QSP_BOOL);
	QSPVar *qspVarReferenceWithType(QSP_CHAR *, QSP_BOOL, QSP_BOOL *);
	void qspSetVarValueByName(QSP_CHAR *, QSPVariant *);
	QSP_CHAR *qspGetVarStrValue(QSP_CHAR *);
	int qspGetVarNumValue(QSP_CHAR *);
	QSPVariant qspGetVar(QSP_CHAR *);
	int qspArraySize(QSP_CHAR *);
	int qspArrayPos(QSPVariant *, int, QSP_BOOL);
	QSPVariant qspArrayMinMaxItem(QSP_CHAR *, QSP_BOOL);
	int qspGetVarsCount();
	void qspSetArgs(QSPVar *, QSPVariant *, int);
	void qspMoveVar(QSPVar *, QSPVar *);
	/* Statements */
	void qspStatementSetVarValue(QSP_CHAR *);
	QSP_BOOL qspStatementCopyArr(QSPVariant *, int, QSP_CHAR **, char);
	QSP_BOOL qspStatementKillVar(QSPVariant *, int, QSP_CHAR **, char);

#endif
