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

#include "regexp.h"
#include "errors.h"
#include "text.h"

int qspCompiledRegExpsCurInd = 0;
QSPRegExp qspCompiledRegExps[QSP_MAXCACHEDREGEXPS];

void qspClearAllRegExps(QSP_BOOL toInit)
{
    int i;
    QSPRegExp *exp = qspCompiledRegExps;
    for (i = 0; i < QSP_MAXCACHEDREGEXPS; ++i)
    {
        if (!toInit && exp->CompiledExp)
        {
            qspFreeString(&exp->Text);
            onig_free(exp->CompiledExp);
        }
        exp->Text = qspNullString;
        exp->CompiledExp = 0;
        ++exp;
    }
    qspCompiledRegExpsCurInd = 0;
}

QSPRegExp *qspRegExpGetCompiled(QSPString exp)
{
    int i;
    regex_t *onigExp;
    QSPString safeExp;
    OnigErrorInfo onigInfo;
    QSPRegExp *compExp = qspCompiledRegExps;
    for (i = 0; i < QSP_MAXCACHEDREGEXPS; ++i)
    {
        if (!compExp->CompiledExp) break;
        if (!qspStrsComp(exp, compExp->Text))
            return compExp;
        ++compExp;
    }
    safeExp = (exp.Str ? exp : QSP_STATIC_STR(QSP_FMT("")));
    if (onig_new(&onigExp, (OnigUChar *)safeExp.Str, (OnigUChar *)safeExp.End,
        ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL_NG, &onigInfo))
    {
        qspSetError(QSP_ERR_INCORRECTREGEXP);
        return 0;
    }
    compExp = qspCompiledRegExps + qspCompiledRegExpsCurInd;
    if (compExp->CompiledExp)
    {
        qspFreeString(&compExp->Text);
        onig_free(compExp->CompiledExp);
    }
    compExp->Text = qspCopyToNewText(exp);
    compExp->CompiledExp = onigExp;
    if (++qspCompiledRegExpsCurInd == QSP_MAXCACHEDREGEXPS)
        qspCompiledRegExpsCurInd = 0;
    return compExp;
}

QSP_BOOL qspRegExpStrMatch(QSPRegExp *exp, QSPString str)
{
    OnigUChar *tempBeg = (OnigUChar *)str.Str, *tempEnd = (OnigUChar *)str.End;
    return (onig_match(exp->CompiledExp, tempBeg, tempEnd, tempBeg, 0, ONIG_OPTION_NONE) == tempEnd - tempBeg);
}

QSP_CHAR *qspRegExpStrSearch(QSPRegExp *exp, QSPString str, int ind, int *foundLen)
{
    QSP_CHAR *foundPos = 0;
    OnigUChar *tempBeg = (OnigUChar *)str.Str, *tempEnd = (OnigUChar *)str.End;
    OnigRegion *onigReg = onig_region_new();
    if (onig_search(exp->CompiledExp, tempBeg, tempEnd, tempBeg, tempEnd, onigReg, ONIG_OPTION_NONE) >= 0)
    {
        int pos = (ind >= 0 ? ind : 0);
        if (pos < onigReg->num_regs && onigReg->beg[pos] >= 0)
        {
            foundPos = (QSP_CHAR *)(tempBeg + onigReg->beg[pos]);
            if (foundLen) *foundLen = (onigReg->end[pos] - onigReg->beg[pos]) / sizeof(QSP_CHAR);
        }
    }
    if (foundLen && !foundPos) *foundLen = 0;
    onig_region_free(onigReg, 1);
    return foundPos;
}
