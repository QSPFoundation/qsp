/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
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
#include "coding.h"

#ifndef QSP_TEXTDEFINES
    #define QSP_TEXTDEFINES

    #define QSP_STRSDELIM QSP_FMT("\r\n")
    #define QSP_LSUBEX QSP_FMT("<<")
    #define QSP_RSUBEX QSP_FMT(">>")

    /* Frequently used classes of characters */
    enum
    {
        QSP_CHAR_SPACE = 1 << 0, /* QSP_SPACES */
        QSP_CHAR_QUOT = 1 << 1, /* QSP_QUOTS */
        QSP_CHAR_DELIM = 1 << 2, /* QSP_DELIMS */
        QSP_CHAR_SIMPLEOP = 1 << 3, /* QSP_ADD QSP_SUB QSP_DIV QSP_MUL */
        QSP_CHAR_EXPSTART = 1 << 4 /* QSP_LQUOT QSP_LRBRACK QSP_LSBRACK */
    };

    static const unsigned char qspAsciiClasses[] =
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x04, 0x06, 0x00, 0x00, 0x00, 0x04, 0x06,
        0x14, 0x04, 0x0c, 0x0c, 0x04, 0x0c, 0x00, 0x0c,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x14, 0x00, 0x04, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x14, 0x00, 0x04, 0x00, 0x00,
    };

    extern QSPString qspNullString;

    /* External functions */
    QSP_CHAR *qspStringToC(QSPString s);
    void qspAddText(QSPString *dest, QSPString val, QSP_BOOL isCreate);
    QSP_BOOL qspClearText(QSPString *s);
    QSP_CHAR *qspInStrRChars(QSPString str, QSP_CHAR *chars);
    QSPString qspJoinStrs(QSPString *s, int count, QSPString delim);
    int qspSplitStr(QSPString str, QSPString delim, QSPString **res);
    void qspCopyStrs(QSPString **dest, QSPString *src, int start, int end);
    void qspFreeStrs(QSPString *strs, int count);
    QSP_BOOL qspIsNumber(QSPString s);
    int qspStrToNum(QSPString s, QSP_BOOL *isValid);
    QSPString qspNumToStr(QSP_CHAR *buf, int val);
    QSP_CHAR *qspDelimPos(QSPString txt, QSP_CHAR ch);
    QSP_CHAR *qspStrPos(QSPString txt, QSPString str, QSP_BOOL isIsolated);
    QSPString qspReplaceText(QSPString txt, QSPString searchTxt, QSPString repTxt);
    QSPString qspFormatText(QSPString txt, QSP_BOOL canReturnSelf);
    int qspToWLower(int);
    int qspToWUpper(int);

    INLINE QSPString qspStringFromC(QSP_CHAR *s)
    {
        QSPString string;
        string.Str = s;
        while (*s) ++s;
        string.End = s;
        return string;
    }

    INLINE QSPString qspStringFromPair(QSP_CHAR *start, QSP_CHAR *end)
    {
        QSPString string;
        string.Str = start;
        string.End = end;
        return string;
    }

    INLINE QSPString qspStringFromLen(QSP_CHAR *s, int len)
    {
        QSPString string;
        string.Str = s;
        string.End = s + len;
        return string;
    }

    INLINE QSPString qspStringFromString(QSPString s, int maxLen)
    {
        int len = (int)(s.End - s.Str);
        if (maxLen < len)
            s.End = s.Str + maxLen;
        return s;
    }

    INLINE int qspStrLen(QSPString s)
    {
        return (int)(s.End - s.Str);
    }

    INLINE QSP_BOOL qspIsEmpty(QSPString s)
    {
        return (s.Str == s.End);
    }

    INLINE void qspFreeString(QSPString s)
    {
        if (s.Str) free(s.Str);
    }

    INLINE void qspUpdateText(QSPString *dest, QSPString val)
    {
        dest->End = dest->Str;
        qspAddText(dest, val, QSP_FALSE);
    }

    INLINE QSPString qspGetNewText(QSPString val)
    {
        QSPString string;
        qspAddText(&string, val, QSP_TRUE);
        return string;
    }

    INLINE QSP_BOOL qspIsInList(QSP_CHAR *list, QSP_CHAR ch)
    {
        while (*list)
            if (*list++ == ch) return QSP_TRUE;
        return QSP_FALSE;
    }

    INLINE QSP_BOOL qspIsInClass(QSP_CHAR ch, int charClass)
    {
        if (ch >= 128)
            return QSP_FALSE;

        return (qspAsciiClasses[ch] & charClass) != 0;
    }

    INLINE QSP_BOOL qspIsAnyInClass(QSPString str, int charClass)
    {
        QSP_CHAR *pos = str.Str;
        while (pos < str.End)
        {
            if (qspIsInClass(*pos, charClass))
                return QSP_TRUE;
            ++pos;
        }
        return QSP_FALSE;
    }

    INLINE QSP_BOOL qspIsDigit(QSP_CHAR ch)
    {
        return (ch >= QSP_FMT('0') && ch <= QSP_FMT('9'));
    }

    INLINE void qspSkipSpaces(QSPString *s)
    {
        QSP_CHAR *pos = s->Str;
        while (pos < s->End && qspIsInClass(*pos, QSP_CHAR_SPACE)) ++pos;
        s->Str = pos;
    }

    INLINE QSPString qspDelSpc(QSPString s)
    {
        QSP_CHAR *begin = s.Str, *end = s.End;
        while (begin < end && qspIsInClass(*begin, QSP_CHAR_SPACE)) ++begin;
        while (begin < end && qspIsInClass(*(end - 1), QSP_CHAR_SPACE)) --end;
        return qspStringFromPair(begin, end);
    }

    INLINE QSP_BOOL qspIsAnyString(QSPString s)
    {
        qspSkipSpaces(&s);
        return (s.Str != s.End);
    }

    INLINE void qspLowerStr(QSPString *str)
    {
        QSP_CHAR *pos = str->Str;
        while (pos < str->End)
        {
            *pos = QSP_CHRLWR(*pos);
            ++pos;
        }
    }

    INLINE void qspUpperStr(QSPString *str)
    {
        QSP_CHAR *pos = str->Str;
        while (pos < str->End)
        {
            *pos = QSP_CHRUPR(*pos);
            ++pos;
        }
    }

    INLINE int qspStrsNComp(QSPString str1, QSPString str2, int maxLen)
    {
        int delta = 0;
        QSP_CHAR *pos1 = str1.Str, *pos2 = str2.Str;
        while (maxLen-- && pos2 < str2.End && pos1 < str1.End && !(delta = (int)*pos1 - *pos2))
            ++pos1, ++pos2;
        return delta;
    }

    INLINE int qspStrsComp(QSPString str1, QSPString str2)
    {
        int delta = 0;
        QSP_CHAR *pos1 = str1.Str, *pos2 = str2.Str;
        while (pos2 < str2.End && pos1 < str1.End && !(delta = (int)*pos1 - *pos2))
            ++pos1, ++pos2;
        if (delta) return delta;
        return (pos1 == str1.End) ? ((pos2 == str2.End) ? 0 : -1) : 1;
    }

    INLINE QSP_CHAR *qspStrChar(QSPString str, QSP_CHAR ch)
    {
        QSP_CHAR *pos = str.Str;
        while (pos < str.End && *pos != ch) ++pos;
        if (*pos == ch) return pos;
        return 0;
    }

    INLINE QSP_CHAR *qspStrPBrk(QSPString str, QSP_CHAR *strCharSet)
    {
        QSP_CHAR *pos = str.Str;
        while (pos < str.End)
        {
            if (qspIsInList(strCharSet, *pos))
                return pos;
            ++pos;
        }
        return 0;
    }

    INLINE QSP_CHAR *qspStrStr(QSPString str, QSPString strSearch)
    {
        QSP_CHAR *s1, *s2, *pos = str.Str;
        while (pos < str.End)
        {
            s1 = pos;
            s2 = strSearch.Str;
            while (s1 < str.End && s2 < strSearch.End && !((int)*s1 - *s2))
                ++s1, ++s2;
            if (s2 == strSearch.End) return pos;
            ++pos;
        }
        return 0;
    }

#endif
