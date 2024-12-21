/* Copyright (C) 2001-2024 Val Argunov (byte AT qsp DOT org) */
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
#include "common.h"
#include "errors.h"
#include "locations.h"
#include "text.h"
#include "variables.h"
#include "variant.h"

INLINE void qspFreeMenuItems(QSPListItem *items, int count);
INLINE void qspFreeMenuLocs(QSPString *locs, int count);

INLINE void qspFreeMenuItems(QSPListItem *items, int count)
{
    QSPListItem *curItem;
    for (curItem = items; count > 0; --count, ++curItem)
    {
        qspFreeString(&curItem->Name);
        qspFreeString(&curItem->Image);
    }
}

INLINE void qspFreeMenuLocs(QSPString *locs, int count)
{
    QSPString *curLoc;
    for (curLoc = locs; count > 0; --count, ++curLoc)
        qspFreeString(curLoc);
}

void qspStatementShowMenu(QSPVariant *args, QSP_TINYINT count, QSP_TINYINT QSP_UNUSED(extArg))
{
    QSPVar *var;
    QSPVariant *curItem;
    int ind, itemsCount, maxItems;
    QSPListItem menuItems[QSP_MAXMENUITEMS];
    QSPString menuLocs[QSP_MAXMENUITEMS], itemName, itemLocation, itemImage, str;
    QSPTuple tuple;
    QSP_CHAR *pos, *pos2;
    if (!((var = qspVarReference(QSP_STR(args[0]), QSP_FALSE)))) return;
    if (count >= 2)
    {
        ind = QSP_TOINT(QSP_NUM(args[1]));
        if (ind < 0) ind = 0;
    }
    else
        ind = 0;
    if (count == 3)
    {
        maxItems = QSP_TOINT(QSP_NUM(args[2]));
        if (maxItems < 0) maxItems = 0;
    }
    else
        maxItems = QSP_MAXMENUITEMS;
    itemsCount = 0;
    for (curItem = var->Values + ind; ind < var->ValsCount; ++ind, ++curItem)
    {
        if (itemsCount == maxItems) break;
        itemName = itemLocation = itemImage = qspNullString;
        switch (QSP_BASETYPE(curItem->Type))
        {
        case QSP_TYPE_TUPLE:
            tuple = QSP_PTUPLE(curItem);
            if (tuple.Items < 2) break;
            if (tuple.Items >= 3)
            {
                itemImage = qspGetVariantAsString(&tuple.Vals[2]);
                if (!qspIsAnyString(itemImage)) qspClearText(&itemImage);
            }
            itemName = qspGetVariantAsString(&tuple.Vals[0]);
            itemLocation = qspGetVariantAsString(&tuple.Vals[1]);
            break;
        case QSP_TYPE_STR:
            str = QSP_PSTR(curItem);
            if (!qspIsAnyString(str)) break;
            if (!(pos2 = qspInStrRChars(str, QSP_MENUDELIM)))
            {
                qspSetError(QSP_ERR_COLONNOTFOUND);
                qspFreeMenuItems(menuItems, itemsCount);
                qspFreeMenuLocs(menuLocs, itemsCount);
                return;
            }
            if (pos = qspInStrRChars(qspStringFromPair(str.Str, pos2), QSP_MENUDELIM))
            {
                itemName = qspCopyToNewText(qspStringFromPair(str.Str, pos));
                itemLocation = qspCopyToNewText(qspStringFromPair(pos + QSP_STATIC_LEN(QSP_MENUDELIM), pos2));
                itemImage = qspStringFromPair(pos2 + QSP_STATIC_LEN(QSP_MENUDELIM), str.End);
                itemImage = (qspIsAnyString(itemImage) ? qspCopyToNewText(itemImage) : qspNullString);
            }
            else
            {
                itemName = qspCopyToNewText(qspStringFromPair(str.Str, pos2));
                itemLocation = qspCopyToNewText(qspStringFromPair(pos2 + QSP_STATIC_LEN(QSP_MENUDELIM), str.End));
            }
            break;
        }
        if (qspIsEmpty(itemName) && qspIsEmpty(itemLocation))
        {
            qspFreeString(&itemName);
            qspFreeString(&itemLocation);
            qspFreeString(&itemImage);
            break;
        }
        if (itemsCount == QSP_MAXMENUITEMS)
        {
            qspSetError(QSP_ERR_CANTADDMENUITEM);
            qspFreeString(&itemName);
            qspFreeString(&itemLocation);
            qspFreeString(&itemImage);
            qspFreeMenuItems(menuItems, itemsCount);
            qspFreeMenuLocs(menuLocs, itemsCount);
            return;
        }
        menuItems[itemsCount].Name = itemName;
        menuItems[itemsCount].Image = itemImage;
        menuLocs[itemsCount] = itemLocation;
        ++itemsCount;
    }
    if (itemsCount)
    {
        int oldLocationState = qspLocationState;
        ind = qspCallShowMenu(menuItems, itemsCount);
        if (qspLocationState != oldLocationState) return;
        if (ind >= 0 && ind < itemsCount)
        {
            QSPVariant arg = qspNumVariant(ind + 1);
            qspExecLocByNameWithArgs(menuLocs[ind], &arg, 1, QSP_FALSE, 0);
        }
        qspFreeMenuItems(menuItems, itemsCount);
        qspFreeMenuLocs(menuLocs, itemsCount);
    }
}
