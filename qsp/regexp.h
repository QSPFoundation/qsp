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

#ifndef QSP_REGEXPDEFINES
	#define QSP_REGEXPDEFINES

	#define QSP_MAXCACHEDREGEXPS 10

	typedef struct
	{
		QSP_CHAR *Text;
		regex_t *CompiledExp;
	} QSPRegExp;

	/* External functions */
	void qspClearRegExps(QSP_BOOL);
	regex_t *qspRegExpGetCompiled(QSP_CHAR *);
	QSP_BOOL qspRegExpStrMatch(regex_t *, QSP_CHAR *);
	QSP_CHAR *qspRegExpStrFind(regex_t *, QSP_CHAR *, int);
	int qspRegExpStrPos(regex_t *, QSP_CHAR *, int);

#endif
