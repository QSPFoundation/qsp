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

#include "math.h"
#include "callbacks.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "objects.h"
#include "text.h"
#include "variables.h"

QSPMathOperation qspOps[qspOpLast_Operation];
long qspOpMaxLen = 0;

static void qspAddOperation(long, QSP_CHAR *, QSP_CHAR *, char, QSP_FUNCTION, char, long, long, ...);
static long qspGetNumber(QSP_CHAR **);
static QSP_CHAR *qspGetName(QSP_CHAR **);
static long qspFunctionOpCode(QSP_CHAR *);
static long qspOperatorOpCode(QSP_CHAR **);
static QSP_CHAR *qspGetString(QSP_CHAR **);
static QSPVariant qspValue(long, QSPVariant *, long *, long *);
static void qspCompileExprPushOpCode(long *, long *, long *, long *, long);
static void qspAppendToCompiled(long, long *, QSPVariant *, long *, long *, long, QSPVariant);
static void qspCompileExpression(QSP_CHAR *, long *, QSPVariant *, long *, long *);
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
static void qspFunctionFunc(QSPVariant *, long, QSPVariant *);

static void qspAddOperation(long opCode,
							QSP_CHAR *opName,
							QSP_CHAR *opAltName,
							char priority,
							QSP_FUNCTION func,
							char resType,
							long minArgs,
							long maxArgs,
							...)
{
	long i;
	va_list marker;
	qspOps[opCode].Names[0] = opName;
	qspOps[opCode].Names[1] = opAltName;
	qspOps[opCode].NamesLens[0] = (opName ? (long)QSP_STRLEN(opName) : 0);
	qspOps[opCode].NamesLens[1] = (opAltName ? (long)QSP_STRLEN(opAltName) : 0);
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
	/* Max length */
	for (i = 0; i < 2; ++i)
		if (qspOps[opCode].NamesLens[i] > qspOpMaxLen)
			qspOpMaxLen = qspOps[opCode].NamesLens[i];
}

void qspInitMath()
{
	/*
	Format:
		qspAddOperation(
			Operation,
			Name,
			Alternative Name,
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
	qspOpMaxLen = 0;
	qspAddOperation(qspOpValue, 0, 0, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpStart, 0, 0, 127, 0, 0, 0, 0);
	qspAddOperation(qspOpEnd, 0, 0, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpOpenBracket, QSP_LRBRACK, 0, 127, 0, 0, 0, 0);
	qspAddOperation(qspOpCloseBracket, QSP_RRBRACK, 0, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpMinus, QSP_UMINUS, 0, 18, 0, 2, 1, 1, 2);
	qspAddOperation(qspOpAdd, QSP_ADD, 0, 14, 0, 0, 2, 2, 0, 0);
	qspAddOperation(qspOpSub, QSP_SUB, 0, 14, 0, 2, 2, 2, 2, 2);
	qspAddOperation(qspOpMul, QSP_MUL, 0, 17, 0, 2, 2, 2, 2, 2);
	qspAddOperation(qspOpDiv, QSP_DIV, 0, 17, 0, 2, 2, 2, 2, 2);
	qspAddOperation(qspOpNe, QSP_NOTEQUAL1, QSP_NOTEQUAL2, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpLeq, QSP_LESSEQ1, QSP_LESSEQ2, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpGeq, QSP_GREATEQ1, QSP_GREATEQ2, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpEq, QSP_EQUAL, 0, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpLt, QSP_LESS, 0, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpGt, QSP_GREAT, 0, 10, 0, 2, 2, 2, 0, 0);
	qspAddOperation(qspOpAppend, QSP_APPEND, 0, 4, 0, 1, 2, 2, 1, 1);
	qspAddOperation(qspOpComma, QSP_COMMA, 0, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpObj, QSP_FMT("OBJ"), 0, 8, 0, 2, 1, 1, 1);
	qspAddOperation(qspOpNot, QSP_FMT("NO"), 0, 8, 0, 2, 1, 1, 2);
	qspAddOperation(qspOpAnd, QSP_FMT("AND"), 0, 7, 0, 2, 2, 2, 2, 2);
	qspAddOperation(qspOpOr, QSP_FMT("OR"), 0, 6, 0, 2, 2, 2, 2, 2);
	qspAddOperation(qspOpMin, QSP_FMT("MIN"), QSP_STRCHAR QSP_FMT("MIN"), 30, 0, 0, 2, 2, 0, 0);
	qspAddOperation(qspOpMax, QSP_FMT("MAX"), QSP_STRCHAR QSP_FMT("MAX"), 30, 0, 0, 2, 2, 0, 0);
	qspAddOperation(qspOpRand, QSP_FMT("RAND"), 0, 30, qspFunctionRand, 2, 1, 2, 2, 2);
	qspAddOperation(qspOpIIf, QSP_FMT("IIF"), QSP_STRCHAR QSP_FMT("IIF"), 30, 0, 0, 3, 3, 2, 0, 0);
	qspAddOperation(qspOpRGB, QSP_FMT("RGB"), 0, 30, qspFunctionRGB, 2, 3, 3, 2, 2, 2);
	qspAddOperation(qspOpLen, QSP_FMT("LEN"), 0, 30, 0, 2, 1, 1, 1);
	qspAddOperation(qspOpIsNum, QSP_FMT("ISNUM"), 0, 30, 0, 2, 1, 1, 0);
	qspAddOperation(qspOpLCase, QSP_FMT("LCASE"), QSP_STRCHAR QSP_FMT("LCASE"), 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpUCase, QSP_FMT("UCASE"), QSP_STRCHAR QSP_FMT("UCASE"), 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpInput, QSP_FMT("INPUT"), QSP_STRCHAR QSP_FMT("INPUT"), 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpStr, QSP_FMT("STR"), QSP_STRCHAR QSP_FMT("STR"), 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpVal, QSP_FMT("VAL"), 0, 30, 0, 2, 1, 1, 0);
	qspAddOperation(qspOpArrSize, QSP_FMT("ARRSIZE"), 0, 30, 0, 2, 1, 1, 1);
	qspAddOperation(qspOpIsPlay, QSP_FMT("ISPLAY"), 0, 30, qspFunctionIsPlay, 2, 1, 1, 1);
	qspAddOperation(qspOpDesc, QSP_FMT("DESC"), QSP_STRCHAR QSP_FMT("DESC"), 30, qspFunctionDesc, 1, 1, 1, 1);
	qspAddOperation(qspOpTrim, QSP_FMT("TRIM"), QSP_STRCHAR QSP_FMT("TRIM"), 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpGetObj, QSP_FMT("GETOBJ"), QSP_STRCHAR QSP_FMT("GETOBJ"), 30, qspFunctionGetObj, 1, 1, 1, 2);
	qspAddOperation(qspOpStrComp, QSP_FMT("STRCOMP"), 0, 30, qspFunctionStrComp, 2, 2, 2, 1, 1);
	qspAddOperation(qspOpStrFind, QSP_FMT("STRFIND"), QSP_STRCHAR QSP_FMT("STRFIND"), 30, qspFunctionStrFind, 1, 2, 3, 1, 1, 2);
	qspAddOperation(qspOpStrPos, QSP_FMT("STRPOS"), 0, 30, qspFunctionStrPos, 2, 2, 3, 1, 1, 2);
	qspAddOperation(qspOpMid, QSP_FMT("MID"), QSP_STRCHAR QSP_FMT("MID"), 30, qspFunctionMid, 1, 2, 3, 1, 2, 2);
	qspAddOperation(qspOpArrPos, QSP_FMT("ARRPOS"), 0, 30, 0, 2, 3, 3, 2, 1, 0);
	qspAddOperation(qspOpArrComp, QSP_FMT("ARRCOMP"), 0, 30, 0, 2, 3, 3, 2, 1, 0);
	qspAddOperation(qspOpInstr, QSP_FMT("INSTR"), 0, 30, qspFunctionInstr, 2, 3, 3, 2, 1, 1);
	qspAddOperation(qspOpFunc, QSP_FMT("FUNC"), QSP_STRCHAR QSP_FMT("FUNC"), 30, qspFunctionFunc, 0, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddOperation(qspOpDynEval, QSP_FMT("DYNEVAL"), QSP_STRCHAR QSP_FMT("DYNEVAL"), 30, 0, 0, 1, 1, 1);
}

static long qspGetNumber(QSP_CHAR **expr)
{
	long i = 0;
	QSP_CHAR buf[11], *pos = *expr;
	while (QSP_ISDIGIT(*pos))
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
	QSP_CHAR *rPos, *startPos = *expr, *pos = startPos;
	do
	{
		if (*(++pos) == QSP_LSBRACK[0])
		{
			rPos = qspStrPos(pos, QSP_RSBRACK, QSP_FALSE);
			if (!rPos)
			{
				qspSetError(QSP_ERR_BRACKNOTFOUND);
				return 0;
			}
			pos = rPos + 1;
			break;
		}
	} while (!qspIsInListEOL(QSP_DELIMS, *pos));
	*expr = qspSkipSpaces(pos);
	return qspGetNewText(startPos, (long)(pos - startPos));
}

static long qspFunctionOpCode(QSP_CHAR *funName)
{
	long i, j;
	QSP_CHAR *uName;
	qspUpperStr(uName = qspGetNewText(funName, -1));
	for (i = qspOpFirst_Function; i < qspOpLast_Operation; ++i)
		for (j = 0; j < 2; ++j)
			if (qspOps[i].Names[j] && !QSP_STRCMP(uName, qspOps[i].Names[j]))
			{
				free(uName);
				return i;
			}
	for (i = qspOpFirst_UnaryKeyword; i < qspOpFirst_NotUnaryOperator; ++i)
		for (j = 0; j < 2; ++j)
			if (qspOps[i].Names[j] && !QSP_STRCMP(uName, qspOps[i].Names[j]))
			{
				free(uName);
				return i;
			}
	free(uName);
	return qspOpUnknown;
}

static long qspOperatorOpCode(QSP_CHAR **expr)
{
	long i, j, len;
	QSP_CHAR *uExpr;
	if (!(**expr)) return qspOpEnd;
	qspUpperStr(uExpr = qspGetNewText(*expr, qspOpMaxLen));
	for (i = qspOpFirst_NotUnaryOperator; i < qspOpFirst_Function; ++i)
		for (j = 0; j < 2; ++j)
			if (qspOps[i].Names[j])
			{
				len = qspOps[i].NamesLens[j];
				if (qspIsEqual(uExpr, qspOps[i].Names[j], len))
				{
					*expr += len;
					free(uExpr);
					return i;
				}
			}
	free(uExpr);
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
	QSPVariant stack[QSP_STACKSIZE], args[QSP_OPMAXARGS], tos;
	char type;
	long i, j, index, opCode, argsCount, len, sp = -1;
	QSP_BOOL convErr = QSP_FALSE;
	tos.IsStr = QSP_FALSE;
	QSP_NUM(tos) = 0;
	for (index = 0; index < itemsCount; ++index)
	{
		opCode = compOpCodes[index];
		argsCount = compArgsCounts[index];
		if (argsCount)
		{
			for (i = argsCount - 2, j = sp, args[i + 1] = tos; i >= 0; --i, --j)
				args[i] = stack[j];
			for (i = 0; i < argsCount; ++i)
			{
				type = qspOps[opCode].ArgsTypes[i];
				if (type) args[i] = qspConvertVariantTo(args[i], type == 1, QSP_TRUE, &convErr);
			}
			if (convErr)
				qspSetError(QSP_ERR_TYPEMISMATCH);
			else
			{
				type = qspOps[opCode].ResType;
				if (type) tos.IsStr = type == 1;
			}
		}
		if (!qspErrorNum)
		{
			switch (opCode)
			{
			case qspOpValue:
				if (sp == QSP_STACKSIZE - 1)
				{
					qspSetError(QSP_ERR_STACKOVERFLOW);
					break;
				}
				stack[++sp] = tos;
				qspCopyVariant(&tos, compValues[index]);
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
				else if (qspIsCanConvertToNum(args[0]) && qspIsCanConvertToNum(args[1]))
				{
					args[0] = qspConvertVariantTo(args[0], QSP_FALSE, QSP_TRUE, 0);
					args[1] = qspConvertVariantTo(args[1], QSP_FALSE, QSP_TRUE, 0);
					tos.IsStr = QSP_FALSE;
					QSP_NUM(tos) = QSP_NUM(args[0]) + QSP_NUM(args[1]);
				}
				else
				{
					args[0] = qspConvertVariantTo(args[0], QSP_TRUE, QSP_TRUE, 0);
					args[1] = qspConvertVariantTo(args[1], QSP_TRUE, QSP_TRUE, 0);
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
				QSP_NUM(tos) = -(!qspAutoConvertCompare(args[0], args[1]));
				break;
			case qspOpLt:
				QSP_NUM(tos) = -(qspAutoConvertCompare(args[0], args[1]) < 0);
				break;
			case qspOpGt:
				QSP_NUM(tos) = -(qspAutoConvertCompare(args[0], args[1]) > 0);
				break;
			case qspOpLeq:
				QSP_NUM(tos) = -(qspAutoConvertCompare(args[0], args[1]) <= 0);
				break;
			case qspOpGeq:
				QSP_NUM(tos) = -(qspAutoConvertCompare(args[0], args[1]) >= 0);
				break;
			case qspOpNe:
				QSP_NUM(tos) = -(qspAutoConvertCompare(args[0], args[1]) != 0);
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
			case qspOpMin:
				qspCopyVariant(&tos, qspAutoConvertCompare(args[0], args[1]) < 0 ? args[0] : args[1]);
				break;
			case qspOpMax:
				qspCopyVariant(&tos, qspAutoConvertCompare(args[0], args[1]) > 0 ? args[0] : args[1]);
				break;
			case qspOpIIf:
				qspCopyVariant(&tos, QSP_NUM(args[0]) ? args[1] : args[2]);
				break;
			case qspOpLen:
				QSP_NUM(tos) = (long)QSP_STRLEN(QSP_STR(args[0]));
				break;
			case qspOpIsNum:
				QSP_NUM(tos) = -qspIsCanConvertToNum(args[0]);
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
				args[0] = qspConvertVariantTo(args[0], QSP_FALSE, QSP_TRUE, &convErr);
				convErr = QSP_FALSE;
				QSP_NUM(tos) = QSP_NUM(args[0]);
				break;
			case qspOpArrSize:
				QSP_NUM(tos) = qspArraySize(QSP_STR(args[0]));
				break;
			case qspOpTrim:
				QSP_STR(tos) = qspDelSpc(QSP_STR(args[0]));
				break;
			case qspOpArrPos:
				QSP_NUM(tos) = qspArrayPos(QSP_STR(args[1]), QSP_NUM(args[0]), args[2], QSP_FALSE);
				break;
			case qspOpArrComp:
				QSP_NUM(tos) = qspArrayPos(QSP_STR(args[1]), QSP_NUM(args[0]), args[2], QSP_TRUE);
				break;
			case qspOpInput:
				QSP_STR(tos) = qspCallInputBox(QSP_STR(args[0]));
				break;
			case qspOpDynEval:
				tos = qspExprValue(QSP_STR(args[0]));
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
		if (qspErrorNum) break;
	}
	if (qspErrorNum)
	{
		qspFreeVariants(stack, sp + 1);
		tos.IsStr = QSP_FALSE;
		QSP_NUM(tos) = 0;
	}
	return tos;
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

static void qspCompileExpression(QSP_CHAR *s, long *itemsCount, QSPVariant *compValues, long *compOpCodes, long *compArgsCounts)
{
	QSPVariant v;
	QSP_CHAR *name;
	QSP_BOOL waitForOperator = QSP_FALSE;
	long opStack[QSP_STACKSIZE], argStack[QSP_STACKSIZE], opCode, opSp = -1, argSp = -1;
	opStack[0] = 0; argStack[0] = 0;
	qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, qspOpStart);
	if (qspErrorNum) return;
	while (1)
	{
		s = qspSkipSpaces(s);
		if (waitForOperator)
		{
			opCode = qspOperatorOpCode(&s);
			if (opCode == qspOpUnknown)
			{
				qspSetError(QSP_ERR_UNKNOWNACTION);
				return;
			}
			if ((opCode == qspOpAnd || opCode == qspOpOr) && !qspIsInList(QSP_SPACES QSP_QUOTS QSP_LRBRACK, *s))
			{
				qspSetError(QSP_ERR_SYNTAX);
				return;
			}
			while (qspOps[opCode].Priority <= qspOps[opStack[opSp]].Priority && qspOps[opStack[opSp]].Priority != 127)
			{
				qspAppendToCompiled(opStack[opSp], itemsCount, compValues, compOpCodes, compArgsCounts, argStack[argSp], v);
				if (qspErrorNum) return;
				if (--opSp < 0 || --argSp < 0)
				{
					qspSetError(QSP_ERR_SYNTAX);
					return;
				}
			}
			switch (opCode)
			{
			case qspOpEnd:
				if (opSp) qspSetError(QSP_ERR_BRACKNOTFOUND);
				return;
			case qspOpCloseBracket:
				if (opStack[opSp] != qspOpOpenBracket)
				{
					qspSetError(QSP_ERR_BRACKNOTFOUND);
					return;
				}
				if (opStack[--opSp] >= qspOpFirst_Function)
				{
					++argStack[argSp];
					if (argStack[argSp] < qspOps[opStack[opSp]].MinArgsCount || argStack[argSp] > qspOps[opStack[opSp]].MaxArgsCount)
					{
						qspSetError(QSP_ERR_ARGSCOUNT);
						return;
					}
				}
				else
					--argSp;
				break;
			case qspOpComma:
				if (!opSp || opStack[opSp - 1] < qspOpFirst_Function)
				{
					qspSetError(QSP_ERR_SYNTAX);
					return;
				}
				if (++argStack[argSp] > qspOps[opStack[opSp - 1]].MaxArgsCount)
				{
					qspSetError(QSP_ERR_ARGSCOUNT);
					return;
				}
				waitForOperator = QSP_FALSE;
				break;
			default:
				qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, opCode);
				if (qspErrorNum) return;
				waitForOperator = QSP_FALSE;
				break;
			}
		}
		else
		{
			if (QSP_ISDIGIT(*s))
			{
				v.IsStr = QSP_FALSE;
				QSP_NUM(v) = qspGetNumber(&s);
				if (opStack[opSp] == qspOpMinus)
				{
					QSP_NUM(v) = -QSP_NUM(v);
					--opSp;
					--argSp;
				}
				qspAppendToCompiled(qspOpValue, itemsCount, compValues, compOpCodes, compArgsCounts, 0, v);
				if (qspErrorNum) return;
				waitForOperator = QSP_TRUE;
			}
			else if (qspIsInList(QSP_QUOTS, *s))
			{
				v.IsStr = QSP_TRUE;
				QSP_STR(v) = qspGetString(&s);
				if (qspErrorNum) return;
				qspAppendToCompiled(qspOpValue, itemsCount, compValues, compOpCodes, compArgsCounts, 0, v);
				if (qspErrorNum)
				{
					free(QSP_STR(v));
					return;
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
				if (qspErrorNum) return;
				++s;
			}
			else if (*s == QSP_LRBRACK[0])
			{
				qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, qspOpOpenBracket);
				if (qspErrorNum) return;
				++s;
			}
			else if (!qspIsInListEOL(QSP_DELIMS, *s))
			{
				name = qspGetName(&s);
				if (qspErrorNum) return;
				opCode = qspFunctionOpCode(name);
				if (opCode == qspOpNot || opCode == qspOpObj)
				{
					qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, opCode);
					if (qspErrorNum)
					{
						free(name);
						return;
					}
				}
				else if (opCode != qspOpUnknown)
				{
					if (*s == QSP_LRBRACK[0])
					{
						qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, opCode);
						if (qspErrorNum)
						{
							free(name);
							return;
						}
						qspCompileExprPushOpCode(opStack, &opSp, argStack, &argSp, qspOpOpenBracket);
						if (qspErrorNum)
						{
							free(name);
							return;
						}
						++s;
						--argSp;
					}
					else
					{
						qspSetError(QSP_ERR_BRACKSNOTFOUND);
						free(name);
						return;
					}
				}
				else
				{
					v = qspGetVar(name);
					if (qspErrorNum)
					{
						free(name);
						return;
					}
					qspAppendToCompiled(qspOpValue, itemsCount, compValues, compOpCodes, compArgsCounts, 0, v);
					if (qspErrorNum)
					{
						if (v.IsStr) free(QSP_STR(v));
						free(name);
						return;
					}
					waitForOperator = QSP_TRUE;
				}
				free(name);
			}
			else
			{
				qspSetError(QSP_ERR_SYNTAX);
				return;
			}
		}
	}
}

QSPVariant qspExprValue(QSP_CHAR *expr)
{
	QSPVariant res, compValues[QSP_MAXITEMS];
	long compOpCodes[QSP_MAXITEMS], compArgsCounts[QSP_MAXITEMS], itemsCount = 0;
	qspCompileExpression(expr, &itemsCount, compValues, compOpCodes, compArgsCounts);
	if (qspErrorNum)
	{
		res.IsStr = QSP_FALSE;
		QSP_NUM(res) = 0;
	}
	else
		res = qspValue(itemsCount, compValues, compOpCodes, compArgsCounts);
	for (--itemsCount; itemsCount >= 0; --itemsCount)
		if (compOpCodes[itemsCount] == qspOpValue && compValues[itemsCount].IsStr)
			free(QSP_STR(compValues[itemsCount]));
	return res;
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
		QSP_NUM(*tos) = -(onig_match(onigExp, tempBeg, tempEnd, tempBeg, onigReg, ONIG_OPTION_NONE) >= 0);
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
			QSP_STR(*tos) = qspGetNewText((QSP_CHAR *)(tempBeg + onigReg->beg[pos]), len);
		}
		else
			QSP_STR(*tos) = qspGetNewText(QSP_FMT(""), 0);
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
			QSP_NUM(*tos) = onigReg->beg[pos] / sizeof(QSP_CHAR) + 1;
		else
			QSP_NUM(*tos) = 0;
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
	QSP_NUM(*tos) = (b << 16) | (g << 8) | r;
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
		QSP_STR(*tos) = qspGetNewText(QSP_STR(args[0]) + beg, len);
	}
	else
		QSP_STR(*tos) = qspGetNewText(QSP_FMT(""), 0);
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
	QSP_NUM(*tos) = rand() % (max - min + 1) + min;
}

static void qspFunctionDesc(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *desc;
	long index = qspLocIndex(QSP_STR(args[0]));
	if (index < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return;
	}
	desc = qspFormatText(qspLocs[index].Desc);
	if (qspErrorNum) return;
	QSP_STR(*tos) = desc;
}

static void qspFunctionGetObj(QSPVariant *args, long count, QSPVariant *tos)
{
	long ind = QSP_NUM(args[0]) - 1;
	if (ind < 0) ind = 0;
	if (ind < qspCurObjectsCount)
		QSP_STR(*tos) = qspGetNewText(qspCurObjects[ind].Desc, -1);
	else
		QSP_STR(*tos) = qspGetNewText(QSP_FMT(""), 0);
}

static void qspFunctionIsPlay(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *file;
	if (qspIsAnyString(QSP_STR(args[0])))
	{
		file = qspGetNewText(qspQstPath, qspQstPathLen);
		file = qspGetAddText(file, QSP_STR(args[0]), qspQstPathLen, -1);
		QSP_NUM(*tos) = -(qspCallIsPlayingFile(file) != 0);
		free(file);
	}
	else
		QSP_NUM(*tos) = 0;
}

static void qspFunctionInstr(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *pos;
	long beg = QSP_NUM(args[0]) - 1;
	if (beg < 0) beg = 0;
	if (beg < (long)QSP_STRLEN(QSP_STR(args[1])))
	{
		pos = QSP_STRSTR(QSP_STR(args[1]) + beg, QSP_STR(args[2]));
		QSP_NUM(*tos) = (pos ? (long)(pos - QSP_STR(args[1])) + 1 : 0);
	}
	else
		QSP_NUM(*tos) = 0;
}

static void qspFunctionFunc(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *text;
	QSPVar local, result, *varRes, *varArgs;
	varArgs = qspVarReference(QSP_FMT("ARGS"), QSP_TRUE);
	if (!varArgs) return;
	varRes = qspVarReference(QSP_FMT("RESULT"), QSP_FALSE);
	if (!varRes) return;
	qspCopyVar(&local, varArgs);
	qspSetArgs(varArgs, args + 1, count - 1);
	qspCopyVar(&result, varRes);
	qspEmptyVar(varRes);
	qspExecLocByName(QSP_STR(args[0]), QSP_FALSE);
	qspEmptyVar(varArgs);
	qspCopyVar(varArgs, &local);
	qspEmptyVar(&local);
	if (qspErrorNum)
	{
		qspEmptyVar(varRes);
		qspCopyVar(varRes, &result);
		qspEmptyVar(&result);
		return;
	}
	if (varRes->ValsCount)
	{
		if (text = varRes->TextValue[0])
		{
			tos->IsStr = QSP_TRUE;
			QSP_STR(*tos) = qspGetNewText(text, -1);
		}
		else
		{
			tos->IsStr = QSP_FALSE;
			QSP_NUM(*tos) = varRes->Value[0];
		}
	}
	else
	{
		tos->IsStr = QSP_TRUE;
		QSP_STR(*tos) = qspGetNewText(QSP_FMT(""), 0);
	}
	qspEmptyVar(varRes);
	qspCopyVar(varRes, &result);
	qspEmptyVar(&result);
}
