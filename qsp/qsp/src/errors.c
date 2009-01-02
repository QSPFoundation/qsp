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

#include "errors.h"

long qspErrorNum = 0;
long qspErrorLoc = -1;
long qspErrorLine = 0;
long qspErrorWhere = 0;
long qspRealCurLoc = -1;
long qspRealLine = 0;
long qspRealWhere = 0;

void qspSetError(long num)
{
	if (!qspErrorNum)
	{
		qspErrorNum = num;
		qspErrorLoc = qspRealCurLoc;
		qspErrorWhere = qspRealWhere;
		qspErrorLine = qspRealLine;
	}
}

void qspResetError(QSP_BOOL isFull)
{
	qspErrorNum = 0;
	qspErrorLoc = -1;
	qspErrorWhere = QSP_AREA_NONE;
	qspErrorLine = 0;
	if (isFull)
	{
		qspRealCurLoc = -1;
		qspRealWhere = QSP_AREA_NONE;
		qspRealLine = 0;
	}
}
