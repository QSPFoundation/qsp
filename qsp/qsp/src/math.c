/* Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru) */
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

void qspFunctionStrComp(QSPVariant *, long, QSPVariant *);
void qspFunctionStrFind(QSPVariant *, long, QSPVariant *);
void qspFunctionStrPos(QSPVariant *, long, QSPVariant *);
void qspFunctionRGB(QSPVariant *, long, QSPVariant *);
void qspFunctionMid(QSPVariant *, long, QSPVariant *);
void qspFunctionRand(QSPVariant *, long, QSPVariant *);
void qspFunctionInput(QSPVariant *, long, QSPVariant *);
void qspFunctionDesc(QSPVariant *, long, QSPVariant *);
void qspFunctionGetObj(QSPVariant *, long, QSPVariant *);
void qspFunctionIsPlay(QSPVariant *, long, QSPVariant *);
void qspFunctionInstr(QSPVariant *, long, QSPVariant *);

void qspAddOperation(long opCode,
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
	qspAddOperation(qspOpInput, QSP_FMT("INPUT"), QSP_STRCHAR QSP_FMT("INPUT"), 30, qspFunctionInput, 1, 1, 1, 1);
	qspAddOperation(qspOpStr, QSP_FMT("STR"), QSP_STRCHAR QSP_FMT("STR"), 30, 0, 1, 1, 1, 1);
	qspAddOperation(qspOpVal, QSP_FMT("VAL"), 0, 30, 0, 2, 1, 1, 0);
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
	qspAddOperation(qspOpDynEval, QSP_FMT("DYNEVAL"), QSP_STRCHAR QSP_FMT("DYNEVAL"), 30, 0, 0, 1, 1, 1);
}

long qspGetNumber(QSP_CHAR **expr)
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

QSP_CHAR *qspGetName(QSP_CHAR **expr)
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

long qspFunctionOpCode(QSP_CHAR *funName)
{
	long i, j;
	QSP_CHAR *uName;
	qspUpperStr(uName = qspGetNewText(funName, QSP_OPMAXLEN));
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

long qspOperatorOpCode(QSP_CHAR **expr)
{
	long i, j, len;
	QSP_CHAR *uExpr;
	if (!(**expr)) return qspOpEnd;
	qspUpperStr(uExpr = qspGetNewText(*expr, QSP_OPMAXLEN));
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

QSP_CHAR *qspGetString(QSP_CHAR **expr)
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

QSPVariant qspValue(long itemsCount, QSPVariant *compValues, long *compOpCodes, long *compArgsCounts)
{
	QSPVariant stack[QSP_STACKSIZE], args[QSP_OPMAXARGS], tos;
	char type;
	long i, j, index, opCode, argsCount, len, sp = -1;
	QSP_BOOL convErr = QSP_FALSE;
	tos.IsStr = QSP_FALSE;
	tos.Num = 0;
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
				tos.Num = args[0].Num * args[1].Num;
				break;
			case qspOpDiv:
				if (!args[1].Num)
				{
					qspSetError(QSP_ERR_DIVBYZERO);
					break;
				}
				tos.Num = args[0].Num / args[1].Num;
				break;
			case qspOpAdd:
				if (args[0].IsStr && args[1].IsStr)
				{
					tos.IsStr = QSP_TRUE;
					len = qspAddText(&tos.Str, args[0].Str, 0, -1, QSP_TRUE);
					tos.Str = qspGetAddText(tos.Str, args[1].Str, len, -1);
				}
				else if (qspIsCanConvertToNum(args[0]) && qspIsCanConvertToNum(args[1]))
				{
					args[0] = qspConvertVariantTo(args[0], QSP_FALSE, QSP_TRUE, 0);
					args[1] = qspConvertVariantTo(args[1], QSP_FALSE, QSP_TRUE, 0);
					tos.IsStr = QSP_FALSE;
					tos.Num = args[0].Num + args[1].Num;
				}
				else
				{
					args[0] = qspConvertVariantTo(args[0], QSP_TRUE, QSP_TRUE, 0);
					args[1] = qspConvertVariantTo(args[1], QSP_TRUE, QSP_TRUE, 0);
					tos.IsStr = QSP_TRUE;
					len = qspAddText(&tos.Str, args[0].Str, 0, -1, QSP_TRUE);
					tos.Str = qspGetAddText(tos.Str, args[1].Str, len, -1);
				}
				break;
			case qspOpSub:
				tos.Num = args[0].Num - args[1].Num;
				break;
			case qspOpAppend:
				len = qspAddText(&tos.Str, args[0].Str, 0, -1, QSP_TRUE);
				tos.Str = qspGetAddText(tos.Str, args[1].Str, len, -1);
				break;
			case qspOpEq:
				tos.Num = -(!qspAutoConvertCompare(args[0], args[1]));
				break;
			case qspOpLt:
				tos.Num = -(qspAutoConvertCompare(args[0], args[1]) < 0);
				break;
			case qspOpGt:
				tos.Num = -(qspAutoConvertCompare(args[0], args[1]) > 0);
				break;
			case qspOpLeq:
				tos.Num = -(qspAutoConvertCompare(args[0], args[1]) <= 0);
				break;
			case qspOpGeq:
				tos.Num = -(qspAutoConvertCompare(args[0], args[1]) >= 0);
				break;
			case qspOpNe:
				tos.Num = -(qspAutoConvertCompare(args[0], args[1]) != 0);
				break;
			case qspOpMinus:
				tos.Num = -args[0].Num;
				break;
			case qspOpObj:
				tos.Num = -(qspObjIndex(args[0].Str) >= 0);
				break;
			case qspOpNot:
				tos.Num = ~args[0].Num;
				break;
			case qspOpAnd:
				tos.Num = args[0].Num & args[1].Num;
				break;
			case qspOpOr:
				tos.Num = args[0].Num | args[1].Num;
				break;
			/* Embedded functions -------------------------------------------------------------- */
			case qspOpMin:
				qspCopyVariant(&tos, qspAutoConvertCompare(args[0], args[1]) < 0 ? args[0] : args[1]);
				break;
			case qspOpMax:
				qspCopyVariant(&tos, qspAutoConvertCompare(args[0], args[1]) > 0 ? args[0] : args[1]);
				break;
			case qspOpIIf:
				qspCopyVariant(&tos, args[0].Num ? args[1] : args[2]);
				break;
			case qspOpLen:
				tos.Num = (long)QSP_STRLEN(args[0].Str);
				break;
			case qspOpIsNum:
				tos.Num = -qspIsCanConvertToNum(args[0]);
				break;
			case qspOpLCase:
				qspLowerStr(tos.Str = qspGetNewText(args[0].Str, -1));
				break;
			case qspOpUCase:
				qspUpperStr(tos.Str = qspGetNewText(args[0].Str, -1));
				break;
			case qspOpStr:
				tos.Str = qspGetNewText(args[0].Str, -1);
				break;
			case qspOpVal:
				args[0] = qspConvertVariantTo(args[0], QSP_FALSE, QSP_TRUE, &convErr);
				convErr = QSP_FALSE;
				tos.Num = args[0].Num;
				break;
			case qspOpTrim:
				tos.Str = qspDelSpc(args[0].Str);
				break;
			case qspOpArrPos:
				tos.Num = qspArrayPos(args[1].Str, args[0].Num, args[2], QSP_FALSE);
				break;
			case qspOpArrComp:
				tos.Num = qspArrayPos(args[1].Str, args[0].Num, args[2], QSP_TRUE);
				break;
			case qspOpDynEval:
				tos = qspExprValue(args[0].Str);
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
		tos.Num = 0;
	}
	return tos;
}

void qspCompileExprPushOpCode(long *opStack, long *opSp, long *argStack, long *argSp, long opCode)
{
	if (*opSp == QSP_STACKSIZE - 1 || *argSp == QSP_STACKSIZE - 1)
	{
		qspSetError(QSP_ERR_STACKOVERFLOW);
		return;
	}
	opStack[++(*opSp)] = opCode;
	argStack[++(*argSp)] = (opCode < qspOpFirst_Function ? qspOps[opCode].MinArgsCount : 0);
}

void qspAppendToCompiled(long opCode, long *itemsCount, QSPVariant *compValues, long *compOpCodes, long *compArgsCounts, long argsCount, QSPVariant v)
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

void qspCompileExpression(QSP_CHAR *s, long *itemsCount, QSPVariant *compValues, long *compOpCodes, long *compArgsCounts)
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
				v.Num = qspGetNumber(&s);
				if (opStack[opSp] == qspOpMinus)
				{
					v.Num = -v.Num;
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
				v.Str = qspGetString(&s);
				if (qspErrorNum) return;
				qspAppendToCompiled(qspOpValue, itemsCount, compValues, compOpCodes, compArgsCounts, 0, v);
				if (qspErrorNum)
				{
					free(v.Str);
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
						if (v.IsStr) free(v.Str);
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
		res.Num = 0;
	}
	else
		res = qspValue(itemsCount, compValues, compOpCodes, compArgsCounts);
	for (--itemsCount; itemsCount >= 0; --itemsCount)
		if (compOpCodes[itemsCount] == qspOpValue && compValues[itemsCount].IsStr)
			free(compValues[itemsCount].Str);
	return res;
}

void qspFunctionStrComp(QSPVariant *args, long count, QSPVariant *tos)
{
	OnigUChar *tempBeg, *tempEnd;
	regex_t *onigExp;
	OnigRegion *onigReg;
	OnigErrorInfo onigInfo;
	tempBeg = (OnigUChar *)args[1].Str;
	tempEnd = (OnigUChar *)qspStrEnd(args[1].Str);
	if (onig_new(&onigExp, tempBeg, tempEnd, ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL, &onigInfo))
		qspSetError(QSP_ERR_INCORRECTREGEXP);
	else
	{
		onigReg = onig_region_new();
		tempBeg = (OnigUChar *)args[0].Str;
		tempEnd = (OnigUChar *)qspStrEnd(args[0].Str);
		tos->Num = -(onig_match(onigExp, tempBeg, tempEnd, tempBeg, onigReg, ONIG_OPTION_NONE) >= 0);
		onig_region_free(onigReg, 1);
		onig_free(onigExp);
	}
}

void qspFunctionStrFind(QSPVariant *args, long count, QSPVariant *tos)
{
	long len, pos;
	OnigUChar *tempBeg, *tempEnd;
	regex_t *onigExp;
	OnigRegion *onigReg;
	OnigErrorInfo onigInfo;
	tempBeg = (OnigUChar *)args[1].Str;
	tempEnd = (OnigUChar *)qspStrEnd(args[1].Str);
	if (onig_new(&onigExp, tempBeg, tempEnd, ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL, &onigInfo))
		qspSetError(QSP_ERR_INCORRECTREGEXP);
	else
	{
		onigReg = onig_region_new();
		tempBeg = (OnigUChar *)args[0].Str;
		tempEnd = (OnigUChar *)qspStrEnd(args[0].Str);
		pos = ((count == 3 && args[2].Num >= 0) ? args[2].Num : 0);
		if (onig_search(onigExp, tempBeg, tempEnd, tempBeg, tempEnd, onigReg, ONIG_OPTION_NONE) >= 0 &&
			pos < onigReg->num_regs && onigReg->beg[pos] >= 0)
		{
			len = (onigReg->end[pos] - onigReg->beg[pos]) / sizeof(QSP_CHAR);
			tos->Str = qspGetNewText((QSP_CHAR *)(tempBeg + onigReg->beg[pos]), len);
		}
		else
			tos->Str = qspGetNewText(QSP_FMT(""), 0);
		onig_region_free(onigReg, 1);
		onig_free(onigExp);
	}
}

void qspFunctionStrPos(QSPVariant *args, long count, QSPVariant *tos)
{
	long pos;
	OnigUChar *tempBeg, *tempEnd;
	regex_t *onigExp;
	OnigRegion *onigReg;
	OnigErrorInfo onigInfo;
	tempBeg = (OnigUChar *)args[1].Str;
	tempEnd = (OnigUChar *)qspStrEnd(args[1].Str);
	if (onig_new(&onigExp, tempBeg, tempEnd, ONIG_OPTION_DEFAULT, QSP_ONIG_ENC, ONIG_SYNTAX_PERL, &onigInfo))
		qspSetError(QSP_ERR_INCORRECTREGEXP);
	else
	{
		onigReg = onig_region_new();
		tempBeg = (OnigUChar *)args[0].Str;
		tempEnd = (OnigUChar *)qspStrEnd(args[0].Str);
		pos = ((count == 3 && args[2].Num >= 0) ? args[2].Num : 0);
		if (onig_search(onigExp, tempBeg, tempEnd, tempBeg, tempEnd, onigReg, ONIG_OPTION_NONE) >= 0 &&
			pos < onigReg->num_regs && onigReg->beg[pos] >= 0)
			tos->Num = onigReg->beg[pos] / sizeof(QSP_CHAR) + 1;
		else
			tos->Num = 0;
		onig_region_free(onigReg, 1);
		onig_free(onigExp);
	}
}

void qspFunctionRGB(QSPVariant *args, long count, QSPVariant *tos)
{
	long r, g, b;
	r = args[0].Num;
	g = args[1].Num;
	b = args[2].Num;
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
	tos->Num = (b << 16) | (g << 8) | r;
}

void qspFunctionMid(QSPVariant *args, long count, QSPVariant *tos)
{
	long len, subLen, beg = args[1].Num - 1;
	if (beg < 0) beg = 0;
	len = (long)QSP_STRLEN(args[0].Str);
	if (beg < len)
	{
		len -= beg;
		if (count == 3)
		{
			subLen = args[2].Num;
			if (subLen < 0)
				len = 0;
			else if (subLen < len)
				len = subLen;
		}
		tos->Str = qspGetNewText(args[0].Str + beg, len);
	}
	else
		tos->Str = qspGetNewText(QSP_FMT(""), 0);
}

void qspFunctionRand(QSPVariant *args, long count, QSPVariant *tos)
{
	long min, max;
	min = args[0].Num;
	max = (count == 2 ? args[1].Num : 0);
	if (min > max)
	{
		min = max;
		max = args[0].Num;
	}
	tos->Num = rand() % (max - min + 1) + min;
}

void qspFunctionInput(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_BOOL prevIsMustWait = qspIsMustWait;
	qspIsMustWait = QSP_FALSE;
	tos->Str = qspCallInputBox(args[0].Str);
	qspIsMustWait = prevIsMustWait;
}

void qspFunctionDesc(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *desc;
	long index = qspLocIndex(args[0].Str);
	if (index < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return;
	}
	desc = qspFormatText(qspLocs[index].Desc);
	if (qspErrorNum) return;
	tos->Str = desc;
}

void qspFunctionGetObj(QSPVariant *args, long count, QSPVariant *tos)
{
	long ind = args[0].Num - 1;
	if (ind < 0) ind = 0;
	if (ind < qspCurObjectsCount)
		tos->Str = qspGetNewText(qspCurObjects[ind].Desc, -1);
	else
		tos->Str = qspGetNewText(QSP_FMT(""), 0);
}

void qspFunctionIsPlay(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *file;
	if (qspIsAnyString(args[0].Str))
	{
		file = qspGetNewText(qspQstPath, qspQstPathLen);
		file = qspGetAddText(file, args[0].Str, qspQstPathLen, -1);
		tos->Num = -(qspCallIsPlayingFile(file) != 0);
		free(file);
	}
	else
		tos->Num = 0;
}

void qspFunctionInstr(QSPVariant *args, long count, QSPVariant *tos)
{
	QSP_CHAR *pos;
	long beg = args[0].Num - 1;
	if (beg < 0) beg = 0;
	if (beg < (long)QSP_STRLEN(args[1].Str))
	{
		pos = QSP_STRSTR(args[1].Str + beg, args[2].Str);
		tos->Num = (pos ? (long)(pos - args[1].Str) + 1 : 0);
	}
	else
		tos->Num = 0;
}
