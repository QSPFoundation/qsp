/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
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

#include "time.h"
#include "callbacks.h"

QSP_BIGINT qspMSCount = 0;

void qspResetTime(QSP_BIGINT msecs)
{
    qspMSCount = msecs;
    qspCallGetMSCount();
}

QSP_BIGINT qspGetTime(void)
{
    if ((qspMSCount += qspCallGetMSCount()) < 0) qspMSCount = 0;
    return qspMSCount;
}
