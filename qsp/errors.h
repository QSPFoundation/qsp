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

#ifndef QSP_ERRSDEFINES
	#define QSP_ERRSDEFINES

	extern int qspErrorNum;
	extern int qspErrorLoc;
	extern int qspErrorLine;
	extern int qspErrorActIndex;
	extern int qspRealCurLoc;
	extern int qspRealLine;
	extern int qspRealActIndex;

	/* External functions */
	void qspSetError(int);
	void qspResetError();
	QSP_CHAR *qspGetErrorDesc(int);

#endif
