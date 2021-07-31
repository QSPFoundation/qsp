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

#include "regexp.h"
#include "errors.h"
#include "text.h"

#if !defined(USE_PCRE) || !USE_PCRE
    RegExT WRONG_REGEX = NULL;
#else
    RegExT WRONG_REGEX = {.rx=NULL, .extra=NULL};
#endif

int qspCompiledRegExpsCurInd = 0;
QSPRegExp qspCompiledRegExps[QSP_MAXCACHEDREGEXPS];

void qspClearRegExps(QSP_BOOL isFirst)
{
    int i;
    QSPRegExp *exp = qspCompiledRegExps;
    for (i = 0; i < QSP_MAXCACHEDREGEXPS; ++i)
    {
        if (!isFirst && CHECK_REGEXP_PRESENCE(exp->CompiledExp))
        {
            qspFreeString(exp->Text);
            #if defined(USE_PCRE)
            pcre_free_study(exp->CompiledExp.extra);
            pcre_free(exp->CompiledExp.rx);
            #else
            onig_free(exp->CompiledExp);
            #endif
        }
        exp->Text = qspNullString;
        exp->CompiledExp = WRONG_REGEX;
        ++exp;
    }
    qspCompiledRegExpsCurInd = 0;
}

RegExT qspRegExpGetCompiled(QSPString exp)
{
    int i;
    RegExT nativeExp;
    QSPString safeExp;
    #if !defined(USE_PCRE)
    OnigErrorInfo onigInfo;
    #endif

    QSPRegExp *compExp = qspCompiledRegExps;
    for (i = 0; i < QSP_MAXCACHEDREGEXPS; ++i)
    {
        if (!CHECK_REGEXP_PRESENCE(compExp->CompiledExp)) break;
        if (!qspStrsComp(exp, compExp->Text))
            return compExp->CompiledExp;
        ++compExp;
    }
    safeExp = (exp.Str ? exp : QSP_STATIC_STR(QSP_FMT("")));
    int rxCompileErrorCode = 0;
    const char * rxCompileErrorPtr = NULL;
    int errorOffset = 0;

    #if defined(USE_PCRE)
    nativeExp.rx = pcre_compile2((const char *) safeExp.Str, PCRE_UTF8 | PCRE_UCP, &rxCompileErrorCode, &rxCompileErrorPtr, &errorOffset, NULL);
    nativeExp.extra = pcre_study(nativeExp.rx, PCRE_STUDY_JIT_COMPILE, &rxCompileErrorPtr);
    #else
    rxCompileErrorCode = onig_new(&nativeExp, (OnigUChar *)safeExp.Str, (OnigUChar *)safeExp.End,
        ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL_NG, &onigInfo);
    #endif

    if (rxCompileErrorCode)
    {
        qspSetError(QSP_ERR_INCORRECTREGEXP);
        return WRONG_REGEX;
    }
    compExp = qspCompiledRegExps + qspCompiledRegExpsCurInd;
    if (CHECK_REGEXP_PRESENCE(compExp->CompiledExp))
    {
        qspFreeString(compExp->Text);
        #if defined(USE_PCRE)
        pcre_free_study(compExp->CompiledExp.extra);
        pcre_free(compExp->CompiledExp.rx);
        #else
        onig_free(compExp->CompiledExp);
        #endif
    }
    compExp->Text = qspGetNewText(exp);
    compExp->CompiledExp = nativeExp;
    if (++qspCompiledRegExpsCurInd == QSP_MAXCACHEDREGEXPS)
        qspCompiledRegExpsCurInd = 0;
    return nativeExp;
}

QSP_BOOL qspRegExpStrMatch(RegExT exp, QSPString str)
{
    #if !defined(USE_PCRE)
    OnigUChar *tempBeg, *tempEnd;
    tempBeg = (OnigUChar *)str.Str;
    tempEnd = (OnigUChar *)str.End;
    return (onig_match(exp, tempBeg, tempEnd, tempBeg, 0, ONIG_OPTION_NONE) == tempEnd - tempBeg);
    #else
    return !pcre_exec(exp.rx, exp.extra, (const char *) str.Str, str.End-str.Str, 0, 0, NULL, 0);
    #endif
}

QSPString qspRegExpStrFind(RegExT exp, QSPString str, int ind)
{
    QSPString res;
    int len, pos;

    #if !defined(USE_PCRE)
    OnigUChar *tempBeg, *tempEnd;
    OnigRegion *onigReg = onig_region_new();
    tempBeg = (OnigUChar *)str.Str;
    tempEnd = (OnigUChar *)str.End;
    if (onig_search(exp, tempBeg, tempEnd, tempBeg, tempEnd, onigReg, ONIG_OPTION_NONE) >= 0)
    {
        pos = (ind >= 0 ? ind : 0);
        if (pos < onigReg->num_regs && onigReg->beg[pos] >= 0)
        {
            len = (onigReg->end[pos] - onigReg->beg[pos]) / sizeof(QSP_CHAR);
            res = qspGetNewText(qspStringFromLen((QSP_CHAR *)(tempBeg + onigReg->beg[pos]), len));
        }
        else
            res = qspNullString;
    }
    else
        res = qspNullString;
    onig_region_free(onigReg, 1);
    #else
	pos = (ind >= 0 ? ind : 0);
    int maxGroups = pos + 1;
    pcre_fullinfo(exp.rx, exp.extra, PCRE_INFO_CAPTURECOUNT, &maxGroups);
    size_t groupsBuffSize = sizeof(int) * maxGroups * 3;
    int* groups = alloca(groupsBuffSize);
    memset(groups, 0, groupsBuffSize);
    const char * tempBeg = (const char *) str.Str;
    if (pcre_exec(exp.rx, exp.extra, tempBeg, str.End-str.Str, 0, PCRE_NEWLINE_ANYCRLF, groups, maxGroups))
    {
        if (pos < maxGroups && groups[pos * 2] >= 0)
        {
            len = (groups[pos * 2+1] - groups[pos * 2]) / sizeof(QSP_CHAR);
            res = qspGetNewText(qspStringFromLen((QSP_CHAR *)(tempBeg + groups[pos * 2]), len));
        }
        else
            res = qspNullString;
    }
    else
        res = qspNullString;
    #endif
    return res;
}

int qspRegExpStrPos(RegExT exp, QSPString str, int ind)
{
    int pos, res;
    #if !defined(USE_PCRE)
    OnigUChar *tempBeg, *tempEnd;
    OnigRegion *onigReg = onig_region_new();
    tempBeg = (OnigUChar *)str.Str;
    tempEnd = (OnigUChar *)str.End;
    if (onig_search(exp, tempBeg, tempEnd, tempBeg, tempEnd, onigReg, ONIG_OPTION_NONE) >= 0)
    {
        pos = (ind >= 0 ? ind : 0);
        if (pos < onigReg->num_regs && onigReg->beg[pos] >= 0)
            res = onigReg->beg[pos] / sizeof(QSP_CHAR) + 1;
        else
            res = 0;
    }
    else
        res = 0;
    onig_region_free(onigReg, 1);
    #else
	pos = (ind >= 0 ? ind : 0);
    int maxGroups = pos + 1;
    pcre_fullinfo(exp.rx, exp.extra, PCRE_INFO_CAPTURECOUNT, &maxGroups);
    size_t groupsBuffSize = sizeof(int) * maxGroups * 3;
    int* groups = alloca(groupsBuffSize);
    memset(groups, 0, groupsBuffSize);
    const char * tempBeg = (const char *) str.Str;
    if (pcre_exec(exp.rx, exp.extra, tempBeg, str.End-str.Str, 0, PCRE_NEWLINE_ANYCRLF, groups, maxGroups))
    {
		if (pos < maxGroups && groups[pos] >= 0)
            res = groups[pos] / sizeof(QSP_CHAR) + 1;
        else
            res = 0;
    }
    else
        res = 0;
    #endif
    return res;
}
