/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
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

QSP_BOOL qspStatementShowMenu(QSPVariant *args, QSP_TINYINT count, QSPString *jumpTo, QSP_TINYINT extArg)
{
    QSPVar *var;
    QSPVariant arg;
    int ind, itemsCount, maxItems;
    QSPListItem menuItems[QSP_MAXMENUITEMS];
    QSPString menuLocs[QSP_MAXMENUITEMS], imgPath, str;
    QSP_CHAR *pos, *pos2;
    if (!(var = qspVarReference(QSP_STR(args[0]), QSP_FALSE))) return QSP_FALSE;
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
        if (!QSP_ISSTR(var->Values[ind].Type)) break;
        str = QSP_STR(var->Values[ind]);
        if (!(str.Str && qspIsAnyString(str))) break;
        if (!(pos2 = qspInStrRChars(str, QSP_MENUDELIM)))
        {
            qspSetError(QSP_ERR_COLONNOTFOUND);
            return QSP_FALSE;
        }
        if (itemsCount == QSP_MAXMENUITEMS)
        {
            qspSetError(QSP_ERR_CANTADDMENUITEM);
            return QSP_FALSE;
        }
        if (pos = qspInStrRChars(qspStringFromPair(str.Str, pos2), QSP_MENUDELIM))
        {
            imgPath = qspStringFromPair(pos2 + QSP_STATIC_LEN(QSP_MENUDELIM), str.End);
            imgPath = (qspIsAnyString(imgPath) ? qspGetNewText(imgPath) : qspNullString);
        }
        else
        {
            pos = pos2;
            pos2 = str.End;
            imgPath = qspNullString;
        }
        menuLocs[itemsCount] = qspGetNewText(qspStringFromPair(pos + QSP_STATIC_LEN(QSP_MENUDELIM), pos2));
        menuItems[itemsCount].Name = qspGetNewText(qspStringFromPair(str.Str, pos));
        menuItems[itemsCount].Image = imgPath;
        ++itemsCount;
        ++ind;
    }
    if (itemsCount)
    {
        ind = qspCallShowMenu(menuItems, itemsCount);
        if (ind >= 0 && ind < itemsCount)
        {
            arg.Type = QSP_TYPE_NUM;
            QSP_NUM(arg) = ind + 1;
            qspExecLocByNameWithArgs(menuLocs[ind], &arg, 1, 0);
        }
        while (--itemsCount >= 0)
        {
            qspFreeString(menuItems[itemsCount].Name);
            qspFreeString(menuItems[itemsCount].Image);
            qspFreeString(menuLocs[itemsCount]);
        }
    }
    return QSP_FALSE;
}
