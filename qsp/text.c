/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "text.h"
#include "codetools.h"
#include "errors.h"
#include "locations.h"
#include "mathops.h"
#include "variant.h"

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
    qspFillSymbolClass(QSP_CHAR_LBRACKET, QSP_LCODE QSP_LRBRACK QSP_LSBRACK);
    qspFillSymbolClass(QSP_CHAR_TYPEPREFIX, QSP_TUPLETYPE QSP_NUMTYPE QSP_STRTYPE);
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

QSPString qspConcatText(QSPString val1, QSPString val2)
{
    QSPString res;
    int firstLen = qspStrLen(val1), secondLen = qspStrLen(val2), destLen = firstLen + secondLen;
    if (destLen)
    {
        QSP_CHAR *dest = (QSP_CHAR *)malloc(destLen * sizeof(QSP_CHAR));
        if (firstLen)
            memcpy(dest, val1.Str, firstLen * sizeof(QSP_CHAR));
        if (secondLen)
            memcpy(dest + firstLen, val2.Str, secondLen * sizeof(QSP_CHAR));
        res.Str = dest;
        res.End = dest + destLen;
    }
    else
    {
        res.Str = res.End = 0; /* assign the null string */
    }
    return res;
}

QSPString qspJoinStrs(QSPString *s, int count, QSPString delim)
{
    int i;
    QSPBufString buf = qspNewBufString(0, 256);
    for (i = 0; i < count; ++i)
    {
        qspAddBufText(&buf, s[i]);
        if (i == count - 1) break; /* don't add the delim */
        qspAddBufText(&buf, delim);
    }
    return qspBufStringToString(buf);
}

int qspSplitStr(QSPString str, QSPString delim, QSPString **res)
{
    QSPString newStr, *ret;
    int count = 0, bufSize = 8, delimLen = qspStrLen(delim);
    QSP_CHAR *delimPos = qspStrStr(str, delim);
    ret = (QSPString *)malloc(bufSize * sizeof(QSPString));
    while (delimPos)
    {
        newStr = qspCopyToNewText(qspStringFromPair(str.Str, delimPos));
        if (count >= bufSize)
        {
            bufSize = count + 16;
            ret = (QSPString *)realloc(ret, bufSize * sizeof(QSPString));
        }
        ret[count++] = newStr;
        str.Str = delimPos + delimLen;
        delimPos = qspStrStr(str, delim);
    }
    newStr = qspCopyToNewText(str);
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
        int i;
        QSPString *destStrs = (QSPString *)malloc(count * sizeof(QSPString));
        for (i = 0; start < end; ++i, ++start)
            destStrs[i] = qspCopyToNewText(src[start]);
        *dest = destStrs;
    }
    else
        *dest = 0;
}

void qspReverseStrs(QSPString *strs, int count)
{
    if (count > 1)
    {
        QSPString temp;
        int start = 0;
        int end = count - 1;
        while (start < end)
        {
            temp = strs[start];
            strs[start] = strs[end];
            strs[end] = temp;
            ++start;
            --end;
        }
    }
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
                QSPBufString res = qspNewBufString(0, 256);
                do
                {
                    qspAddBufText(&res, qspStringFromPair(txt.Str, pos));
                    qspAddBufText(&res, repTxt);
                    txt.Str = pos + searchLen;
                    if (--maxReplacements == 0) break;
                    pos = qspStrStr(txt, searchTxt);
                } while (pos);
                qspAddBufText(&res, txt);
                return qspBufStringToString(res);
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
    res = qspNewBufString(64, 128);
    oldLocationState = qspLocationState;
    do
    {
        qspAddBufText(&res, qspStringFromPair(txt.Str, pos));
        txt.Str = pos + QSP_STATIC_LEN(QSP_LSUBEX);
        pos = qspKeywordPos(txt, QSP_STATIC_STR(QSP_RSUBEX), QSP_FALSE);
        if (!pos)
        {
            qspSetError(QSP_ERR_BRACKNOTFOUND);
            qspFreeBufString(&res);
            return qspNullString;
        }
        expr = qspStringFromPair(txt.Str, pos);
        /* Looks like it's ok to modify the original string here */
        qspPrepareStringToExecution(&expr);
        val = qspCalculateExprValue(expr);
        if (qspLocationState != oldLocationState)
        {
            qspFreeBufString(&res);
            return qspNullString;
        }
        qspConvertVariantTo(&val, QSP_TYPE_STR);
        qspAddBufText(&res, QSP_STR(val));
        qspFreeVariant(&val);
        txt.Str = pos + QSP_STATIC_LEN(QSP_RSUBEX);
        pos = qspStrStr(txt, QSP_STATIC_STR(QSP_LSUBEX));
    } while (pos);
    qspAddBufText(&res, txt);
    return qspBufStringToString(res);
}
