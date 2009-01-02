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

#include "variant.h"
#include "coding.h"
#include "text.h"

void qspFreeVariants(QSPVariant *args, long count)
{
	while (--count >= 0)
		if (args[count].IsStr) free(QSP_STR(args[count]));
}

QSPVariant qspGetEmptyVariant(QSP_BOOL isStringType)
{
	QSPVariant ret;
	if (ret.IsStr = isStringType)
		QSP_STR(ret) = qspGetNewText(QSP_FMT(""), 0);
	else
		QSP_NUM(ret) = 0;
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
			QSP_STR(res) = (isFreeStr ? QSP_STR(val) : qspGetNewText(QSP_STR(val), -1));
		else
		{
			QSP_NUM(res) = qspStrToNum(QSP_STR(val), &temp);
			if (*temp) *isError = QSP_TRUE;
			if (isFreeStr) free(QSP_STR(val));
		}
	}
	else
	{
		if (isToString)
			QSP_STR(res) = qspGetNewText(qspNumToStr(buf, QSP_NUM(val)), -1);
		else
			QSP_NUM(res) = QSP_NUM(val);
	}
	return res;
}

void qspCopyVariant(QSPVariant *dest, QSPVariant src)
{
	if (dest->IsStr = src.IsStr)
		QSP_STR(*dest) = qspGetNewText(QSP_STR(src), -1);
	else
		QSP_NUM(*dest) = QSP_NUM(src);
}

QSP_BOOL qspIsCanConvertToNum(QSPVariant val)
{
	QSP_CHAR *temp;
	if (val.IsStr)
	{
		qspStrToNum(QSP_STR(val), &temp);
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
		res = QSP_STRCOLL(QSP_STR(v1), QSP_STR(v2));
		if (isFree1) free(QSP_STR(v1));
		if (isFree2) free(QSP_STR(v2));
	}
	else
		res = (QSP_NUM(v1) > QSP_NUM(v2) ? 1 : (QSP_NUM(v1) < QSP_NUM(v2) ? -1 : 0));
	return res;
}
