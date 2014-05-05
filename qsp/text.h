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

	extern QSPString qspNullString;
	extern QSPString qspEmptyString;

	static QSPString qspStringFromC(QSP_CHAR *s)
	{
		QSPString string;
		string.Str = s;
		while (*s) ++s;
		string.End = s;
		return string;
	}

	static QSPString qspStringFromPair(QSP_CHAR *start, QSP_CHAR *end)
	{
		QSPString string;
		string.Str = start;
		string.End = end;
		return string;
	}

	static QSPString qspStringFromLen(QSP_CHAR *s, int len)
	{
		QSPString string;
		string.Str = s;
		string.End = s + len;
		return string;
	}

	static QSPString qspStringFromString(QSPString s, int maxLen)
	{
		int len = (int)(s.End - s.Str);
		if (maxLen < len)
			s.End -= (len - maxLen);
		return s;
	}

	static int qspStrLen(QSPString s)
	{
		return (int)(s.End - s.Str);
	}

	static QSP_BOOL qspIsEmpty(QSPString s)
	{
		return (s.Str == s.End);
	}

	static void qspFreeString(QSPString s)
	{
		if (s.Str) free(s.Str);
	}

	/* Helpers */
	#define QSP_STATIC_LEN(x) (sizeof(x) / sizeof(QSP_CHAR) - 1)
	#define QSP_STATIC_STR(x) (qspStringFromLen(x, QSP_STATIC_LEN(x)))

	/* External functions */
	QSP_CHAR *qspStringToC(QSPString s);
	void qspAddText(QSPString *dest, QSPString val, QSP_BOOL isCreate);
	void qspUpdateText(QSPString *dest, QSPString val);
	QSPString qspGetNewText(QSPString val);
	QSPString qspGetAddText(QSPString dest, QSPString val);
	QSPString qspNewEmptyString();
	QSP_BOOL qspClearText(QSPString *s);
	QSP_BOOL qspIsInList(QSP_CHAR *list, QSP_CHAR ch);
	QSP_BOOL qspIsDigit(QSP_CHAR ch);
	void qspSkipSpaces(QSPString *s);
	QSPString qspDelSpc(QSPString s);
	QSP_BOOL qspIsAnyString(QSPString s);
	void qspLowerStr(QSPString *str);
	void qspUpperStr(QSPString *str);
	int qspStrsNComp(QSPString str1, QSPString str2, int maxLen);
	int qspStrsComp(QSPString str1, QSPString str2);
	QSP_CHAR *qspStrChar(QSPString str, QSP_CHAR ch);
	QSP_CHAR *qspStrStr(QSPString str, QSPString strSearch);
	QSP_CHAR *qspStrPBrk(QSPString str, QSP_CHAR *strCharSet);
	QSP_CHAR *qspInStrRChars(QSPString str, QSP_CHAR *chars);
	QSPString qspJoinStrs(QSPString *s, int count, QSPString delim);
	int qspSplitStr(QSPString str, QSPString delim, QSPString **res);
	void qspCopyStrs(QSPString **dest, QSPString *src, int start, int end);
	void qspFreeGameStrs(char **strs, int count);
	void qspFreeStrs(QSPString *strs, int count);
	QSP_BOOL qspIsNumber(QSPString s);
	int qspStrToNum(QSPString s, QSP_BOOL *isValid);
	QSPString qspNumToStr(QSP_CHAR *buf, int val);
	QSP_CHAR *qspStrPos(QSPString txt, QSPString str, QSP_BOOL isIsolated);
	QSPString qspReplaceText(QSPString txt, QSPString searchTxt, QSPString repTxt);
	QSPString qspFormatText(QSPString txt, QSP_BOOL canReturnSelf);
	int qspToWLower(int);
	int qspToWUpper(int);

#endif
