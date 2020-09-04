/* Copyright (C) 2001-2020 Valeriy Argunov (val AT time DOT guru) */
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

#include "codetools.h"
#include "statements.h"
#include "text.h"

INLINE int qspStatStringCompare(const void *, const void *);
INLINE int qspGetStatCode(QSPString s, QSP_CHAR **pos);
INLINE int qspInitStatArgs(QSPCachedArg **args, int statCode, QSPString s, QSP_CHAR *origStart, int *errorCode);
INLINE int qspInitSetArgs(QSPCachedArg **args, int statCode, QSPString s, QSP_CHAR *origStart, int *errorCode);
INLINE int qspInitRegularArgs(QSPCachedArg **args, int statCode, QSPString s, QSP_CHAR *origStart, int *errorCode);
INLINE int qspProcessPreformattedStrings(QSPString data, QSPLineOfCode **strs);
INLINE int qspProcessEOLExtensions(QSPLineOfCode *s, int count, QSPLineOfCode **strs);

INLINE int qspStatStringCompare(const void *name, const void *compareTo)
{
    QSPStatName *statName = (QSPStatName *)compareTo;
    return qspStrsNComp(*(QSPString *)name, statName->Name, qspStrLen(statName->Name));
}

INLINE int qspGetStatCode(QSPString s, QSP_CHAR **pos)
{
    int i, strLen, nameLen;
    QSPStatName *name;
    if (qspIsEmpty(s)) return qspStatUnknown;
    if (*s.Str == QSP_LABEL[0]) return qspStatLabel;
    if (*s.Str == QSP_COMMENT[0]) return qspStatComment;
    /* ------------------------------------------------------------------ */
    strLen = qspStrLen(s);
    for (i = 0; i < QSP_STATSLEVELS; ++i)
    {
        name = (QSPStatName *)bsearch(&s, qspStatsNames[i], qspStatsNamesCounts[i], sizeof(QSPStatName), qspStatStringCompare);
        if (name)
        {
            nameLen = qspStrLen(name->Name);
            if (nameLen == strLen || (nameLen < strLen && qspIsInClass(s.Str[nameLen], QSP_CHAR_DELIM)))
            {
                *pos = s.Str + nameLen;
                return name->Code;
            }
        }
    }
    return qspStatUnknown;
}

INLINE int qspInitStatArgs(QSPCachedArg **args, int statCode, QSPString s, QSP_CHAR *origStart, int *errorCode)
{
    *args = 0;
    *errorCode = 0;
    switch (statCode)
    {
        case qspStatUnknown:
        case qspStatLabel:
        case qspStatElse:
        case qspStatEnd:
        case qspStatComment:
        case qspStatLoop:
            return 0;
        case qspStatSet:
        case qspStatLocal:
            return qspInitSetArgs(args, statCode, s, origStart, errorCode);
        default:
            return qspInitRegularArgs(args, statCode, s, origStart, errorCode);
    }
}

INLINE int qspInitSetArgs(QSPCachedArg **args, int statCode, QSPString s, QSP_CHAR *origStart, int *errorCode)
{
    int argsCount;
    QSPString names, values, op;
    QSPCachedArg *foundArgs;
    QSP_CHAR *pos;
    qspSkipSpaces(&s);
    pos = qspStrPos(s, QSP_STATIC_STR(QSP_EQUAL), QSP_FALSE);
    if (pos)
    {
        op = qspStringFromPair(pos, pos + QSP_STATIC_LEN(QSP_EQUAL));
        values = qspDelSpc(qspStringFromPair(op.End, s.End));
        if (op.Str != s.Str && qspIsInClass(*(op.Str - 1), QSP_CHAR_SIMPLEOP)) --op.Str;
        names = qspDelSpc(qspStringFromPair(s.Str, op.Str));
        if (qspIsEmpty(names) || qspIsEmpty(values))
            *errorCode = QSP_ERR_SYNTAX;

        argsCount = 3;
        foundArgs = (QSPCachedArg *)malloc(argsCount * sizeof(QSPCachedArg));
        foundArgs[0].StartPos = (int)(names.Str - origStart);
        foundArgs[0].EndPos = (int)(names.End - origStart);
        foundArgs[1].StartPos = (int)(op.Str - origStart);
        foundArgs[1].EndPos = (int)(op.End - origStart);
        foundArgs[2].StartPos = (int)(values.Str - origStart);
        foundArgs[2].EndPos = (int)(values.End - origStart);
    }
    else
    {
        names = qspDelSpc(s);
        if (qspIsEmpty(names))
            *errorCode = QSP_ERR_SYNTAX;

        argsCount = 1;
        foundArgs = (QSPCachedArg *)malloc(argsCount * sizeof(QSPCachedArg));
        foundArgs[0].StartPos = (int)(names.Str - origStart);
        foundArgs[0].EndPos = (int)(names.End - origStart);
    }
    *args = foundArgs;
    return argsCount;
}

INLINE int qspInitRegularArgs(QSPCachedArg **args, int statCode, QSPString s, QSP_CHAR *origStart, int *errorCode)
{
    QSPCachedArg *foundArgs;
    QSP_CHAR *pos, *brack;
    int bufSize, count;
    qspSkipSpaces(&s);
    if (!qspIsEmpty(s) && *s.Str == QSP_LRBRACK[0])
    {
        if (!(brack = qspStrPos(s, QSP_STATIC_STR(QSP_RRBRACK), QSP_FALSE)))
        {
            *errorCode = QSP_ERR_BRACKNOTFOUND;
            return 0;
        }
        if (!qspIsAnyString(qspStringFromPair(brack + QSP_STATIC_LEN(QSP_RRBRACK), s.End)))
        {
            s = qspStringFromPair(s.Str + QSP_STATIC_LEN(QSP_LRBRACK), brack);
            qspSkipSpaces(&s);
        }
    }
    count = 0;
    foundArgs = 0;
    if (!qspIsEmpty(s))
    {
        bufSize = 0;
        while (1)
        {
            if (count >= qspStats[statCode].MaxArgsCount)
            {
                *errorCode = QSP_ERR_ARGSCOUNT;
                break;
            }
            if (count >= bufSize)
            {
                bufSize = count + 4;
                foundArgs = (QSPCachedArg *)realloc(foundArgs, bufSize * sizeof(QSPCachedArg));
            }
            pos = qspStrPos(s, QSP_STATIC_STR(QSP_COMMA), QSP_FALSE);
            if (pos)
            {
                foundArgs[count].StartPos = (int)(s.Str - origStart);
                foundArgs[count].EndPos = (int)(pos - origStart);
                ++count;
            }
            else
            {
                foundArgs[count].StartPos = (int)(s.Str - origStart);
                foundArgs[count].EndPos = (int)(s.End - origStart);
                ++count;
                break;
            }
            s.Str = pos + QSP_STATIC_LEN(QSP_COMMA);
            qspSkipSpaces(&s);
            if (qspIsEmpty(s))
            {
                *errorCode = QSP_ERR_SYNTAX;
                break;
            }
        }
    }
    if (count < qspStats[statCode].MinArgsCount)
        *errorCode = QSP_ERR_ARGSCOUNT;
    *args = foundArgs;
    return count;
}

QSPString qspGetLineLabel(QSPString str)
{
    QSP_CHAR *delimPos;
    qspSkipSpaces(&str);
    if (!qspIsEmpty(str) && *str.Str == QSP_LABEL[0])
    {
        delimPos = qspStrChar(str, QSP_STATDELIM[0]);
        if (delimPos)
            str = qspStringFromPair(str.Str + QSP_STATIC_LEN(QSP_LABEL), delimPos);
        else
            str = qspStringFromPair(str.Str + QSP_STATIC_LEN(QSP_LABEL), str.End);
        str = qspGetNewText(qspDelSpc(str));
        qspUpperStr(&str);
        return str;
    }
    return qspNullString;
}

void qspInitLineOfCode(QSPLineOfCode *line, QSPString str, int lineNum)
{
    QSP_BOOL isInLoop, isSearchElse;
    int statCode, count = 0;
    QSP_CHAR *temp, *nextPos, *elsePos, *delimPos = 0, *paramPos = 0;
    line->Str = str;
    line->LineNum = lineNum;
    line->StatsCount = 0;
    line->Stats = 0;
    qspSkipSpaces(&str);
    statCode = qspGetStatCode(str, &paramPos);
    if (!qspIsEmpty(str) && statCode != qspStatComment)
    {
        isInLoop = isSearchElse = QSP_TRUE;
        elsePos = 0;
        switch (statCode)
        {
            case qspStatAct:
            case qspStatLoop:
            case qspStatIf:
            case qspStatElseIf:
                delimPos = qspStrPos(str, QSP_STATIC_STR(QSP_COLONDELIM), QSP_FALSE);
                if (delimPos)
                {
                    nextPos = delimPos + QSP_STATIC_LEN(QSP_COLONDELIM);
                    if (nextPos == str.End) isInLoop = QSP_FALSE;
                }
                break;
            case qspStatElse:
                str.Str = paramPos;
                qspSkipSpaces(&str);
                nextPos = str.Str;
                if (nextPos != str.End)
                {
                    if (*nextPos == QSP_COLONDELIM[0]) ++nextPos;
                    delimPos = nextPos;
                }
                else
                    delimPos = 0;
                break;
            default:
                delimPos = qspStrPos(str, QSP_STATIC_STR(QSP_STATDELIM), QSP_FALSE);
                if (delimPos) nextPos = delimPos + QSP_STATIC_LEN(QSP_STATDELIM);
                elsePos = qspStrPos(str, QSP_STATIC_STR(QSP_STATELSE), QSP_TRUE);
                if (elsePos)
                {
                    if (!delimPos || elsePos < delimPos)
                    {
                        nextPos = delimPos = elsePos;
                        elsePos = 0;
                    }
                }
                else
                    isSearchElse = QSP_FALSE;
                if (statCode == qspStatUnknown && str.Str != delimPos)
                {
                    if (delimPos)
                        temp = qspStrPos(qspStringFromPair(str.Str, delimPos), QSP_STATIC_STR(QSP_EQUAL), QSP_FALSE);
                    else
                        temp = qspStrPos(str, QSP_STATIC_STR(QSP_EQUAL), QSP_FALSE);
                    statCode = (temp ? qspStatSet : qspStatMPL);
                }
                break;
        }
        while (delimPos && isInLoop)
        {
            line->StatsCount++;
            line->Stats = (QSPCachedStat *)realloc(line->Stats, line->StatsCount * sizeof(QSPCachedStat));
            line->Stats[count].Stat = statCode;
            if (paramPos)
            {
                str.Str = paramPos;
                qspSkipSpaces(&str);
            }
            line->Stats[count].ParamPos = (int)(str.Str - line->Str.Str);
            line->Stats[count].EndPos = (int)(delimPos - line->Str.Str);
            line->Stats[count].ArgsCount = qspInitStatArgs(&line->Stats[count].Args, statCode, qspStringFromPair(str.Str, delimPos), line->Str.Str, &line->Stats[count].ErrorCode);
            ++count;
            str.Str = nextPos;
            qspSkipSpaces(&str);
            paramPos = 0;
            statCode = qspGetStatCode(str, &paramPos);
            if (!qspIsEmpty(str) && statCode != qspStatComment)
            {
                switch (statCode)
                {
                    case qspStatAct:
                    case qspStatLoop:
                    case qspStatIf:
                    case qspStatElseIf:
                        delimPos = qspStrPos(str, QSP_STATIC_STR(QSP_COLONDELIM), QSP_FALSE);
                        if (delimPos)
                        {
                            nextPos = delimPos + QSP_STATIC_LEN(QSP_COLONDELIM);
                            if (nextPos == str.End) isInLoop = QSP_FALSE;
                        }
                        break;
                    case qspStatElse:
                        str.Str = paramPos;
                        qspSkipSpaces(&str);
                        nextPos = str.Str;
                        if (nextPos != str.End)
                        {
                            if (*nextPos == QSP_COLONDELIM[0]) ++nextPos;
                            delimPos = nextPos;
                        }
                        else
                            delimPos = 0;
                        break;
                    default:
                        delimPos = qspStrPos(str, QSP_STATIC_STR(QSP_STATDELIM), QSP_FALSE);
                        if (delimPos) nextPos = delimPos + QSP_STATIC_LEN(QSP_STATDELIM);
                        if (elsePos && str.Str >= elsePos) elsePos = 0;
                        if (!elsePos && isSearchElse)
                        {
                            elsePos = qspStrPos(str, QSP_STATIC_STR(QSP_STATELSE), QSP_TRUE);
                            if (!elsePos) isSearchElse = QSP_FALSE;
                        }
                        if (elsePos && (!delimPos || elsePos < delimPos))
                        {
                            nextPos = delimPos = elsePos;
                            elsePos = 0;
                        }
                        if (statCode == qspStatUnknown && str.Str != delimPos)
                        {
                            if (delimPos)
                                temp = qspStrPos(qspStringFromPair(str.Str, delimPos), QSP_STATIC_STR(QSP_EQUAL), QSP_FALSE);
                            else
                                temp = qspStrPos(str, QSP_STATIC_STR(QSP_EQUAL), QSP_FALSE);
                            statCode = (temp ? qspStatSet : qspStatMPL);
                        }
                        break;
                }
            }
            else
                delimPos = 0;
        }
    }
    /* Check for ELSE IF */
    if (count == 1 && line->Stats[0].Stat == qspStatElse && statCode == qspStatIf &&
        *(line->Str.Str + line->Stats[0].ParamPos) != QSP_COLONDELIM[0])
    {
        count = 0;
        statCode = qspStatElseIf;
    }
    else
    {
        line->StatsCount++;
        line->Stats = (QSPCachedStat *)realloc(line->Stats, line->StatsCount * sizeof(QSPCachedStat));
    }
    line->Stats[count].Stat = statCode;
    if (paramPos)
    {
        str.Str = paramPos;
        qspSkipSpaces(&str);
    }
    line->Stats[count].ParamPos = (int)(str.Str - line->Str.Str);
    if (delimPos)
    {
        line->Stats[count].EndPos = (int)(delimPos - line->Str.Str);
        line->Stats[count].ArgsCount = qspInitStatArgs(&line->Stats[count].Args, statCode, qspStringFromPair(str.Str, delimPos), line->Str.Str, &line->Stats[count].ErrorCode);
    }
    else
    {
        line->Stats[count].EndPos = (int)(str.End - line->Str.Str);
        line->Stats[count].ArgsCount = qspInitStatArgs(&line->Stats[count].Args, statCode, str, line->Str.Str, &line->Stats[count].ErrorCode);
    }
    switch (line->Stats[0].Stat)
    {
        case qspStatAct:
        case qspStatLoop:
        case qspStatIf:
        case qspStatElseIf:
            line->IsMultiline = (line->StatsCount == 1 && *(line->Str.End - 1) == QSP_COLONDELIM[0]);
            break;
        default:
            line->IsMultiline = QSP_FALSE;
            break;
    }
    line->Label = qspGetLineLabel(line->Str);
}

void qspFreeLineOfCode(QSPLineOfCode *line)
{
    int i;
    qspFreeString(line->Label);
    if (line->Stats)
    {
        for (i = 0; i < line->StatsCount; ++i)
            if (line->Stats[i].Args) free(line->Stats[i].Args);
        free(line->Stats);
    }
}

void qspFreePrepLines(QSPLineOfCode *strs, int count)
{
    if (strs)
    {
        while (--count >= 0)
        {
            qspFreeString(strs[count].Str);
            qspFreeLineOfCode(strs + count);
        }
        free(strs);
    }
}

void qspCopyPrepLines(QSPLineOfCode **dest, QSPLineOfCode *src, int start, int end)
{
    QSPLineOfCode *line;
    int i, j, statsCount, argsCount, linesCount = end - start;
    if (src && linesCount)
    {
        *dest = (QSPLineOfCode *)malloc(linesCount * sizeof(QSPLineOfCode));
        line = *dest;
        while (start < end)
        {
            line->Str = qspGetNewText(src[start].Str);
            line->LineNum = src[start].LineNum;
            statsCount = line->StatsCount = src[start].StatsCount;
            if (statsCount)
            {
                line->Stats = (QSPCachedStat *)malloc(statsCount * sizeof(QSPCachedStat));
                for (i = 0; i < statsCount; ++i)
                {
                    line->Stats[i].Stat = src[start].Stats[i].Stat;
                    line->Stats[i].ParamPos = src[start].Stats[i].ParamPos;
                    line->Stats[i].EndPos = src[start].Stats[i].EndPos;
                    line->Stats[i].ErrorCode = src[start].Stats[i].ErrorCode;
                    argsCount = line->Stats[i].ArgsCount = src[start].Stats[i].ArgsCount;
                    if (argsCount)
                    {
                        line->Stats[i].Args = (QSPCachedArg *)malloc(argsCount * sizeof(QSPCachedArg));
                        for (j = 0; j < argsCount; ++j)
                        {
                            line->Stats[i].Args[j].StartPos = src[start].Stats[i].Args[j].StartPos;
                            line->Stats[i].Args[j].EndPos = src[start].Stats[i].Args[j].EndPos;
                        }
                    }
                    else
                        line->Stats[i].Args = 0;
                }
            }
            else
                line->Stats = 0;
            line->IsMultiline = src[start].IsMultiline;
            if (src[start].Label.Str)
                line->Label = qspGetNewText(src[start].Label);
            else
                line->Label = qspNullString;
            ++line;
            ++start;
        }
    }
    else
        *dest = 0;
}

QSPString qspJoinPrepLines(QSPLineOfCode *s, int count, QSPString delim)
{
    int i, itemLen, txtLen = 0, txtRealLen = 0, bufSize = 256, delimLen = qspStrLen(delim);
    QSP_CHAR *txt = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
    for (i = 0; i < count; ++i)
    {
        itemLen = qspStrLen(s[i].Str);
        if ((txtLen += itemLen) > bufSize)
        {
            bufSize = txtLen + 128;
            txt = (QSP_CHAR *)realloc(txt, bufSize * sizeof(QSP_CHAR));
        }
        memcpy(txt + txtRealLen, s[i].Str.Str, itemLen * sizeof(QSP_CHAR));
        if (i == count - 1) break;
        txtRealLen = txtLen;
        if ((txtLen += delimLen) > bufSize)
        {
            bufSize = txtLen + 128;
            txt = (QSP_CHAR *)realloc(txt, bufSize * sizeof(QSP_CHAR));
        }
        memcpy(txt + txtRealLen, delim.Str, delimLen * sizeof(QSP_CHAR));
        txtRealLen = txtLen;
    }
    return qspStringFromLen(txt, txtLen);
}

void qspPrepareStringToExecution(QSPString *str)
{
    QSP_CHAR *pos, quot = 0, quotsCount = 0;
    pos = str->Str;
    while (pos < str->End)
    {
        if (quot)
        {
            if (*pos == quot)
            {
                ++pos;
                if (pos >= str->End) break;
                if (*pos != quot)
                {
                    quot = 0;
                    continue;
                }
            }
        }
        else
        {
            if (*pos == QSP_LQUOT[0])
                ++quotsCount;
            else if (*pos == QSP_RQUOT[0])
            {
                if (quotsCount) --quotsCount;
            }
            else if (qspIsInClass(*pos, QSP_CHAR_QUOT))
                quot = *pos;
            else if (!quotsCount) /* we have to keep q-strings untouched */
                *pos = qspToWUpper(*pos);
        }
        ++pos;
    }
}

INLINE int qspProcessPreformattedStrings(QSPString data, QSPLineOfCode **strs)
{
    QSPLineOfCode *ret, *line;
    QSP_BOOL isNewLine;
    QSP_CHAR *str, *pos, quot = 0;
    int lineNum = 0, lastLineNum = 0, count = 0, quotsCount = 0, strLen = 0, bufSize = 8, strBufSize = 256;
    str = (QSP_CHAR *)malloc(strBufSize * sizeof(QSP_CHAR));
    ret = (QSPLineOfCode *)malloc(bufSize * sizeof(QSPLineOfCode));
    pos = data.Str;
    while (pos < data.End)
    {
        isNewLine = (qspStrsNComp(data, QSP_STATIC_STR(QSP_STRSDELIM), QSP_STATIC_LEN(QSP_STRSDELIM)) == 0);
        if (isNewLine) ++lineNum;
        if (quotsCount || quot || !isNewLine)
        {
            if (strLen >= strBufSize)
            {
                strBufSize = strLen + 256;
                str = (QSP_CHAR *)realloc(str, strBufSize * sizeof(QSP_CHAR));
            }
            str[strLen++] = *pos;
            if (quot)
            {
                if (*pos == quot)
                {
                    if (++pos < data.End && *pos == quot)
                    {
                        /* escape code */
                        if (strLen >= strBufSize)
                        {
                            strBufSize = strLen + 256;
                            str = (QSP_CHAR *)realloc(str, strBufSize * sizeof(QSP_CHAR));
                        }
                        str[strLen++] = *pos++;
                    }
                    else
                    {
                        /* termination of the string */
                        quot = 0;
                    }
                }
                else
                    ++pos;
            }
            else
            {
                if (*pos == QSP_LQUOT[0])
                    ++quotsCount;
                else if (*pos == QSP_RQUOT[0])
                {
                    if (quotsCount) --quotsCount;
                }
                else if (qspIsInClass(*pos, QSP_CHAR_QUOT))
                    quot = *pos;
                ++pos;
            }
        }
        else
        {
            if (count >= bufSize)
            {
                bufSize = count + 16;
                ret = (QSPLineOfCode *)realloc(ret, bufSize * sizeof(QSPLineOfCode));
            }
            line = ret + count++;
            line->Str = qspGetNewText(qspDelSpc(qspStringFromLen(str, strLen)));
            line->LineNum = lastLineNum;
            line->Label = qspNullString;
            line->Stats = 0;
            lastLineNum = lineNum;
            strLen = 0;
            pos += QSP_STATIC_LEN(QSP_STRSDELIM);
        }
        data.Str = pos;
    }
    if (count >= bufSize)
        ret = (QSPLineOfCode *)realloc(ret, (count + 1) * sizeof(QSPLineOfCode));
    line = ret + count++;
    line->Str = qspGetNewText(qspDelSpc(qspStringFromLen(str, strLen)));
    line->LineNum = lastLineNum;
    line->Label = qspNullString;
    line->Stats = 0;
    free(str);
    *strs = ret;
    return count;
}

INLINE int qspProcessEOLExtensions(QSPLineOfCode *s, int count, QSPLineOfCode **strs)
{
    QSPLineOfCode *ret;
    QSPString strBuf, eol;
    int len, lastNum = 0, i = 0, bufSize = 8, newCount = 0;
    ret = (QSPLineOfCode *)malloc(bufSize * sizeof(QSPLineOfCode));
    while (i < count)
    {
        qspAddText(&strBuf, s[i].Str, QSP_TRUE);
        len = qspStrLen(strBuf);
        if (len >= QSP_STATIC_LEN(QSP_PREEOLEXT QSP_EOLEXT))
        {
            eol = strBuf;
            eol.Str += len - QSP_STATIC_LEN(QSP_PREEOLEXT QSP_EOLEXT);
            while (!qspStrsComp(eol, QSP_STATIC_STR(QSP_PREEOLEXT QSP_EOLEXT)))
            {
                if (++i >= count) break;
                strBuf.End -= QSP_STATIC_LEN(QSP_EOLEXT);
                qspAddText(&strBuf, s[i].Str, QSP_FALSE);
                len = qspStrLen(strBuf);
                eol = strBuf;
                eol.Str += len - QSP_STATIC_LEN(QSP_PREEOLEXT QSP_EOLEXT);
            }
        }
        if (newCount >= bufSize)
        {
            bufSize = newCount + 16;
            ret = (QSPLineOfCode *)realloc(ret, bufSize * sizeof(QSPLineOfCode));
        }
        /* prepare the buffer to execution */
        qspPrepareStringToExecution(&strBuf);
        /* transfer ownership of the buffer to QSPLineOfCode */
        qspInitLineOfCode(ret + newCount, strBuf, lastNum);
        ++newCount;
        ++i;
        lastNum = s[i].LineNum;
    }
    *strs = ret;
    return newCount;
}

int qspPreprocessData(QSPString data, QSPLineOfCode **strs)
{
    QSPLineOfCode *s;
    int res, count = qspProcessPreformattedStrings(data, &s);
    res = qspProcessEOLExtensions(s, count, strs);
    qspFreePrepLines(s, count);
    return res;
}
