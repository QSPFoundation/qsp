/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "text.h"

static QSP_CHAR *qspStrEnd(QSP_CHAR *);

int qspAddText(QSP_CHAR **dest, QSP_CHAR *val, int destLen, int valLen, QSP_BOOL isCreate)
{
    int ret;
    QSP_CHAR *destPtr;
    if (valLen < 0) valLen = (int)QSP_STRLEN(val);
    if (!isCreate && *dest)
    {
        if (destLen < 0) destLen = (int)QSP_STRLEN(*dest);
        ret = destLen + valLen;
        destPtr = (QSP_CHAR *)realloc(*dest, (ret + 1) * sizeof(QSP_CHAR));
        *dest = destPtr;
        destPtr += destLen;
    }
    else
    {
        ret = valLen;
        destPtr = (QSP_CHAR *)malloc((ret + 1) * sizeof(QSP_CHAR));
        *dest = destPtr;
    }
    QSP_STRNCPY(destPtr, val, valLen);
    destPtr[valLen] = 0;
    return ret;
}

QSP_BOOL qspIsInList(QSP_CHAR *list, QSP_CHAR ch)
{
    while (*list)
        if (*list++ == ch) return QSP_TRUE;
    return QSP_FALSE;
}

QSP_CHAR *qspSkipSpaces(QSP_CHAR *s)
{
    while (qspIsInList(QSP_SPACES, *s)) ++s;
    return s;
}

static QSP_CHAR *qspStrEnd(QSP_CHAR *s)
{
    while (*s) ++s;
    return s;
}

QSP_CHAR *qspDelSpc(QSP_CHAR *s)
{
    int len;
    QSP_CHAR *str, *begin = qspSkipSpaces(s), *end = qspStrEnd(begin);
    while (begin < end && qspIsInList(QSP_SPACES, *(end - 1))) --end;
    len = (int)(end - begin);
    str = (QSP_CHAR *)malloc((len + 1) * sizeof(QSP_CHAR));
    QSP_STRNCPY(str, begin, len);
    str[len] = 0;
    return str;
}

int qspStrLen(QSP_CHAR *str)
{
    QSP_CHAR *bos = str;
    while (*str) ++str;
    return (int)(str - bos);
}

QSP_CHAR *qspStrStr(QSP_CHAR *str, QSP_CHAR *strSearch)
{
    QSP_CHAR *s1, *s2;
    while (*str)
    {
        s1 = str;
        s2 = strSearch;
        while (*s1 && *s2 && !((int)*s1 - *s2))
            ++s1, ++s2;
        if (!(*s2)) return str;
        ++str;
    }
    return 0;
}

QSP_CHAR *qspStrCopy(QSP_CHAR *strDest, QSP_CHAR *strSource)
{
    QSP_CHAR *ret = strDest;
    while (*strDest++ = *strSource++);
    return ret;
}

QSP_CHAR *qspStrNCopy(QSP_CHAR *strDest, QSP_CHAR *strSource, int maxLen)
{
    QSP_CHAR *ret = strDest;
    while (maxLen-- && (*strDest++ = *strSource++));
    return ret;
}

QSP_CHAR *qspReplaceText(QSP_CHAR *txt, QSP_CHAR *searchTxt, QSP_CHAR *repTxt)
{
    int txtLen, oldTxtLen, bufSize, searchLen, repLen, len;
    QSP_CHAR *newTxt, *pos = qspStrStr(txt, searchTxt);
    if (!pos)
    {
        qspAddText(&newTxt, txt, 0, -1, QSP_TRUE);
        return newTxt;
    }
    bufSize = 256;
    txtLen = oldTxtLen = 0;
    searchLen = qspStrLen(searchTxt);
    repLen = qspStrLen(repTxt);
    newTxt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
    do
    {
        len = (int)(pos - txt);
        if ((txtLen += len + repLen) >= bufSize)
        {
            bufSize = txtLen + 128;
            newTxt = (QSP_CHAR *)realloc(newTxt, bufSize * sizeof(QSP_CHAR));
        }
        qspStrNCopy(newTxt + oldTxtLen, txt, len);
        qspStrCopy(newTxt + oldTxtLen + len, repTxt);
        oldTxtLen = txtLen;
        txt = pos + searchLen;
        pos = qspStrStr(txt, searchTxt);
    } while (pos);
    qspAddText(&newTxt, txt, txtLen, -1, QSP_FALSE);
    return newTxt;
}

int qspStrsComp(QSP_CHAR *str1, QSP_CHAR *str2)
{
    int ret = 0;
    while (!(ret = (int)*str1 - *str2) && *str2)
        ++str1, ++str2;
    if (ret < 0)
        return -1;
    else if (ret > 0)
        return 1;
    return 0;
}

QSP_BOOL qspIsEqual(QSP_CHAR *str1, QSP_CHAR *str2, int maxLen)
{
    int delta = 0;
    while (maxLen-- && !(delta = (int)(*str1 - *str2)) && *str2)
        ++str1, ++str2;
    return (delta == 0);
}

int qspSplitStr(QSP_CHAR *str, QSP_CHAR *delim, QSP_CHAR ***res)
{
    int allocChars, count = 0, bufSize = 8, delimLen = qspStrLen(delim);
    QSP_CHAR *newStr, **ret, *curPos = str, *found = qspStrStr(str, delim);
    ret = (QSP_CHAR **)malloc(bufSize * sizeof(QSP_CHAR *));
    while (found)
    {
        allocChars = (int)(found - curPos);
        newStr = (QSP_CHAR *)malloc((allocChars + 1) * sizeof(QSP_CHAR));
        qspStrNCopy(newStr, curPos, allocChars);
        newStr[allocChars] = 0;
        if (++count > bufSize)
        {
            bufSize += 16;
            ret = (QSP_CHAR **)realloc(ret, bufSize * sizeof(QSP_CHAR *));
        }
        ret[count - 1] = newStr;
        curPos = found + delimLen;
        found = qspStrStr(curPos, delim);
    }
    newStr = (QSP_CHAR *)malloc((qspStrLen(curPos) + 1) * sizeof(QSP_CHAR));
    qspStrCopy(newStr, curPos);
    if (++count > bufSize)
        ret = (QSP_CHAR **)realloc(ret, count * sizeof(QSP_CHAR *));
    ret[count - 1] = newStr;
    *res = ret;
    return count;
}

void qspFreeStrs(void **strs, int count)
{
    if (strs)
    {
        while (--count >= 0)
            if (strs[count]) free(strs[count]);
        free(strs);
    }
}

QSP_BOOL qspIsDigit(QSP_CHAR ch)
{
    return (ch >= QSP_FMT('0') && ch <= QSP_FMT('9'));
}

int qspStrToNum(QSP_CHAR *s, QSP_BOOL *isValid)
{
    int num;
    QSP_BOOL isNeg = QSP_FALSE;
    s = qspSkipSpaces(s);
    if (*s == QSP_FMT('-'))
    {
        isNeg = QSP_TRUE;
        ++s;
    }
    else if (*s == QSP_FMT('+'))
        ++s;
    else if (!(*s)) /* special case, i.e. empty string must be convertible to 0 */
    {
        if (isValid) *isValid = QSP_TRUE;
        return 0;
    }
    if (qspIsDigit(*s))
    {
        num = 0;
        do
        {
            num = num * 10 + (*s - QSP_FMT('0'));
            ++s;
        } while (qspIsDigit(*s));
    }
    else
    {
        if (isValid) *isValid = QSP_FALSE;
        return 0;
    }
    s = qspSkipSpaces(s);
    if (*s)
    {
        if (isValid) *isValid = QSP_FALSE;
        return 0;
    }
    if (isValid) *isValid = QSP_TRUE;
    if (isNeg) return -num;
    return num;
}

QSP_CHAR *qspNumToStr(QSP_CHAR *buf, int val)
{
    QSP_CHAR temp, *str = buf, *first = str;
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
    *str-- = 0;
    while (first < str)
    {
        temp = *str;
        *str = *first;
        *first = temp;
        --str;
        ++first;
    }
    return buf;
}
