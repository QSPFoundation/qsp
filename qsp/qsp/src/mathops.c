/* Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include "mathops.h"
#include "callbacks.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "objects.h"
#include "text.h"
#include "time.h"
#include "variables.h"

QSPMathOperation qspOps[qspOpLast_Operation];
QSPMathOpName qspOpsNames[QSP_OPSLEVELS][QSP_MAXOPSNAMES];
long qspOpsNamesCounts[QSP_OPSLEVELS];
long qspOpMaxLen = 0;

static void qspAddOperation(long, char, QSP_FUNCTION, char, long, long, ...);
static void qspAddOpName(long, QSP_CHAR *, long);
static int qspMathOpsCompare(const void *, const void *);
static int qspMathOpStringFullCompare(const void *, const void *);
static int qspMathOpStringCompare(const void *, const void *);
static long qspGetNumber(QSP_CHAR **);
static QSP_CHAR *qspGetName(QSP_CHAR **);
static long qspFunctionOpCode(QSP_CHAR *);
static long qspOperatorOpCode(QSP_CHAR **);
static QSP_CHAR *qspGetString(QSP_CHAR **);
static QSPVariant qspValue(long, QSPVariant *, long *, long *);
static void qspCompileExprPushOpCode(long *, long *, long *, long *, long);
static void qspAppendToCompiled(long, long *, QSPVariant *, long *, long *, long, QSPVariant);
static long qspCompileExpression(QSP_CHAR *, QSPVariant *, long *, long *);
static void qspFunctionStrComp(QSPVariant *, long, QSPVariant *);
static void qspFunctionStrFind(QSPVariant *, long, QSPVariant *);
static void qspFunctionStrPos(QSPVariant *, long, QSPVariant *);
static void qspFunctionRGB(QSPVariant *, long, QSPVariant *);
static void qspFunctionMid(QSPVariant *, long, QSPVariant *);
static void qspFunctionRand(QSPVariant *, long, QSPVariant *);
static void qspFunctionDesc(QSPVariant *, long, QSPVariant *);
static void qspFunctionGetObj(QSPVariant *, long, QSPVariant *);
static void qspFunctionIsPlay(QSPVariant *, long, QSPVariant *);
static void qspFunctionInstr(QSPVariant *, long, QSPVariant *);
static void qspFunctionReplace(QSPVariant *, long, QSPVariant *);
static void qspFunctionFunc(QSPVariant *, long, QSPVariant *);
static void qspFunctionDynEval(QSPVariant *, long, QSPVariant *);
static void qspFunctionMin(QSPVariant *, long, QSPVariant *);
static void qspFunctionMax(QSPVariant *, long, QSPVariant *);

static void qspAddOperation(long opCode, char priority, QSP_FUNCTION func, char resType, long minArgs, long maxArgs, ...)
{
	long i;
	va_list marker;
	qspOps[opCode].Priority = priority;
	qspOps[opCode].Func = func;
	qspOps[opCode].ResType = resType;
	qspOps[opCode].MinArgsCount = minArgs;
	qspOps[opCode].MaxArgsCount = maxArgs;
	if (maxArgs > 0)
	{
		va_start(marker, maxArgs);
		for (i = 0; i < maxArgs; ++i)
			qspOps[opCode].ArgsTypes[i] = va_arg(marker, int);
		va_end(marker);
	}
}

static void qspAddOpName(long opCode, QSP_CHAR *opName, long level)
{
	long count, len = (long)QSP_STRLEN(opName);
	count = qspOpsNamesCounts[level];
	qspOpsNames[level][count].Name = opName;
	qspOpsNames[level][count].NameLen = len;
	qspOpsNames[level][count].Code = opCode;
	qspOpsNamesCounts[level] = count + 1;
	/* Max length */
	if (len > qspOpMaxLen) qspOpMaxLen = len;
}

static int qspMathOpsCompare(const void *opName1, const void *opName2)
{
	return QSP_STRCMP(((QSPMathOpName *)opName1)->Name, ((QSPMathOpName *)opName2)->Name);
}

static int qspMathOpStringFullCompare(const void *name, const void *compareTo)
{
	return QSP_STRCMP((QSP_CHAR *)name, ((QSPMathOpName *)compareTo)->Name);
}

static int qspMathOpStringCompare(const void *name, const void *compareTo)
{
	return qspStrsComp((QSP_CHAR *)name, ((QSPMathOpName *)compareTo)->Name, ((QSPMathOpName *)compareTo)->NameLen);
}

void qspInitMath()
{
	/*
	Format:
		qspAddOperation(
			Operation,
			Priority,
			Function's Function,
			Result's Type,
			Minimum Arguments' Count,
			Maximum Arguments' Count,
			Arguments' Types [optional]
		);

		"Result's Type" and "Arguments' Types":
		0 - Unknown / Any
		1 - String
		2 - Number
	*/
	long i;
	for (i = 0; i < QSP_OPSLEVELS; ++i) qspOpsNamesCounts[i] = 0;
	qspOpMaxLen = 0;
	qspAddOperation(qspOpValue, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpStart, 127, 0, 0, 0, 0);
	qspAddOperation(qspOpEnd, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpOpenBracket, 127, 0, 0, 0, 0);
	qspAddOperation(qspOpCloseBracket, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpMinus, 18, 0, 2, 1, 1, 2);
	qspAddOperation(qspOpAdd, 14, 0, 0, 2, 2, 0, 0);
	qspAddOperation(qspOpSub, 14, 0, 2, 2, 2, 2, 2);
	qspAddOperation(qspOpMul, 17, 0, 2, 2, 2, 2, 2);
	qspAddOperation(qspOpDiv, 17, 0, 2, 2, 2, 2, 2);
	qspAddOperation(qspOpNe, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpLeq, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpGeq, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpEq, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpLt, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpGt, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpAppend, 4, 0, 1, 2, 2, 1, 1);
	qspAddOperation(qspOpComma, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpAnd, 7, 0, 2, 2, 2, 2, 2);
	qspAddOperation(qspOpOr, 6, 0, 2, 2, 2, 2, 2);
	qspAddOperation(qspOpObj, 8, 0, 2, 1, 1, 1);
	qspAddOperation(qspOpNot, 8, 0, 2, 1, 1, 2);
	qspAddOperation(qspOpMin, 30, qspFunctionMin, 0, 1, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpMax, 30, qspFunctionMax, 0, 1, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpRand, 30, qspFunctionRand, 2, 1, 2, 2, 2);
	qspAddOperation(qspOpIIf, 30, 0, 0, 3, 3, 2, 0, 0);
	qspAddOperation(qspOpRGB, 30, qspFunctionRGB, 2, 3, 3, 2, 2, 2);
	qspAddOperation(qspOpLen, 30, 0, 2, 1, 1, 1);
	qspAddOperation(qspOpIsNum, 30, 0, 2, 1, 1, 0);
	qspAddOperation(qspOpLCase, 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpUCase, 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpInput, 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpStr, 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpVal, 30, 0, 2, 1, 1, 0);
	qspAddOperation(qspOpArrSize, 30, 0, 2, 1, 1, 1);
	qspAddOperation(qspOpIsPlay, 30, qspFunctionIsPlay, 2, 1, 1, 1);
	qspAddOperation(qspOpDesc, 30, qspFunctionDesc, 1, 1, 1, 1);
	qspAddOperation(qspOpTrim, 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpGetObj, 30, qspFunctionGetObj, 1, 1, 1, 2);
	qspAddOperation(qspOpStrComp, 30, qspFunctionStrComp, 2, 2, 2, 1, 1);
	qspAddOperation(qspOpStrFind, 30, qspFunctionStrFind, 1, 2, 3, 1, 1, 2);
	qspAddOperation(qspOpStrPos, 30, qspFunctionStrPos, 2, 2, 3, 1, 1, 2);
	qspAddOperation(qspOpMid, 30, qspFunctionMid, 1, 2, 3, 1, 2, 2);
	qspAddOperation(qspOpArrPos, 30, 0, 2, 2, 3, 0, 0, 0);
	qspAddOperation(qspOpArrComp, 30, 0, 2, 2, 3, 0, 0, 0);
	qspAddOperation(qspOpInstr, 30, qspFunctionInstr, 2, 2, 3, 0, 1, 1);
	qspAddOperation(qspOpReplace, 30, qspFunctionReplace, 1, 2, 3, 1, 1, 1);
	qspAddOperation(qspOpFunc, 30, qspFunctionFunc, 0, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpDynEval, 30, qspFunctionDynEval, 0, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpRnd, 30, 0, 2, 0, 0);
	qspAddOperation(qspOpCountObj, 30, 0, 2, 0, 0);
	qspAddOperation(qspOpMsecsCount, 30, 0, 2, 0, 0);
	qspAddOperation(qspOpQSPVer, 30, 0, 1, 0, 0);
	qspAddOperation(qspOpUserText, 30, 0, 1, 0, 0);
	qspAddOperation(qspOpCurLoc, 30, 0, 1, 0, 0);
	qspAddOperation(qspOpSelObj, 30, 0, 1, 0, 0);
	qspAddOperation(qspOpSelAct, 30, 0, 1, 0, 0);
	qspAddOperation(qspOpMainText, 30, 0, 1, 0, 0);
	qspAddOperation(qspOpStatText, 30, 0, 1, 0, 0);
	qspAddOperation(qspOpCurActs, 30, 0, 1, 0, 0);
	/* Names */
	qspAddOpName(qspOpOpenBracket, QSP_LRBRACK, 1);
	qspAddOpName(qspOpCloseBracket, QSP_RRBRACK, 1);
	qspAddOpName(qspOpMinus, QSP_UMINUS, 1);
	qspAddOpName(qspOpAdd, QSP_ADD, 1);
	qspAddOpName(qspOpSub, QSP_SUB, 1);
	qspAddOpName(qspOpMul, QSP_MUL, 1);
	qspAddOpName(qspOpDiv, QSP_DIV, 1);
	qspAddOpName(qspOpNe, QSP_NOTEQUAL1, 1);
	qspAddOpName(qspOpNe, QSP_NOTEQUAL2, 0);
	qspAddOpName(qspOpLeq, QSP_LESSEQ1, 0);
	qspAddOpName(qspOpLeq, QSP_LESSEQ2, 0);
	qspAddOpName(qspOpGeq, QSP_GREATEQ1, 0);
	qspAddOpName(qspOpGeq, QSP_GREATEQ2, 0);
	qspAddOpName(qspOpEq, QSP_EQUAL, 1);
	qspAddOpName(qspOpLt, QSP_LESS, 1);
	qspAddOpName(qspOpGt, QSP_GREAT, 1);
	qspAddOpName(qspOpAppend, QSP_APPEND, 1);
	qspAddOpName(qspOpComma, QSP_COMMA, 1);
	qspAddOpName(qspOpAnd, QSP_FMT("AND"), 1);
	qspAddOpName(qspOpOr, QSP_FMT("OR"), 1);
	qspAddOpName(qspOpObj, QSP_FMT("OBJ"), 1);
	qspAddOpName(qspOpNot, QSP_FMT("NO"), 1);
	qspAddOpName(qspOpMin, QSP_FMT("MIN"), 1);
	qspAddOpName(qspOpMin, QSP_STRCHAR QSP_FMT("MIN"), 1);
	qspAddOpName(qspOpMax, QSP_FMT("MAX"), 1);
	qspAddOpName(qspOpMax, QSP_STRCHAR QSP_FMT("MAX"), 1);
	qspAddOpName(qspOpRand, QSP_FMT("RAND"), 1);
	qspAddOpName(qspOpIIf, QSP_FMT("IIF"), 1);
	qspAddOpName(qspOpIIf, QSP_STRCHAR QSP_FMT("IIF"), 1);
	qspAddOpName(qspOpRGB, QSP_FMT("RGB"), 1);
	qspAddOpName(qspOpLen, QSP_FMT("LEN"), 1);
	qspAddOpName(qspOpIsNum, QSP_FMT("ISNUM"), 1);
	qspAddOpName(qspOpLCase, QSP_FMT("LCASE"), 1);
	qspAddOpName(qspOpLCase, QSP_STRCHAR QSP_FMT("LCASE"), 1);
	qspAddOpName(qspOpUCase, QSP_FMT("UCASE"), 1);
	qspAddOpName(qspOpUCase, QSP_STRCHAR QSP_FMT("UCASE"), 1);
	qspAddOpName(qspOpInput, QSP_FMT("INPUT"), 1);
	qspAddOpName(qspOpInput, QSP_STRCHAR QSP_FMT("INPUT"), 1);
	qspAddOpName(qspOpStr, QSP_FMT("STR"), 1);
	qspAddOpName(qspOpStr, QSP_STRCHAR QSP_FMT("STR"), 1);
	qspAddOpName(qspOpVal, QSP_FMT("VAL"), 1);
	qspAddOpName(qspOpArrSize, QSP_FMT("ARRSIZE"), 1);
	qspAddOpName(qspOpIsPlay, QSP_FMT("ISPLAY"), 1);
	qspAddOpName(qspOpDesc, QSP_FMT("DESC"), 1);
	qspAddOpName(qspOpDesc, QSP_STRCHAR QSP_FMT("DESC"), 1);
	qspAddOpName(qspOpTrim, QSP_FMT("TRIM"), 1);
	qspAddOpName(qspOpTrim, QSP_STRCHAR QSP_FMT("TRIM"), 1);
	qspAddOpName(qspOpGetObj, QSP_FMT("GETOBJ"), 1);
	qspAddOpName(qspOpGetObj, QSP_STRCHAR QSP_FMT("GETOBJ"), 1);
	qspAddOpName(qspOpStrComp, QSP_FMT("STRCOMP"), 1);
	qspAddOpName(qspOpStrFind, QSP_FMT("STRFIND"), 1);
	qspAddOpName(qspOpStrFind, QSP_STRCHAR QSP_FMT("STRFIND"), 1);
	qspAddOpName(qspOpStrPos, QSP_FMT("STRPOS"), 1);
	qspAddOpName(qspOpMid, QSP_FMT("MID"), 1);
	qspAddOpName(qspOpMid, QSP_STRCHAR QSP_FMT("MID"), 1);
	qspAddOpName(qspOpArrPos, QSP_FMT("ARRPOS"), 1);
	qspAddOpName(qspOpArrComp, QSP_FMT("ARRCOMP"), 1);
	qspAddOpName(qspOpInstr, QSP_FMT("INSTR"), 1);
	qspAddOpName(qspOpReplace, QSP_FMT("REPLACE"), 1);
	qspAddOpName(qspOpReplace, QSP_STRCHAR QSP_FMT("REPLACE"), 1);
	qspAddOpName(qspOpFunc, QSP_FMT("FUNC"), 1);
	qspAddOpName(qspOpFunc, QSP_STRCHAR QSP_FMT("FUNC"), 1);
	qspAddOpName(qspOpDynEval, QSP_FMT("DYNEVAL"), 1);
	qspAddOpName(qspOpDynEval, QSP_STRCHAR QSP_FMT("DYNEVAL"), 1);
	qspAddOpName(qspOpRnd, QSP_FMT("RND"), 1);
	qspAddOpName(qspOpCountObj, QSP_FMT("COUNTOBJ"), 1);
	qspAddOpName(qspOpMsecsCount, QSP_FMT("MSECSCOUNT"), 1);
	qspAddOpName(qspOpQSPVer, QSP_FMT("QSPVER"), 1);
	qspAddOpName(qspOpQSPVer, QSP_STRCHAR QSP_FMT("QSPVER"), 1);
	qspAddOpName(qspOpUserText, QSP_FMT("USER_TEXT"), 1);
	qspAddOpName(qspOpUserText, QSP_STRCHAR QSP_FMT("USER_TEXT"), 1);
	qspAddOpName(qspOpUserText, QSP_FMT("USRTXT"), 1);
	qspAddOpName(qspOpUserText, QSP_STRCHAR QSP_FMT("USRTXT"), 1);
	qspAddOpName(qspOpCurLoc, QSP_FMT("CURLOC"), 1);
	qspAddOpName(qspOpCurLoc, QSP_STRCHAR QSP_FMT("CURLOC"), 1);
	qspAddOpName(qspOpSelObj, QSP_FMT("SELOBJ"), 1);
	qspAddOpName(qspOpSelObj, QSP_STRCHAR QSP_FMT("SELOBJ"), 1);
	qspAddOpName(qspOpSelAct, QSP_FMT("SELACT"), 1);
	qspAddOpName(qspOpSelAct, QSP_STRCHAR QSP_FMT("SELACT"), 1);
	qspAddOpName(qspOpMainText, QSP_FMT("MAINTXT"), 1);
	qspAddOpName(qspOpMainText, QSP_STRCHAR QSP_FMT("MAINTXT"), 1);
	qspAddOpName(qspOpStatText, QSP_FMT("STATTXT"), 1);
	qspAddOpName(qspOpStatText, QSP_STRCHAR QSP_FMT("STATTXT"), 1);
	qspAddOpName(qspOpCurActs, QSP_FMT("CURACTS"), 1);
	qspAddOpName(qspOpCurActs, QSP_STRCHAR QSP_FMT("CURACTS"), 1);
	for (i = 0; i < QSP_OPSLEVELS; ++i)
		qsort(qspOpsNames[i], qspOpsNamesCounts[i], sizeof(QSPMathOpName), qspMathOpsCompare);
}

static long qspGetNumber(QSP_CHAR **expr)
{
	long i = 0;
	QSP_CHAR buf[11], *pos = *expr;
	while (qspIsDigit(*pos))
	{
		if (i < QSP_LEN(buf)) buf[i] = *pos;
		++i;
		++pos;
	}
	*expr = pos;
	if (i > QSP_LEN(buf)) return LONG_MAX;
	buf[i] = 0;
	return qspStrToNum(buf, 0);
}

static QSP_CHAR *qspGetName(QSP_CHAR **expr)
{
	QSP_CHAR *startPos = *expr, *pos = startPos;
	do
	{
		if (*(++pos) == QSP_LSBRACK[0])
		{
			if (!(pos = qspStrPos(pos, QSP_RSBRACK, QSP_FALSE)))
			{
				qspSetError(QSP_ERR_BRACKNOTFOUND);
				return 0;
			}
			++pos;
			break;
		}
	} while (!qspIsInListEOL(QSP_DELIMS, *pos));
	*expr = qspSkipSpaces(pos);
	return qspGetNewText(startPos, (long)(pos - startPos));
}

static long qspFunctionOpCode(QSP_CHAR *funName)
{
	QSP_CHAR *uName;
	QSPMathOpName *name;
	qspUpperStr(uName = qspGetNewText(funName, -1));
	name = (QSPMathOpName *)bsearch(uName, qspOpsNames[QSP_OPSLEVELS - 1], qspOpsNamesCounts[QSP_OPSLEVELS - 1], sizeof(QSPMathOpName), qspMathOpStringFullCompare);
	free(uName);
	if (name) return name->Code;
	return qspOpUnknown;
}

static long qspOperatorOpCode(QSP_CHAR **expr)
{
	long i;
	QSP_CHAR *uStr;
	QSPMathOpName *name;
	if (!(**expr)) return qspOpEnd;
	qspUpperStr(uStr = qspGetNewText(*expr, qspOpMaxLen));
	for (i = 0; i < QSP_OPSLEVELS; ++i)
	{
		name = (QSPMathOpName *)bsearch(uStr, qspOpsNames[i], qspOpsNamesCounts[i], sizeof(QSPMathOpName), qspMathOpStringCompare);
		if (name)
		{
			*expr += name->NameLen;
			free(uStr);
			return name->Code;
		}
	}
	free(uStr);
	return qspOpUnknown;
}

static QSP_CHAR *qspGetString(QSP_CHAR **expr)
{
	long strLen = 0, bufSize = 16;
	QSP_CHAR *buf, *pos = *expr, quot = *pos;
	buf = (QSP_CHAR *)malloc(bufSize * sizeof(QSP_CHAR));
	while (1)
	{
		if (!(*(++pos)))
		{
			qspSetError(QSP_ERR_QUOTNOTFOUND);
			free(buf);
			return 0;
		}
		if (*pos == quot && *(++pos) != quot) break;
		if (++strLen >= bufSize)
		{
			bufSize <<= 1;
			buf = (QSP_CHAR *)realloc(buf, bufSize * sizeof(QSP_CHAR));
		}
		buf[strLen - 1] = *pos;
	}
	buf[strLen] = 0;
	*expr = pos;
	pos = qspFormatText(buf);
	free(buf);
	return pos;
}

static QSPVariant qspValue(long itemsCount, QSPVariant *compValues, long *compOpCodes, long *compArgsCounts)
{
	char type;
	QSPVariant stack[QSP_STACKSIZE], args[QSP_OPMAXARGS], tos;
	long i, j, oldRefreshCount, opCode, argsCount, len, sp = -1, index = 0;
	tos.IsStr = QSP_FALSE;
	QSP_NUM(tos) = 0;
	oldRefreshCount = qspRefreshCount;
	while (1)
	{
		if (index == itemsCount) return tos;
		opCode = compOpCodes[index];
		argsCount = compArgsCounts[index];
		if (argsCount)
		{
			for (i = argsCount - 2, j = sp, args[i + 1] = tos; i >= 0; --i, --j)
				args[i] = stack[j];
			for (i = 0; i < argsCount; ++i)
			{
				type = qspOps[opCode].ArgsTypes[i];
				if (type && qspConvertVariantTo(args + i, type == 1))
				{
					qspSetError(QSP_ERR_TYPEMISMATCH);
					break;
				}
			}
		}
		else
		{
			if (sp == QSP_STACKSIZE - 1)
			{
				qspSetError(QSP_ERR_STACKOVERFLOW);
				if (tos.IsStr) free(QSP_STR(tos));
				break;
			}
			stack[++sp] = tos;
		}
		if (!qspErrorNum)
		{
			type = qspOps[opCode].ResType;
			if (type) tos.IsStr = type == 1;
			switch (opCode)
			{
			case qspOpValue:
				tos = compValues[index];
				break;
			case qspOpMul:
				QSP_NUM(tos) = QSP_NUM(args[0]) * QSP_NUM(args[1]);
				break;
			case qspOpDiv:
				if (!QSP_NUM(args[1]))
				{
					qspSetError(QSP_ERR_DIVBYZERO);
					break;
				}
				QSP_NUM(tos) = QSP_NUM(args[0]) / QSP_NUM(args[1]);
				break;
			case qspOpAdd:
				if (args[0].IsStr && args[1].IsStr)
				{
					tos.IsStr = QSP_TRUE;
					len = qspAddText(&QSP_STR(tos), QSP_STR(args[0]), 0, -1, QSP_TRUE);
					QSP_STR(tos) = qspGetAddText(QSP_STR(tos), QSP_STR(args[1]), len, -1);
				}
				else if (qspIsCanConvertToNum(args) && qspIsCanConvertToNum(args + 1))
				{
					qspConvertVariantTo(args, QSP_FALSE);
					qspConvertVariantTo(args + 1, QSP_FALSE);
					tos.IsStr = QSP_FALSE;
					QSP_NUM(tos) = QSP_NUM(args[0]) + QSP_NUM(args[1]);
				}
				else
				{
					qspConvertVariantTo(args, QSP_TRUE);
					qspConvertVariantTo(args + 1, QSP_TRUE);
					tos.IsStr = QSP_TRUE;
					len = qspAddText(&QSP_STR(tos), QSP_STR(args[0]), 0, -1, QSP_TRUE);
					QSP_STR(tos) = qspGetAddText(QSP_STR(tos), QSP_STR(args[1]), len, -1);
				}
				break;
			case qspOpSub:
				QSP_NUM(tos) = QSP_NUM(args[0]) - QSP_NUM(args[1]);
				break;
			case qspOpAppend:
				len = qspAddText(&QSP_STR(tos), QSP_STR(args[0]), 0, -1, QSP_TRUE);
				QSP_STR(tos) = qspGetAddText(QSP_STR(tos), QSP_STR(args[1]), len, -1);
				break;
			case qspOpEq:
				QSP_NUM(tos) = -(!qspAutoConvertCompare(args, args + 1));
				break;
			case qspOpLt:
				QSP_NUM(tos) = -(qspAutoConvertCompare(args, args + 1) < 0);
				break;
			case qspOpGt:
				QSP_NUM(tos) = -(qspAutoConvertCompare(args, args + 1) > 0);
				break;
			case qspOpLeq:
				QSP_NUM(tos) = -(qspAutoConvertCompare(args, args + 1) <= 0);
				break;
			case qspOpGeq:
				QSP_NUM(tos) = -(qspAutoConvertCompare(args, args + 1) >= 0);
				break;
			case qspOpNe:
				QSP_NUM(tos) = -(qspAutoConvertCompare(args, args + 1) != 0);
				break;
			case qspOpMinus:
				QSP_NUM(tos) = -QSP_NUM(args[0]);
				break;
			case qspOpObj:
				QSP_NUM(tos) = -(qspObjIndex(QSP_STR(args[0])) >= 0);
				break;
			case qspOpNot:
				QSP_NUM(tos) = ~QSP_NUM(args[0]);
				break;
			case qspOpAnd:
				QSP_NUM(tos) = QSP_NUM(args[0]) & QSP_NUM(args[1]);
				break;
			case qspOpOr:
				QSP_NUM(tos) = QSP_NUM(args[0]) | QSP_NUM(args[1]);
				break;
			/* Embedded functions -------------------------------------------------------------- */
			case qspOpIIf:
				qspCopyVariant(&tos, QSP_NUM(args[0]) ? args + 1 : args + 2);
				break;
			case qspOpLen:
				QSP_NUM(tos) = (long)QSP_STRLEN(QSP_STR(args[0]));
				break;
			case qspOpIsNum:
				QSP_NUM(tos) = -qspIsCanConvertToNum(args);
				break;
			case qspOpLCase:
				qspLowerStr(QSP_STR(tos) = qspGetNewText(QSP_STR(args[0]), -1));
				break;
			case qspOpUCase:
				qspUpperStr(QSP_STR(tos) = qspGetNewText(QSP_STR(args[0]), -1));
				break;
			case qspOpStr:
				QSP_STR(tos) = qspGetNewText(QSP_STR(args[0]), -1);
				break;
			case qspOpVal:
				if (qspConvertVariantTo(args, QSP_FALSE))
					QSP_NUM(tos) = 0;
				else
					QSP_NUM(tos) = QSP_NUM(args[0]);
				break;
			case qspOpArrSize:
				QSP_NUM(tos) = qspArraySize(QSP_STR(args[0]));
				break;
			case qspOpTrim:
				QSP_STR(tos) = qspDelSpc(QSP_STR(args[0]));
				break;
			case qspOpArrPos:
				QSP_NUM(tos) = qspArrayPos(args, argsCount, QSP_FALSE);
				break;
			case qspOpArrComp:
				QSP_NUM(tos) = qspArrayPos(args, argsCount, QSP_TRUE);
				break;
			case qspOpInput:
				QSP_STR(tos) = qspCallInputBox(QSP_STR(args[0]));
				break;
			case qspOpRnd:
				QSP_NUM(tos) = rand() % 1000 + 1;
				break;
			case qspOpCountObj:
				QSP_NUM(tos) = qspCurObjectsCount;
				break;
			case qspOpMsecsCount:
				QSP_NUM(tos) = qspGetTime();
				break;
			case qspOpQSPVer:
				QSP_STR(tos) = qspGetNewText(QSP_VER, QSP_LEN(QSP_VER));
				break;
			case qspOpUserText:
				QSP_STR(tos) = (qspCurInput ? qspGetNewText(qspCurInput, qspCurInputLen) : qspGetNewText(QSP_FMT(""), 0));
				break;
			case qspOpCurLoc:
				QSP_STR(tos) = (qspCurLoc >= 0 ? qspGetNewText(qspLocs[qspCurLoc].Name, -1) : qspGetNewText(QSP_FMT(""), 0));
				break;
			case qspOpSelObj:
				QSP_STR(tos) = (qspCurSelObject >= 0 ? qspGetNewText(qspCurObjects[qspCurSelObject].Desc, -1) : qspGetNewText(QSP_FMT(""), 0));
				break;
			case qspOpSelAct:
				QSP_STR(tos) = (qspCurSelAction >= 0 ? qspGetNewText(qspCurActions[qspCurSelAction].Desc, -1) : qspGetNewText(QSP_FMT(""), 0));
				break;
			case qspOpMainText:
				QSP_STR(tos) = (qspCurDesc ? qspGetNewText(qspCurDesc, qspCurDescLen) : qspGetNewText(QSP_FMT(""), 0));
				break;
			case qspOpStatText:
				QSP_STR(tos) = (qspCurVars ? qspGetNewText(qspCurVars, qspCurVarsLen) : qspGetNewText(QSP_FMT(""), 0));
				break;
			case qspOpCurActs:
				QSP_STR(tos) = qspGetAllActionsAsCode();
				break;
			/* External functions -------------------------------------------------------------- */
			default:
				qspOps[opCode].Func(args, argsCount, &tos);
				break;
			}
		}
		if (argsCount)
		{
			qspFreeVariants(args, argsCount);
			sp -= argsCount - 1;
		}
		if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
		++index;
	}
	qspFreeVariants(stack, sp + 1);
	while (index < itemsCount)
	{
		if (compOpCodes[index] == qspOpValue && compValues[index].IsStr)
			free(QSP_STR(compValues[index]));
		++index;
	}
	return qspGetEmptyVariant(QSP_FALSE);
}

static void qspCompileExprPushOpCode(long *opStack, long *opSp, long *argStack, long *argSp, long opCode)
{
	if (*opSp == QSP_STACKSIZE - 1 || *argSp == QSP_STACKSIZE - 1)
	{
		qspSetError(QSP_ERR_STACKOVERFLOW);
		return;
	}
	opStack[++(*opSp)] = opCode;
	argStack[++(*argSp)] = (opCode < qspOpFirst_Function ? qspOps[opCode].MinArgsCount : 0);
}

static void qspAppendToCompiled(long opCode, long *itemsCount, QSPVariant *compValues, long *compOpCodes, long *compArgsCounts, long argsCount, QSPVariant v)
{
	if (*itemsCount == QSP_MAXITEMS)
	{
		qspSetError(QSP_ERR_TOOMANYITEMS);
		return;
	}
	compOpCodes[*itemsCount] = opCode;
	compArgsCounts[*itemsCount] = argsCount;
	if (opCode == qspOpValue) compValues[*itemsCount] = v;
	++(*itemsCount);
}

static long qspCompileExpression(QSP_CHAR *s, QSPVariant *compValues, long *compOpCodes, long *compArgsCounts)
{
	QSPVariant v;
	QSP_CHAR *name;
	QSP_BOOL waitForOperator = QSP_FALSE;
	long oldRefreshCount, opStack[QSP_STACKSIZE], argStack[QSP_STACKSIZE], opCode, itemsCount = 0, opSp = -1, argSp = -1;
	qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, qspOpStart);
	if (qspErrorNum) return 0;
	oldRefreshCount = qspRefreshCount;
	while (1)
	{
		s = qspSkipSpaces(s);
		if (waitForOperator)
		{
			opCode = qspOperatorOpCode(&s);
			if (opCode == qspOpUnknown)
			{
				qspSetError(QSP_ERR_UNKNOWNACTION);
				break;
			}
			if ((opCode == qspOpAnd || opCode == qspOpOr) && !qspIsInList(QSP_SPACES QSP_QUOTS QSP_LRBRACK, *s))
			{
				qspSetError(QSP_ERR_SYNTAX);
				break;
			}
			while (qspOps[opCode].Priority <= qspOps[opStack[opSp]].Priority && qspOps[opStack[opSp]].Priority != 127)
			{
				if (opStack[opSp] >= qspOpFirst_Function) ++argStack[argSp];
				qspAppendToCompiled(opStack[opSp], &itemsCount, compValues, compOpCodes, compArgsCounts, argStack[argSp], v);
				if (qspErrorNum) break;
				if (--opSp < 0 || --argSp < 0)
				{
					qspSetError(QSP_ERR_SYNTAX);
					break;
				}
			}
			if (qspErrorNum) break;
			switch (opCode)
			{
			case qspOpEnd:
				if (opSp)
				{
					qspSetError(QSP_ERR_BRACKNOTFOUND);
					break;
				}
				return itemsCount;
			case qspOpCloseBracket:
				if (opStack[opSp] != qspOpOpenBracket)
				{
					qspSetError(QSP_ERR_BRACKNOTFOUND);
					break;
				}
				opCode = opStack[--opSp];
				if (opCode >= qspOpFirst_Function)
				{
					if (argStack[argSp] + 1 < qspOps[opCode].MinArgsCount || argStack[argSp] + 1 > qspOps[opCode].MaxArgsCount)
						qspSetError(QSP_ERR_ARGSCOUNT);
				}
				else
					--argSp;
				break;
			case qspOpComma:
				if (!opSp || opStack[opSp - 1] < qspOpFirst_Function)
				{
					qspSetError(QSP_ERR_SYNTAX);
					break;
				}
				if (++argStack[argSp] > qspOps[opStack[opSp - 1]].MaxArgsCount)
				{
					qspSetError(QSP_ERR_ARGSCOUNT);
					break;
				}
				waitForOperator = QSP_FALSE;
				break;
			default:
				qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, opCode);
				if (qspErrorNum) break;
				waitForOperator = QSP_FALSE;
				break;
			}
			if (qspErrorNum) break;
		}
		else
		{
			if (qspIsDigit(*s))
			{
				v.IsStr = QSP_FALSE;
				QSP_NUM(v) = qspGetNumber(&s);
				if (opStack[opSp] == qspOpMinus)
				{
					QSP_NUM(v) = -QSP_NUM(v);
					--opSp;
					--argSp;
				}
				qspAppendToCompiled(qspOpValue, &itemsCount, compValues, compOpCodes, compArgsCounts, 0, v);
				if (qspErrorNum) break;
				waitForOperator = QSP_TRUE;
			}
			else if (qspIsInList(QSP_QUOTS, *s))
			{
				v.IsStr = QSP_TRUE;
				QSP_STR(v) = qspGetString(&s);
				if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
				qspAppendToCompiled(qspOpValue, &itemsCount, compValues, compOpCodes, compArgsCounts, 0, v);
				if (qspErrorNum)
				{
					free(QSP_STR(v));
					break;
				}
				waitForOperator = QSP_TRUE;
			}
			else if (*s == QSP_UPLUS[0])
			{
				++s;
			}
			else if (*s == QSP_UMINUS[0])
			{
				qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, qspOpMinus);
				if (qspErrorNum) break;
				++s;
			}
			else if (*s == QSP_LRBRACK[0])
			{
				qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, qspOpOpenBracket);
				if (qspErrorNum) break;
				++s;
			}
			else if (*s == QSP_RRBRACK[0])
			{
				opCode = opStack[opSp];
				if (opCode != qspOpOpenBracket)
				{
					if (opCode >= qspOpFirst_Function)
						qspSetError(QSP_ERR_ARGSCOUNT);
					else
						qspSetError(QSP_ERR_SYNTAX);
					break;
				}
				opCode = opStack[--opSp];
				if (opCode < qspOpFirst_Function)
				{
					qspSetError(QSP_ERR_SYNTAX);
					break;
				}
				if (qspOps[opCode].MinArgsCount)
				{
					qspSetError(QSP_ERR_ARGSCOUNT);
					break;
				}
				++s;
				qspAppendToCompiled(opCode, &itemsCount, compValues, compOpCodes, compArgsCounts, 0, v);
				if (qspErrorNum) break;
				--opSp;
				--argSp;
				waitForOperator = QSP_TRUE;
			}
			else if (!qspIsInListEOL(QSP_DELIMS, *s))
			{
				name = qspGetName(&s);
				if (qspErrorNum) break;
				opCode = qspFunctionOpCode(name);
				if (opCode != qspOpUnknown)
				{
					free(name);
					if (*s == QSP_LRBRACK[0])
					{
						qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, opCode);
						if (qspErrorNum) break;
						qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, qspOpOpenBracket);
						if (qspErrorNum) break;
						++s;
						--argSp;
					}
					else if (qspOps[opCode].MinArgsCount < 2)
					{
						if (qspOps[opCode].MinArgsCount)
						{
							qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, opCode);
							if (qspErrorNum) break;
						}
						else
						{
							qspAppendToCompiled(opCode, &itemsCount, compValues, compOpCodes, compArgsCounts, 0, v);
							if (qspErrorNum) break;
							waitForOperator = QSP_TRUE;
						}
					}
					else
					{
						qspSetError(QSP_ERR_BRACKSNOTFOUND);
						break;
					}
				}
				else
				{
					v = qspGetVar(name);
					free(name);
					if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
					qspAppendToCompiled(qspOpValue, &itemsCount, compValues, compOpCodes, compArgsCounts, 0, v);
					if (qspErrorNum)
					{
						if (v.IsStr) free(QSP_STR(v));
						break;
					}
					waitForOperator = QSP_TRUE;
				}
			}
			else
			{
				if (opStack[opSp] >= qspOpFirst_Function)
					qspSetError(QSP_ERR_ARGSCOUNT);
				else
					qspSetError(QSP_ERR_SYNTAX);
				break;
			}
		}
	}
	while (--itemsCount >= 0)
	{
		if (compOpCodes[itemsCount] == qspOpValue && compValues[itemsCount].IsStr)
			free(QSP_STR(compValues[itemsCount]));
	}
	return 0;
}

QSPVariant qspExprValue(QSP_CHAR *expr)
{
	QSPVariant compValues[QSP_MAXITEMS];
	long compOpCodes[QSP_MAXITEMS], compArgsCounts[QSP_MAXITEMS], itemsCount, oldRefreshCount = qspRefreshCount;
	itemsCount = qspCompileExpression(expr, compValues, compOpCodes, compArgsCounts);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		return qspGetEmptyVariant(QSP_FALSE);
	return qspValue(itemsCount, compValues, compOpCodes, compArgsCounts);
}

static void qspFunctionStrComp(QSPVariant *args, long count, QSPVariant *tos)
{
	OnigUChar *tempBeg, *tempEnd;
	regex_t *onigExp;
	OnigRegion *onigReg;
	OnigErrorInfo onigInfo;
	tempBeg = (OnigUChar *)QSP_STR(args[1]);
	tempEnd = (OnigUChar *)qspStrEnd(QSP_STR(args[1]));
	if (onig_new(&onigExp, tempBeg, tempEnd, ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL, &onigInfo))
		qspSetError(QSP_ERR_INCORRECTREGEXP);
	else
	{
		onigReg = onig_region_new();
		tempBeg = (OnigUChar *)QSP_STR(args[0]);
		tempEnd = (OnigUChar *)qspStrEnd(QSP_STR(args[0]));
		QSP_PNUM(tos) = -(onig_match(onigExp, tempBeg, tempEnd, tempBeg, onigReg, ONIG_OPTION_NONE) >= 0);
		onig_region_free(onigReg, 1);
		onig_free(onigExp);
	}
}

static void qspFunctionStrFind(QSPVariant *args, long count, QSPVariant *tos)
{
	long len, pos;
	OnigUChar *tempBeg, *tempEnd;
	regex_t *onigExp;
	OnigRegion *onigReg;
	OnigErrorInfo onigInfo;
	tempBeg = (OnigUChar *)QSP_STR(args[1]);
	tempEnd = (OnigUChar *)qspStrEnd(QSP_STR(args[1]));
	if (onig_new(&onigExp, tempBeg, tempEnd, ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL, &onigInfo))
		qspSetError(QSP_ERR_INCORRECTREGEXP);
	else
	{
		onigReg = onig_region_new();
		tempBeg = (OnigUChar *)QSP_STR(args[0]);
		tempEnd = (OnigUChar *)qspStrEnd(QSP_STR(args[0]));
		pos = ((count == 3 && QSP_NUM(args[2]) >= 0) ? QSP_NUM(args[2]) : 0);
		if (onig_search(onigExp, tempBeg, tempEnd, tempBeg, tempEnd, onigReg, ONIG_OPTION_NONE) >= 0 &&
			pos < onigReg->num_regs && onigReg->beg[pos] >= 0)
		{
			len = (onigReg->end[pos] - onigReg->beg[pos]) / sizeof(QSP_CHAR);
			QSP_PSTR(tos) = qspGetNewText((QSP_CHAR *)(tempBeg + onigReg->beg[pos]), len);
		}
		else
			QSP_PSTR(tos) = qspGetNewText(QSP_FMT(""), 0);
		onig_region_free(onigReg, 1);
		onig_free(onigExp);
	}
}

static void qspFunctionStrPos(QSPVariant *args, long count, QSPVariant *tos)
{
	long pos;
	OnigUChar *tempBeg, *tempEnd;
	regex_t *onigExp;
	OnigRegion *onigReg;
	OnigErrorInfo onigInfo;
	tempBeg = (OnigUChar *)QSP_STR(args[1]);
	tempEnd = (OnigUChar *)qspStrEnd(QSP_STR(args[1]));
	if (onig_new(&onigExp, tempBeg, tempEnd, ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL, &onigInfo))
		qspSetError(QSP_ERR_INCORRECTREGEXP);
	else
	{
		onigReg = onig_region_new();
		tempBeg = (OnigUChar *)QSP_STR(args[0]);
		tempEnd = (OnigUChar *)qspStrEnd(QSP_STR(args[0]));
		pos = ((count == 3 && QSP_NUM(args[2]) >= 0) ? QSP_NUM(args[2]) : 0);
		if (onig_search(onigExp, tempBeg, tempEnd, tempBeg, tempEnd, onigReg, ONIG_OPTION_NONE) >= 0 &&
			pos < onigReg->num_regs && onigReg->beg[pos] >= 0)
			QSP_PNUM(tos) = onigReg->beg[pos] / sizeof(QSP_CHAR) + 1;
		else
			QSP_PNUM(tos) = 0;
		onig_region_free(onigReg, 1);
		onig_free(onigExp);
	}
}

static void qspFunctionRGB(QSPVariant *args, long count, QSPVariant *tos)
{
	long r, g, b;
	r = QSP_NUM(args[0]);
	g = QSP_NUM(args[1]);
	b = QSP_NUM(args[2]);
	if (r < 0)
		r = 0;
	else if (r > 255)
		r = 255;
	if (g < 0)
		g = 0;
	else if (g > 255)
		g = 255;
	if (b < 0)
		b = 0;
	else if (b > 255)
		b = 255;
	QSP_PNUM(tos) = (b << 16) | (g << 8) | r;
}

static void qspFunctionMid(QSPVariant *args, long count, QSPVariant *tos)
{
	long len, subLen, beg = QSP_NUM(args[1]) - 1;
	if (beg < 0) beg = 0;
	len = (long)QSP_STRLEN(QSP_STR(args[0]));
	if (beg < len)
	{
		len -= beg;
		if (count == 3)
		{
			subLen = QSP_NUM(args[2]);
			if (subLen < 0)
				len = 0;
			else if (subLen < len)
				len = subLen;
		}
		QSP_PSTR(tos) = qspGetNewText(QSP_STR(args[0]) + beg, len);
	}
	else
		QSP_PSTR(tos) = qspGetNewText(QSP_FMT(""), 0);
}

static void qspFunctionRand(QSPVariant *args, long count, QSPVariant *tos)
{
	long min, max;
	min = QSP_NUM(args[0]);
	max = (count == 2 ? QSP_NUM(args[1]) : 0);
	if (min > max)
	{
		min = max;
		max = QSP_NUM(args[0]);
	}
	QSP_PNUM(tos) = rand() % (max - min + 1) + min;
}

static void qspFunctionDesc(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *desc;
	long oldRefreshCount, index = qspLocIndex(QSP_STR(args[0]));
	if (index < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return;
	}
	oldRefreshCount = qspRefreshCount;
	desc = qspFormatText(qspLocs[index].Desc);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return;
	QSP_PSTR(tos) = desc;
}

static void qspFunctionGetObj(QSPVariant *args, long count, QSPVariant *tos)
{
	long ind = QSP_NUM(args[0]) - 1;
	if (ind < 0) ind = 0;
	if (ind < qspCurObjectsCount)
		QSP_PSTR(tos) = qspGetNewText(qspCurObjects[ind].Desc, -1);
	else
		QSP_PSTR(tos) = qspGetNewText(QSP_FMT(""), 0);
}

static void qspFunctionIsPlay(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *file;
	if (qspIsAnyString(QSP_STR(args[0])))
	{
		file = qspGetAbsFromRelPath(QSP_STR(args[0]));
		QSP_PNUM(tos) = -(qspCallIsPlayingFile(file) != 0);
		free(file);
	}
	else
		QSP_PNUM(tos) = 0;
}

static void qspFunctionInstr(QSPVariant *args, long count, QSPVariant *tos)
{
	long beg;
	QSP_CHAR *txt, *str;
	if (qspConvertVariantTo(args, count == 2))
	{
		qspSetError(QSP_ERR_TYPEMISMATCH);
		return;
	}
	if (count == 2)
	{
		txt = QSP_STR(args[0]);
		str = QSP_STR(args[1]);
		beg = 0;
	}
	else
	{
		txt = QSP_STR(args[1]);
		str = QSP_STR(args[2]);
		beg = QSP_NUM(args[0]) - 1;
		if (beg < 0) beg = 0;
	}
	if (beg < (long)QSP_STRLEN(txt))
	{
		str = QSP_STRSTR(txt + beg, str);
		QSP_PNUM(tos) = (str ? (long)(str - txt) + 1 : 0);
	}
	else
		QSP_PNUM(tos) = 0;
}

static void qspFunctionReplace(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *searchTxt = QSP_STR(args[1]);
	if (!(*searchTxt))
		QSP_PSTR(tos) = qspGetNewText(QSP_STR(args[0]), -1);
	else if (count == 2)
		QSP_PSTR(tos) = qspReplaceText(QSP_STR(args[0]), searchTxt, QSP_FMT(""));
	else
		QSP_PSTR(tos) = qspReplaceText(QSP_STR(args[0]), searchTxt, QSP_STR(args[2]));
}

static void qspFunctionFunc(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *text;
	long oldRefreshCount;
	QSPVar result, *varRes;
	if (!(varRes = qspVarReference(QSP_FMT("RESULT"), QSP_TRUE))) return;
	qspMoveVar(&result, varRes);
	oldRefreshCount = qspRefreshCount;
	qspExecLocByNameWithArgs(QSP_STR(args[0]), args + 1, count - 1);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
	{
		qspEmptyVar(&result);
		return;
	}
	if (!(varRes = qspVarReference(QSP_FMT("RESULT"), QSP_TRUE)))
	{
		qspEmptyVar(&result);
		return;
	}
	if (varRes->ValsCount)
	{
		if (text = varRes->TextValue[0])
		{
			tos->IsStr = QSP_TRUE;
			QSP_PSTR(tos) = qspGetNewText(text, -1);
		}
		else
		{
			tos->IsStr = QSP_FALSE;
			QSP_PNUM(tos) = varRes->Value[0];
		}
	}
	else
	{
		tos->IsStr = QSP_TRUE;
		QSP_PSTR(tos) = qspGetNewText(QSP_FMT(""), 0);
	}
	qspEmptyVar(varRes);
	qspMoveVar(varRes, &result);
}

static void qspFunctionDynEval(QSPVariant *args, long count, QSPVariant *tos)
{
	QSPVar local, *var;
	long oldRefreshCount;
	if (!(var = qspVarReference(QSP_FMT("ARGS"), QSP_TRUE))) return;
	qspMoveVar(&local, var);
	qspSetArgs(var, args + 1, count - 1);
	oldRefreshCount = qspRefreshCount;
	*tos = qspExprValue(QSP_STR(args[0]));
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
	{
		qspEmptyVar(&local);
		return;
	}
	if (!(var = qspVarReference(QSP_FMT("ARGS"), QSP_TRUE)))
	{
		qspEmptyVar(&local);
		return;
	}
	qspEmptyVar(var);
	qspMoveVar(var, &local);
}

static void qspFunctionMin(QSPVariant *args, long count, QSPVariant *tos)
{
	long i, minInd;
	if (count == 1)
	{
		qspConvertVariantTo(args, QSP_TRUE);
		*tos = qspArrayMinMaxItem(QSP_STR(args[0]), QSP_TRUE);
	}
	else
	{
		minInd = 0;
		for (i = 1; i < count; ++i)
		{
			if (qspAutoConvertCompare(args + i, args + minInd) < 0)
				minInd = i;
		}
		qspCopyVariant(tos, args + minInd);
	}
}

static void qspFunctionMax(QSPVariant *args, long count, QSPVariant *tos)
{
	long i, maxInd;
	if (count == 1)
	{
		qspConvertVariantTo(args, QSP_TRUE);
		*tos = qspArrayMinMaxItem(QSP_STR(args[0]), QSP_FALSE);
	}
	else
	{
		maxInd = 0;
		for (i = 1; i < count; ++i)
		{
			if (qspAutoConvertCompare(args + i, args + maxInd) > 0)
				maxInd = i;
		}
		qspCopyVariant(tos, args + maxInd);
	}
}
