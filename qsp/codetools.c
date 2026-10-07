/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "codetools.h"
#include "statements.h"
#include "text.h"
#include "variables.h"
#include "variant.h"

QSPCachedCodeBlocksBucket qspCachedCodeBlocks[QSP_CACHEDCODEBUCKETS];

INLINE int qspStatStringCompare(const void *name, const void *compareTo);
INLINE QSP_TINYINT qspGetStatCode(QSPString s, QSP_CHAR **pos);
INLINE void qspInitArg(QSPCachedArg *arg, QSPString s);
INLINE void qspInitStatData(QSPCachedStat *stat, QSPString s, QSPString lineStr);
INLINE QSPCachedAssignment *qspNewAssignment(QSP_TINYINT statCode, QSPString s, QSP_TINYINT *errorCode);
INLINE QSPCachedLoop *qspNewLoop(QSPString s, QSPString lineStr, QSP_TINYINT *errorCode);
INLINE QSPCachedArg *qspNewUserCallArgs(QSPString s, QSP_TINYINT *argsCount, QSP_TINYINT *errorCode);
INLINE QSPCachedAct *qspNewAct(QSPString s, QSP_TINYINT *argsCount, QSP_TINYINT *errorCode);
INLINE QSPCachedArg *qspNewSingleArg(QSPString s, QSP_TINYINT *argsCount);
INLINE QSPCachedArg *qspNewRegularArgs(QSP_TINYINT statCode, QSPString s, QSP_TINYINT *argsCount, QSP_TINYINT *errorCode);
INLINE int qspInitAssignmentTargets(QSPCachedTarget *targets, QSPString names, QSP_BOOL hasValue, QSP_TINYINT *errorCode);
INLINE void qspInitAssignmentTarget(QSPCachedTarget *target, QSPString s, QSP_BOOL hasValue, QSP_TINYINT *errorCode);
INLINE QSP_TINYINT qspAppendRegularArgs(QSPCachedArg *foundArgs, QSP_TINYINT argsCount, QSP_TINYINT statCode, QSPString s, QSP_TINYINT *errorCode);
INLINE QSPCachedArg *qspCopyToNewArgs(QSPCachedArg *foundArgs, QSP_TINYINT argsCount);
INLINE void qspFreeArg(QSPCachedArg *arg);
INLINE void qspFreeArgs(QSPCachedArg *args, int count);
INLINE void qspFreeLineOfCode(QSPLineOfCode *line);
INLINE QSP_CHAR *qspSkipQuotedString(QSP_CHAR *pos, QSP_CHAR *endPos);
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

INLINE void qspInitArg(QSPCachedArg *arg, QSPString s)
{
    s = qspDelSpc(s);
    arg->Data.Text = s;
    arg->Type = qspArgExpression;
    if (qspIsEmpty(s))
        arg->Type = qspArgEmpty;
    else if (qspIsStrNumber(s)) /* signed literals don't need compiling */
    {
        arg->Type = qspArgNumber;
        arg->Data.Number = qspStrToNum(s, 0);
    }
    else if (qspStrLen(s) >= 2) /* literals keep the text between the delimiters */
    {
        QSP_CHAR first = *s.Str, *lastPos = s.End - QSP_CHAR_LEN;
        QSPString text = qspStringFromPair(s.Str + QSP_CHAR_LEN, lastPos);
        if (qspIsInClass(first, QSP_CHAR_QUOT))
        {
            /* Simple strings only: without doubled quotes & subexpressions */
            if (*lastPos == first && !qspStrChar(text, first) && !qspStrStr(text, QSP_STATIC_STR(QSP_LSUBEX)))
            {
                arg->Type = qspArgString;
                arg->Data.Text = text;
            }
        }
        else if (first == QSP_LCODE_CHAR && qspDelimPos(s, QSP_RCODE_CHAR) == lastPos)
        {
            arg->Type = qspArgCode;
            arg->Data.Text = text;
        }
    }
}

INLINE void qspInitStatData(QSPCachedStat *stat, QSPString s, QSPString lineStr)
{
    stat->ErrorCode = 0;
    stat->ArgsCount = 0;
    switch (stat->Stat)
    {
    case qspStatUnknown:
    case qspStatLabel:
    case qspStatElse:
    case qspStatEnd:
    case qspStatComment:
        stat->Data.Args = 0;
        break;
    case qspStatSet:
    case qspStatLocal:
        stat->Data.Assignment = qspNewAssignment(stat->Stat, s, &stat->ErrorCode);
        break;
    case qspStatLoop:
        stat->Data.Loop = qspNewLoop(s, lineStr, &stat->ErrorCode);
        break;
    case qspStatUserCall:
        stat->Data.Args = qspNewUserCallArgs(s, &stat->ArgsCount, &stat->ErrorCode);
        break;
    case qspStatAct:
        stat->Data.Act = qspNewAct(s, &stat->ArgsCount, &stat->ErrorCode);
        break;
    case qspStatImplicitStatement:
    case qspStatIf:
    case qspStatElseIf:
        stat->Data.Args = qspNewSingleArg(s, &stat->ArgsCount);
        break;
    default:
        stat->Data.Args = qspNewRegularArgs(stat->Stat, s, &stat->ArgsCount, &stat->ErrorCode);
        break;
    }
}

INLINE QSPCachedAssignment *qspNewAssignment(QSP_TINYINT statCode, QSPString s, QSP_TINYINT *errorCode)
{
    int targetsCount;
    QSPCachedTarget targets[QSP_MAXSTATARGS];
    QSPCachedAssignment *assignment;
    QSPString names = s, value = qspNullString;
    QSP_CHAR operation = 0, *equalPos = qspDelimPos(s, QSP_EQUAL_CHAR);
    if (equalPos)
    {
        /* A compound operation like += starts before the equal sign */
        QSP_CHAR *opPos = equalPos;
        if (qspIsInClassAtPos(s, opPos - QSP_CHAR_LEN, QSP_CHAR_SIMPLEOP))
            opPos -= QSP_CHAR_LEN;
        names.End = opPos;
        operation = *opPos;
        value = qspStringFromPair(equalPos + QSP_CHAR_LEN, s.End);
        if (statCode == qspStatLocal && operation != QSP_EQUAL_CHAR)
        {
            *errorCode = QSP_ERR_SYNTAX;
            return 0;
        }
    }
    else if (statCode == qspStatSet)
    {
        *errorCode = QSP_ERR_SYNTAX; /* SET requires a value */
        return 0;
    }
    targetsCount = qspInitAssignmentTargets(targets, names, operation != 0, errorCode);
    if (*errorCode) return 0;
    assignment = (QSPCachedAssignment *)malloc(sizeof(QSPCachedAssignment));
    assignment->Targets = (QSPCachedTarget *)malloc(targetsCount * sizeof(QSPCachedTarget));
    memcpy(assignment->Targets, targets, targetsCount * sizeof(QSPCachedTarget));
    assignment->TargetsCount = targetsCount;
    assignment->Operation = operation;
    qspInitArg(&assignment->Value, value);
    return assignment;
}

INLINE QSPCachedLoop *qspNewLoop(QSPString s, QSPString lineStr, QSP_TINYINT *errorCode)
{
    QSPCachedLoop *loop;
    QSPString condition, iterator;
    QSP_CHAR *whilePos, *stepPos;
    if (!qspIsCharAtPos(lineStr, s.End, QSP_COLONDELIM_CHAR))
    {
        *errorCode = QSP_ERR_COLONNOTFOUND;
        return 0;
    }
    whilePos = qspKeywordPos(s, QSP_STATIC_STR(QSP_STATLOOPWHILE), QSP_TRUE);
    if (!whilePos)
    {
        *errorCode = QSP_ERR_LOOPWHILENOTFOUND;
        return 0;
    }
    condition = qspStringFromPair(whilePos + QSP_STATIC_LEN(QSP_STATLOOPWHILE), s.End);
    iterator = qspNullString;
    stepPos = qspKeywordPos(condition, QSP_STATIC_STR(QSP_STATLOOPSTEP), QSP_TRUE);
    if (stepPos)
    {
        condition.End = stepPos;
        iterator = qspStringFromPair(stepPos + QSP_STATIC_LEN(QSP_STATLOOPSTEP), s.End);
        if (!qspIsAnyString(iterator))
        {
            *errorCode = QSP_ERR_CODENOTFOUND; /* STEP without code */
            return 0;
        }
    }
    loop = (QSPCachedLoop *)malloc(sizeof(QSPCachedLoop));
    qspInitLineOfCode(&loop->Initializer, qspStringFromPair(s.Str, whilePos), 0);
    qspInitLineOfCode(&loop->Iterator, iterator, 0);
    qspInitArg(&loop->Condition, condition);
    return loop;
}

INLINE QSPCachedArg *qspNewUserCallArgs(QSPString s, QSP_TINYINT *argsCount, QSP_TINYINT *errorCode)
{
    QSPCachedArg *args;
    QSP_CHAR *nameEnd = qspStrCharClass(s, QSP_CHAR_DELIM);
    if (nameEnd)
    {
        QSP_TINYINT count;
        QSPCachedArg foundArgs[QSP_MAXSTATARGS];
        foundArgs[0].Data.Text = qspStringFromPair(s.Str, nameEnd); /* the name goes first */
        foundArgs[0].Type = qspArgString;
        count = qspAppendRegularArgs(foundArgs, 1, qspStatUserCall, qspStringFromPair(nameEnd, s.End), errorCode);
        if (*errorCode) return 0;
        *argsCount = count;
        return qspCopyToNewArgs(foundArgs, count);
    }
    args = (QSPCachedArg *)malloc(sizeof(QSPCachedArg));
    args->Data.Text = s; /* the name only */
    args->Type = qspArgString;
    *argsCount = 1;
    return args;
}

INLINE QSPCachedAct *qspNewAct(QSPString s, QSP_TINYINT *argsCount, QSP_TINYINT *errorCode)
{
    QSPCachedAct *act;
    QSPCachedArg foundArgs[2];
    QSP_TINYINT count = qspAppendRegularArgs(foundArgs, 0, qspStatAct, s, errorCode);
    if (*errorCode) return 0;
    act = (QSPCachedAct *)malloc(sizeof(QSPCachedAct));
    memcpy(act->Args, foundArgs, count * sizeof(QSPCachedArg));
    act->OnPressCode = 0;
    *argsCount = count;
    return act;
}

INLINE QSPCachedArg *qspNewSingleArg(QSPString s, QSP_TINYINT *argsCount)
{
    QSPCachedArg *args = (QSPCachedArg *)malloc(sizeof(QSPCachedArg));
    qspInitArg(args, s); /* the whole text is the argument */
    *argsCount = 1;
    return args;
}

INLINE QSPCachedArg *qspNewRegularArgs(QSP_TINYINT statCode, QSPString s, QSP_TINYINT *argsCount, QSP_TINYINT *errorCode)
{
    QSPCachedArg foundArgs[QSP_MAXSTATARGS];
    QSP_TINYINT count = qspAppendRegularArgs(foundArgs, 0, statCode, s, errorCode);
    if (*errorCode) return 0;
    *argsCount = count;
    return qspCopyToNewArgs(foundArgs, count);
}

INLINE int qspInitAssignmentTargets(QSPCachedTarget *targets, QSPString names, QSP_BOOL hasValue, QSP_TINYINT *errorCode)
{
    QSP_CHAR *comma;
    QSPString items[QSP_MAXSTATARGS];
    int i, itemsCount = 0;
    /* Split the list first, its errors take precedence over the target errors */
    while (1)
    {
        if (!qspIsAnyString(names))
        {
            *errorCode = QSP_ERR_SYNTAX;
            return 0;
        }
        if (itemsCount == QSP_MAXSTATARGS)
        {
            *errorCode = QSP_ERR_ARGSCOUNT;
            return 0;
        }
        comma = qspDelimPos(names, QSP_COMMA_CHAR);
        if (!comma) break;
        items[itemsCount] = qspStringFromPair(names.Str, comma);
        ++itemsCount;
        names.Str = comma + QSP_CHAR_LEN;
    }
    items[itemsCount] = names;
    ++itemsCount;
    for (i = 0; i < itemsCount; ++i)
    {
        qspInitAssignmentTarget(targets + i, items[i], hasValue, errorCode);
        if (*errorCode) return 0;
    }
    return itemsCount;
}

INLINE void qspInitAssignmentTarget(QSPCachedTarget *target, QSPString s, QSP_BOOL hasValue, QSP_TINYINT *errorCode)
{
    unsigned int nameHash;
    QSP_CHAR *nameEnd, *indexEnd;
    QSPString name;
    s = qspDelSpc(s);
    nameEnd = qspStrCharClass(s, QSP_CHAR_DELIM);
    target->Index.Type = qspArgNone; /* no index */
    if (nameEnd)
    {
        target->Name = qspStringFromPair(s.Str, nameEnd);
        if (hasValue) /* LOCAL without a value ignores the rest */
        {
            QSPString rest = qspStringFromPair(nameEnd, s.End);
            qspSkipSpaces(&rest);
            if (!qspIsCharAtPos(rest, rest.Str, QSP_LSBRACK_CHAR))
            {
                *errorCode = QSP_ERR_INCORRECTNAME;
                return;
            }
            indexEnd = qspDelimPos(rest, QSP_RSBRACK_CHAR);
            if (!indexEnd)
            {
                *errorCode = QSP_ERR_BRACKETNOTFOUND;
                return;
            }
            qspInitArg(&target->Index, qspStringFromPair(rest.Str + QSP_CHAR_LEN, indexEnd));
        }
    }
    else
        target->Name = s; /* the name only */

    name = qspPrepareVarName(target->Name, &nameHash); /* validates the name */
    if (qspIsEmpty(name))
        *errorCode = QSP_ERR_INCORRECTNAME;
}

INLINE QSP_TINYINT qspAppendRegularArgs(QSPCachedArg *foundArgs, QSP_TINYINT argsCount, QSP_TINYINT statCode, QSPString s, QSP_TINYINT *errorCode)
{
    qspSkipSpaces(&s);
    if (!qspIsEmpty(s))
    {
        if (*s.Str == QSP_LRBRACK_CHAR) /* arguments might be specified using parentheses */
        {
            QSP_CHAR *bracket = qspDelimPos(s, QSP_RRBRACK_CHAR);
            if (!bracket)
            {
                *errorCode = QSP_ERR_BRACKETNOTFOUND;
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
            while (1)
            {
                if (argsCount >= qspStats[statCode].MaxArgsCount)
                {
                    *errorCode = QSP_ERR_ARGSCOUNT;
                    break;
                }
                pos = qspDelimPos(s, QSP_COMMA_CHAR);
                if (pos)
                {
                    qspInitArg(foundArgs + argsCount, qspStringFromPair(s.Str, pos));
                    ++argsCount;
                }
                else
                {
                    qspInitArg(foundArgs + argsCount, s);
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
        }
    }
    if (argsCount < qspStats[statCode].MinArgsCount)
        *errorCode = QSP_ERR_ARGSCOUNT;
    return argsCount;
}

INLINE QSPCachedArg *qspCopyToNewArgs(QSPCachedArg *foundArgs, QSP_TINYINT argsCount)
{
    if (argsCount)
    {
        QSPCachedArg *args = (QSPCachedArg *)malloc(argsCount * sizeof(QSPCachedArg));
        memcpy(args, foundArgs, argsCount * sizeof(QSPCachedArg));
        return args;
    }
    return 0;
}

void qspInitLineOfCode(QSPLineOfCode *line, QSPString str, int lineNum)
{
    int statInd;
    QSP_TINYINT statCode;
    QSP_CHAR *statDelimPos, *paramPos;
    /* 'nextPos' points to the next position to search for a statement */
    /* 'statDelimPos' points to the statement separator (':' or '&') */
    line->Str = str;
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
            line->Stats[statInd].EndPos = (int)(statDelimPos - line->Str.Str);
            qspInitStatData(line->Stats + statInd, qspStringFromPair(str.Str, statDelimPos), line->Str);
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
        && !qspIsCharAtPos(line->Str, line->Str.Str + line->Stats[0].EndPos, QSP_COLONDELIM_CHAR))
    {
        /* Convert multiline ELSE IF to ELSEIF */
        statCode = qspStatElseIf; /* move current IF as ELSEIF to index 0, it's safe to overwrite ELSE */
        statInd = 0;
    }
    else if (statInd == 2
        && line->Stats[0].Stat == qspStatElse && line->Stats[1].Stat == qspStatIf && statCode == qspStatComment
        && !qspIsCharAtPos(line->Str, line->Str.Str + line->Stats[0].EndPos, QSP_COLONDELIM_CHAR))
    {
        /* Convert multiline ELSE IF with a comment to ELSEIF with the comment */
        line->Stats[0].Stat = qspStatElseIf; /* move IF as ELSEIF to index 0, it's safe to overwrite ELSE */
        line->Stats[0].EndPos = line->Stats[1].EndPos;
        line->Stats[0].ArgsCount = line->Stats[1].ArgsCount;
        line->Stats[0].Data.Args = line->Stats[1].Data.Args;
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
    if (statDelimPos)
    {
        line->Stats[statInd].EndPos = (int)(statDelimPos - line->Str.Str);
        qspInitStatData(line->Stats + statInd, qspStringFromPair(str.Str, statDelimPos), line->Str);
    }
    else
    {
        line->Stats[statInd].EndPos = (int)(str.End - line->Str.Str);
        qspInitStatData(line->Stats + statInd, str, line->Str);
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
}

INLINE void qspFreeArg(QSPCachedArg *arg)
{
    if (arg->Type == qspArgCompiled)
        qspFreeMathExpression(arg->Data.Expression);
}

INLINE void qspFreeArgs(QSPCachedArg *args, int count)
{
    while (--count >= 0)
    {
        qspFreeArg(args);
        ++args;
    }
}

INLINE void qspFreeLineOfCode(QSPLineOfCode *line)
{
    /* We don't release the line text here */
    if (line->Stats)
    {
        int i, j;
        QSPCachedStat *stat = line->Stats;
        for (i = 0; i < line->StatsCount; ++i, ++stat)
        {
            switch (stat->Stat)
            {
            case qspStatLoop:
                if (stat->Data.Loop)
                {
                    QSPCachedLoop *loop = stat->Data.Loop;
                    qspFreeArg(&loop->Condition);
                    qspFreeLineOfCode(&loop->Initializer);
                    qspFreeLineOfCode(&loop->Iterator);
                    free(loop);
                }
                break;
            case qspStatSet:
            case qspStatLocal:
                if (stat->Data.Assignment)
                {
                    QSPCachedTarget *target;
                    QSPCachedAssignment *assignment = stat->Data.Assignment;
                    qspFreeArg(&assignment->Value);
                    for (j = assignment->TargetsCount, target = assignment->Targets; j > 0; --j, ++target)
                        qspFreeArg(&target->Index);
                    free(assignment->Targets);
                    free(assignment);
                }
                break;
            case qspStatAct:
                if (stat->Data.Act)
                {
                    QSPCachedAct *act = stat->Data.Act;
                    qspFreeArgs(act->Args, stat->ArgsCount);
                    if (act->OnPressCode) qspReleaseCodeBlock(act->OnPressCode);
                    free(act);
                }
                break;
            default:
                if (stat->Data.Args)
                {
                    qspFreeArgs(stat->Data.Args, stat->ArgsCount);
                    free(stat->Data.Args);
                }
                break;
            }
        }
        free(line->Stats);
    }
}

void qspFreePrepLines(QSPLineOfCode *lines, int count)
{
    if (lines)
    {
        QSPLineOfCode *curLine = lines;
        while (--count >= 0)
        {
            qspFreeString(&curLine->Str);
            qspFreeLineOfCode(curLine);
            ++curLine;
        }
        free(lines);
    }
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

int qspCollectLabels(QSPLineOfCode *lines, int linesCount, QSPCodeLabel **labels)
{
    QSPString name;
    QSP_CHAR *delimPos;
    QSPCodeLabel *foundLabels = 0;
    int i, labelsCount = 0, labelsCapacity = 0;
    for (i = 0; i < linesCount; ++i, ++lines)
    {
        if (lines->Stats && lines->Stats->Stat == qspStatLabel)
        {
            name = lines->Str;
            qspSkipSpaces(&name);
            name.Str += QSP_STATIC_LEN(QSP_LABEL);
            delimPos = qspDelimPos(name, QSP_STATDELIM_CHAR);
            if (delimPos) name.End = delimPos;
            name = qspCopyToNewText(qspDelSpc(name));
            qspUpperStr(&name);
            if (labelsCount >= labelsCapacity)
            {
                labelsCapacity = labelsCount + 4;
                foundLabels = (QSPCodeLabel *)realloc(foundLabels, labelsCapacity * sizeof(QSPCodeLabel));
            }
            foundLabels[labelsCount].Name = name;
            foundLabels[labelsCount].LineIndex = i;
            ++labelsCount;
        }
    }
    /* Release the spare capacity */
    if (labelsCount < labelsCapacity)
        foundLabels = (QSPCodeLabel *)realloc(foundLabels, labelsCount * sizeof(QSPCodeLabel));
    *labels = foundLabels;
    return labelsCount;
}

void qspFreeLabels(QSPCodeLabel *labels, int count)
{
    if (labels)
    {
        QSPCodeLabel *label = labels;
        while (--count >= 0)
        {
            qspFreeString(&label->Name);
            ++label;
        }
        free(labels);
    }
}

QSPCodeBlock *qspGetSinglelineActCode(QSPLineOfCode *line, int statPos, int endPos)
{
    QSPCachedAct *act = line->Stats[statPos].Data.Act;
    if (!act->OnPressCode)
    {
        QSPLineOfCode *actLine;
        QSPString actText;
        QSP_CHAR *firstPos, *lastPos;
        firstPos = line->Str.Str + line->Stats[statPos].EndPos + QSP_CHAR_LEN; /* skip the colon */
        lastPos = line->Str.Str + line->Stats[endPos - 1].EndPos;
        if (qspIsCharAtPos(line->Str, lastPos, QSP_COLONDELIM_CHAR))
            lastPos += QSP_CHAR_LEN;
        actText = qspCopyToNewText(qspStringFromPair(firstPos, lastPos));
        actLine = (QSPLineOfCode *)malloc(sizeof(QSPLineOfCode));
        qspInitLineOfCode(actLine, actText, line->LineNum); /* the line takes the text */
        actLine->IsMultiline = QSP_FALSE; /* it's a part of the single-line statement */
        act->OnPressCode = qspNewCodeBlock(actLine, 1);
    }
    return act->OnPressCode;
}

INLINE QSP_CHAR *qspSkipQuotedString(QSP_CHAR *pos, QSP_CHAR *endPos)
{
    QSP_CHAR quote = *pos;
    while (++pos < endPos)
    {
        if (*pos == quote)
        {
            ++pos;
            if (pos >= endPos || *pos != quote) break;
        }
    }
    /* It's either past the closing quote or past the last valid position */
    return pos;
}

QSP_CHAR *qspDelimPos(QSPString txt, QSP_CHAR ch)
{
    int roundBrackets = 0, squareBrackets = 0, codeBrackets = 0;
    QSP_CHAR *pos = txt.Str;
    while (pos < txt.End)
    {
        if (qspIsInClass(*pos, QSP_CHAR_QUOT))
        {
            pos = qspSkipQuotedString(pos, txt.End);
            continue;
        }
        switch (*pos) /* allow interleaving brackets like "([)]" because the actual validation happens during code execution */
        {
        case QSP_LRBRACK_CHAR: if (!codeBrackets) QSP_INC_POSITIVE(roundBrackets); break;
        case QSP_RRBRACK_CHAR: if (!codeBrackets) QSP_DEC_POSITIVE(roundBrackets); break;
        case QSP_LSBRACK_CHAR: if (!codeBrackets) QSP_INC_POSITIVE(squareBrackets); break;
        case QSP_RSBRACK_CHAR: if (!codeBrackets) QSP_DEC_POSITIVE(squareBrackets); break;
        case QSP_LCODE_CHAR: QSP_INC_POSITIVE(codeBrackets); break;
        case QSP_RCODE_CHAR: QSP_DEC_POSITIVE(codeBrackets); break;
        }
        if (*pos == ch && !roundBrackets && !squareBrackets && !codeBrackets) /* include brackets */
            return pos;
        ++pos;
    }
    return 0;
}

QSP_CHAR *qspKeywordPos(QSPString txt, QSPString str, QSP_BOOL isIsolated)
{
    QSPString prefix;
    QSP_CHAR *startPos, *lastPos, *pos;
    int roundBrackets, squareBrackets, codeBrackets, strLen;

    strLen = qspStrLen(str);
    if (!strLen) return txt.Str;

    pos = qspStrStr(txt, str);
    if (!pos) return 0;

    startPos = txt.Str;
    lastPos = txt.End - strLen;
    prefix = qspStringFromPair(startPos, pos);
    /* Only quotes & brackets in the prefix affect parsing */
    if (qspStrCharClass(prefix, QSP_CHAR_QUOT | QSP_CHAR_LBRACKET))
        pos = startPos;
    else if (!isIsolated)
        return pos;

    roundBrackets = squareBrackets = codeBrackets = 0;
    while (pos <= lastPos)
    {
        if (qspIsInClass(*pos, QSP_CHAR_QUOT))
        {
            pos = qspSkipQuotedString(pos, lastPos + QSP_CHAR_LEN);
            continue;
        }
        switch (*pos) /* allow interleaving brackets like "([)]" because the actual validation happens during code execution */
        {
        case QSP_LRBRACK_CHAR: if (!codeBrackets) QSP_INC_POSITIVE(roundBrackets); break;
        case QSP_RRBRACK_CHAR: if (!codeBrackets) QSP_DEC_POSITIVE(roundBrackets); break;
        case QSP_LSBRACK_CHAR: if (!codeBrackets) QSP_INC_POSITIVE(squareBrackets); break;
        case QSP_RSBRACK_CHAR: if (!codeBrackets) QSP_DEC_POSITIVE(squareBrackets); break;
        case QSP_LCODE_CHAR: QSP_INC_POSITIVE(codeBrackets); break;
        case QSP_RCODE_CHAR: QSP_DEC_POSITIVE(codeBrackets); break;
        }
        if (!roundBrackets && !squareBrackets && !codeBrackets) /* include brackets */
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
    int codeBrackets = 0;
    QSP_CHAR *pos = str->Str, *endPos = str->End;
    while (pos < endPos)
    {
        if (qspIsInClass(*pos, QSP_CHAR_QUOT)) /* we have to keep strings untouched */
        {
            pos = qspSkipQuotedString(pos, endPos);
            continue;
        }
        switch (*pos)
        {
        case QSP_LCODE_CHAR: QSP_INC_POSITIVE(codeBrackets); break;
        case QSP_RCODE_CHAR: QSP_DEC_POSITIVE(codeBrackets); break;
        default:
            if (!codeBrackets) /* we have to keep code blocks untouched */
                *pos = QSP_CHRUPR(*pos);
            break;
        }
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

QSPCodeBlock *qspPreprocessData(QSPString data)
{
    QSPLineOfCode *lines;
    QSPBufString combinedBuf, strBuf;
    QSP_CHAR *pos, quote = 0;
    QSP_BOOL isComment = QSP_FALSE, isStatementStart = QSP_TRUE;
    int codeBrackets = 0, roundBrackets = 0, squareBrackets = 0;
    int lineNum = 0, lastLineNum = 0, linesCount = 0, linesBufSize = 8;

    if (qspIsEmpty(data)) return 0;

    strBuf = qspNewBufString(0, 256);
    combinedBuf = qspNewBufString(0, 0); /* the lines take the buffer, no spare capacity */
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
                    combinedBuf = qspNewBufString(0, 0);
                    isComment = QSP_FALSE;
                    isStatementStart = QSP_TRUE;
                    lastLineNum = lineNum;
                    ++linesCount;
                }
                qspUpdateBufString(&strBuf, qspNullString);
            }
            else
            {
                /* The newline is inside a string / brackets / code block */
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
        else if (qspIsInClass(*pos, QSP_CHAR_QUOT)) /* new quoted string */
        {
            quote = *pos;
            isStatementStart = QSP_FALSE;
        }
        else if (codeBrackets) /* inside a code block */
        {
            switch (*pos)
            {
            case QSP_LCODE_CHAR: ++codeBrackets; break;
            case QSP_RCODE_CHAR: --codeBrackets; break;
            }
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
    if (linesCount + 1 != linesBufSize)
        lines = (QSPLineOfCode *)realloc(lines, (linesCount + 1) * sizeof(QSPLineOfCode));
    qspAppendLastLineToResult(qspDelSpc(qspBufStringToString(strBuf)), lastLineNum, &combinedBuf, lines + linesCount);
    qspFreeBufString(&strBuf);
    ++linesCount;

    return qspNewCodeBlock(lines, linesCount);
}

void qspClearAllCodeBlocks(QSP_BOOL toInit)
{
    int i, j;
    QSPCachedCodeBlock *block;
    QSPCachedCodeBlocksBucket *bucket = qspCachedCodeBlocks;
    for (i = 0; i < QSP_CACHEDCODEBUCKETS; ++i, ++bucket)
    {
        if (!toInit && bucket->BlocksCount)
        {
            for (j = bucket->BlocksCount, block = bucket->Blocks; j > 0; --j, ++block)
            {
                qspFreeString(&block->Text);
                qspReleaseCodeBlock(block->Code);
            }
        }
        bucket->BlocksCount = 0;
        bucket->BlockToEvict = 0;
    }
}

QSPCodeBlock *qspGetCachedCodeBlock(QSPString s)
{
    QSPCachedCodeBlock *block;
    QSPCachedCodeBlocksBucket *bucket;
    int i, blocksCount;
    if (qspIsEmpty(s)) return 0;
    if (qspStrLen(s) > QSP_MAXCACHEDCODELEN) /* too long to cache */
        return qspPreprocessData(s);
    /* Find a correct bucket by hash value */
    bucket = qspCachedCodeBlocks + qspGetTextHash(s) % QSP_CACHEDCODEBUCKETS;
    /* Search for existing item in the bucket */
    blocksCount = bucket->BlocksCount;
    for (i = blocksCount, block = bucket->Blocks; i > 0; --i, ++block)
    {
        if (qspStrsEqual(block->Text, s))
        {
            qspAcquireCodeBlock(block->Code);
            return block->Code;
        }
    }
    if (blocksCount < QSP_MAXCACHEDCODEBUCKETSIZE)
    {
        /* Add a new entry */
        block = bucket->Blocks + blocksCount;
        bucket->BlocksCount++;
    }
    else
    {
        /* Release the old code */
        block = bucket->Blocks + bucket->BlockToEvict;
        qspFreeString(&block->Text);
        qspReleaseCodeBlock(block->Code);
        /* Update the next item to be evicted */
        bucket->BlockToEvict = (bucket->BlockToEvict + 1) % QSP_MAXCACHEDCODEBUCKETSIZE;
    }
    /* Preprocess the new code */
    block->Code = qspPreprocessData(s);
    block->Text = qspCopyToNewText(s);
    qspAcquireCodeBlock(block->Code);
    return block->Code;
}

QSPVariant qspCalculateArgValue(QSPCachedArg *arg)
{
    QSPMathExpression *expression;
    switch (arg->Type)
    {
    case qspArgCompiled:
        return qspCalculateValue(arg->Data.Expression, arg->Data.Expression->ItemsCount - 1);
    case qspArgNumber:
        return qspNumVariant(arg->Data.Number);
    case qspArgString:
        return qspStrVariant(qspCopyToNewText(arg->Data.Text), QSP_TYPE_STR);
    case qspArgCode:
        return qspStrVariant(qspCopyToNewText(arg->Data.Text), QSP_TYPE_CODE);
    }
    /* The text is compiled on the first evaluation, it stays until the compilation succeeds */
    expression = qspCompileMathExpression(arg->Data.Text);
    if (!expression) return qspGetEmptyVariant(QSP_TYPE_UNDEF);
    arg->Data.Expression = expression;
    arg->Type = qspArgCompiled;
    return qspCalculateValue(expression, expression->ItemsCount - 1);
}
