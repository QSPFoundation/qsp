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
	QSP_CHAR *imgPath, *str, *pos, *pos2, *endPos;
	long i, varInd = qspVarIndexWithType(QSP_STR(args[0]), QSP_FALSE, 0);
	if (varInd < 0) return QSP_FALSE;
	qspClearMenu(QSP_FALSE);
	qspCallDeleteMenu();
	for (i = 0; i < qspVars[varInd].ValsCount; ++i)
	{
		if (!((str = qspVars[varInd].TextValue[i]) && qspIsAnyString(str))) break;
		pos2 = qspInStrRChar(str, QSP_MENUDELIM[0], 0);
		if (!pos2)
		{
			qspSetError(QSP_ERR_COLONNOTFOUND);
			return QSP_FALSE;
		}
		if (qspCurMenuItems == QSP_MAXMENUITEMS)
		{
			qspSetError(QSP_ERR_CANTADDMENUITEM);
			return QSP_FALSE;
		}
		endPos = qspStrEnd(str);
		pos = qspInStrRChar(str, QSP_MENUDELIM[0], pos2);
		if (!pos)
		{
			pos = pos2;
			pos2 = endPos;
		}
		qspCurMenuLocs[qspCurMenuItems++] = qspGetNewText(pos + 1, (long)(pos2 - pos) - 1);
		if (pos2 < endPos && qspIsAnyString(++pos2))
		{
			imgPath = qspGetNewText(qspQstPath, qspQstPathLen);
			imgPath = qspGetAddText(imgPath, pos2, qspQstPathLen, (long)(endPos - pos2));
		}
		else
			imgPath = 0;
		*pos = 0;
		qspCallAddMenuItem(str, imgPath);
		*pos = QSP_MENUDELIM[0];
		if (imgPath) free(imgPath);
	}
	qspCallShowMenu();
	return QSP_FALSE;
}
