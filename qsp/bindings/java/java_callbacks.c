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

#include "../../declarations.h"

#ifdef _JAVA_BINDING

#include "../../callbacks.h"
#include "../../actions.h"
#include "../../coding.h"
#include "../../common.h"
#include "../../errors.h"
#include "../../objects.h"
#include "../../text.h"

INLINE JNIEnv *qspGetJniEnv()
{
    JNIEnv *javaEnv;
    /* Callbacks should be called on the JVM threads only */
    (*qspJvm)->GetEnv(qspJvm, (void **)&javaEnv, JNI_VERSION_1_6);
    return javaEnv;
}

void qspInitCallbacks(void)
{
    int i;
    qspIsInCallback = QSP_FALSE;
    qspToDisableCodeExec = QSP_FALSE;
    for (i = 0; i < QSP_CALL_DUMMY; ++i)
        qspCallbacks[i] = 0;
}

void qspSetCallback(int type, QSP_CALLBACK func)
{
    qspCallbacks[type] = func;
}

void qspCallDebug(QSPString str)
{
    /* Jump into the debugger */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_DEBUG])
    {
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniStr = qspToJavaString(javaEnv, str);

        qspPrepareCallback(&state, QSP_FALSE, QSP_FALSE);

        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_DEBUG], jniStr);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniStr);

        qspFinalizeCallback(&state);
        qspResetError(QSP_FALSE);
    }
}

void qspCallSetTimer(int msecs)
{
    /* Set timer interval */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SETTIMER])
    {
        JNIEnv *javaEnv = qspGetJniEnv();

        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_SETTIMER], msecs);
        qspFinalizeCallback(&state);
    }
}

void qspCallRefreshInt(QSP_BOOL isForced)
{
    /* Refresh UI to show the latest state */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_REFRESHINT])
    {
        JNIEnv *javaEnv = qspGetJniEnv();

        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_REFRESHINT], isForced);
        qspFinalizeCallback(&state);
    }
}

void qspCallSetInputStrText(QSPString text)
{
    /* Set value of the text input control */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SETINPUTSTRTEXT])
    {
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniText = qspToJavaString(javaEnv, text);

        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);

        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_SETINPUTSTRTEXT], jniText);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniText);

        qspFinalizeCallback(&state);
    }
}

void qspCallSystem(QSPString cmd)
{
    /* Execute system call */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SYSTEM])
    {
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniCmd = qspToJavaString(javaEnv, cmd);

        qspPrepareCallback(&state, QSP_FALSE, QSP_FALSE);

        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_SYSTEM], jniCmd);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniCmd);

        qspFinalizeCallback(&state);
    }
}

void qspCallOpenGame(QSPString file, QSP_BOOL isNewGame)
{
    /* Open game file */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_OPENGAME])
    {
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniFile = qspToJavaString(javaEnv, file);

        qspPrepareCallback(&state, QSP_FALSE, QSP_FALSE);

        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_OPENGAME], jniFile, isNewGame);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniFile);

        qspFinalizeCallback(&state);
    }
}

void qspCallOpenGameStatus(QSPString file)
{
    /* Open game state (showing the dialog to choose a file) */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_OPENGAMESTATUS])
    {
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniFile = qspToJavaString(javaEnv, file);

        qspPrepareCallback(&state, QSP_FALSE, QSP_FALSE);

        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_OPENGAMESTATUS], jniFile);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniFile);

        qspFinalizeCallback(&state);
    }
}

void qspCallSaveGameStatus(QSPString file)
{
    /* Save game state (showing the dialog to choose a file) */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SAVEGAMESTATUS])
    {
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniFile = qspToJavaString(javaEnv, file);

        qspPrepareCallback(&state, QSP_FALSE, QSP_FALSE);

        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_SAVEGAMESTATUS], jniFile);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniFile);

        qspFinalizeCallback(&state);
    }
}

void qspCallShowMessage(QSPString text)
{
    /* Show a message */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SHOWMSGSTR])
    {
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniText = qspToJavaString(javaEnv, text);

        qspPrepareCallback(&state, QSP_TRUE, QSP_TRUE);

        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_SHOWMSGSTR], jniText);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniText);

        qspFinalizeCallback(&state);
    }
}

int qspCallShowMenu(QSPListItem *items, int count)
{
    /* Show a menu */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SHOWMENU])
    {
        int i, index;
        JNIListItem *jniItems;
        jobjectArray jniMenuArray;
        JNIEnv *javaEnv = qspGetJniEnv();

        qspPrepareCallback(&state, QSP_TRUE, QSP_TRUE);

        /* Allocate an array */
        jniItems = (JNIListItem *)malloc(count * sizeof(JNIListItem));
        jniMenuArray = (*javaEnv)->NewObjectArray(javaEnv, count, qspListItemClass, 0);
        for (i = 0; i < count; ++i)
        {
            jniItems[i] = qspToJavaListItem(javaEnv, items[i].Image, items[i].Name);
            (*javaEnv)->SetObjectArrayElement(javaEnv, jniMenuArray, i, jniItems[i].ListItem);
        }

        /* Process user input */
        index = (*javaEnv)->CallIntMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_SHOWMENU], jniMenuArray);

        /* Deallocate the resources */
        for (i = 0; i < count; ++i)
            qspReleaseJavaListItem(javaEnv, jniItems + i);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniMenuArray);
        free(jniItems);

        qspFinalizeCallback(&state);
        return index;
    }
    return -1;
}

void qspCallShowPicture(QSPString file)
{
    /* Show an image */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SHOWIMAGE])
    {
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniFile = qspToJavaString(javaEnv, file);

        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);

        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_SHOWIMAGE], jniFile);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniFile);

        qspFinalizeCallback(&state);
    }
}

void qspCallShowWindow(int type, QSP_BOOL toShow)
{
    /* Show (hide) a region of the UI */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SHOWWINDOW])
    {
        JNIEnv *javaEnv = qspGetJniEnv();

        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_SHOWWINDOW], type, toShow);
        qspFinalizeCallback(&state);
    }
}

void qspCallPlayFile(QSPString file, int volume)
{
    /* Start playing a music file */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_PLAYFILE])
    {
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniFile = qspToJavaString(javaEnv, file);

        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);

        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_PLAYFILE], jniFile, volume);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniFile);

        qspFinalizeCallback(&state);
    }
}

QSP_BOOL qspCallIsPlayingFile(QSPString file)
{
    /* Check whether a file is still playing */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_ISPLAYINGFILE])
    {
        QSP_BOOL isPlaying;
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniFile = qspToJavaString(javaEnv, file);

        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);

        isPlaying = (QSP_BOOL)(*javaEnv)->CallBooleanMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_ISPLAYINGFILE], jniFile);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniFile);

        qspFinalizeCallback(&state);
        return isPlaying;
    }
    return QSP_FALSE;
}

void qspCallCloseFile(QSPString file)
{
    /* Stop playing a file */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_CLOSEFILE])
    {
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniFile = qspToJavaString(javaEnv, file);

        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);

        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_CLOSEFILE], jniFile);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniFile);

        qspFinalizeCallback(&state);
    }
}

void qspCallSleep(int msecs)
{
    /* Wait for the specified number of milliseconds */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_SLEEP])
    {
        JNIEnv *javaEnv = qspGetJniEnv();

        qspPrepareCallback(&state, QSP_TRUE, QSP_TRUE);
        (*javaEnv)->CallVoidMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_SLEEP], msecs);
        qspFinalizeCallback(&state);
    }
}

int qspCallGetMSCount(void)
{
    /* Get the number of milliseconds since the last call of this function */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_GETMSCOUNT])
    {
        int count;
        JNIEnv *javaEnv = qspGetJniEnv();

        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);
        count = (*javaEnv)->CallIntMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_GETMSCOUNT]);
        qspFinalizeCallback(&state);
        return count;
    }
    return 0;
}

QSPString qspCallInputBox(QSPString text)
{
    /* Get input from the user */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_INPUTBOX])
    {
        QSPString res;
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniRes, jniText = qspToJavaString(javaEnv, text);

        qspPrepareCallback(&state, QSP_TRUE, QSP_TRUE);

        jniRes = (*javaEnv)->CallObjectMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_INPUTBOX], jniText);
        res = qspFromJavaString(javaEnv, jniRes);

        (*javaEnv)->DeleteLocalRef(javaEnv, jniText);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniRes);

        qspFinalizeCallback(&state);
        return res;
    }
    return qspNullString;
}

QSPString qspCallVersion(QSPString param)
{
    /* Get info from the player */
    QSPCallState state;
    if (qspCallbacks[QSP_CALL_VERSION])
    {
        QSPString res;
        JNIEnv *javaEnv = qspGetJniEnv();
        jstring jniRes, jniParam = qspToJavaString(javaEnv, param);

        qspPrepareCallback(&state, QSP_TRUE, QSP_FALSE);

        jniRes = (*javaEnv)->CallObjectMethod(javaEnv, qspApiObject, qspCallbacks[QSP_CALL_VERSION], jniParam);
        res = qspFromJavaString(javaEnv, jniRes);

        (*javaEnv)->DeleteLocalRef(javaEnv, jniParam);
        (*javaEnv)->DeleteLocalRef(javaEnv, jniRes);

        qspFinalizeCallback(&state);
        return res;
    }
    return qspCopyToNewText(QSP_STATIC_STR(QSP_VER));
}

#endif
