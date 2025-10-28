/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "codetools.h"
#include "statements.h"
#include "text.h"

INLINE int qspStatStringCompare(const void *name, const void *compareTo);
INLINE QSP_TINYINT qspGetStatCode(QSPString s, QSP_CHAR **pos);
INLINE QSP_TINYINT qspInitStatArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode);
INLINE QSP_TINYINT qspInitSetArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode);
INLINE QSP_TINYINT qspAppendRegularArgs(QSPCachedArg **args, QSP_TINYINT argsCount, QSP_TINYINT minArgs, QSP_TINYINT maxArgs, QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode);
INLINE QSP_TINYINT qspInitUserCallArgs(QSPCachedArg **args, QSP_TINYINT QSP_UNUSED(statCode), QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode);
INLINE QSP_TINYINT qspInitSingleArg(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode);
INLINE QSP_TINYINT qspInitRegularArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode);
INLINE QSP_BOOL qspSkipCodeBlock(QSPString *str);
INLINE QSP_BOOL qspSkipQuotedString(QSPString *str);
INLINE QSP_BOOL qspAppendLineToResult(QSPString str, int lineNum, QSPBufString *strBuf, QSPLineOfCode *line);
INLINE void qspAppendLastLineToResult(QSPString str, int lineNum, QSPBufString *strBuf, QSPLineOfCode *line);

INLINE int qspStatStringCompare(const void *name, const void *compareTo)
{
    QSPStatName *statName = (QSPStatName *)compareTo;
    return qspStrsPartCompare(*(QSPString *)name, statName->Name);
}

INLINE QSP_TINYINT qspGetStatCode(QSPString s, QSP_CHAR **pos)
{
    int i, strLen, nameLen;
    QSPStatName *name;
    if (qspIsEmpty(s)) return qspStatUnknown;
    strLen = qspStrLen(s);
    for (i = 0; i < QSP_STATSLEVELS; ++i)
    {
        name = (QSPStatName *)bsearch(&s, qspStatsNames[i], qspStatsNamesCounts[i], sizeof(QSPStatName), qspStatStringCompare);
        if (name)
        {
            nameLen = qspStrLen(name->Name);
            if (name->IsIsolated)
            {
                if (nameLen == strLen || (nameLen < strLen && qspIsInClass(s.Str[nameLen], QSP_CHAR_DELIM)))
                {
                    *pos = s.Str + nameLen;
                    return name->Code;
                }
            }
            else
            {
                *pos = s.Str + nameLen;
                return name->Code;
            }
        }
    }
    return qspStatUnknown;
}

INLINE QSP_TINYINT qspInitStatArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode)
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
        case qspStatUserCall:
            return qspInitUserCallArgs(args, statCode, s, origStart, errorCode);
        case qspStatImplicitStatement:
        case qspStatIf:
        case qspStatElseIf:
            return qspInitSingleArg(args, statCode, s, origStart, errorCode);
        default:
            return qspInitRegularArgs(args, statCode, s, origStart, errorCode);
    }
}

INLINE QSP_TINYINT qspInitSetArgs(QSPCachedArg **args, QSP_TINYINT QSP_UNUSED(statCode), QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode)
{
    QSP_TINYINT argsCount;
    QSPCachedArg *foundArgs;
    QSP_CHAR *pos;
    qspSkipSpaces(&s);
    pos = qspDelimPos(s, QSP_EQUAL_CHAR);
    if (pos)
    {
        QSPString names, values, op;
        op = qspStringFromPair(pos, pos + QSP_CHAR_LEN);
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

INLINE QSP_TINYINT qspAppendRegularArgs(QSPCachedArg **args, QSP_TINYINT argsCount, QSP_TINYINT minArgs, QSP_TINYINT maxArgs, QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode)
{
    qspSkipSpaces(&s);
    if (!qspIsEmpty(s))
    {
        if (*s.Str == QSP_LRBRACK_CHAR) /* arguments might be specified using parentheses */
        {
            QSP_CHAR *bracket = qspDelimPos(s, QSP_RRBRACK_CHAR);
            if (!bracket)
            {
                *errorCode = QSP_ERR_BRACKNOTFOUND;
                return argsCount;
            }
            if (!qspIsAnyString(qspStringFromPair(bracket + QSP_CHAR_LEN, s.End)))
            {
                /* We'll parse arguments between parentheses */
                s = qspStringFromPair(s.Str + QSP_CHAR_LEN, bracket);
                qspSkipSpaces(&s);
            }
        }
        if (!qspIsEmpty(s))
        {
            QSP_CHAR *pos;
            int bufSize = argsCount;
            QSPCachedArg *foundArgs = *args;
            while (1)
            {
                if (argsCount >= maxArgs)
                {
                    *errorCode = QSP_ERR_ARGSCOUNT;
                    break;
                }
                if (argsCount >= bufSize)
                {
                    bufSize = argsCount + 4;
                    foundArgs = (QSPCachedArg *)realloc(foundArgs, bufSize * sizeof(QSPCachedArg));
                }
                pos = qspDelimPos(s, QSP_COMMA_CHAR);
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
                s.Str = pos + QSP_CHAR_LEN;
                qspSkipSpaces(&s);
                if (qspIsEmpty(s))
                {
                    *errorCode = QSP_ERR_SYNTAX;
                    break;
                }
            }
            *args = foundArgs;
        }
    }
    if (argsCount < minArgs)
        *errorCode = QSP_ERR_ARGSCOUNT;
    return argsCount;
}

INLINE QSP_TINYINT qspInitUserCallArgs(QSPCachedArg **args, QSP_TINYINT QSP_UNUSED(statCode), QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode)
{
    QSPCachedArg *foundArgs;
    QSP_TINYINT argsCount;
    QSP_CHAR *nameEnd = qspStrCharClass(s, QSP_CHAR_DELIM);
    foundArgs = (QSPCachedArg *)malloc(sizeof(QSPCachedArg));
    if (nameEnd)
    {
        foundArgs[0].StartPos = (int)(s.Str - origStart);
        foundArgs[0].EndPos = (int)(nameEnd - origStart);
        s.Str = nameEnd;
        argsCount = qspAppendRegularArgs(&foundArgs, 1, 1, QSP_STATMAXARGS, s, origStart, errorCode);
    }
    else
    {
        /* No extra arguments */
        foundArgs[0].StartPos = (int)(s.Str - origStart);
        foundArgs[0].EndPos = (int)(s.End - origStart);
        argsCount = 1;
    }
    *args = foundArgs;
    return argsCount;
}

INLINE QSP_TINYINT qspInitSingleArg(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode)
{
    QSPCachedArg *foundArgs = 0;
    QSP_TINYINT argsCount = 0;
    qspSkipSpaces(&s);
    if (!qspIsEmpty(s))
    {
        /* Consider the whole string as 1 argument */
        if (qspStats[statCode].MaxArgsCount)
        {
            foundArgs = (QSPCachedArg *)malloc(sizeof(QSPCachedArg));
            foundArgs[0].StartPos = (int)(s.Str - origStart);
            foundArgs[0].EndPos = (int)(s.End - origStart);
            argsCount = 1;
        }
        else
            *errorCode = QSP_ERR_ARGSCOUNT;
    }
    if (argsCount < qspStats[statCode].MinArgsCount)
        *errorCode = QSP_ERR_ARGSCOUNT;
    *args = foundArgs;
    return argsCount;
}

INLINE QSP_TINYINT qspInitRegularArgs(QSPCachedArg **args, QSP_TINYINT statCode, QSPString s, QSP_CHAR *origStart, QSP_TINYINT *errorCode)
{
    return qspAppendRegularArgs(args, 0, qspStats[statCode].MinArgsCount, qspStats[statCode].MaxArgsCount, s, origStart, errorCode);
}

QSPString qspGetLineLabel(QSPString str)
{
    qspSkipSpaces(&str);
    if (!qspIsEmpty(str) && *str.Str == QSP_LABEL_CHAR)
    {
        QSP_CHAR *delimPos = qspDelimPos(str, QSP_STATDELIM_CHAR);
        if (delimPos)
            str = qspStringFromPair(str.Str + QSP_CHAR_LEN, delimPos);
        else
            str = qspStringFromPair(str.Str + QSP_CHAR_LEN, str.End);
        str = qspCopyToNewText(qspDelSpc(str));
        qspUpperStr(&str);
        return str;
    }
    return qspNullString;
}

void qspInitLineOfCode(QSPLineOfCode *line, QSPString str, int lineNum)
{
    int statInd;
    QSP_TINYINT statCode;
    QSP_CHAR *statDelimPos, *paramPos;
    /* 'nextPos' points to the next position to search for a statement */
    /* 'statDelimPos' points to the statement separator (':' or '&') */
    line->Str = str;
    line->Label = qspNullString;
    line->LineNum = lineNum;
    line->LinesToElse = line->LinesToEnd = 0;
    line->IsMultiline = QSP_FALSE;
    line->StatsCount = 0;
    line->Stats = 0;
    qspSkipSpaces(&str);
    if (qspIsEmpty(str)) return;
    statInd = 0;
    statDelimPos = paramPos = 0;
    statCode = qspGetStatCode(str, &paramPos);
    if (statCode != qspStatComment)
    {
        QSP_CHAR *temp, *elsePos = 0, *nextPos = 0;
        QSP_BOOL toSearchElse = QSP_TRUE;
        switch (statCode)
        {
        case qspStatAct:
        case qspStatLoop:
        case qspStatIf:
        case qspStatElseIf:
            statDelimPos = qspDelimPos(str, QSP_COLONDELIM_CHAR);
            if (statDelimPos)
            {
                nextPos = statDelimPos + QSP_CHAR_LEN;
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
                if (*nextPos == QSP_COLONDELIM_CHAR)
                {
                    nextPos += QSP_CHAR_LEN;
                    if (nextPos == str.End) nextPos = 0;
                }
            }
            break;
        default:
            statDelimPos = qspDelimPos(str, QSP_STATDELIM_CHAR);
            if (statDelimPos) nextPos = statDelimPos + QSP_CHAR_LEN;
            elsePos = qspKeywordPos(str, QSP_STATIC_STR(QSP_STATELSE), QSP_TRUE);
            temp = qspKeywordPos(str, QSP_STATIC_STR(QSP_STATELSEIF), QSP_TRUE);
            if (temp && !(elsePos && elsePos < temp)) elsePos = temp; /* keep ELSE if it goes before ELSEIF */
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
                    temp = qspDelimPos(qspStringFromPair(str.Str, statDelimPos), QSP_EQUAL_CHAR);
                else
                    temp = qspDelimPos(str, QSP_EQUAL_CHAR);
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
                    statDelimPos = qspDelimPos(str, QSP_COLONDELIM_CHAR);
                    if (statDelimPos)
                    {
                        nextPos = statDelimPos + QSP_CHAR_LEN;
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
                        if (*nextPos == QSP_COLONDELIM_CHAR)
                        {
                            nextPos += QSP_CHAR_LEN;
                            if (nextPos == str.End) nextPos = 0;
                        }
                    }
                    else
                        statDelimPos = 0;
                    break;
                default:
                    statDelimPos = qspDelimPos(str, QSP_STATDELIM_CHAR);
                    if (statDelimPos) nextPos = statDelimPos + QSP_CHAR_LEN;
                    if (elsePos && str.Str >= elsePos) elsePos = 0;
                    if (!elsePos && toSearchElse)
                    {
                        elsePos = qspKeywordPos(str, QSP_STATIC_STR(QSP_STATELSE), QSP_TRUE);
                        temp = qspKeywordPos(str, QSP_STATIC_STR(QSP_STATELSEIF), QSP_TRUE);
                        if (temp && !(elsePos && elsePos < temp)) elsePos = temp; /* keep ELSE if it goes before ELSEIF */
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
                            temp = qspDelimPos(qspStringFromPair(str.Str, statDelimPos), QSP_EQUAL_CHAR);
                        else
                            temp = qspDelimPos(str, QSP_EQUAL_CHAR);
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
    if (statInd == 1
        && line->Stats[0].Stat == qspStatElse && statCode == qspStatIf
        && !qspIsCharAtPos(line->Str, line->Str.Str + line->Stats[0].ParamPos, QSP_COLONDELIM_CHAR))
    {
        /* Convert multiline ELSE IF to ELSEIF */
        statCode = qspStatElseIf; /* move current IF as ELSEIF to index 0, it's safe to overwrite ELSE */
        statInd = 0;
    }
    else if (statInd == 2
        && line->Stats[0].Stat == qspStatElse && line->Stats[1].Stat == qspStatIf && statCode == qspStatComment
        && !qspIsCharAtPos(line->Str, line->Str.Str + line->Stats[0].ParamPos, QSP_COLONDELIM_CHAR))
    {
        /* Convert multiline ELSE IF with a comment to ELSEIF with the comment */
        line->Stats[0].Stat = qspStatElseIf; /* move IF as ELSEIF to index 0, it's safe to overwrite ELSE */
        line->Stats[0].ParamPos = line->Stats[1].ParamPos;
        line->Stats[0].EndPos = line->Stats[1].EndPos;
        line->Stats[0].ArgsCount = line->Stats[1].ArgsCount;
        line->Stats[0].Args = line->Stats[1].Args;
        line->Stats[0].ErrorCode = line->Stats[1].ErrorCode;
        statInd = 1; /* move current comment to index 1 */
    }
    else
    {
        line->StatsCount++;
        line->Stats = (QSPCachedStat *)realloc(line->Stats, line->StatsCount * sizeof(QSPCachedStat));
    }
    /* Add the last statement */
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
    case qspStatElseIf:
        if (qspIsCharAtPos(line->Str, line->Str.Str + line->Stats[0].EndPos, QSP_COLONDELIM_CHAR))
        {
            if (line->StatsCount == 1)
                line->IsMultiline = QSP_TRUE;
            else if (line->StatsCount == 2 && line->Stats[1].Stat == qspStatComment)
                line->IsMultiline = QSP_TRUE;
        }
        /* Always search next ELSE/END starting next line since we don't have all the lines ready to find the right ones yet */
        line->LinesToEnd = line->LinesToElse = 1;
        break;
    case qspStatElse:
        if (line->StatsCount == 1)
            line->IsMultiline = QSP_TRUE;
        else if (line->StatsCount == 2 && line->Stats[1].Stat == qspStatComment)
            line->IsMultiline = QSP_TRUE;
        /* Always search next ELSE/END starting next line since we don't have all the lines ready to find the right ones yet */
        line->LinesToEnd = line->LinesToElse = 1;
        break;
    }
    line->Label = qspGetLineLabel(line->Str);
}

void qspFreeLineOfCode(QSPLineOfCode *line)
{
    /* We don't release the line text here */
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
            line->IsMultiline = src[start].IsMultiline;
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
    QSPBufString res = qspNewBufString(0, 256);
    for (i = 0; i < count; ++i)
    {
        qspAddBufText(&res, s[i].Str);
        if (i == count - 1) break; /* don't add the delim */
        qspAddBufText(&res, delim);
    }
    return qspBufStringToString(res);
}

INLINE QSP_BOOL qspSkipCodeBlock(QSPString *str)
{
    int codeBrackets = 1;
    QSP_CHAR *pos = str->Str, *endPos = str->End;
    while (++pos < endPos)
    {
        switch (*pos)
        {
        case QSP_LCODE_CHAR:
            ++codeBrackets;
            break;
        case QSP_RCODE_CHAR:
            if (!(--codeBrackets))
            {
                /* It's at the closing bracket */
                str->Str = pos;
                return QSP_TRUE;
            }
            break;
        }
    }
    str->Str = pos;
    return QSP_FALSE;
}

INLINE QSP_BOOL qspSkipQuotedString(QSPString *str)
{
    QSP_CHAR *pos = str->Str, *endPos = str->End, quote = *pos;
    while (++pos < endPos)
    {
        if (*pos == quote)
        {
            ++pos;
            if (pos >= endPos || *pos != quote)
            {
                /* It's past the closing quote */
                str->Str = pos;
                return QSP_TRUE;
            }
        }
    }
    str->Str = pos;
    return QSP_FALSE;
}

QSP_CHAR *qspDelimPos(QSPString txt, QSP_CHAR ch)
{
    int roundBrackets = 0, squareBrackets = 0;
    QSP_CHAR *pos = txt.Str;
    while (pos < txt.End)
    {
        if (qspIsInClass(*pos, QSP_CHAR_QUOT))
        {
            txt.Str = pos;
            if (!qspSkipQuotedString(&txt)) break;
            pos = txt.Str;
            continue;
        }
        if (*pos == QSP_LCODE_CHAR)
        {
            txt.Str = pos;
            if (!qspSkipCodeBlock(&txt)) break;
            pos = txt.Str;
            continue;
        }
        switch (*pos) /* allow interleaving brackets like "([)]" because the actual validation happens during code execution */
        {
        case QSP_LRBRACK_CHAR: QSP_INC_POSITIVE(roundBrackets); break;
        case QSP_RRBRACK_CHAR: QSP_DEC_POSITIVE(roundBrackets); break;
        case QSP_LSBRACK_CHAR: QSP_INC_POSITIVE(squareBrackets); break;
        case QSP_RSBRACK_CHAR: QSP_DEC_POSITIVE(squareBrackets); break;
        }
        if (*pos == ch && !roundBrackets && !squareBrackets) /* include brackets */
            return pos;
        ++pos;
    }
    return 0;
}

QSP_CHAR *qspKeywordPos(QSPString txt, QSPString str, QSP_BOOL isIsolated)
{
    QSPString prefix;
    QSP_CHAR *startPos, *lastPos, *pos;
    int roundBrackets, squareBrackets, strLen;

    strLen = qspStrLen(str);
    if (!strLen) return txt.Str;

    pos = qspStrStr(txt, str);
    if (!pos) return 0;

    startPos = txt.Str;
    lastPos = txt.End - strLen;
    prefix = qspStringFromPair(startPos, pos);
    if (!qspStrCharClass(prefix, QSP_CHAR_QUOT | QSP_CHAR_LBRACKET))
    {
        /* Don't parse the string */
        if (isIsolated)
        {
            if ((pos == startPos || qspIsInClass(pos[-QSP_CHAR_LEN], QSP_CHAR_DELIM)) && /* delimiter before */
                (pos >= lastPos || qspIsInClass(pos[strLen], QSP_CHAR_DELIM))) /* delimiter after */
                return pos;
            return 0;
        }
        return pos;
    }

    roundBrackets = squareBrackets = 0;
    pos = startPos;
    while (pos <= lastPos)
    {
        if (qspIsInClass(*pos, QSP_CHAR_QUOT))
        {
            txt.Str = pos;
            if (!qspSkipQuotedString(&txt)) break;
            pos = txt.Str;
            continue;
        }
        if (*pos == QSP_LCODE_CHAR)
        {
            txt.Str = pos;
            if (!qspSkipCodeBlock(&txt)) break;
            pos = txt.Str;
            continue;
        }
        switch (*pos) /* allow interleaving brackets like "([)]" because the actual validation happens during code execution */
        {
        case QSP_LRBRACK_CHAR: QSP_INC_POSITIVE(roundBrackets); break;
        case QSP_RRBRACK_CHAR: QSP_DEC_POSITIVE(roundBrackets); break;
        case QSP_LSBRACK_CHAR: QSP_INC_POSITIVE(squareBrackets); break;
        case QSP_RSBRACK_CHAR: QSP_DEC_POSITIVE(squareBrackets); break;
        }
        if (!roundBrackets && !squareBrackets) /* include brackets */
        {
            if (isIsolated)
            {
                if ((pos == startPos || qspIsInClass(pos[-QSP_CHAR_LEN], QSP_CHAR_DELIM)) && /* delimiter before */
                    (pos >= lastPos || qspIsInClass(pos[strLen], QSP_CHAR_DELIM))) /* delimiter after */
                {
                    txt.Str = pos;
                    if (!qspStrsPartCompare(txt, str)) return pos;
                }
            }
            else
            {
                /* It must support searching for delimiters */
                txt.Str = pos;
                if (!qspStrsPartCompare(txt, str)) return pos;
            }
        }
        ++pos;
    }
    return 0;
}

void qspPrepareStringToExecution(QSPString *str)
{
    QSPString tempStr;
    QSP_CHAR *pos = str->Str, *endPos = str->End;
    while (pos < endPos)
    {
        if (qspIsInClass(*pos, QSP_CHAR_QUOT)) /* we have to keep strings untouched */
        {
            tempStr = qspStringFromPair(pos, endPos);
            if (!qspSkipQuotedString(&tempStr)) break;
            pos = tempStr.Str;
            continue;
        }
        if (*pos == QSP_LCODE_CHAR) /* we have to keep code blocks untouched */
        {
            tempStr = qspStringFromPair(pos, endPos);
            if (!qspSkipCodeBlock(&tempStr)) break;
            pos = tempStr.Str;
            continue;
        }
        *pos = QSP_CHRUPR(*pos);
        ++pos;
    }
}

INLINE QSP_BOOL qspAppendLineToResult(QSPString str, int lineNum, QSPBufString *strBuf, QSPLineOfCode *line)
{
    QSPString lineStr;
    int eolLen = QSP_STATIC_LEN(QSP_PREEOLEXT QSP_EOLEXT);
    /* Check line ending only if we add something to the combined line */
    if (qspAddBufText(strBuf, str) && strBuf->Len >= eolLen)
    {
        QSPString eol = qspStringFromLen(strBuf->Str + strBuf->Len - eolLen, eolLen);
        if (qspStrsEqual(eol, QSP_STATIC_STR(QSP_PREEOLEXT QSP_EOLEXT)))
        {
            strBuf->Len -= QSP_STATIC_LEN(QSP_EOLEXT); /* keep QSP_PREEOLEXT */
            return QSP_FALSE;
        }
    }
    lineStr = qspBufStringToString(*strBuf);
    /* Prepare the buffer to execution */
    qspPrepareStringToExecution(&lineStr);
    /* Transfer ownership of the buffer to QSPLineOfCode */
    qspInitLineOfCode(line, lineStr, lineNum);
    return QSP_TRUE;
}

INLINE void qspAppendLastLineToResult(QSPString str, int lineNum, QSPBufString *strBuf, QSPLineOfCode *line)
{
    QSPString lineStr;
    qspAddBufText(strBuf, str);
    lineStr = qspBufStringToString(*strBuf);
    /* Prepare the buffer to execution */
    qspPrepareStringToExecution(&lineStr);
    /* Transfer ownership of the buffer to QSPLineOfCode */
    qspInitLineOfCode(line, lineStr, lineNum);
}

int qspPreprocessData(QSPString data, QSPLineOfCode **strs)
{
    QSPLineOfCode *lines;
    QSPBufString combinedBuf, strBuf;
    QSP_CHAR *pos, quote = 0;
    QSP_BOOL isComment = QSP_FALSE, isStatementStart = QSP_TRUE;
    int codeBrackets = 0, roundBrackets = 0, squareBrackets = 0;
    int lineNum = 0, lastLineNum = 0, linesCount = 0, linesBufSize = 8;

    if (qspIsEmpty(data))
    {
        *strs = 0;
        return 0;
    }

    strBuf = qspNewBufString(0, 256);
    combinedBuf = qspNewBufString(0, 64);
    lines = (QSPLineOfCode *)malloc(linesBufSize * sizeof(QSPLineOfCode));

    pos = data.Str;
    while (pos < data.End)
    {
        data.Str = pos;
        if (!qspStrsPartCompare(data, QSP_STATIC_STR(QSP_STRSDELIM))) /* newline */
        {
            ++lineNum;
            if (!quote && !roundBrackets && !squareBrackets && !codeBrackets)
            {
                /* Flush the current line */
                if (linesCount >= linesBufSize)
                {
                    linesBufSize = linesCount + 16;
                    lines = (QSPLineOfCode *)realloc(lines, linesBufSize * sizeof(QSPLineOfCode));
                }
                if (qspAppendLineToResult(qspDelSpc(qspBufStringToString(strBuf)), lastLineNum, &combinedBuf, lines + linesCount))
                {
                    /* Reset state for the next line */
                    combinedBuf = qspNewBufString(0, 64);
                    isComment = QSP_FALSE;
                    isStatementStart = QSP_TRUE;
                    lastLineNum = lineNum;
                    ++linesCount;
                }
                qspUpdateBufString(&strBuf, qspNullString);
            }
            else
            {
                /* The newline is inside a string / brackets */
                qspAddBufText(&strBuf, QSP_STATIC_STR(QSP_STRSDELIM));
            }
            pos += QSP_STATIC_LEN(QSP_STRSDELIM);
            continue;
        }

        /* Add current character to buffer */
        qspAddBufChar(&strBuf, *pos);

        if (quote) /* inside a quoted string */
        {
            if (*pos == quote)
            {
                if (pos + 1 < data.End && *(pos + 1) == quote)
                {
                    ++pos;
                    qspAddBufChar(&strBuf, quote);
                }
                else
                    quote = 0; /* end of string */
            }
        }
        else if (codeBrackets) /* inside a code block */
        {
            switch (*pos)
            {
            case QSP_LCODE_CHAR: ++codeBrackets; break;
            case QSP_RCODE_CHAR: --codeBrackets; break;
            }
        }
        else if (qspIsInClass(*pos, QSP_CHAR_QUOT)) /* new quoted string */
        {
            quote = *pos;
            isStatementStart = QSP_FALSE;
        }
        else if (*pos == QSP_LCODE_CHAR) /* new code block */
        {
            codeBrackets = 1;
            isStatementStart = QSP_FALSE;
        }
        else if (!isComment) /* ignore () [] brackets inside strings, code blocks and comments */
        {
            /* Allow interleaving brackets like "([)]" because the actual validation happens during code execution */
            switch (*pos)
            {
            case QSP_COMMENT_CHAR:
                if (isStatementStart)
                {
                    isComment = QSP_TRUE;
                    isStatementStart = QSP_FALSE;
                }
                break;
            case QSP_STATDELIM_CHAR:
                isStatementStart = (!roundBrackets && !squareBrackets);
                break;
            case QSP_LRBRACK_CHAR: QSP_INC_POSITIVE(roundBrackets); isStatementStart = QSP_FALSE; break;
            case QSP_RRBRACK_CHAR: QSP_DEC_POSITIVE(roundBrackets); isStatementStart = QSP_FALSE; break;
            case QSP_LSBRACK_CHAR: QSP_INC_POSITIVE(squareBrackets); isStatementStart = QSP_FALSE; break;
            case QSP_RSBRACK_CHAR: QSP_DEC_POSITIVE(squareBrackets); isStatementStart = QSP_FALSE; break;
            default:
                if (isStatementStart && !qspIsInClass(*pos, QSP_CHAR_SPACE))
                    isStatementStart = QSP_FALSE;
                break;
            }
        }
        ++pos;
    }
    /* Append the final line */
    if (linesCount >= linesBufSize)
        lines = (QSPLineOfCode *)realloc(lines, (linesCount + 1) * sizeof(QSPLineOfCode));
    qspAppendLastLineToResult(qspDelSpc(qspBufStringToString(strBuf)), lastLineNum, &combinedBuf, lines + linesCount);
    qspFreeBufString(&strBuf);
    ++linesCount;

    *strs = lines;
    return linesCount;
}
