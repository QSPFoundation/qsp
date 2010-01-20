/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "locations.h"

QSPLocation *qspLocs = 0;
int qspLocsCount = 0;

void qspCreateWorld(int locsCount)
{
	int i;
	for (i = 0; i < qspLocsCount; ++i)
	{
		free(qspLocs[i].Name);
		free(qspLocs[i].OnVisit);
	}
	if (qspLocsCount != locsCount)
	{
		qspLocsCount = locsCount;
		qspLocs = (QSPLocation *)realloc(qspLocs, qspLocsCount * sizeof(QSPLocation));
	}
}
