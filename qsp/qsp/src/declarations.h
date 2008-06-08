/* Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <wctype.h>
#include "qsp.h"

/* MEMWATCH */

#ifdef _DEBUG
	#define MEMWATCH
	#define MEMWATCH_STDIO

	#include "memwatch.h"
#endif

/* -------- */

#include "onig/oniguruma.h"

#ifndef QSP_DEFINES
	#define QSP_DEFINES

	#ifdef _UNICODE
		#define QSP_STRCPY wcscpy
		#define QSP_STRNCPY wcsncpy
		#define QSP_STRLEN wcslen
		#define QSP_STRSTR wcsstr
		#define QSP_STRCHR wcschr
		#define QSP_STRTOL wcstol
		#define QSP_CHRLWR towlower
		#define QSP_CHRUPR towupper
		#define QSP_STRCMP wcscmp
		#define QSP_STRCOLL wcscmp
		#define QSP_STRPBRK wcspbrk
		#define QSP_ISDIGIT iswdigit
		#define QSP_WCSTOMBSLEN(a) wcstombs(0, a, 0)
		#define QSP_WCSTOMBS wcstombs
		#define QSP_MBTOSB(a) ((a) % 256)
		#if defined(_WIN) || defined(_BEOS)
			#define QSP_ONIG_ENC ONIG_ENCODING_UTF16_LE
		#else
			#define QSP_ONIG_ENC ONIG_ENCODING_UTF32_LE
		#endif
		#define QSP_FROM_OS_CHAR(a) qspReverseConvertUC(a, qspCP1251ToUnicodeTable)
		#define QSP_TO_OS_CHAR(a) qspDirectConvertUC(a, qspCP1251ToUnicodeTable)
		#define QSP_WCTOB
		#define QSP_BTOWC
	#else
		#define QSP_STRCPY strcpy
		#define QSP_STRNCPY strncpy
		#define QSP_STRLEN strlen
		#define QSP_STRSTR strstr
		#define QSP_STRCHR strchr
		#define QSP_STRTOL strtol
		#define QSP_STRCMP strcmp
		#define QSP_STRPBRK strpbrk
		#define QSP_ISDIGIT isdigit
		#define QSP_WCSTOMBSLEN strlen
		#define QSP_WCSTOMBS strncpy
		#define QSP_MBTOSB
		#ifdef _WIN
			#define QSP_FROM_OS_CHAR
			#define QSP_TO_OS_CHAR
			#define QSP_WCTOB(a) qspReverseConvertUC(a, qspCP1251ToUnicodeTable)
			#define QSP_BTOWC(a) qspDirectConvertUC(a, qspCP1251ToUnicodeTable)
			#define QSP_CHRLWR(a) qspCP1251ToLowerTable[(unsigned char)(a)]
			#define QSP_CHRUPR(a) qspCP1251ToUpperTable[(unsigned char)(a)]
			#define QSP_STRCOLL(a, b) qspStrCmpSB(a, b, qspCP1251OrderTable)
			#define QSP_ONIG_ENC ONIG_ENCODING_CP1251
		#else
			#define QSP_FROM_OS_CHAR(a) qspReverseConvertSB(a, qspCP1251ToKOI8RTable)
			#define QSP_TO_OS_CHAR(a) qspDirectConvertSB(a, qspCP1251ToKOI8RTable)
			#define QSP_WCTOB(a) qspReverseConvertUC(a, qspKOI8RToUnicodeTable)
			#define QSP_BTOWC(a) qspDirectConvertUC(a, qspKOI8RToUnicodeTable)
			#define QSP_CHRLWR(a) qspKOI8RToLowerTable[(unsigned char)(a)]
			#define QSP_CHRUPR(a) qspKOI8RToUpperTable[(unsigned char)(a)]
			#define QSP_STRCOLL(a, b) qspStrCmpSB(a, b, qspKOI8ROrderTable)
			#define QSP_ONIG_ENC ONIG_ENCODING_KOI8_R
		#endif
	#endif
	#ifdef _WIN
		#define QSP_PATHDELIM QSP_FMT("\\")
	#else
		#define QSP_PATHDELIM QSP_FMT("/")
	#endif

	#define QSP_VER QSP_FMT("5.4.0")
	#define QSP_GAMEMINVER QSP_FMT("5.4.0")
	#define QSP_STRSDELIM QSP_FMT("\r\n")
	#define QSP_LOCALE "russian"
	#define QSP_GAMEID QSP_FMT("QSPGAME")
	#define QSP_SAVEDGAMEID QSP_FMT("QSPSAVEDGAME")
	#define QSP_EOLEXT QSP_FMT(" _")
	#define QSP_LSUBEX QSP_FMT("<<")
	#define QSP_RSUBEX QSP_FMT(">>")
	#define QSP_LABEL QSP_FMT(":")
	#define QSP_COMMENT QSP_FMT("!")
	#define QSP_STRCHAR QSP_FMT("$")
	#define QSP_QUOTS QSP_FMT("'\"")
	#define QSP_STATDELIM QSP_FMT("&")
	#define QSP_COLONDELIM QSP_FMT(":")
	#define QSP_MENUDELIM QSP_FMT(":")
	#define QSP_PLVOLUMEDELIM QSP_FMT("*")
	#define QSP_PLFILEDELIM QSP_FMT("|")
	#define QSP_SPACES QSP_FMT(" \t")
	#define QSP_COMMA QSP_FMT(",")
	#define QSP_EQUAL QSP_FMT("=")
	#define QSP_NOTEQUAL1 QSP_FMT("!")
	#define QSP_NOTEQUAL2 QSP_FMT("<>")
	#define QSP_LESS QSP_FMT("<")
	#define QSP_GREAT QSP_FMT(">")
	#define QSP_LESSEQ1 QSP_FMT("<=")
	#define QSP_LESSEQ2 QSP_FMT("=<")
	#define QSP_GREATEQ1 QSP_FMT(">=")
	#define QSP_GREATEQ2 QSP_FMT("=>")
	#define QSP_LSBRACK QSP_FMT("[")
	#define QSP_RSBRACK QSP_FMT("]")
	#define QSP_LRBRACK QSP_FMT("(")
	#define QSP_RRBRACK QSP_FMT(")")
	#define QSP_APPEND QSP_FMT("&")
	#define QSP_UPLUS QSP_FMT("+")
	#define QSP_UMINUS QSP_FMT("-")
	#define QSP_ADD QSP_FMT("+")
	#define QSP_SUB QSP_FMT("-")
	#define QSP_DIV QSP_FMT("/")
	#define QSP_MUL QSP_FMT("*")
	#define QSP_DELIMS \
		QSP_SPACES QSP_STATDELIM QSP_QUOTS QSP_LRBRACK QSP_RRBRACK QSP_LSBRACK \
		QSP_RSBRACK QSP_EQUAL QSP_NOTEQUAL1 QSP_NOTEQUAL2 QSP_LESS QSP_GREAT \
		QSP_LESSEQ1 QSP_LESSEQ2 QSP_GREATEQ1 QSP_GREATEQ2 QSP_ADD QSP_SUB \
		QSP_DIV QSP_MUL QSP_UPLUS QSP_UMINUS QSP_COLONDELIM QSP_COMMA \
		QSP_APPEND QSP_LABEL QSP_COMMENT
	#define QSP_MAXACTIONS 50
	#define QSP_MAXOBJECTS 1000
	#define QSP_MAXMENUITEMS 100
	#define QSP_MAXINCFILES 100
	#define QSP_DEFTIMERINTERVAL 500
	#define QSP_CODREMOV 5
	#define QSP_VARSSEEK 50
	#define QSP_VARSCOUNT 256 * QSP_VARSSEEK

	/* Types */
	typedef struct
	{
		QSP_CHAR *Image;
		QSP_CHAR *Desc;
	} QSPObj;
	typedef struct
	{
		QSP_CHAR *Image;
		QSP_CHAR *Desc;
		QSP_CHAR **OnPressLines;
		long OnPressLinesCount;
		long Location;
		long Where;
		long StartLine;
	} QSPCurAct;
	typedef struct
	{
		QSP_CHAR *Image;
		QSP_CHAR *Desc;
		QSP_CHAR **OnPressLines;
		long OnPressLinesCount;
	} QSPLocAct;
	typedef struct
	{
		QSP_CHAR *Name;
		QSP_CHAR *Desc;
		QSP_CHAR **OnVisitLines;
		long OnVisitLinesCount;
		QSPLocAct Actions[QSP_MAXACTIONS];
	} QSPLocation;
	typedef struct
	{
		QSP_CHAR *Name;
		long *Value;
		QSP_CHAR **TextValue;
		long ValsCount;
		QSP_CHAR **TextIndex;
		long IndsCount;
		long Type;
	} QSPVar;
	typedef struct
	{
		union
		{
			QSP_CHAR *Str;
			long Num;
		};
		QSP_BOOL IsStr;
	} QSPVariant;

	/* Helpers */
	#define QSP_LEN(x) (sizeof(x) / sizeof(QSP_CHAR) - 1)

	/* Variables */
	extern volatile QSP_BOOL qspIsMustWait;
	extern QSP_BOOL qspIsMainDescChanged;
	extern QSP_BOOL qspIsVarsDescChanged;
	extern QSP_BOOL qspIsObjectsChanged;
	extern QSP_BOOL qspIsActionsChanged;
	extern long qspRefreshCount;
	extern long qspFullRefreshCount;
	extern long qspErrorNum;
	extern long qspErrorLoc;
	extern long qspErrorLine;
	extern long qspErrorWhere;
	extern long qspRealCurLoc;
	extern long qspRealLine;
	extern long qspRealWhere;
	extern QSPVar qspVars[QSP_VARSCOUNT];
	extern QSPLocation *qspLocs;
	extern long qspLocsCount;
	extern long qspCurLoc;
	extern QSPObj qspCurObjects[QSP_MAXOBJECTS];
	extern long qspCurObjectsCount;
	extern long qspCurSelObject;
	extern QSP_CHAR *qspCurMenuLocs[QSP_MAXMENUITEMS];
	extern long qspCurMenuItems;
	extern QSP_CHAR *qspCurDesc;
	extern long qspCurDescLen;
	extern QSP_CHAR *qspCurVars;
	extern long qspCurVarsLen;
	extern QSP_CHAR *qspCurInput;
	extern long qspCurInputLen;
	extern QSP_CHAR *qspPlayList;
	extern long qspPlayListLen;
	extern QSPCurAct qspCurActions[QSP_MAXACTIONS];
	extern long qspCurActionsCount;
	extern long qspCurSelAction;
	extern long qspMSCount;
	extern QSP_BOOL qspCurIsShowObjs;
	extern QSP_BOOL qspCurIsShowActs;
	extern QSP_BOOL qspCurIsShowVars;
	extern QSP_BOOL qspCurIsShowInput;
	extern QSP_CHAR *qspQstPath;
	extern long qspQstPathLen;
	extern QSP_CHAR *qspQstFullPath;
	extern long qspQstCRC;

	/* Tables */
	extern unsigned char qspCP1251ToUpperTable[];
	extern unsigned char qspCP1251ToLowerTable[];
	extern unsigned char qspKOI8RToUpperTable[];
	extern unsigned char qspKOI8RToLowerTable[];
	extern unsigned char qspCP1251OrderTable[];
	extern unsigned char qspKOI8ROrderTable[];

	/* Functions */
	/* ---------------------------------------------------------------- actions.c */
	void qspClearActions(QSP_BOOL);
	void qspAddAction(QSPVariant *, long, QSP_CHAR **, long, long, QSP_BOOL);
	void qspExecAction(long);
	/* -- */
	void qspStatementAddAct(QSP_CHAR *);
	QSP_BOOL qspStatementDelAct(QSPVariant *, long, QSP_CHAR **, char);
	/* ---------------------------------------------------------------- callbacks.c */
	void qspInitCallBacks();
	void qspSetCallBack(long, QSP_CALLBACK);
	void qspCallSetTimer(long);
	void qspCallRefreshInt(QSP_BOOL);
	void qspCallSetInputStrText(QSP_CHAR *);
	void qspCallAddMenuItem(QSP_CHAR *, QSP_CHAR *);
	void qspCallSystem(QSP_CHAR *);
	void qspCallOpenGame();
	void qspCallSaveGame();
	void qspCallShowMessage(QSP_CHAR *);
	void qspCallShowMenu();
	void qspCallShowPicture(QSP_CHAR *);
	void qspCallShowWindow(long, QSP_BOOL);
	void qspCallPlayFile(QSP_CHAR *, long);
	QSP_BOOL qspCallIsPlayingFile(QSP_CHAR *);
	void qspCallSleep(long);
	long qspCallGetMSCount();
	void qspCallCloseFile(QSP_CHAR *);
	void qspCallDeleteMenu();
	QSP_CHAR *qspCallInputBox(QSP_CHAR *);
	/* ---------------------------------------------------------------- codetools.c */
	long qspPreprocessData(QSP_CHAR *, QSP_CHAR ***);
	/* ---------------------------------------------------------------- coding.c */
	int qspStrCmpSB(char *, char *, unsigned char *);
	QSP_CHAR *qspCodeReCode(QSP_CHAR *, QSP_BOOL);
	char *qspFromQSPString(QSP_CHAR *);
	QSP_CHAR *qspGameToQSPString(char *, QSP_BOOL, QSP_BOOL);
	long qspSplitGameStr(char *, QSP_BOOL, QSP_CHAR *, char ***);
	long qspReCodeGetIntVal(QSP_CHAR *);
	long qspCodeWriteIntVal(QSP_CHAR **, long, long, QSP_BOOL);
	long qspCodeWriteVal(QSP_CHAR **, long, QSP_CHAR *, QSP_BOOL);
	/* ---------------------------------------------------------------- common.c */
	void qspSetError(long);
	void qspResetError();
	void qspPrepareExecution();
	void qspClearMenu(QSP_BOOL);
	void qspMemClear(QSP_BOOL);
	void qspRefresh(QSP_BOOL);
	QSP_CHAR *qspFormatText(QSP_CHAR *);
	/* ---------------------------------------------------------------- game.c */
	void qspClearIncludes(QSP_BOOL);
	void qspNewGame(QSP_BOOL);
	void qspOpenQuest(QSP_CHAR *, QSP_BOOL);
	void qspSaveGameStatus(QSP_CHAR *);
	void qspOpenGameStatus(QSP_CHAR *);
	/* -- */
	QSP_BOOL qspStatementOpenQst(QSPVariant *, long, QSP_CHAR **, char);
	QSP_BOOL qspStatementOpenGame(QSPVariant *, long, QSP_CHAR **, char);
	QSP_BOOL qspStatementSaveGame(QSPVariant *, long, QSP_CHAR **, char);
	/* ---------------------------------------------------------------- locations.c */
	void qspCreateWorld(long, long);
	long qspLocIndex(QSP_CHAR *);
	long qspLocIndexByVarName(QSP_CHAR *);
	void qspExecLocByIndex(long, QSP_BOOL);
	void qspExecLocByName(QSP_CHAR *, QSP_BOOL);
	void qspExecLocByVarName(QSP_CHAR *);
	/* ---------------------------------------------------------------- math.c */
	void qspInitMath();
	QSPVariant qspExprValue(QSP_CHAR *);
	/* ---------------------------------------------------------------- objects.c */
	void qspClearObjects(QSP_BOOL);
	void qspClearObjectsWithNotify();
	long qspObjIndex(QSP_CHAR *);
	/* -- */
	QSP_BOOL qspStatementAddObject(QSPVariant *, long, QSP_CHAR **, char);
	QSP_BOOL qspStatementDelObj(QSPVariant *, long, QSP_CHAR **, char);
	QSP_BOOL qspStatementUnSelect(QSPVariant *, long, QSP_CHAR **, char);
	/* ---------------------------------------------------------------- playlist.c */
	void qspPlayPLFiles();
	void qspRefreshPlayList();
	/* -- */
	QSP_BOOL qspStatementPlayFile(QSPVariant *, long, QSP_CHAR **, char);
	QSP_BOOL qspStatementCloseFile(QSPVariant *, long, QSP_CHAR **, char);
	/* ---------------------------------------------------------------- statements.c */
	void qspInitStats();
	long qspGetStatArgs(QSP_CHAR *, long, QSPVariant *);
	QSP_BOOL qspExecString(QSP_CHAR *, QSP_CHAR **);
	QSP_BOOL qspExecCode(QSP_CHAR **, long, long, long, QSP_CHAR **, QSP_BOOL);
	/* ---------------------------------------------------------------- text.c */
	long qspAddText(QSP_CHAR **, QSP_CHAR *, long, long, QSP_BOOL);
	QSP_CHAR *qspGetNewText(QSP_CHAR *, long);
	QSP_CHAR *qspGetAddText(QSP_CHAR *, QSP_CHAR *, long, long);
	QSP_BOOL qspClearText(void **, long *);
	QSP_BOOL qspIsInList(QSP_CHAR *, QSP_CHAR);
	QSP_BOOL qspIsInListEOL(QSP_CHAR *, QSP_CHAR);
	QSP_CHAR *qspSkipSpaces(QSP_CHAR *);
	QSP_CHAR *qspStrEnd(QSP_CHAR *);
	QSP_CHAR *qspDelSpc(QSP_CHAR *);
	QSP_BOOL qspIsAnyString(QSP_CHAR *);
	void qspLowerStr(QSP_CHAR *);
	void qspUpperStr(QSP_CHAR *);
	QSP_BOOL qspIsEqual(QSP_CHAR *, QSP_CHAR *, long);
	QSP_CHAR *qspInStrRChar(QSP_CHAR *, QSP_CHAR, QSP_CHAR *);
	QSP_CHAR *qspJoinStrs(QSP_CHAR **, long, QSP_CHAR *);
	long qspSplitStr(QSP_CHAR *, QSP_CHAR *, QSP_CHAR ***);
	void qspCopyStrs(QSP_CHAR ***, QSP_CHAR **, long, long);
	void qspFreeStrs(void **, long, QSP_BOOL);
	long qspStrToNum(QSP_CHAR *, QSP_CHAR **);
	QSP_CHAR *qspNumToStr(QSP_CHAR *, long);
	QSP_CHAR *qspStrPos(QSP_CHAR *, QSP_CHAR *, QSP_BOOL);
	/* ---------------------------------------------------------------- variables.c */
	void qspClearVars(QSP_BOOL);
	void qspInitVars();
	long qspVarIndex(QSP_CHAR *, QSP_BOOL);
	void qspSetVarValueByIndex(long, long, QSPVariant, QSP_BOOL);
	QSPVariant qspGetVarValueByName(QSP_CHAR *);
	QSPVariant qspGetVar(QSP_CHAR *);
	long qspArrayPos(QSP_CHAR *, long, QSPVariant, QSP_BOOL);
	long qspGetVarsCount();
	/* -- */
	void qspStatementSetVarValue(QSP_CHAR *);
	QSP_BOOL qspStatementCopyArr(QSPVariant *, long, QSP_CHAR **, char);
	/* ---------------------------------------------------------------- variant.c */
	void qspFreeVariants(QSPVariant *, long);
	QSPVariant qspGetEmptyVariant(QSP_BOOL);
	QSPVariant qspConvertVariantTo(QSPVariant, QSP_BOOL, QSP_BOOL, QSP_BOOL *);
	void qspCopyVariant(QSPVariant *, QSPVariant);
	QSP_BOOL qspIsCanConvertToNum(QSPVariant);
	int qspAutoConvertCompare(QSPVariant, QSPVariant);

#endif
