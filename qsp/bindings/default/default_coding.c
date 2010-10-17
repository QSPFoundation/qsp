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

#include "../../declarations.h"

#ifdef _DEFAULT_BINDING

#include "../../text.h"

#ifdef _UNICODE
	#define QSP_WCSTOMBSLEN(a) (int)wcstombs(0, a, 0)
	#define QSP_WCSTOMBS wcstombs
#else
	#define QSP_WCSTOMBSLEN qspStrLen
	#define QSP_WCSTOMBS qspStrNCopy
#endif

char *qspToSysString(QSP_CHAR *s)
{
	int len = QSP_WCSTOMBSLEN(s) + 1;
	char *ret = (char *)malloc(len);
	QSP_WCSTOMBS(ret, s, len);
	return ret;
}

#endif
