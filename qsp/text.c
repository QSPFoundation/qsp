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

#include "text.h"
#include "errors.h"
#include "locations.h"
#include "mathops.h"

QSPString qspNullString;
unsigned char qspAsciiClasses[128];

INLINE void qspFillSymbolClass(unsigned char symbolClass, QSP_CHAR *symbols)
{
    while (*symbols)
    {
        qspAsciiClasses[*symbols] |= symbolClass;
        ++symbols;
    }
}

void qspInitSymbolClasses(void)
{
    int i;
    for (i = 0; i < sizeof(qspAsciiClasses); ++i)
        qspAsciiClasses[i] = 0;

    qspFillSymbolClass(QSP_CHAR_SPACE, QSP_SPACES);
    qspFillSymbolClass(QSP_CHAR_QUOT, QSP_QUOTS);
    qspFillSymbolClass(QSP_CHAR_DIGIT, QSP_DIGITS);
    qspFillSymbolClass(QSP_CHAR_DELIM, QSP_DELIMS);
    qspFillSymbolClass(QSP_CHAR_SIMPLEOP, QSP_ADD QSP_SUB QSP_DIV QSP_MUL);
    qspFillSymbolClass(QSP_CHAR_EXPSTART, QSP_LQUOT QSP_LRBRACK QSP_LSBRACK);
}

QSP_CHAR *qspStringToC(QSPString s)
{
    QSP_CHAR *string;
    int stringLen = qspStrLen(s);
    string = (QSP_CHAR *)malloc((stringLen + 1) * sizeof(QSP_CHAR));
    memcpy(string, s.Str, stringLen * sizeof(QSP_CHAR));
    string[stringLen] = 0;
    return string;
}

QSP_BOOL qspAddText(QSPString *dest, QSPString val, QSP_BOOL toCreate)
{
    int valLen = qspStrLen(val);
    if (!toCreate && dest->Str)
    {
        if (valLen)
        {
            int destLen = qspStrLen(*dest);
            QSP_CHAR *destPtr = (QSP_CHAR *)realloc(dest->Str, (destLen + valLen) * sizeof(QSP_CHAR));
            dest->Str = destPtr;
            destPtr += destLen;
            dest->End = destPtr + valLen;
            memcpy(destPtr, val.Str, valLen * sizeof(QSP_CHAR));
            return QSP_TRUE;
        }
    }
    else
    {
        if (valLen)
        {
            QSP_CHAR *destPtr = (QSP_CHAR *)malloc(valLen * sizeof(QSP_CHAR));
            dest->Str = destPtr;
            dest->End = destPtr + valLen;
            memcpy(destPtr, val.Str, valLen * sizeof(QSP_CHAR));
            return QSP_TRUE;
        }
        else
        {
            dest->Str = dest->End = 0; /* assign the null string */
        }
    }
    return QSP_FALSE;
}

QSP_BOOL qspAddBufText(QSPBufString *dest, QSPString val)
{
    int valLen = qspStrLen(val);
    if (valLen)
    {
        if (dest->Str)
        {
            if (dest->Len + valLen > dest->Capacity)
            {
                dest->Capacity = dest->Len + valLen + dest->CapacityIncrement;
                dest->Str = (QSP_CHAR *)realloc(dest->Str, dest->Capacity * sizeof(QSP_CHAR));
            }
            memcpy(dest->Str + dest->Len, val.Str, valLen * sizeof(QSP_CHAR));
            dest->Len += valLen;
        }
        else
        {
            dest->Capacity = valLen + dest->CapacityIncrement;
            dest->Str = (QSP_CHAR *)malloc(dest->Capacity * sizeof(QSP_CHAR));
            memcpy(dest->Str, val.Str, valLen * sizeof(QSP_CHAR));
            dest->Len = valLen;
        }
        return QSP_TRUE;
    }
    return QSP_FALSE;
}

QSP_CHAR *qspInStrRChars(QSPString str, QSP_CHAR *chars)
{
    QSP_CHAR *lastPos = 0, *pos = str.Str;
    while (pos < str.End)
    {
        if (qspIsInList(*pos, chars)) lastPos = pos;
        ++pos;
    }
    return lastPos;
}

QSPString qspJoinStrs(QSPString *s, int count, QSPString delim)
{
    int i;
    QSPBufString buf = qspNewBufString(256);
    for (i = 0; i < count; ++i)
    {
        qspAddBufText(&buf, s[i]);
        if (i == count - 1) break; /* don't add the delim */
        qspAddBufText(&buf, delim);
    }
    return qspBufTextToString(buf);
}

int qspSplitStr(QSPString str, QSPString delim, QSPString **res)
{
    QSPString newStr, *ret;
    int count = 0, bufSize = 8, delimLen = qspStrLen(delim);
    QSP_CHAR *delimPos = qspStrStr(str, delim);
    ret = (QSPString *)malloc(bufSize * sizeof(QSPString));
    while (delimPos)
    {
        qspAddText(&newStr, qspStringFromPair(str.Str, delimPos), QSP_TRUE);
        if (count >= bufSize)
        {
            bufSize = count + 16;
            ret = (QSPString *)realloc(ret, bufSize * sizeof(QSPString));
        }
        ret[count++] = newStr;
        str.Str = delimPos + delimLen;
        delimPos = qspStrStr(str, delim);
    }
    qspAddText(&newStr, str, QSP_TRUE);
    if (count >= bufSize)
        ret = (QSPString *)realloc(ret, (count + 1) * sizeof(QSPString));
    ret[count++] = newStr;
    *res = ret;
    return count;
}

void qspCopyStrs(QSPString **dest, QSPString *src, int start, int end)
{
    int count = end - start;
    if (src && count)
    {
        int i = 0;
        *dest = (QSPString *)malloc(count * sizeof(QSPString));
        while (start < end)
            qspAddText(*dest + i++, src[start++], QSP_TRUE);
    }
    else
        *dest = 0;
}

void qspFreeStrs(QSPString *strs, int count)
{
    if (strs)
    {
        QSPString *curStr;
        for (curStr = strs; count > 0; --count, ++curStr)
            qspFreeString(curStr);
        free(strs);
    }
}

QSP_BOOL qspIsStrNumber(QSPString s)
{
    QSP_CHAR *pos;
    qspSkipSpaces(&s);
    pos = s.Str;
    if (pos < s.End)
    {
        if (*pos == QSP_FMT('-'))
            ++pos;
        else if (*pos == QSP_FMT('+'))
            ++pos;
    }
    else
        return QSP_FALSE;
    if (pos < s.End && qspIsInClass(*pos, QSP_CHAR_DIGIT))
    {
        do
        {
            ++pos;
        } while (pos < s.End && qspIsInClass(*pos, QSP_CHAR_DIGIT));
    }
    else
        return QSP_FALSE;
    s.Str = pos;
    qspSkipSpaces(&s);
    return qspIsEmpty(s);
}

QSP_BIGINT qspStrToNum(QSPString s, QSP_BOOL *isValid)
{
    QSP_BIGINT num;
    QSP_CHAR *pos;
    QSP_BOOL isNeg = QSP_FALSE;
    qspSkipSpaces(&s);
    pos = s.Str;
    if (pos < s.End)
    {
        if (*pos == QSP_FMT('-'))
        {
            isNeg = QSP_TRUE;
            ++pos;
        }
        else if (*pos == QSP_FMT('+'))
            ++pos;
    }
    else /* a special case, i.e. an empty string must be convertible to 0 */
    {
        if (isValid) *isValid = QSP_TRUE;
        return 0;
    }
    if (pos < s.End && qspIsInClass(*pos, QSP_CHAR_DIGIT))
    {
        num = 0;
        do
        {
            num = num * 10 + (*pos - QSP_FMT('0'));
            ++pos;
        } while (pos < s.End && qspIsInClass(*pos, QSP_CHAR_DIGIT));
        if (num < 0) num = QSP_MAX_BIGINT; /* simple overflow protection */
    }
    else
    {
        if (isValid) *isValid = QSP_FALSE;
        return 0;
    }
    s.Str = pos;
    qspSkipSpaces(&s);
    if (!qspIsEmpty(s))
    {
        if (isValid) *isValid = QSP_FALSE;
        return 0;
    }
    if (isValid) *isValid = QSP_TRUE;
    if (isNeg) return -num;
    return num;
}

QSPString qspNumToStr(QSP_CHAR *buf, QSP_BIGINT val)
{
    QSP_CHAR temp, *last, *str = buf, *first = str;
    if (val < 0)
    {
        *str++ = QSP_FMT('-');
        val = -val;
        ++first;
    }
    do
    {
        *str++ = (QSP_CHAR)(val % 10 + QSP_FMT('0'));
        val /= 10;
    } while (val > 0);
    last = str--;
    while (first < str)
    {
        temp = *str;
        *str = *first;
        *first = temp;
        --str;
        ++first;
    }
    return qspStringFromPair(buf, last);
}

QSP_CHAR *qspDelimPos(QSPString txt, QSP_CHAR ch)
{
    int c1 = 0, c2 = 0, c3 = 0;
    QSP_CHAR *pos = txt.Str;
    while (pos < txt.End)
    {
        if (qspIsInClass(*pos, QSP_CHAR_QUOT))
        {
            QSP_CHAR quot = *pos;
            while (++pos < txt.End)
            {
                if (*pos == quot)
                {
                    ++pos;
                    if (pos >= txt.End) break;
                    if (*pos != quot) break;
                }
            }
            continue;
        }
        else if (*pos == QSP_LRBRACK[0])
            ++c1;
        else if (*pos == QSP_RRBRACK[0])
        {
            if (c1) --c1;
        }
        else if (*pos == QSP_LSBRACK[0])
            ++c2;
        else if (*pos == QSP_RSBRACK[0])
        {
            if (c2) --c2;
        }
        else if (*pos == QSP_LQUOT[0])
            ++c3;
        else if (*pos == QSP_RQUOT[0])
        {
            if (c3) --c3;
        }
        if (!(c1 || c2 || c3) && *pos == ch) /* include brackets */
            return pos;
        ++pos;
    }
    return 0;
}

QSP_CHAR *qspStrPos(QSPString txt, QSPString str, QSP_BOOL isIsolated)
{
    QSP_BOOL isPrevDelim;
    QSP_CHAR *lastPos, *pos;
    int c1, c2, c3, strLen = qspStrLen(str);
    if (!strLen) return txt.Str;
    pos = qspStrStr(txt, str);
    if (!pos) return 0;
    if (!isIsolated)
    {
        QSPString prefix = qspStringFromPair(txt.Str, pos);
        if (!qspIsAnyInClass(prefix, QSP_CHAR_QUOT | QSP_CHAR_EXPSTART)) return pos;
    }
    c1 = c2 = c3 = 0;
    isPrevDelim = QSP_TRUE;
    pos = txt.Str;
    lastPos = txt.End - strLen;
    while (pos <= lastPos)
    {
        if (qspIsInClass(*pos, QSP_CHAR_QUOT))
        {
            QSP_CHAR quot = *pos;
            while (++pos <= lastPos)
            {
                if (*pos == quot)
                {
                    ++pos;
                    if (pos > lastPos) break;
                    if (*pos != quot) break;
                }
            }
            isPrevDelim = QSP_TRUE;
            continue;
        }
        else if (*pos == QSP_LRBRACK[0])
            ++c1;
        else if (*pos == QSP_RRBRACK[0])
        {
            if (c1) --c1;
        }
        else if (*pos == QSP_LSBRACK[0])
            ++c2;
        else if (*pos == QSP_RSBRACK[0])
        {
            if (c2) --c2;
        }
        else if (*pos == QSP_LQUOT[0])
            ++c3;
        else if (*pos == QSP_RQUOT[0])
        {
            if (c3) --c3;
        }
        if (!(c1 || c2 || c3)) /* include brackets */
        {
            if (isIsolated)
            {
                if (qspIsInClass(*pos, QSP_CHAR_DELIM))
                    isPrevDelim = QSP_TRUE;
                else if (isPrevDelim)
                {
                    if (pos >= lastPos || qspIsInClass(pos[strLen], QSP_CHAR_DELIM))
                    {
                        txt.Str = pos;
                        if (!qspStrsNComp(txt, str, strLen)) return pos;
                    }
                    isPrevDelim = QSP_FALSE;
                }
            }
            else
            {
                /* It must support searching for delimiters */
                txt.Str = pos;
                if (!qspStrsNComp(txt, str, strLen)) return pos;
            }
        }
        ++pos;
    }
    return 0;
}

QSPString qspReplaceText(QSPString txt, QSPString searchTxt, QSPString repTxt, int maxReplacements, QSP_BOOL canReturnSelf)
{
    if (maxReplacements > 0)
    {
        int searchLen = qspStrLen(searchTxt);
        if (searchLen)
        {
            QSP_CHAR *pos = qspStrStr(txt, searchTxt);
            if (pos)
            {
                QSPBufString res = qspNewBufString(256);
                do
                {
                    qspAddBufText(&res, qspStringFromPair(txt.Str, pos));
                    qspAddBufText(&res, repTxt);
                    txt.Str = pos + searchLen;
                    if (--maxReplacements == 0) break;
                    pos = qspStrStr(txt, searchTxt);
                } while (pos);
                qspAddBufText(&res, txt);
                return qspBufTextToString(res);
            }
        }
    }
    if (canReturnSelf) return txt;
    return qspCopyToNewText(txt);
}

QSPString qspFormatText(QSPString txt, QSP_BOOL canReturnSelf)
{
    QSPVariant val;
    QSPString expr;
    QSPBufString res;
    int oldLocationState;
    QSP_CHAR *pos = qspStrStr(txt, QSP_STATIC_STR(QSP_LSUBEX));
    if (!pos)
    {
        if (canReturnSelf) return txt;
        return qspCopyToNewText(txt);
    }
    res = qspNewBufString(128);
    oldLocationState = qspLocationState;
    do
    {
        qspAddBufText(&res, qspStringFromPair(txt.Str, pos));
        txt.Str = pos + QSP_STATIC_LEN(QSP_LSUBEX);
        pos = qspStrPos(txt, QSP_STATIC_STR(QSP_RSUBEX), QSP_FALSE);
        if (!pos)
        {
            qspSetError(QSP_ERR_BRACKNOTFOUND);
            qspFreeBufString(&res);
            return qspNullString;
        }
        expr = qspStringFromPair(txt.Str, pos);
        /* looks like it's ok to modify the original string here */
        qspPrepareStringToExecution(&expr);
        val = qspCalculateExprValue(expr);
        if (qspLocationState != oldLocationState)
        {
            qspFreeBufString(&res);
            return qspNullString;
        }
        qspConvertVariantTo(&val, QSP_TYPE_STR);
        qspAddBufText(&res, QSP_STR(val));
        qspFreeString(&QSP_STR(val));
        txt.Str = pos + QSP_STATIC_LEN(QSP_RSUBEX);
        pos = qspStrStr(txt, QSP_STATIC_STR(QSP_LSUBEX));
    } while (pos);
    qspAddBufText(&res, txt);
    return qspBufTextToString(res);
}
