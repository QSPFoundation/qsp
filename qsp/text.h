/* Copyright (C) 2001-2024 Val Argunov (byte AT qsp DOT org) */
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

    /* Frequently used classes of characters */
    enum
    {
        QSP_CHAR_SPACE = 1 << 0, /* spaces */
        QSP_CHAR_QUOT = 1 << 1, /* quotes */
        QSP_CHAR_DIGIT = 1 << 2, /* digits */
        QSP_CHAR_DELIM = 1 << 3, /* delimiters */
        QSP_CHAR_SIMPLEOP = 1 << 4, /* simple math operations */
        QSP_CHAR_EXPSTART = 1 << 5 /* beginning of an expression */
    };

    typedef struct
    {
        QSP_CHAR *Str;
        int Len;
        int Capacity;
        int CapacityIncrement;
    } QSPBufString;

    extern QSPString qspNullString;
    extern unsigned char qspAsciiClasses[128];

    /* External functions */
    void qspInitSymbolClasses(void);
    QSP_CHAR *qspStringToC(QSPString s);
    QSP_BOOL qspAddText(QSPString *dest, QSPString val, QSP_BOOL toCreate);
    QSP_BOOL qspAddBufText(QSPBufString *dest, QSPString val);
    QSP_CHAR *qspInStrRChars(QSPString str, QSP_CHAR *chars);
    QSPString qspJoinStrs(QSPString *s, int count, QSPString delim);
    int qspSplitStr(QSPString str, QSPString delim, QSPString **res);
    void qspCopyStrs(QSPString **dest, QSPString *src, int start, int end);
    void qspFreeStrs(QSPString *strs, int count);
    QSP_BOOL qspIsStrNumber(QSPString s);
    QSP_BIGINT qspStrToNum(QSPString s, QSP_BOOL *isValid);
    QSPString qspNumToStr(QSP_CHAR *buf, QSP_BIGINT val);
    QSP_CHAR *qspDelimPos(QSPString txt, QSP_CHAR ch);
    QSP_CHAR *qspStrPos(QSPString txt, QSPString str, QSP_BOOL isIsolated);
    QSPString qspReplaceText(QSPString txt, QSPString searchTxt, QSPString repTxt, QSP_BOOL canReturnSelf);
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
    #if defined(__GNUC__)
        return (QSPString) { start, end };
    #else
        QSPString string;
        string.Str = start;
        string.End = end;
        return string;
    #endif
    }

    INLINE QSPString qspStringFromLen(QSP_CHAR *s, int len)
    {
    #if defined(__GNUC__)
        return (QSPString) { s, (s + len) };
    #else
        QSPString string;
        string.Str = s;
        string.End = s + len;
        return string;
    #endif
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

    INLINE void qspFreeString(QSPString *s)
    {
        if (s->Str) free(s->Str);
    }

    INLINE void qspFreeNewString(QSPString *s, QSPString *old)
    {
        if (s->Str && s->Str != old->Str) free(s->Str);
    }

    INLINE void qspClearText(QSPString *s)
    {
        if (s->Str)
        {
            free(s->Str);
            s->Str = s->End = 0; /* assign the null string */
        }
    }

    INLINE void qspUpdateText(QSPString *dest, QSPString val)
    {
        qspFreeString(dest);
        qspAddText(dest, val, QSP_TRUE);
    }

    INLINE QSPString qspCopyToNewText(QSPString s)
    {
        QSPString string;
        qspAddText(&string, s, QSP_TRUE);
        return string;
    }

    INLINE QSPString qspMoveText(QSPString *s)
    {
        QSPString string = *s;
        s->Str = s->End = 0; /* assign the null string */
        return string;
    }

    INLINE QSP_BOOL qspIsCharAtPos(QSPString str, QSP_CHAR *pos, QSP_CHAR ch)
    {
        return (pos < str.End && *pos == ch);
    }

    INLINE QSP_BOOL qspIsInList(QSP_CHAR ch, QSP_CHAR *list)
    {
        while (*list)
            if (*list++ == ch) return QSP_TRUE;
        return QSP_FALSE;
    }

    INLINE QSP_BOOL qspIsInClass(QSP_CHAR ch, int charClass)
    {
        if (ch < sizeof(qspAsciiClasses))
            return (qspAsciiClasses[ch] & charClass) != 0;

        return QSP_FALSE;
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
            *pos = (QSP_CHAR)QSP_CHRLWR(*pos);
            ++pos;
        }
    }

    INLINE void qspUpperStr(QSPString *str)
    {
        QSP_CHAR *pos = str->Str;
        while (pos < str->End)
        {
            *pos = (QSP_CHAR)QSP_CHRUPR(*pos);
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
            if (qspIsInList(*pos, strCharSet))
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

    INLINE QSPBufString qspNewBufString(int capacityIncrement)
    {
        QSPBufString res;
        res.Str = 0;
        res.Len = 0;
        res.Capacity = 0;
        res.CapacityIncrement = capacityIncrement;
        return res;
    }

    INLINE QSPBufString qspStringToBufString(QSPString str, int capacityIncrement)
    {
        QSPBufString res;
        res.Str = str.Str;
        res.Len = qspStrLen(str);
        res.Capacity = res.Len;
        res.CapacityIncrement = capacityIncrement;
        return res;
    }

    INLINE QSPString qspBufTextToString(QSPBufString buf)
    {
    #if defined(__GNUC__)
        return (QSPString) { buf.Str, (buf.Str + buf.Len) };
    #else
        QSPString res;
        res.Str = buf.Str;
        res.End = buf.Str + buf.Len;
        return res;
    #endif
    }

    INLINE void qspFreeBufString(QSPBufString *buf)
    {
        if (buf->Str) free(buf->Str);
    }

    INLINE void qspClearBufString(QSPBufString *s)
    {
        if (s->Str)
        {
            free(s->Str);
            s->Str = 0;
            s->Len = 0;
            s->Capacity = 0;
            /* Keep old CapacityIncrement */
        }
    }

#endif
