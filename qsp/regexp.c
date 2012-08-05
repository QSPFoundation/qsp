/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
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

void qspClearRegExps(QSP_BOOL isFirst)
{
	int i;
	QSPRegExp *exp = qspCompiledRegExps;
	for (i = 0; i < QSP_MAXCACHEDREGEXPS; ++i)
	{
		if (!isFirst && exp->Text)
		{
			free(exp->Text);
			onig_free(exp->CompiledExp);
		}
		exp->Text = 0;
		exp->CompiledExp = 0;
		++exp;
	}
	qspCompiledRegExpsCurInd = 0;
}

regex_t *qspRegExpGetCompiled(QSP_CHAR *exp)
{
	int i;
	regex_t *onigExp;
	OnigUChar *tempBeg, *tempEnd;
	OnigErrorInfo onigInfo;
	QSPRegExp *compExp = qspCompiledRegExps;
	for (i = 0; i < QSP_MAXCACHEDREGEXPS; ++i)
	{
		if (!compExp->Text) break;
		if (!qspStrsComp(exp, compExp->Text))
			return compExp->CompiledExp;
		++compExp;
	}
	tempBeg = (OnigUChar *)exp;
	tempEnd = (OnigUChar *)qspStrEnd(exp);
	if (onig_new(&onigExp, tempBeg, tempEnd, ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL_NG, &onigInfo))
	{
		qspSetError(QSP_ERR_INCORRECTREGEXP);
		return 0;
	}
	compExp = qspCompiledRegExps + qspCompiledRegExpsCurInd;
	if (compExp->Text)
	{
		free(compExp->Text);
		onig_free(compExp->CompiledExp);
	}
	compExp->Text = qspGetNewText(exp, -1);
	compExp->CompiledExp = onigExp;
	if (++qspCompiledRegExpsCurInd == QSP_MAXCACHEDREGEXPS)
		qspCompiledRegExpsCurInd = 0;
	return onigExp;
}

QSP_BOOL qspRegExpStrMatch(regex_t *exp, QSP_CHAR *str)
{
	OnigUChar *tempBeg, *tempEnd;
	tempBeg = (OnigUChar *)str;
	tempEnd = (OnigUChar *)qspStrEnd(str);
	return (onig_match(exp, tempBeg, tempEnd, tempBeg, 0, ONIG_OPTION_NONE) == tempEnd - tempBeg);
}

QSP_CHAR *qspRegExpStrFind(regex_t *exp, QSP_CHAR *str, int ind)
{
	QSP_CHAR *res;
	int len, pos;
	OnigUChar *tempBeg, *tempEnd;
	OnigRegion *onigReg = onig_region_new();
	tempBeg = (OnigUChar *)str;
	tempEnd = (OnigUChar *)qspStrEnd(str);
	if (onig_search(exp, tempBeg, tempEnd, tempBeg, tempEnd, onigReg, ONIG_OPTION_NONE) >= 0)
	{
		pos = (ind >= 0 ? ind : 0);
		if (pos < onigReg->num_regs && onigReg->beg[pos] >= 0)
		{
			len = (onigReg->end[pos] - onigReg->beg[pos]) / sizeof(QSP_CHAR);
			res = qspGetNewText((QSP_CHAR *)(tempBeg + onigReg->beg[pos]), len);
		}
		else
			res = qspGetNewText(QSP_FMT(""), 0);
	}
	else
		res = qspGetNewText(QSP_FMT(""), 0);
	onig_region_free(onigReg, 1);
	return res;
}

int qspRegExpStrPos(regex_t *exp, QSP_CHAR *str, int ind)
{
	int pos, res;
	OnigUChar *tempBeg, *tempEnd;
	OnigRegion *onigReg = onig_region_new();
	tempBeg = (OnigUChar *)str;
	tempEnd = (OnigUChar *)qspStrEnd(str);
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
	return res;
}
