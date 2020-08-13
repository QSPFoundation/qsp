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

#include "declarations.h"

#ifndef QSP_LOCSDEFINES
    #define QSP_LOCSDEFINES

    #define QSP_GAMEID QSP_FMT("QSPGAME")
    #define QSP_PASSWD QSP_FMT("No")
    #define QSP_MAXACTIONS 50

    typedef struct
    {
        QSP_CHAR *Image;
        QSP_CHAR *Desc;
        QSP_CHAR *Code;
    } QSPLocAct;
    typedef struct
    {
        QSP_CHAR *Name;
        QSP_CHAR *Desc;
        QSP_CHAR *OnVisit;
        QSPLocAct Actions[QSP_MAXACTIONS];
    } QSPLocation;

    extern QSPLocation *qspLocs;
    extern int qspLocsCount;

    /* External functions */
    void qspCreateWorld(int);
    int qspGetLocsStrings(QSP_CHAR *, QSP_CHAR, QSP_CHAR, QSP_BOOL, QSP_CHAR **);
    int qspOpenTextData(QSP_CHAR *, QSP_CHAR, QSP_CHAR, QSP_BOOL);
    char *qspSaveQuestToText(QSP_CHAR, QSP_CHAR, QSP_BOOL, int *);
    QSP_BOOL qspOpenQuest(char *, int, QSP_CHAR *);
    char *qspSaveQuest(QSP_BOOL, QSP_BOOL, QSP_CHAR *, int *);

#endif
