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

#ifndef QSP_DEFAULTDEFINES
    #define QSP_DEFAULTDEFINES

    static int qspEndiannessTestValue = 1;

    #ifdef _UNICODE
        typedef wchar_t QSP_CHAR;
        #define QSP_FMT2(x) L##x
        #define QSP_FMT(x) QSP_FMT2(x)

        #define QSP_ONIG_ENC ((*(char *)&(qspEndiannessTestValue) == 1) ? \
                    (sizeof(QSP_CHAR) == 2 ? ONIG_ENCODING_UTF16_LE : ONIG_ENCODING_UTF32_LE) : \
                    (sizeof(QSP_CHAR) == 2 ? ONIG_ENCODING_UTF16_BE : ONIG_ENCODING_UTF32_BE))
        #define QSP_TO_GAME_SB(a) (char)qspReverseConvertUC((a), qspCP1251ToUnicodeTable)
        #define QSP_TO_GAME_UC(a) (unsigned short)QSP_FIXBYTEORDER((unsigned short)(a))
        #define QSP_FROM_GAME_SB(a) qspDirectConvertUC((a), qspCP1251ToUnicodeTable)
        #define QSP_FROM_GAME_UC(a) QSP_FIXBYTEORDER(a)
        #define QSP_CHRLWR(a) qspToWLower(a)
        #define QSP_CHRUPR(a) qspToWUpper(a)
    #else
        typedef char QSP_CHAR;
        #define QSP_FMT(x) x

        #if defined(WIN32)
            #define QSP_ONIG_ENC ONIG_ENCODING_CP1251
            #define QSP_TO_GAME_SB(a) (char)(a)
            #define QSP_TO_GAME_UC(a) (unsigned short)QSP_FIXBYTEORDER((unsigned short)qspDirectConvertUC((a), qspCP1251ToUnicodeTable))
            #define QSP_FROM_GAME_SB(a) (a)
            #define QSP_FROM_GAME_UC(a) qspReverseConvertUC(QSP_FIXBYTEORDER(a), qspCP1251ToUnicodeTable)
            #define QSP_CHRLWR(a) qspCP1251ToLowerTable[(unsigned char)(a)]
            #define QSP_CHRUPR(a) qspCP1251ToUpperTable[(unsigned char)(a)]
        #else
            #define QSP_ONIG_ENC ONIG_ENCODING_KOI8_R
            #define QSP_TO_GAME_SB(a) (char)qspReverseConvertSB((a), qspCP1251ToKOI8RTable)
            #define QSP_TO_GAME_UC(a) (unsigned short)QSP_FIXBYTEORDER((unsigned short)qspDirectConvertUC((a), qspKOI8RToUnicodeTable))
            #define QSP_FROM_GAME_SB(a) qspDirectConvertSB((a), qspCP1251ToKOI8RTable)
            #define QSP_FROM_GAME_UC(a) qspReverseConvertUC(QSP_FIXBYTEORDER(a), qspKOI8RToUnicodeTable)
            #define QSP_CHRLWR(a) qspKOI8RToLowerTable[(unsigned char)(a)]
            #define QSP_CHRUPR(a) qspKOI8RToUpperTable[(unsigned char)(a)]
        #endif
    #endif

    #define QSP_FIXBYTEORDER(a) ((*(char *)&(qspEndiannessTestValue) == 1) ? \
                                (a) : \
                                ((unsigned short)(((a) << 8) | ((a) >> 8))))

    #if defined(_MSC_VER)
        #define QSP_TIME _time64
    #else
        #define QSP_TIME time
    #endif

    #ifdef __cplusplus
        typedef int (*QSP_CALLBACK)(...);
    #else
        typedef int (*QSP_CALLBACK)();
    #endif

    #include "../qsp.h"

    #ifdef __cplusplus
    extern "C"
    {
    #endif

    QSP_EXTERN QSPString QSPStringFromPair(QSP_CHAR *start, QSP_CHAR *end);
    QSP_EXTERN QSPString QSPStringFromLen(QSP_CHAR *start, int length);
    QSP_EXTERN QSPString QSPStringFromC(QSP_CHAR *s);

    QSP_EXTERN void QSPInit(void);
    QSP_EXTERN void QSPTerminate(void);
    QSP_EXTERN void QSPSetCallback(int type, QSP_CALLBACK func);
    QSP_EXTERN void QSPEnableDebugMode(QSP_BOOL isDebug);
    QSP_EXTERN void QSPGetCurStateData(QSPString *loc, int *actIndex, int *lineNum);
    QSP_EXTERN QSPString QSPGetVersion(void);
    QSP_EXTERN QSPString QSPGetCompiledDateTime(void);
    QSP_EXTERN int QSPGetFullRefreshCount(void);
    /* Main desc */
    QSP_EXTERN QSPString QSPGetMainDesc(void);
    QSP_EXTERN QSP_BOOL QSPIsMainDescChanged(void);
    /* Vars desc */
    QSP_EXTERN QSPString QSPGetVarsDesc(void);
    QSP_EXTERN QSP_BOOL QSPIsVarsDescChanged(void);
    /* Input string */
    QSP_EXTERN void QSPSetInputStrText(QSPString str);
    /* Actions */
    QSP_EXTERN int QSPGetActions(QSPListItem *items, int itemsBufSize);
    QSP_EXTERN QSP_BOOL QSPSetSelActionIndex(int ind, QSP_BOOL toRefreshUI);
    QSP_EXTERN int QSPGetSelActionIndex(void);
    QSP_EXTERN QSP_BOOL QSPIsActionsChanged(void);
    QSP_EXTERN QSP_BOOL QSPExecuteSelActionCode(QSP_BOOL toRefreshUI);
    /* Objects */
    QSP_EXTERN int QSPGetObjects(QSPListItem *items, int itemsBufSize);
    QSP_EXTERN QSP_BOOL QSPSetSelObjectIndex(int ind, QSP_BOOL toRefreshUI);
    QSP_EXTERN int QSPGetSelObjectIndex(void);
    QSP_EXTERN QSP_BOOL QSPIsObjectsChanged(void);
    /* Windows */
    QSP_EXTERN void QSPShowWindow(int type, QSP_BOOL toShow);
    /* Code execution */
    QSP_EXTERN QSP_BOOL QSPExecString(QSPString str, QSP_BOOL toRefreshUI);
    QSP_EXTERN QSP_BOOL QSPCalculateStrExpression(QSPString s, QSP_CHAR *buf, int bufSize, QSP_BOOL toRefreshUI);
    QSP_EXTERN QSP_BOOL QSPCalculateNumExpression(QSPString s, int *res, QSP_BOOL toRefreshUI);
    QSP_EXTERN QSP_BOOL QSPExecCounter(QSP_BOOL toRefreshUI);
    QSP_EXTERN QSP_BOOL QSPExecUserInput(QSP_BOOL toRefreshUI);
    QSP_EXTERN QSP_BOOL QSPExecLocationCode(QSPString name, QSP_BOOL toRefreshUI);
    /* Errors */
    QSP_EXTERN QSPErrorInfo QSPGetLastErrorData(void);
    QSP_EXTERN QSPString QSPGetErrorDesc(int errorNum);
    /* Game */
    QSP_EXTERN QSP_BOOL QSPLoadGameWorldFromData(const void *data, int dataSize, QSP_BOOL isNewGame);
    QSP_EXTERN QSP_BOOL QSPSaveGameAsData(void *buf, int *bufSize, QSP_BOOL toRefreshUI);
    QSP_EXTERN QSP_BOOL QSPOpenSavedGameFromData(const void *data, int dataSize, QSP_BOOL toRefreshUI);

    QSP_EXTERN QSP_BOOL QSPRestartGame(QSP_BOOL toRefreshUI);
    /* Variables */
    QSP_EXTERN QSP_BOOL QSPGetVarValuesCount(QSPString name, int *count);
    QSP_EXTERN QSP_BOOL QSPGetVarIndexByString(QSPString name, QSPString str, int *index);
    QSP_EXTERN QSP_BOOL QSPGetVarValue(QSPString name, int ind, QSPVariant *res);
    QSP_EXTERN QSP_BOOL QSPConvertValueToString(QSPVariant value, QSP_CHAR *buf, int bufSize);
    QSP_EXTERN QSP_BOOL QSPGetNumVarValue(QSPString name, int ind, int *res);
    QSP_EXTERN QSP_BOOL QSPGetStrVarValue(QSPString name, int ind, QSPString *res);

    #ifdef __cplusplus
    }
    #endif

#endif
