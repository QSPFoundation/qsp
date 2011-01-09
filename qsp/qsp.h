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

#ifndef QSP_H
	#define QSP_H

	#ifdef EXPORT
		#ifdef _WIN
			#define QSP_EXTERN __declspec(dllexport)
		#else
			#define QSP_EXTERN extern
		#endif
	#else
		#define QSP_EXTERN
	#endif

	enum
	{
		QSP_ERR_DIVBYZERO = 100,
		QSP_ERR_TYPEMISMATCH,
		QSP_ERR_STACKOVERFLOW,
		QSP_ERR_TOOMANYITEMS,
		QSP_ERR_FILENOTFOUND,
		QSP_ERR_CANTLOADFILE,
		QSP_ERR_GAMENOTLOADED,
		QSP_ERR_COLONNOTFOUND,
		QSP_ERR_CANTINCFILE,
		QSP_ERR_CANTADDACTION,
		QSP_ERR_EQNOTFOUND,
		QSP_ERR_LOCNOTFOUND,
		QSP_ERR_ENDNOTFOUND,
		QSP_ERR_LABELNOTFOUND,
		QSP_ERR_NOTCORRECTNAME,
		QSP_ERR_QUOTNOTFOUND,
		QSP_ERR_BRACKNOTFOUND,
		QSP_ERR_BRACKSNOTFOUND,
		QSP_ERR_SYNTAX,
		QSP_ERR_UNKNOWNACTION,
		QSP_ERR_ARGSCOUNT,
		QSP_ERR_CANTADDOBJECT,
		QSP_ERR_CANTADDMENUITEM,
		QSP_ERR_TOOMANYVARS,
		QSP_ERR_INCORRECTREGEXP,
		QSP_ERR_CODENOTFOUND,
		QSP_ERR_TONOTFOUND
	};

	enum
	{
		QSP_WIN_ACTS,
		QSP_WIN_OBJS,
		QSP_WIN_VARS,
		QSP_WIN_INPUT
	};

	enum
	{
		QSP_CALL_DEBUG, /* void func(const QSP_CHAR *str) */
		QSP_CALL_ISPLAYINGFILE, /* QSP_BOOL func(const QSP_CHAR *file) */
		QSP_CALL_PLAYFILE, /* void func(const QSP_CHAR *file, int volume) */
		QSP_CALL_CLOSEFILE, /* void func(const QSP_CHAR *file) */
		QSP_CALL_SHOWIMAGE, /* void func(const QSP_CHAR *file) */
		QSP_CALL_SHOWWINDOW, /* void func(int type, QSP_BOOL isShow) */
		QSP_CALL_DELETEMENU, /* void func() */
		QSP_CALL_ADDMENUITEM, /* void func(const QSP_CHAR *name, const QSP_CHAR *imgPath) */
		QSP_CALL_SHOWMENU, /* int func() */
		QSP_CALL_SHOWMSGSTR, /* void func(const QSP_CHAR *str) */
		QSP_CALL_REFRESHINT, /* void func(QSP_BOOL isRedraw) */
		QSP_CALL_SETTIMER, /* void func(int msecs) */
		QSP_CALL_SETINPUTSTRTEXT, /* void func(const QSP_CHAR *text) */
		QSP_CALL_SYSTEM, /* void func(const QSP_CHAR *str) */
		QSP_CALL_OPENGAMESTATUS, /* void func(const QSP_CHAR *file) */
		QSP_CALL_SAVEGAMESTATUS, /* void func(const QSP_CHAR *file) */
		QSP_CALL_SLEEP, /* void func(int msecs) */
		QSP_CALL_GETMSCOUNT, /* int func() */
		QSP_CALL_INPUTBOX, /* void func(const QSP_CHAR *text, QSP_CHAR *buffer, int maxLen) */
		QSP_CALL_DUMMY
	};

	#ifdef _UNICODE
		#define QSP_FMT2(x) L##x
		#define QSP_FMT(x) QSP_FMT2(x)
	#else
		typedef char QSP_CHAR;
		#define QSP_FMT(x) x
	#endif

	typedef int QSP_BOOL;

	#define QSP_TRUE 1
	#define QSP_FALSE 0

#endif
