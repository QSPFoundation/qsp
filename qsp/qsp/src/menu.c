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

#include "menu.h"
#include "callbacks.h"
#include "errors.h"
#include "game.h"
#include "text.h"
#include "variables.h"

QSP_CHAR *qspCurMenuLocs[QSP_MAXMENUITEMS];
long qspCurMenuItems = 0;

void qspClearMenu(QSP_BOOL isFirst)
{
	long i;
	if (!isFirst)
	{
		for (i = 0; i < qspCurMenuItems; ++i)
			free(qspCurMenuLocs[i]);
	}
	qspCurMenuItems = 0;
}

QSP_BOOL qspStatementShowMenu(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long ind, maxItems, len;
	QSPVar *var;
	QSP_CHAR *imgPath, *str, *pos, *pos2;
	if (!(var = qspVarReferenceWithType(QSP_STR(args[0]), QSP_FALSE, 0))) return QSP_FALSE;
	qspClearMenu(QSP_FALSE);
	qspCallDeleteMenu();
	if (count == 1)
	{
		ind = 0;
		maxItems = QSP_MAXMENUITEMS;
	}
	else
	{
		ind = QSP_NUM(args[1]);
		if (ind < 0) ind = 0;
		if (count == 2)
			maxItems = QSP_MAXMENUITEMS;
		else
		{
			maxItems = QSP_NUM(args[2]);
			if (maxItems < 0) maxItems = 0;
		}
	}
	while (ind < var->ValsCount)
	{
		if (qspCurMenuItems == maxItems) break;
		if (!((str = var->StrVals[ind]) && qspIsAnyString(str))) break;
		if (!(pos2 = qspInStrRChars(str, QSP_MENUDELIM, 0)))
		{
			qspSetError(QSP_ERR_COLONNOTFOUND);
			return QSP_FALSE;
		}
		if (qspCurMenuItems == QSP_MAXMENUITEMS)
		{
			qspSetError(QSP_ERR_CANTADDMENUITEM);
			return QSP_FALSE;
		}
		if (pos = qspInStrRChars(str, QSP_MENUDELIM, pos2))
		{
			len = (long)(pos2 - pos) - 1;
			imgPath = (qspIsAnyString(++pos2) ? qspGetAbsFromRelPath(pos2) : 0);
		}
		else
		{
			pos = pos2;
			len = -1;
			imgPath = 0;
		}
		qspCurMenuLocs[qspCurMenuItems++] = qspGetNewText(pos + 1, len);
		*pos = 0;
		qspCallAddMenuItem(str, imgPath);
		*pos = QSP_MENUDELIM[0];
		if (imgPath) free(imgPath);
		++ind;
	}
	if (qspCurMenuItems) qspCallShowMenu();
	return QSP_FALSE;
}
