/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
        QSP_CHAR_LBRACKET = 1 << 5, /* opening brackets of an expression */
        QSP_CHAR_TYPEPREFIX = 1 << 6 /* type prefix */
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
    QSPString qspConcatText(QSPString val1, QSPString val2);
    QSPString qspJoinStrs(QSPString *s, int count, QSPString delim);
    int qspSplitStr(QSPString str, QSPString delim, QSPString **res);
    void qspCopyStrs(QSPString **dest, QSPString *src, int start, int end);
    void qspFreeStrs(QSPString *strs, int count);
    QSP_BOOL qspIsStrNumber(QSPString s);
    QSP_BIGINT qspStrToNum(QSPString s, QSP_BOOL *isValid);
    QSPString qspNumToStr(QSP_CHAR *buf, QSP_BIGINT val);
    QSP_CHAR *qspDelimPos(QSPString txt, QSP_CHAR ch);
    QSP_CHAR *qspStrPos(QSPString txt, QSPString str, QSP_BOOL isIsolated);
    QSPString qspReplaceText(QSPString txt, QSPString searchTxt, QSPString repTxt, int maxReplacements, QSP_BOOL canReturnSelf);
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

    INLINE void qspFreeNewString(QSPString *strToRelease, QSPString *strToKeep)
    {
        if (strToRelease->Str && strToRelease->Str != strToKeep->Str) free(strToRelease->Str);
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

    INLINE QSPString qspMoveToNewText(QSPString *s)
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
        return (ch < sizeof(qspAsciiClasses)) && ((qspAsciiClasses[ch] & charClass) != 0);
    }

    INLINE void qspSkipSpaces(QSPString *s)
    {
        QSP_CHAR *pos = s->Str, *end = s->End;
        while (pos < end && qspIsInClass(*pos, QSP_CHAR_SPACE)) ++pos;
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
        QSP_CHAR *pos = str->Str, *end = str->End;
        while (pos < end)
        {
            *pos = (QSP_CHAR)QSP_CHRLWR(*pos);
            ++pos;
        }
    }

    INLINE void qspUpperStr(QSPString *str)
    {
        QSP_CHAR *pos = str->Str, *end = str->End;
        while (pos < end)
        {
            *pos = (QSP_CHAR)QSP_CHRUPR(*pos);
            ++pos;
        }
    }

    INLINE int qspStrsPartCompare(QSPString str1, QSPString str2, int maxLen)
    {
        int delta;
        QSP_CHAR *pos1 = str1.Str, *pos2 = str2.Str;
        QSP_CHAR *end1 = str1.End, *end2 = str2.End;
        while (maxLen && pos2 < end2 && pos1 < end1)
        {
            if ((delta = (int)*pos1 - *pos2)) return delta;
            ++pos1, ++pos2;
            --maxLen;
        }
        if (maxLen) return (pos1 == end1) ? ((pos2 == end2) ? 0 : -1) : 1;
        return 0;
    }

    INLINE int qspStrsCompare(QSPString str1, QSPString str2)
    {
        int delta;
        QSP_CHAR *pos1 = str1.Str, *pos2 = str2.Str;
        QSP_CHAR *end1 = str1.End, *end2 = str2.End;
        while (pos2 < end2 && pos1 < end1)
        {
            if ((delta = (int)*pos1 - *pos2)) return delta;
            ++pos1, ++pos2;
        }
        return (pos1 == end1) ? ((pos2 == end2) ? 0 : -1) : 1;
    }

    INLINE QSP_CHAR *qspStrChar(QSPString str, QSP_CHAR ch)
    {
        QSP_CHAR *pos = str.Str;
        while (pos < str.End)
        {
            if (*pos == ch) return pos;
            ++pos;
        }
        return 0;
    }

    INLINE QSP_CHAR *qspStrRChar(QSPString str, QSP_CHAR ch)
    {
        QSP_CHAR *lastPos = 0, *pos = str.Str;
        while (pos < str.End)
        {
            if (*pos == ch) lastPos = pos;
            ++pos;
        }
        return lastPos;
    }

    INLINE QSP_CHAR *qspStrCharClass(QSPString str, int charClass)
    {
        QSP_CHAR *pos = str.Str;
        while (pos < str.End)
        {
            if (qspIsInClass(*pos, charClass))
                return pos;
            ++pos;
        }
        return 0;
    }

    INLINE QSP_CHAR *qspStrStr(QSPString str, QSPString strSearch)
    {
        int searchLen = qspStrLen(strSearch);
        if (!searchLen) return str.Str;
        if (searchLen <= qspStrLen(str))
        {
            size_t bytesToCompare = searchLen * sizeof(QSP_CHAR);
            QSP_CHAR *pos, *lastPos = str.End - searchLen;
            for (pos = str.Str; pos <= lastPos; ++pos)
                if (!memcmp(pos, strSearch.Str, bytesToCompare)) return pos;
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

    INLINE void qspUpdateBufString(QSPBufString *buf, QSPString val)
    {
        buf->Len = 0; /* assign the whole string */
        qspAddBufText(buf, val);
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
