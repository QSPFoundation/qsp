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

#include "declarations.h"

#ifndef QSP_VARIANTDEFINES
	#define QSP_VARIANTDEFINES

	#define QSP_STR(a) (a).Val.Str
	#define QSP_NUM(a) (a).Val.Num
	#define QSP_PSTR(a) (a)->Val.Str
	#define QSP_PNUM(a) (a)->Val.Num

	typedef struct
	{
		union
		{
			QSP_CHAR *Str;
			int Num;
		} Val;
		QSP_BOOL IsStr;
	} QSPVariant;

	/* External functions */
	void qspFreeVariants(QSPVariant *, int);
	QSPVariant qspGetEmptyVariant(QSP_BOOL);
	QSP_BOOL qspConvertVariantTo(QSPVariant *, QSP_BOOL);
	void qspCopyVariant(QSPVariant *, QSPVariant *);
	QSP_BOOL qspIsCanConvertToNum(QSPVariant *);
	int qspAutoConvertCompare(QSPVariant *, QSPVariant *);

#endif
