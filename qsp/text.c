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

void qspInitSymbolClasses()
{
    int i;
    for (i = 0; i < sizeof(qspAsciiClasses); ++i)
        qspAsciiClasses[i] = 0;

    qspFillSymbolClass(QSP_CHAR_SPACE, QSP_SPACES);
    qspFillSymbolClass(QSP_CHAR_QUOT, QSP_QUOTS);
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

void qspAddText(QSPString *dest, QSPString val, QSP_BOOL toCreate)
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
        }
        else
        {
            /* Assign a null string */
            dest->Str = 0;
            dest->End = 0;
        }
    }
}

void qspAddBufText(QSPBufString *dest, QSPString val, int extraCapacity)
{
    int valLen = qspStrLen(val);
    if (valLen)
    {
        if (dest->Str)
        {
            if (dest->Len + valLen > dest->Capacity)
            {
                dest->Capacity = dest->Len + valLen + extraCapacity;
                dest->Str = (QSP_CHAR *)realloc(dest->Str, dest->Capacity * sizeof(QSP_CHAR));
            }
            memcpy(dest->Str + dest->Len, val.Str, valLen * sizeof(QSP_CHAR));
            dest->Len += valLen;
        }
        else
        {
            dest->Capacity = valLen + extraCapacity;
            dest->Str = (QSP_CHAR *)malloc(dest->Capacity * sizeof(QSP_CHAR));
            memcpy(dest->Str, val.Str, valLen * sizeof(QSP_CHAR));
            dest->Len = valLen;
        }
    }
}

QSP_BOOL qspClearText(QSPString *s)
{
    if (s->Str)
    {
        int oldLen = (int)(s->End - s->Str);
        free(s->Str);
        s->Str = s->End = 0;
        if (oldLen) return QSP_TRUE;
    }
    return QSP_FALSE;
}

QSP_CHAR *qspInStrRChars(QSPString str, QSP_CHAR *chars)
{
    QSP_CHAR *lastPos = 0, *pos = str.Str;
    while (pos < str.End)
    {
        if (qspIsInList(chars, *pos)) lastPos = pos;
        ++pos;
    }
    return lastPos;
}

QSPString qspJoinStrs(QSPString *s, int count, QSPString delim)
{
    int i, curLen, txtLen = 0, txtRealLen = 0, bufSize = 256, delimLen = qspStrLen(delim);
    QSP_CHAR *txt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
    for (i = 0; i < count; ++i)
    {
        curLen = qspStrLen(s[i]);
        if (curLen)
        {
            if ((txtLen += curLen) > bufSize)
            {
                bufSize = txtLen + 128;
                txt = (QSP_CHAR *)realloc(txt, bufSize * sizeof(QSP_CHAR));
            }
            memcpy(txt + txtRealLen, s[i].Str, curLen * sizeof(QSP_CHAR));
            txtRealLen = txtLen;
        }
        if (i == count - 1) break;
        if (delimLen)
        {
            if ((txtLen += delimLen) > bufSize)
            {
                bufSize = txtLen + 128;
                txt = (QSP_CHAR *)realloc(txt, bufSize * sizeof(QSP_CHAR));
            }
            memcpy(txt + txtRealLen, delim.Str, delimLen * sizeof(QSP_CHAR));
            txtRealLen = txtLen;
        }
    }
    return qspStringFromLen(txt, txtLen);
}

int qspSplitStr(QSPString str, QSPString delim, QSPString **res)
{
    QSPString *ret;
    int newStrLen, count = 0, bufSize = 8, delimLen = qspStrLen(delim);
    QSP_CHAR *newStr, *found = qspStrStr(str, delim);
    ret = (QSPString *)malloc(bufSize * sizeof(QSPString));
    while (found)
    {
        newStrLen = (int)(found - str.Str);
        newStr = (QSP_CHAR *)malloc(newStrLen * sizeof(QSP_CHAR));
        memcpy(newStr, str.Str, newStrLen * sizeof(QSP_CHAR));
        if (count >= bufSize)
        {
            bufSize = count + 16;
            ret = (QSPString *)realloc(ret, bufSize * sizeof(QSPString));
        }
        ret[count++] = qspStringFromLen(newStr, newStrLen);
        str.Str = found + delimLen;
        found = qspStrStr(str, delim);
    }
    newStrLen = qspStrLen(str);
    newStr = (QSP_CHAR *)malloc(newStrLen * sizeof(QSP_CHAR));
    memcpy(newStr, str.Str, newStrLen * sizeof(QSP_CHAR));
    if (count >= bufSize)
        ret = (QSPString *)realloc(ret, (count + 1) * sizeof(QSPString));
    ret[count++] = qspStringFromLen(newStr, newStrLen);
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
        while (--count >= 0) qspFreeString(strs[count]);
        free(strs);
    }
}

QSP_BOOL qspIsNumber(QSPString s)
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
    if (pos < s.End && qspIsDigit(*pos))
    {
        do
        {
            ++pos;
        } while (pos < s.End && qspIsDigit(*pos));
    }
    else
        return QSP_FALSE;
    s.Str = pos;
    qspSkipSpaces(&s);
    return qspIsEmpty(s);
}

int qspStrToNum(QSPString s, QSP_BOOL *isValid)
{
    int num;
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
    if (pos < s.End && qspIsDigit(*pos))
    {
        num = 0;
        do
        {
            num = num * 10 + (*pos - QSP_FMT('0'));
            ++pos;
        } while (pos < s.End && qspIsDigit(*pos));
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

QSPString qspNumToStr(QSP_CHAR *buf, int val)
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
    QSP_CHAR quot, *pos = txt.Str;
    while (pos < txt.End)
    {
        if (qspIsInClass(*pos, QSP_CHAR_QUOT))
        {
            quot = *pos;
            while (++pos < txt.End)
                if (*pos == quot && (++pos >= txt.End || *pos != quot)) break;
            if (pos >= txt.End) return 0;
        }
        if (*pos == QSP_LRBRACK[0])
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
        if (!(c1 || c2 || c3) && *pos == ch) return pos;
        ++pos;
    }
    return 0;
}

QSP_CHAR *qspStrPos(QSPString txt, QSPString str, QSP_BOOL isIsolated)
{
    QSP_BOOL isLastDelim;
    QSP_CHAR quot, *lastPos, *pos;
    int c1, c2, c3, strLen = qspStrLen(str);
    if (!strLen) return txt.Str;
    pos = qspStrStr(txt, str);
    if (!pos) return 0;
    if (!(isIsolated || qspIsAnyInClass(txt, QSP_CHAR_QUOT | QSP_CHAR_EXPSTART))) return pos;
    c1 = c2 = c3 = 0;
    isLastDelim = QSP_TRUE;
    pos = txt.Str;
    lastPos = txt.End - strLen;
    while (pos <= lastPos)
    {
        if (qspIsInClass(*pos, QSP_CHAR_QUOT))
        {
            quot = *pos;
            while (++pos <= lastPos)
                if (*pos == quot && (++pos > lastPos || *pos != quot)) break;
            if (pos > lastPos) return 0;
            isLastDelim = QSP_TRUE;
        }
        if (*pos == QSP_LRBRACK[0])
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
        if (!(c1 || c2 || c3))
        {
            if (isIsolated)
            {
                if (qspIsInClass(*pos, QSP_CHAR_DELIM))
                    isLastDelim = QSP_TRUE;
                else if (isLastDelim)
                {
                    if (pos >= lastPos || qspIsInClass(pos[strLen], QSP_CHAR_DELIM))
                    {
                        txt.Str = pos;
                        if (!qspStrsNComp(txt, str, strLen)) return pos;
                    }
                    isLastDelim = QSP_FALSE;
                }
            }
            else
            {
                txt.Str = pos;
                if (!qspStrsNComp(txt, str, strLen)) return pos;
            }
        }
        ++pos;
    }
    return 0;
}

QSPString qspReplaceText(QSPString txt, QSPString searchTxt, QSPString repTxt)
{
    QSPString res;
    int txtLen, oldTxtLen, bufSize, searchLen, repLen, len;
    QSP_CHAR *newTxt, *pos = qspStrStr(txt, searchTxt);
    if (!pos) return qspGetNewText(txt);
    bufSize = 256;
    txtLen = oldTxtLen = 0;
    searchLen = qspStrLen(searchTxt);
    repLen = qspStrLen(repTxt);
    newTxt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
    do
    {
        len = (int)(pos - txt.Str);
        if ((txtLen += len + repLen) > bufSize)
        {
            bufSize = txtLen + 128;
            newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
        }
        if (len) memcpy(newTxt + oldTxtLen, txt.Str, len * sizeof(QSP_CHAR));
        if (repLen) memcpy(newTxt + oldTxtLen + len, repTxt.Str, repLen * sizeof(QSP_CHAR));
        oldTxtLen = txtLen;
        txt.Str = pos + searchLen;
        pos = qspStrStr(txt, searchTxt);
    } while (pos);
    res = qspStringFromLen(newTxt, txtLen);
    qspAddText(&res, txt, QSP_FALSE);
    return res;
}

QSPString qspFormatText(QSPString txt, QSP_BOOL canReturnSelf)
{
    QSPVariant val;
    QSPString expr, res;
    QSP_CHAR *newTxt, *lPos, *rPos;
    int oldRefreshCount, len, txtLen, oldTxtLen, bufSize;
    lPos = qspStrStr(txt, QSP_STATIC_STR(QSP_LSUBEX));
    if (!lPos)
    {
        if (canReturnSelf) return txt;
        return qspGetNewText(txt);
    }
    bufSize = qspStrLen(txt);
    newTxt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
    txtLen = oldTxtLen = 0;
    oldRefreshCount = qspRefreshCount;
    do
    {
        len = (int)(lPos - txt.Str);
        if (len)
        {
            txtLen += len;
            if (txtLen > bufSize)
            {
                bufSize = txtLen + 128;
                newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
            }
            memcpy(newTxt + oldTxtLen, txt.Str, len * sizeof(QSP_CHAR));
            oldTxtLen = txtLen;
        }
        txt.Str = lPos + QSP_STATIC_LEN(QSP_LSUBEX);
        rPos = qspStrPos(txt, QSP_STATIC_STR(QSP_RSUBEX), QSP_FALSE);
        if (!rPos)
        {
            qspSetError(QSP_ERR_BRACKNOTFOUND);
            free(newTxt);
            return qspNullString;
        }
        expr = qspStringFromPair(txt.Str, rPos);
        /* looks like it's ok to modify the original string here */
        qspPrepareStringToExecution(&expr);
        val = qspExprValue(expr);
        if (qspRefreshCount != oldRefreshCount || qspErrorNum)
        {
            free(newTxt);
            return qspNullString;
        }
        qspConvertVariantTo(&val, QSP_TYPE_STR);
        len = qspStrLen(QSP_STR(val));
        if (len)
        {
            txtLen += len;
            if (txtLen > bufSize)
            {
                bufSize = txtLen + 128;
                newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
            }
            memcpy(newTxt + oldTxtLen, QSP_STR(val).Str, len * sizeof(QSP_CHAR));
            oldTxtLen = txtLen;
        }
        qspFreeString(QSP_STR(val));
        txt.Str = rPos + QSP_STATIC_LEN(QSP_RSUBEX);
        lPos = qspStrStr(txt, QSP_STATIC_STR(QSP_LSUBEX));
    } while (lPos);
    res = qspStringFromLen(newTxt, txtLen);
    qspAddText(&res, txt, QSP_FALSE);
    return res;
}
