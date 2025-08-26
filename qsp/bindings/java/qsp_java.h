/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
    extern jclass qspObjectItemClass;
    extern jclass qspExecutionStateClass;
    extern jclass qspErrorInfoClass;

    typedef struct
    {
        jstring Name;
        jstring Title;
        jstring Image;
        jobject ListItem;
    } JNIListItem;

    jstring qspToJavaString(JNIEnv *env, QSPString str);
    QSPString qspFromJavaString(JNIEnv *env, jstring str);
    JNIListItem qspToJavaListItem(JNIEnv *env, QSPString name, QSPString image);
    JNIListItem qspToJavaObjectItem(JNIEnv *env, QSPString name, QSPString title, QSPString image);
    void qspReleaseJavaListItem(JNIEnv *env, JNIListItem *listItem);

#endif
