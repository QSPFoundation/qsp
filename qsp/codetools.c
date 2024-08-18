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

#include "codetools.h"
#include "statements.h"
#include "text.h"

INLINE int qspStatStringCompare(const void *name, const void *compareTo);
INLINE QSP_TINYINT qspGetStatCode(QSPString s, QSP_CHAR **pos);
INLINE QSP_TINYINT qspInitStatArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, int *errorCode);
INLINE QSP_TINYINT qspInitSetArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, int *errorCode);
INLINE QSP_TINYINT qspInitRegularArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, int *errorCode);
INLINE int qspProcessPreformattedStrings(QSPString data, QSPLineOfCode **strs);
INLINE int qspProcessEOLExtensions(QSPLineOfCode *s, int count, QSPLineOfCode **strs);

INLINE int qspStatStringCompare(const void *name, const void *compareTo)
{
    QSPStatName *statName = (QSPStatName *)compareTo;
    return qspStrsNComp(*(QSPString *)name, statName->Name, qspStrLen(statName->Name));
}

INLINE QSP_TINYINT qspGetStatCode(QSPString s, QSP_CHAR **pos)
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

INLINE QSP_TINYINT qspInitStatArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, int *errorCode)
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

INLINE QSP_TINYINT qspInitSetArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, int *errorCode)
{
    QSP_TINYINT argsCount;
    QSPCachedArg *foundArgs;
    QSP_CHAR *pos;
    qspSkipSpaces(&s);
    pos = qspDelimPos(s, QSP_EQUAL[0]);
    if (pos)
    {
        QSPString names, values, op;
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
        QSPString names = qspDelSpc(s);
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

INLINE QSP_TINYINT qspInitRegularArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, int *errorCode)
{
    QSPCachedArg *foundArgs = 0;
    QSP_TINYINT argsCount = 0;
    qspSkipSpaces(&s);
    if (!qspIsEmpty(s))
    {
        if (statCode == qspStatImplicitStatement)
        {
            /* It's always 1 argument only */
            argsCount = 1;
            foundArgs = (QSPCachedArg *)malloc(argsCount * sizeof(QSPCachedArg));
            foundArgs[0].StartPos = (int)(s.Str - origStart);
            foundArgs[0].EndPos = (int)(s.End - origStart);
        }
        else
        {
            QSP_CHAR *pos;
            int bufSize = 0;
            if (*s.Str == QSP_LRBRACK[0]) /* arguments might be specified using parentheses */
            {
                QSP_CHAR *bracket;
                if (!(bracket = qspDelimPos(s, QSP_RRBRACK[0])))
                {
                    *errorCode = QSP_ERR_BRACKNOTFOUND;
                    return 0;
                }
                if (!qspIsAnyString(qspStringFromPair(bracket + QSP_STATIC_LEN(QSP_RRBRACK), s.End)))
                {
                    /* We'll parse arguments between parentheses */
                    s = qspStringFromPair(s.Str + QSP_STATIC_LEN(QSP_LRBRACK), bracket);
                    qspSkipSpaces(&s);
                }
            }
            while (1)
            {
                if (argsCount >= qspStats[statCode].MaxArgsCount)
                {
                    *errorCode = QSP_ERR_ARGSCOUNT;
                    break;
                }
                if (argsCount >= bufSize)
                {
                    bufSize = argsCount + 4;
                    foundArgs = (QSPCachedArg *)realloc(foundArgs, bufSize * sizeof(QSPCachedArg));
                }
                pos = qspDelimPos(s, QSP_COMMA[0]);
                if (pos)
                {
                    foundArgs[argsCount].StartPos = (int)(s.Str - origStart);
                    foundArgs[argsCount].EndPos = (int)(pos - origStart);
                    ++argsCount;
                }
                else
                {
                    foundArgs[argsCount].StartPos = (int)(s.Str - origStart);
                    foundArgs[argsCount].EndPos = (int)(s.End - origStart);
                    ++argsCount;
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
    }
    if (argsCount < qspStats[statCode].MinArgsCount)
        *errorCode = QSP_ERR_ARGSCOUNT;
    *args = foundArgs;
    return argsCount;
}

QSPString qspGetLineLabel(QSPString str)
{
    qspSkipSpaces(&str);
    if (!qspIsEmpty(str) && *str.Str == QSP_LABEL[0])
    {
        QSP_CHAR *delimPos = qspStrChar(str, QSP_STATDELIM[0]);
        if (delimPos)
            str = qspStringFromPair(str.Str + QSP_STATIC_LEN(QSP_LABEL), delimPos);
        else
            str = qspStringFromPair(str.Str + QSP_STATIC_LEN(QSP_LABEL), str.End);
        str = qspCopyToNewText(qspDelSpc(str));
        qspUpperStr(&str);
        return str;
    }
    return qspNullString;
}

void qspInitLineOfCode(QSPLineOfCode *line, QSPString str, int lineNum)
{
    QSP_TINYINT statCode;
    int statInd = 0;
    QSP_CHAR *nextPos, *elsePos, *statDelimPos = 0, *paramPos = 0;
    /* 'nextPos' points to the next position to search for a statement */
    /* 'statDelimPos' points to the statement separator (':' or '&') */
    line->Str = str;
    line->Label = qspNullString;
    line->LineNum = lineNum;
    line->LinesToElse = line->LinesToEnd = 0;
    line->StatsCount = 0;
    line->Stats = 0;
    qspSkipSpaces(&str);
    if (qspIsEmpty(str)) return;
    statCode = qspGetStatCode(str, &paramPos);
    if (statCode != qspStatComment)
    {
        QSP_CHAR *temp;
        QSP_BOOL toSearchElse = QSP_TRUE;
        elsePos = 0;
        switch (statCode)
        {
            case qspStatAct:
            case qspStatLoop:
            case qspStatIf:
            case qspStatElseIf:
                statDelimPos = qspDelimPos(str, QSP_COLONDELIM[0]);
                if (statDelimPos)
                {
                    nextPos = statDelimPos + QSP_STATIC_LEN(QSP_COLONDELIM);
                    if (nextPos == str.End) nextPos = 0;
                }
                break;
            case qspStatElse:
                str.Str = paramPos;
                qspSkipSpaces(&str);
                nextPos = str.Str;
                if (nextPos < str.End)
                {
                    statDelimPos = nextPos;
                    if (*nextPos == QSP_COLONDELIM[0])
                    {
                        nextPos += QSP_STATIC_LEN(QSP_COLONDELIM);
                        if (nextPos == str.End) nextPos = 0;
                    }
                }
                break;
            default:
                statDelimPos = qspDelimPos(str, QSP_STATDELIM[0]);
                if (statDelimPos) nextPos = statDelimPos + QSP_STATIC_LEN(QSP_STATDELIM);
                elsePos = qspStrPos(str, QSP_STATIC_STR(QSP_STATELSE), QSP_TRUE);
                if (elsePos)
                {
                    if (!statDelimPos || elsePos < statDelimPos)
                    {
                        nextPos = statDelimPos = elsePos;
                        elsePos = 0;
                    }
                }
                else
                    toSearchElse = QSP_FALSE;
                if (statCode == qspStatUnknown && str.Str != statDelimPos)
                {
                    if (statDelimPos)
                        temp = qspDelimPos(qspStringFromPair(str.Str, statDelimPos), QSP_EQUAL[0]);
                    else
                        temp = qspDelimPos(str, QSP_EQUAL[0]);
                    statCode = (temp ? qspStatSet : qspStatImplicitStatement);
                }
                break;
        }
        while (statDelimPos && nextPos)
        {
            line->StatsCount++;
            line->Stats = (QSPCachedStat *)realloc(line->Stats, line->StatsCount * sizeof(QSPCachedStat));
            line->Stats[statInd].Stat = statCode;
            if (paramPos)
            {
                str.Str = paramPos;
                qspSkipSpaces(&str);
            }
            line->Stats[statInd].ParamPos = (int)(str.Str - line->Str.Str);
            line->Stats[statInd].EndPos = (int)(statDelimPos - line->Str.Str);
            line->Stats[statInd].ArgsCount = qspInitStatArgs(&line->Stats[statInd].Args, statCode, qspStringFromPair(str.Str, statDelimPos), line->Str.Str, &line->Stats[statInd].ErrorCode);
            ++statInd;
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
                        statDelimPos = qspDelimPos(str, QSP_COLONDELIM[0]);
                        if (statDelimPos)
                        {
                            nextPos = statDelimPos + QSP_STATIC_LEN(QSP_COLONDELIM);
                            if (nextPos == str.End) nextPos = 0;
                        }
                        break;
                    case qspStatElse:
                        str.Str = paramPos;
                        qspSkipSpaces(&str);
                        nextPos = str.Str;
                        if (nextPos < str.End)
                        {
                            statDelimPos = nextPos;
                            if (*nextPos == QSP_COLONDELIM[0])
                            {
                                nextPos += QSP_STATIC_LEN(QSP_COLONDELIM);
                                if (nextPos == str.End) nextPos = 0;
                            }
                        }
                        else
                            statDelimPos = 0;
                        break;
                    default:
                        statDelimPos = qspDelimPos(str, QSP_STATDELIM[0]);
                        if (statDelimPos) nextPos = statDelimPos + QSP_STATIC_LEN(QSP_STATDELIM);
                        if (elsePos && str.Str >= elsePos) elsePos = 0;
                        if (!elsePos && toSearchElse)
                        {
                            elsePos = qspStrPos(str, QSP_STATIC_STR(QSP_STATELSE), QSP_TRUE);
                            if (!elsePos) toSearchElse = QSP_FALSE;
                        }
                        if (elsePos && (!statDelimPos || elsePos < statDelimPos))
                        {
                            nextPos = statDelimPos = elsePos;
                            elsePos = 0;
                        }
                        if (statCode == qspStatUnknown && str.Str != statDelimPos)
                        {
                            if (statDelimPos)
                                temp = qspDelimPos(qspStringFromPair(str.Str, statDelimPos), QSP_EQUAL[0]);
                            else
                                temp = qspDelimPos(str, QSP_EQUAL[0]);
                            statCode = (temp ? qspStatSet : qspStatImplicitStatement);
                        }
                        break;
                }
            }
            else
                statDelimPos = 0;
        }
    }
    /* Check for ELSE IF */
    if (statInd == 1 && line->Stats[0].Stat == qspStatElse && statCode == qspStatIf &&
        *(line->Str.Str + line->Stats[0].ParamPos) != QSP_COLONDELIM[0])
    {
        /* Convert a multi-line ELSE IF to ELSEIF */
        statInd = 0;
        statCode = qspStatElseIf;
    }
    else
    {
        line->StatsCount++;
        line->Stats = (QSPCachedStat *)realloc(line->Stats, line->StatsCount * sizeof(QSPCachedStat));
    }
    line->Stats[statInd].Stat = statCode;
    if (paramPos)
    {
        str.Str = paramPos;
        qspSkipSpaces(&str);
    }
    line->Stats[statInd].ParamPos = (int)(str.Str - line->Str.Str);
    if (statDelimPos)
    {
        line->Stats[statInd].EndPos = (int)(statDelimPos - line->Str.Str);
        line->Stats[statInd].ArgsCount = qspInitStatArgs(&line->Stats[statInd].Args, statCode, qspStringFromPair(str.Str, statDelimPos), line->Str.Str, &line->Stats[statInd].ErrorCode);
    }
    else
    {
        line->Stats[statInd].EndPos = (int)(str.End - line->Str.Str);
        line->Stats[statInd].ArgsCount = qspInitStatArgs(&line->Stats[statInd].Args, statCode, str, line->Str.Str, &line->Stats[statInd].ErrorCode);
    }
    switch (line->Stats[0].Stat)
    {
        case qspStatAct:
        case qspStatLoop:
        case qspStatIf:
            if (line->StatsCount == 1 && *(line->Str.End - 1) == QSP_COLONDELIM[0])
                line->LinesToElse = line->LinesToEnd = 1; /* we don't have all the lines ready to find the right ones yet */
            break;
        case qspStatElseIf:
        case qspStatElse:
            if (line->StatsCount == 1)
                line->LinesToEnd = 1; /* we don't have all the lines ready to find the right ones yet */
            line->LinesToElse = 1; /* always search next ELSE starting next line */
            break;
    }
    line->Label = qspGetLineLabel(line->Str);
}

void qspFreeLineOfCode(QSPLineOfCode *line)
{
    qspFreeString(&line->Label);
    if (line->Stats)
    {
        int i;
        QSPCachedStat *stat = line->Stats;
        for (i = 0; i < line->StatsCount; ++i, ++stat)
            if (stat->Args) free(stat->Args);
        free(line->Stats);
    }
}

void qspFreePrepLines(QSPLineOfCode *strs, int count)
{
    if (strs)
    {
        QSPLineOfCode *curStr = strs;
        while (--count >= 0)
        {
            qspFreeString(&curStr->Str);
            qspFreeLineOfCode(curStr);
            ++curStr;
        }
        free(strs);
    }
}

void qspCopyPrepStatements(QSPCachedStat **dest, QSPCachedStat *src, int start, int end, int codeOffset)
{
    int statsCount = end - start;
    if (src && statsCount)
    {
        QSP_TINYINT i, argsCount;
        QSPCachedStat *stat;
        *dest = (QSPCachedStat *)malloc(statsCount * sizeof(QSPCachedStat));
        stat = *dest;
        while (start < end)
        {
            stat->Stat = src[start].Stat;
            stat->ParamPos = src[start].ParamPos - codeOffset;
            stat->EndPos = src[start].EndPos - codeOffset;
            stat->ErrorCode = src[start].ErrorCode;
            argsCount = stat->ArgsCount = src[start].ArgsCount;
            if (argsCount)
            {
                stat->Args = (QSPCachedArg *)malloc(argsCount * sizeof(QSPCachedArg));
                for (i = 0; i < argsCount; ++i)
                {
                    stat->Args[i].StartPos = src[start].Args[i].StartPos - codeOffset;
                    stat->Args[i].EndPos = src[start].Args[i].EndPos - codeOffset;
                }
            }
            else
                stat->Args = 0;
            ++stat;
            ++start;
        }
    }
    else
        *dest = 0;
}

void qspCopyPrepLines(QSPLineOfCode **dest, QSPLineOfCode *src, int start, int end)
{
    int linesCount = end - start;
    if (src && linesCount)
    {
        QSPLineOfCode *line;
        *dest = (QSPLineOfCode *)malloc(linesCount * sizeof(QSPLineOfCode));
        line = *dest;
        while (start < end)
        {
            line->Str = qspCopyToNewText(src[start].Str);
            line->LineNum = src[start].LineNum;
            line->LinesToEnd = src[start].LinesToEnd;
            line->LinesToElse = src[start].LinesToElse;
            line->Label = qspCopyToNewText(src[start].Label);
            line->StatsCount = src[start].StatsCount;
            qspCopyPrepStatements(&line->Stats, src[start].Stats, 0, src[start].StatsCount, 0);
            ++line;
            ++start;
        }
    }
    else
        *dest = 0;
}

QSPString qspJoinPrepLines(QSPLineOfCode *s, int count, QSPString delim)
{
    int i;
    QSPBufString res = qspNewBufString(256);
    for (i = 0; i < count; ++i)
    {
        qspAddBufText(&res, s[i].Str);
        if (i == count - 1) break; /* don't add the delim */
        qspAddBufText(&res, delim);
    }
    return qspBufTextToString(res);
}

void qspPrepareStringToExecution(QSPString *str)
{
    int quotsCount = 0;
    QSP_CHAR *pos = str->Str;
    while (pos < str->End)
    {
        if (*pos == QSP_LQUOT[0])
            ++quotsCount;
        else if (*pos == QSP_RQUOT[0])
        {
            if (quotsCount) --quotsCount;
        }
        else if (qspIsInClass(*pos, QSP_CHAR_QUOT))
        {
            QSP_CHAR quot = *pos;
            while (++pos < str->End)
            {
                if (*pos == quot)
                {
                    ++pos;
                    if (pos >= str->End) break;
                    if (*pos != quot) break;
                }
            }
            continue;
        }
        else if (!quotsCount) /* we have to keep q-strings untouched */
            *pos = QSP_CHRUPR(*pos);
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
            /* Initialize the line data, the line gets updated a bit later */
            line = ret + count++;
            line->Str = qspCopyToNewText(qspDelSpc(qspStringFromLen(str, strLen)));
            line->LineNum = lastLineNum;
            line->Label = qspNullString;
            line->LinesToElse = line->LinesToEnd = 0;
            line->Stats = 0;
            line->StatsCount = 0;
            lastLineNum = lineNum;
            strLen = 0;
            pos += QSP_STATIC_LEN(QSP_STRSDELIM);
        }
        data.Str = pos;
    }
    if (count >= bufSize)
        ret = (QSPLineOfCode *)realloc(ret, (count + 1) * sizeof(QSPLineOfCode));
    /* Initialize the line data, the line gets updated a bit later */
    line = ret + count++;
    line->Str = qspCopyToNewText(qspDelSpc(qspStringFromLen(str, strLen)));
    line->LineNum = lastLineNum;
    line->Label = qspNullString;
    line->LinesToElse = line->LinesToEnd = 0;
    line->Stats = 0;
    line->StatsCount = 0;
    free(str);
    *strs = ret;
    return count;
}

INLINE int qspProcessEOLExtensions(QSPLineOfCode *s, int count, QSPLineOfCode **strs)
{
    QSPLineOfCode *ret;
    QSPString eol, line;
    QSPBufString strBuf;
    int lastNum = 0, i = 0, bufSize = 8, newCount = 0;
    int eolLen = QSP_STATIC_LEN(QSP_PREEOLEXT QSP_EOLEXT);
    ret = (QSPLineOfCode *)malloc(bufSize * sizeof(QSPLineOfCode));
    while (i < count)
    {
        strBuf = qspNewBufString(128);
        qspAddBufText(&strBuf, s[i].Str);
        if (strBuf.Len >= eolLen)
        {
            eol = qspStringFromLen(strBuf.Str + strBuf.Len - eolLen, eolLen);
            while (!qspStrsComp(eol, QSP_STATIC_STR(QSP_PREEOLEXT QSP_EOLEXT)))
            {
                if (++i >= count) break;
                strBuf.Len -= QSP_STATIC_LEN(QSP_EOLEXT);
                qspAddBufText(&strBuf, s[i].Str);
                eol = qspStringFromLen(strBuf.Str + strBuf.Len - eolLen, eolLen);
            }
        }
        if (newCount >= bufSize)
        {
            bufSize = newCount + 16;
            ret = (QSPLineOfCode *)realloc(ret, bufSize * sizeof(QSPLineOfCode));
        }
        line = qspBufTextToString(strBuf);
        /* prepare the buffer to execution */
        qspPrepareStringToExecution(&line);
        /* transfer ownership of the buffer to QSPLineOfCode */
        qspInitLineOfCode(ret + newCount, line, lastNum);
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
