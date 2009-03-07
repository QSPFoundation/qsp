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

#ifndef QSP_TEXTDEFINES
	#define QSP_TEXTDEFINES

	#define QSP_STRSDELIM QSP_FMT("\r\n")
	#define QSP_LSUBEX QSP_FMT("<<")
	#define QSP_RSUBEX QSP_FMT(">>")

	/* Helpers */
	#define QSP_LEN(x) (sizeof(x) / sizeof(QSP_CHAR) - 1)

	/* External functions */
	long qspAddText(QSP_CHAR **, QSP_CHAR *, long, long, QSP_BOOL);
	QSP_CHAR *qspGetNewText(QSP_CHAR *, long);
	QSP_CHAR *qspGetAddText(QSP_CHAR *, QSP_CHAR *, long, long);
	QSP_BOOL qspClearText(void **, long *);
	QSP_BOOL qspIsInList(QSP_CHAR *, QSP_CHAR);
	QSP_BOOL qspIsInListEOL(QSP_CHAR *, QSP_CHAR);
	QSP_BOOL qspIsDigit(QSP_CHAR);
	QSP_CHAR *qspSkipSpaces(QSP_CHAR *);
	QSP_CHAR *qspStrEnd(QSP_CHAR *);
	QSP_CHAR *qspDelSpc(QSP_CHAR *);
	QSP_BOOL qspIsAnyString(QSP_CHAR *);
	void qspLowerStr(QSP_CHAR *);
	void qspUpperStr(QSP_CHAR *);
	QSP_BOOL qspIsEqual(QSP_CHAR *, QSP_CHAR *, long);
	QSP_CHAR *qspInStrRChars(QSP_CHAR *, QSP_CHAR *, QSP_CHAR *);
	QSP_CHAR *qspJoinStrs(QSP_CHAR **, long, QSP_CHAR *);
	long qspSplitStr(QSP_CHAR *, QSP_CHAR *, QSP_CHAR ***);
	void qspCopyStrs(QSP_CHAR ***, QSP_CHAR **, long, long);
	void qspFreeStrs(void **, long, QSP_BOOL);
	long qspStrToNum(QSP_CHAR *, QSP_CHAR **);
	QSP_CHAR *qspNumToStr(QSP_CHAR *, long);
	QSP_CHAR *qspStrPos(QSP_CHAR *, QSP_CHAR *, QSP_BOOL);
	QSP_CHAR *qspFormatText(QSP_CHAR *);

#endif
