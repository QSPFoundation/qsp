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

#include <stddef.h>
#include <jni.h>

#ifndef QSP_JAVADEFINES
    #define QSP_JAVADEFINES

    #ifdef _UNICODE
        #ifdef _WIN32
            typedef wchar_t QSP_CHAR;
            #define QSP_FMT2(x) L##x
            #define QSP_FMT(x) QSP_FMT2(x)
        #else
            typedef unsigned short QSP_CHAR;
            #define QSP_FMT2(x) u##x
            #define QSP_FMT(x) QSP_FMT2(x)
        #endif

        #define QSP_ONIG_ENC ONIG_ENCODING_UTF16_LE
        #define QSP_TO_GAME_SB(a) (char)qspReverseConvertUC((a), qspCP1251ToUnicodeTable)
        #define QSP_TO_GAME_UC(a) (unsigned short)(a)
        #define QSP_FROM_GAME_SB(a) qspDirectConvertUC((a), qspCP1251ToUnicodeTable)
        #define QSP_FROM_GAME_UC(a) (a)
        #define QSP_CHRLWR(a) qspToWLower(a)
        #define QSP_CHRUPR(a) qspToWUpper(a)
    #else
        #error "Non-Unicode build using Java binding is not supported"
    #endif

    #if defined(_MSC_VER)
        #define QSP_TIME _time64
    #else
        #define QSP_TIME time
    #endif

    typedef jmethodID QSP_CALLBACK;

    #include "../qsp.h"
    #include "com_libqsp_jni_QSPLib.h"

    extern JavaVM *qspJvm;
    extern jclass qspApiClass;
    extern jobject qspApiObject;

    extern jclass qspListItemClass;
    extern jclass qspExecutionStateClass;
    extern jclass qspErrorInfoClass;

    typedef struct
    {
        jstring Image;
        jstring Name;
        jobject ListItem;
    } JNIListItem;

    jstring qspToJavaString(JNIEnv *env, QSPString str);
    QSPString qspFromJavaString(JNIEnv *env, jstring str);
    JNIListItem qspToJavaListItem(JNIEnv *env, QSPString image, QSPString name);
    void qspReleaseJavaListItem(JNIEnv *env, JNIListItem *listItem);

#endif
