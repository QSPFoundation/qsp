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

#ifndef QSP_COMMONDEFINES
	#define QSP_COMMONDEFINES

	#define QSP_RANDMASK 0x7FFFFFFF

	extern QSP_BOOL qspIsDebug;
	extern QSP_CHAR *qspCurDesc;
	extern int qspCurDescLen;
	extern QSP_CHAR *qspCurVars;
	extern int qspCurVarsLen;
	extern QSP_CHAR *qspCurInput;
	extern int qspCurInputLen;
	extern QSP_CHAR *qspViewPath;
	extern int qspTimerInterval;
	extern QSP_BOOL qspIsMainDescChanged;
	extern QSP_BOOL qspIsVarsDescChanged;
	extern QSP_BOOL qspCurIsShowVars;
	extern QSP_BOOL qspCurIsShowInput;

	/* External functions */
	void qspPrepareExecution();
	void qspMemClear(QSP_BOOL);
	void qspSetSeed(unsigned int);
	int qspRand();

#endif
