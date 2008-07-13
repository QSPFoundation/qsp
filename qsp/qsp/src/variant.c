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

#include "declarations.h"

void qspFreeVariants(QSPVariant *args, long count)
{
	while (--count >= 0)
		if (args[count].IsStr) free(args[count].Str);
}

QSPVariant qspGetEmptyVariant(QSP_BOOL isStringType)
{
	QSPVariant ret;
	if (ret.IsStr = isStringType)
		ret.Str = qspGetNewText(QSP_FMT(""), 0);
	else
		ret.Num = 0;
	return ret;
}

QSPVariant qspConvertVariantTo(QSPVariant val, QSP_BOOL isToString, QSP_BOOL isFreeStr, QSP_BOOL *isError)
{
	QSPVariant res;
	QSP_CHAR *temp, buf[12];
	res.IsStr = isToString;
	if (val.IsStr)
	{
		if (isToString)
			res.Str = (isFreeStr ? val.Str : qspGetNewText(val.Str, -1));
		else
		{
			res.Num = qspStrToNum(val.Str, &temp);
			if (*temp) *isError = QSP_TRUE;
			if (isFreeStr) free(val.Str);
		}
	}
	else
	{
		if (isToString)
			res.Str = qspGetNewText(qspNumToStr(buf, val.Num), -1);
		else
			res.Num = val.Num;
	}
	return res;
}

void qspCopyVariant(QSPVariant *dest, QSPVariant src)
{
	if (dest->IsStr = src.IsStr)
		dest->Str = qspGetNewText(src.Str, -1);
	else
		dest->Num = src.Num;
}

QSP_BOOL qspIsCanConvertToNum(QSPVariant val)
{
	QSP_CHAR *temp;
	if (val.IsStr)
	{
		qspStrToNum(val.Str, &temp);
		if (*temp) return QSP_FALSE;
	}
	return QSP_TRUE;
}

int qspAutoConvertCompare(QSPVariant v1, QSPVariant v2)
{
	int res;
	QSP_BOOL isFree1 = QSP_FALSE, isFree2 = QSP_FALSE;
	if (v1.IsStr != v2.IsStr)
	{
		if (v2.IsStr)
		{
			if (qspIsCanConvertToNum(v2))
				v2 = qspConvertVariantTo(v2, QSP_FALSE, QSP_FALSE, 0);
			else
			{
				v1 = qspConvertVariantTo(v1, QSP_TRUE, QSP_FALSE, 0);
				isFree1 = QSP_TRUE;
			}
		}
		else
		{
			if (qspIsCanConvertToNum(v1))
				v1 = qspConvertVariantTo(v1, QSP_FALSE, QSP_FALSE, 0);
			else
			{
				v2 = qspConvertVariantTo(v2, QSP_TRUE, QSP_FALSE, 0);
				isFree2 = QSP_TRUE;
			}
		}
	}
	if (v1.IsStr)
	{
		res = QSP_STRCOLL(v1.Str, v2.Str);
		if (isFree1) free(v1.Str);
		if (isFree2) free(v2.Str);
	}
	else
		res = (v1.Num > v2.Num ? 1 : (v1.Num < v2.Num ? -1 : 0));
	return res;
}
