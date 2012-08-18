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

#include "variant.h"
#include "coding.h"
#include "text.h"

void qspFreeVariants(QSPVariant *args, int count)
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

QSP_BOOL qspConvertVariantTo(QSPVariant *val, QSP_BOOL isToString)
{
	int num;
	QSP_CHAR buf[12];
	QSP_BOOL isValid;
	if (val->IsStr)
	{
		if (!isToString)
		{
			num = qspStrToNum(QSP_PSTR(val), &isValid);
			if (!isValid) return QSP_TRUE;
			free(QSP_PSTR(val));
			QSP_PNUM(val) = num;
			val->IsStr = QSP_FALSE;
		}
	}
	else if (isToString)
	{
		QSP_PSTR(val) = qspGetNewText(qspNumToStr(buf, QSP_PNUM(val)), -1);
		val->IsStr = QSP_TRUE;
	}
	return QSP_FALSE;
}

void qspCopyVariant(QSPVariant *dest, QSPVariant *src)
{
	if (dest->IsStr = src->IsStr)
		QSP_PSTR(dest) = qspGetNewText(QSP_PSTR(src), -1);
	else
		QSP_PNUM(dest) = QSP_PNUM(src);
}

QSP_BOOL qspIsCanConvertToNum(QSPVariant *val)
{
	QSP_BOOL isValid;
	if (val->IsStr)
	{
		qspStrToNum(QSP_PSTR(val), &isValid);
		if (!isValid) return QSP_FALSE;
	}
	return QSP_TRUE;
}

int qspAutoConvertCompare(QSPVariant *v1, QSPVariant *v2)
{
	int res;
	if (v1->IsStr != v2->IsStr)
	{
		if (v2->IsStr)
		{
			if (qspIsCanConvertToNum(v2))
				qspConvertVariantTo(v2, QSP_FALSE);
			else
				qspConvertVariantTo(v1, QSP_TRUE);
		}
		else
		{
			if (qspIsCanConvertToNum(v1))
				qspConvertVariantTo(v1, QSP_FALSE);
			else
				qspConvertVariantTo(v2, QSP_TRUE);
		}
	}
	if (v1->IsStr)
		res = QSP_STRCOLL(QSP_PSTR(v1), QSP_PSTR(v2));
	else
		res = (QSP_PNUM(v1) > QSP_PNUM(v2) ? 1 : (QSP_PNUM(v1) < QSP_PNUM(v2) ? -1 : 0));
	return res;
}
