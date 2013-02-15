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

#ifndef QSP_TEXTDEFINES
	#define QSP_TEXTDEFINES

	#define QSP_STRSDELIM QSP_FMT("\r\n")
	#define QSP_LSUBEX QSP_FMT("<<")
	#define QSP_RSUBEX QSP_FMT(">>")

	/* Helpers */
	#define QSP_LEN(x) (sizeof(x) / sizeof(QSP_CHAR) - 1)

	/* External functions */
	int qspAddText(QSP_CHAR **, QSP_CHAR *, int, int, QSP_BOOL);
	QSP_CHAR *qspGetNewText(QSP_CHAR *, int);
	QSP_CHAR *qspGetAddText(QSP_CHAR *, QSP_CHAR *, int, int);
	QSP_BOOL qspClearText(void **, int *);
	QSP_BOOL qspIsInList(QSP_CHAR *, QSP_CHAR);
	QSP_BOOL qspIsInListEOL(QSP_CHAR *, QSP_CHAR);
	QSP_BOOL qspIsDigit(QSP_CHAR);
	QSP_CHAR *qspSkipSpaces(QSP_CHAR *);
	QSP_CHAR *qspStrEnd(QSP_CHAR *);
	QSP_CHAR *qspDelSpc(QSP_CHAR *);
	QSP_CHAR *qspDelSpcCanRetSelf(QSP_CHAR *);
	QSP_BOOL qspIsAnyString(QSP_CHAR *);
	void qspLowerStr(QSP_CHAR *);
	void qspUpperStr(QSP_CHAR *);
	int qspStrsNComp(QSP_CHAR *, QSP_CHAR *, int);
	int qspStrsComp(QSP_CHAR *, QSP_CHAR *);
	QSP_CHAR *qspStrCopy(QSP_CHAR *, QSP_CHAR *);
	QSP_CHAR *qspStrChar(QSP_CHAR *, QSP_CHAR);
	QSP_CHAR *qspStrNCopy(QSP_CHAR *, QSP_CHAR *, int);
	int qspStrLen(QSP_CHAR *);
	QSP_CHAR *qspStrStr(QSP_CHAR *, QSP_CHAR *);
	QSP_CHAR *qspStrPBrk(QSP_CHAR *, QSP_CHAR *);
	QSP_CHAR *qspInStrRChars(QSP_CHAR *, QSP_CHAR *, QSP_CHAR *);
	QSP_CHAR *qspJoinStrs(QSP_CHAR **, int, QSP_CHAR *);
	int qspSplitStr(QSP_CHAR *, QSP_CHAR *, QSP_CHAR ***);
	void qspCopyStrs(QSP_CHAR ***, QSP_CHAR **, int, int);
	void qspFreeStrs(void **, int);
	QSP_BOOL qspIsNumber(QSP_CHAR *);
	int qspStrToNum(QSP_CHAR *, QSP_BOOL *);
	QSP_CHAR *qspNumToStr(QSP_CHAR *, int);
	QSP_CHAR *qspStrPos(QSP_CHAR *, QSP_CHAR *, QSP_BOOL);
	QSP_CHAR *qspStrPosPartial(QSP_CHAR *, QSP_CHAR *, QSP_CHAR *, QSP_BOOL);
	QSP_CHAR *qspReplaceText(QSP_CHAR *, QSP_CHAR *, QSP_CHAR *);
	QSP_CHAR *qspFormatText(QSP_CHAR *, QSP_BOOL);

	#ifdef _UNICODE
		int qspToWLower(int);
		int qspToWUpper(int);
	#endif

#endif
