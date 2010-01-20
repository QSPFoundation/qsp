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

#include "errors.h"

int qspErrorNum = 0;
int qspErrorLoc = -1;
int qspErrorLine = 0;
int qspErrorActIndex = -1;
int qspRealCurLoc = -1;
int qspRealLine = 0;
int qspRealActIndex = -1;

void qspSetError(int num)
{
	if (!qspErrorNum)
	{
		qspErrorNum = num;
		qspErrorLoc = qspRealCurLoc;
		qspErrorActIndex = qspRealActIndex;
		qspErrorLine = qspRealLine;
	}
}

void qspResetError()
{
	qspErrorNum = 0;
	qspErrorLoc = -1;
	qspErrorActIndex = -1;
	qspErrorLine = 0;
}
