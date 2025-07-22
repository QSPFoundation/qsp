/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
            if (!(pos2 = qspStrRChar(str, QSP_MENUDELIM_CHAR)))
            {
                qspSetError(QSP_ERR_COLONNOTFOUND);
                qspFreeMenuItems(menuItems, itemsCount);
                qspFreeMenuLocs(menuLocs, itemsCount);
                return;
            }
            if ((pos = qspStrRChar(qspStringFromPair(str.Str, pos2), QSP_MENUDELIM_CHAR)))
            {
                itemName = qspCopyToNewText(qspStringFromPair(str.Str, pos));
                itemLocation = qspCopyToNewText(qspStringFromPair(pos + QSP_CHAR_LEN, pos2));
                itemImage = qspStringFromPair(pos2 + QSP_CHAR_LEN, str.End);
                itemImage = (qspIsAnyString(itemImage) ? qspCopyToNewText(itemImage) : qspNullString);
            }
            else
            {
                itemName = qspCopyToNewText(qspStringFromPair(str.Str, pos2));
                itemLocation = qspCopyToNewText(qspStringFromPair(pos2 + QSP_CHAR_LEN, str.End));
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
