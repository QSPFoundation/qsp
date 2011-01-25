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

#include "menu.h"
#include "callbacks.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "text.h"
#include "variables.h"

QSP_BOOL qspStatementShowMenu(QSPVariant *args, int count, QSP_CHAR **jumpTo, int extArg)
{
	QSPVar *var;
	QSPVariant arg;
	int ind, itemsCount, maxItems, len;
	QSP_CHAR *menuLocs[QSP_MAXMENUITEMS], *imgPath, *str, *pos, *pos2;
	if (!(var = qspVarReferenceWithType(QSP_STR(args[0]), QSP_FALSE, 0))) return QSP_FALSE;
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
	itemsCount = 0;
	while (ind < var->ValsCount)
	{
		if (itemsCount == maxItems) break;
		if (!((str = var->Values[ind].Str) && qspIsAnyString(str))) break;
		if (!(pos2 = qspInStrRChars(str, QSP_MENUDELIM, 0)))
		{
			qspSetError(QSP_ERR_COLONNOTFOUND);
			return QSP_FALSE;
		}
		if (itemsCount == QSP_MAXMENUITEMS)
		{
			qspSetError(QSP_ERR_CANTADDMENUITEM);
			return QSP_FALSE;
		}
		if (pos = qspInStrRChars(str, QSP_MENUDELIM, pos2))
		{
			len = (int)(pos2 - pos) - 1;
			imgPath = (qspIsAnyString(++pos2) ? qspGetAbsFromRelPath(pos2) : 0);
		}
		else
		{
			pos = pos2;
			len = -1;
			imgPath = 0;
		}
		menuLocs[itemsCount++] = qspGetNewText(pos + 1, len);
		*pos = 0;
		qspCallAddMenuItem(str, imgPath);
		*pos = QSP_MENUDELIM[0];
		if (imgPath) free(imgPath);
		++ind;
	}
	if (itemsCount)
	{
		ind = qspCallShowMenu();
		if (ind >= 0 && ind < itemsCount)
		{
			arg.IsStr = QSP_FALSE;
			QSP_NUM(arg) = ind + 1;
			qspExecLocByNameWithArgs(menuLocs[ind], &arg, 1, 0);
		}
		while (--itemsCount >= 0)
			free(menuLocs[itemsCount]);
	}
	return QSP_FALSE;
}
