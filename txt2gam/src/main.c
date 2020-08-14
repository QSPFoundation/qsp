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

#include "main.h"
#include "coding.h"
#include "locations.h"
#include "text.h"

static QSP_BOOL qspLoadTextFileContents(char *, QSP_CHAR **);
static QSP_BOOL qspExportStrings(char *, char *, QSP_CHAR *, QSP_CHAR *, QSP_BOOL, QSP_BOOL);
static QSP_BOOL qspOpenQuestFromTextFile(char *, QSP_CHAR *, QSP_CHAR *);
static QSP_BOOL qspSaveGameFile(char *, QSP_BOOL, QSP_BOOL, QSP_CHAR *);
static QSP_BOOL qspOpenGameFile(char *, QSP_CHAR *);
static QSP_BOOL qspSaveQuestToTextFile(char *, QSP_CHAR *, QSP_CHAR *, QSP_BOOL);

static QSP_BOOL qspLoadTextFileContents(char *file, QSP_CHAR **data)
{
    int fileSize;
    char *buf, *resBuf;
    QSP_BOOL isUCS2;
    FILE *f;
    /* Loading file's contents */
    if (!(f = fopen(file, "rb"))) return QSP_FALSE;
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    buf = (char *)malloc(fileSize + 3);
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, fileSize, f);
    fclose(f);
    buf[fileSize] = buf[fileSize + 1] = buf[fileSize + 2] = 0;
    resBuf = buf;
    if ((unsigned char)resBuf[0] == (unsigned char)TXT2GAM_BOM[0] && (unsigned char)resBuf[1] == (unsigned char)TXT2GAM_BOM[1])
    {
        resBuf += 2;
        isUCS2 = QSP_TRUE;
    }
    else if (resBuf[0] && !resBuf[1])
        isUCS2 = QSP_TRUE;
    else
        isUCS2 = QSP_FALSE;
    *data = qspGameToQSPString(resBuf, isUCS2, QSP_FALSE);
    free(buf);
    return QSP_TRUE;
}

static QSP_BOOL qspExportStrings(char *file, char *outFile, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isGetQStrings, QSP_BOOL isUCS2)
{
    int len;
    char *buf;
    QSP_CHAR *data, *strs;
    FILE *f;
    if (!qspLoadTextFileContents(file, &data)) return QSP_FALSE;
    len = qspGetLocsStrings(data, locStart, locEnd, isGetQStrings, &strs);
    free(data);
    if (len)
    {
        if (!(f = fopen(outFile, "wb")))
        {
            free(strs);
            return QSP_FALSE;
        }
        buf = qspQSPToGameString(strs, isUCS2, QSP_FALSE);
        free(strs);
        fwrite(buf, isUCS2 ? 2 : 1, len, f);
        free(buf);
        fclose(f);
    }
    return QSP_TRUE;
}

static QSP_BOOL qspOpenQuestFromTextFile(char *file, QSP_CHAR *locStart, QSP_CHAR *locEnd)
{
    int locsCount;
    QSP_CHAR *data;
    if (!qspLoadTextFileContents(file, &data)) return QSP_FALSE;
    locsCount = qspOpenTextData(data, locStart, locEnd, QSP_FALSE);
    qspCreateWorld(locsCount);
    qspOpenTextData(data, locStart, locEnd, QSP_TRUE);
    free(data);
    return QSP_TRUE;
}

static QSP_BOOL qspSaveGameFile(char *file, QSP_BOOL isOldFormat, QSP_BOOL isUCS2, QSP_CHAR *passwd)
{
    int len;
    char *buf;
    FILE *f;
    buf = qspSaveQuest(isOldFormat, isUCS2, passwd, &len);
    if (!buf) return QSP_FALSE;
    if (!(f = fopen(file, "wb"))) return QSP_FALSE;
    fwrite(buf, 1, len, f);
    free(buf);
    fclose(f);
    return QSP_TRUE;
}

static QSP_BOOL qspOpenGameFile(char *file, QSP_CHAR *password)
{
    int fileSize;
    char *data;
    FILE *f;
    if (!(f = fopen(file, "rb"))) return QSP_FALSE;
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    data = (char *)malloc(fileSize);
    fseek(f, 0, SEEK_SET);
    fread(data, 1, fileSize, f);
    fclose(f);
    if (!qspOpenQuest(data, fileSize, password))
    {
        free(data);
        return QSP_FALSE;
    }
    free(data);
    return QSP_TRUE;
}

static QSP_BOOL qspSaveQuestToTextFile(char *file, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isUCS2)
{
    int len;
    char *buf;
    FILE *f;
    buf = qspSaveQuestToText(locStart, locEnd, isUCS2, &len);
    if (!buf) return QSP_FALSE;
    if (!(f = fopen(file, "wb"))) return QSP_FALSE;
    if (isUCS2)
    {
        fwrite(TXT2GAM_BOM, 1, sizeof(TXT2GAM_BOM) - 1, f);
    }
    fwrite(buf, 1, len, f);
    free(buf);
    fclose(f);
    return QSP_TRUE;
}

void ShowHelp()
{
    char *temp = qspFromQSPString(QSP_VER);
    printf("TXT2GAM utility, ver. %s\n", temp);
    free(temp);
    printf("Using:\n");
    printf("  txt2gam [input file] [output file] [options]\n");
    printf("Options:\n");
    printf("  a, A - ANSI mode, default is Unicode (UCS-2 / UTF-16) mode\n");
    printf("  o, O - Save game in old format, default is new format\n");
    temp = qspFromQSPString(QSP_STARTLOC);
    printf("  s[string], S[string] - 'Begin of loc' prefix, default is '%s'\n", temp);
    free(temp);
    temp = qspFromQSPString(QSP_ENDLOC);
    printf("  e[string], E[string] - 'End of loc' prefix, default is '%s'\n", temp);
    free(temp);
    temp = qspFromQSPString(QSP_PASSWD);
    printf("  p[pass], P[pass] - Password, default is '%s'\n", temp);
    free(temp);
    printf("  c, C - Encode text file into game file (default mode)\n");
    printf("  d, D - Decode game file into text file\n");
    printf("  t, T - Extract strings from text\n");
    printf("  q, Q - Extract q-strings from text\n");
    printf("Examples:\n");
    printf("  txt2gam file.txt gamefile.qsp pMyPassword\n");
    printf("  txt2gam file.txt gamefile.qsp\n");
    printf("  txt2gam file.txt gamefile.qsp u\n");
    printf("  txt2gam file.txt gamefile.qsp o pMyPassword\n");
    printf("  txt2gam file.txt gamefile.qsp o e@ pMyPassword\n");
    printf("  txt2gam file.txt gamefile.qsp o \"pMy Password\"\n");
    printf("  txt2gam file.txt gamefile.qsp u o \"pMy Password\"\n");
    printf("  txt2gam file.txt gamefile.qsp o\n");
    printf("  txt2gam file.txt gamefile.qsp o e@\n");
    printf("  txt2gam file.txt gamefile.qsp s@ e~\n");
    printf("  txt2gam file.txt gamefile.qsp s@ E~ o\n");
    printf("  txt2gam file.txt strsfile.txt t u\n");
}

int main(int argc, char **argv)
{
    int i, workMode;
    QSP_BOOL isOldFormat, isUCS2, isErr;
    QSP_CHAR *passwd, *locStart, *locEnd, ch;
    setlocale(LC_ALL, QSP_LOCALE);
    if (argc < 3)
    {
        ShowHelp();
        return 0;
    }
    workMode = QSP_ENCODE_INTO_GAME;
    isOldFormat = QSP_FALSE;
    isUCS2 = QSP_TRUE;
    qspAddText(&locStart, QSP_STARTLOC, 0, -1, QSP_TRUE);
    qspAddText(&locEnd, QSP_ENDLOC, 0, -1, QSP_TRUE);
    qspAddText(&passwd, QSP_PASSWD, 0, -1, QSP_TRUE);
    for (i = 3; i < argc; ++i)
    {
        switch (*argv[i])
        {
        case 'o': case 'O':
            isOldFormat = QSP_TRUE;
            break;
        case 'a': case 'A':
            isUCS2 = QSP_FALSE;
            break;
        case 's': case 'S':
        case 'e': case 'E':
        case 'p': case 'P':
            if (argv[i][1])
            {
                switch (*argv[i])
                {
                case 's': case 'S':
                    free(locStart);
                    locStart = qspToQSPString(argv[i] + 1);
                    break;
                case 'e': case 'E':
                    free(locEnd);
                    locEnd = qspToQSPString(argv[i] + 1);
                    break;
                case 'p': case 'P':
                    free(passwd);
                    passwd = qspToQSPString(argv[i] + 1);
                    break;
                }
            }
            break;
        case 'c': case 'C':
            workMode = QSP_ENCODE_INTO_GAME;
            break;
        case 'd': case 'D':
            workMode = QSP_DECODE_INTO_TEXT;
            break;
        case 't': case 'T':
            workMode = QSP_EXTRACT_STRINGS;
            break;
        case 'q': case 'Q':
            workMode = QSP_EXTRACT_QSTRINGS;
            break;
        }
    }
    qspLocs = 0;
    qspLocsCount = 0;
    switch (workMode)
    {
        case QSP_EXTRACT_STRINGS:
        case QSP_EXTRACT_QSTRINGS:
            if (isErr = !qspExportStrings(argv[1], argv[2], locStart, locEnd, workMode == QSP_EXTRACT_QSTRINGS, isUCS2))
                printf("Strings extracting failed!\n");
            break;
        case QSP_ENCODE_INTO_GAME:
            do
            {
                if (isErr = !qspOpenQuestFromTextFile(argv[1], locStart, locEnd))
                {
                    printf("Loading text file failed!\n");
                    break;
                }
                if (isErr = !qspSaveGameFile(argv[2], isOldFormat, isUCS2, passwd))
                    printf("Saving game failed!\n");
            } while (0);
            break;
        case QSP_DECODE_INTO_TEXT:
            do
            {
                if (isErr = !qspOpenGameFile(argv[1], passwd))
                {
                    printf("Can't open game!\n");
                    break;
                }
                if (isErr = !qspSaveQuestToTextFile(argv[2], locStart, locEnd, isUCS2))
                    printf("Saving text file failed!\n");
            } while (0);
            break;
    }
    qspCreateWorld(0);
    free(locStart);
    free(locEnd);
    free(passwd);
    return (isErr == QSP_TRUE);
}
