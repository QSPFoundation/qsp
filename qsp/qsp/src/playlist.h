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

#ifndef QSP_PLAYLISTDEFINES
	#define QSP_PLAYLISTDEFINES

	#define QSP_PLVOLUMEDELIM QSP_FMT("*")
	#define QSP_MAXPLFILES 500

	extern QSP_CHAR *qspPLFiles[QSP_MAXPLFILES];
	extern int qspPLFilesCount;

	/* External functions */
	void qspClearPlayList(QSP_BOOL);
	void qspPlayPLFiles();
	void qspRefreshPlayList();
	/* Statements */
	QSP_BOOL qspStatementPlayFile(QSPVariant *, int, QSP_CHAR **, int);
	QSP_BOOL qspStatementCloseFile(QSPVariant *, int, QSP_CHAR **, int);

#endif
