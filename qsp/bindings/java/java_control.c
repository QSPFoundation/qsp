/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../../declarations.h"

#ifdef _JAVA_BINDING

#include "../../actions.h"
#include "../../callbacks.h"
#include "../../common.h"
#include "../../errors.h"
#include "../../game.h"
#include "../../locations.h"
#include "../../mathops.h"
#include "../../menu.h"
#include "../../objects.h"
#include "../../statements.h"
#include "../../text.h"
#include "../../time.h"
#include "../../tuples.h"
#include "../../variables.h"
#include "../../variant.h"

JavaVM *qspJvm;
jclass qspApiClass;
jobject qspApiObject;

jclass qspListItemClass;
jclass qspObjectItemClass;
jclass qspExecutionStateClass;
jclass qspErrorInfoClass;

jstring qspToJavaString(JNIEnv *env, QSPString str)
{
    return (*env)->NewString(env, (jchar *)str.Str, qspStrLen(str));
}

QSPString qspFromJavaString(JNIEnv *env, jstring str)
{
    jsize length;
    jchar *chars;
    QSPString res;
    length = (*env)->GetStringLength(env, str);
    chars = (jchar *)(*env)->GetStringChars(env, str, 0);
    res = qspCopyToNewText(qspStringFromLen(chars, length));
    (*env)->ReleaseStringChars(env, str, chars);
    return res;
}

JNIListItem qspToJavaListItem(JNIEnv *env, QSPString name, QSPString image)
{
    JNIListItem res;
    jfieldID fieldId;
    jobject jniListItem = (*env)->AllocObject(env, qspListItemClass);

    res.ListItem = jniListItem;
    res.Name = qspToJavaString(env, name);
    res.Title = 0;
    res.Image = qspToJavaString(env, image);

    fieldId = (*env)->GetFieldID(env, qspListItemClass , "name", "Ljava/lang/String;");
    (*env)->SetObjectField(env, jniListItem, fieldId, res.Name);

    fieldId = (*env)->GetFieldID(env, qspListItemClass , "image", "Ljava/lang/String;");
    (*env)->SetObjectField(env, jniListItem, fieldId, res.Image);

    return res;
}

JNIListItem qspToJavaObjectItem(JNIEnv *env, QSPString name, QSPString title, QSPString image)
{
    JNIListItem res;
    jfieldID fieldId;
    jobject jniListItem = (*env)->AllocObject(env, qspObjectItemClass);

    res.ListItem = jniListItem;
    res.Name = qspToJavaString(env, name);
    res.Title = qspToJavaString(env, title);
    res.Image = qspToJavaString(env, image);

    fieldId = (*env)->GetFieldID(env, qspObjectItemClass , "name", "Ljava/lang/String;");
    (*env)->SetObjectField(env, jniListItem, fieldId, res.Name);

    fieldId = (*env)->GetFieldID(env, qspObjectItemClass , "title", "Ljava/lang/String;");
    (*env)->SetObjectField(env, jniListItem, fieldId, res.Title);

    fieldId = (*env)->GetFieldID(env, qspObjectItemClass , "image", "Ljava/lang/String;");
    (*env)->SetObjectField(env, jniListItem, fieldId, res.Image);

    return res;
}

void qspReleaseJavaListItem(JNIEnv *env, JNIListItem *listItem)
{
    (*env)->DeleteLocalRef(env, listItem->Name);
    (*env)->DeleteLocalRef(env, listItem->Title);
    (*env)->DeleteLocalRef(env, listItem->Image);
    (*env)->DeleteLocalRef(env, listItem->ListItem);
}

/* ------------------------------------------------------------ */
/* Debugger */

/* Enable the debugger */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_enableDebugMode(JNIEnv *env, jobject api, jboolean isDebug)
{
    qspIsDebug = (QSP_BOOL)isDebug;
}
/* Get current execution state */
JNIEXPORT jobject JNICALL Java_com_libqsp_jni_QSPLib_getCurrentState(JNIEnv *env, jobject api)
{
    jfieldID fieldId;
    QSPString locName;
    jobject jniExecutionState = (*env)->AllocObject(env, qspExecutionStateClass);

    locName = ((qspRealCurLoc >= 0 && qspRealCurLoc < qspLocsCount) ? qspLocs[qspRealCurLoc].Name : qspNullString);

    fieldId = (*env)->GetFieldID(env, qspExecutionStateClass , "loc", "Ljava/lang/String;");
    (*env)->SetObjectField(env, jniExecutionState, fieldId, qspToJavaString(env, locName));

    fieldId = (*env)->GetFieldID(env, qspExecutionStateClass , "actIndex", "I");
    (*env)->SetIntField(env, jniExecutionState, fieldId, qspRealActIndex);

    fieldId = (*env)->GetFieldID(env, qspExecutionStateClass , "lineNum", "I");
    (*env)->SetIntField(env, jniExecutionState, fieldId, qspRealLineNum);

    return jniExecutionState;
}
/* ------------------------------------------------------------ */
/* Version details */

/* Get version of the libqsp */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getVersion(JNIEnv *env, jobject api)
{
    return qspToJavaString(env, QSP_STATIC_STR(QSP_VER));
}
/* Get build datetime of the libqsp */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getCompiledDateTime(JNIEnv *env, jobject api)
{
    return qspToJavaString(env, QSP_STATIC_STR(QSP_FMT(__DATE__) QSP_FMT(", ") QSP_FMT(__TIME__)));
}
/* ------------------------------------------------------------ */
/* Main description */

/* Get text of the main description */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getMainDesc(JNIEnv *env, jobject api)
{
    return qspToJavaString(env, qspBufStringToString(qspCurDesc));
}
/* ------------------------------------------------------------ */
/* Additional description */

/* Get text of the additional description */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getVarsDesc(JNIEnv *env, jobject api)
{
    return qspToJavaString(env, qspBufStringToString(qspCurVars));
}
/* ------------------------------------------------------------ */
/* Synchronize the value of the text input control */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_setInputStrText(JNIEnv *env, jobject api, jstring val)
{
    QSPString str = qspFromJavaString(env, val);
    qspUpdateText(&qspCurInput, str);
    qspFreeString(&str);
}
/* ------------------------------------------------------------ */
/* Actions */

/* Get current actions */
JNIEXPORT jobjectArray JNICALL Java_com_libqsp_jni_QSPLib_getActions(JNIEnv *env, jobject api)
{
    int i;
    JNIListItem item;
    jobjectArray res = (*env)->NewObjectArray(env, qspCurActsCount, qspListItemClass, 0);
    for (i = 0; i < qspCurActsCount; ++i)
    {
        item = qspToJavaListItem(env, qspCurActions[i].Desc, qspCurActions[i].Image);
        (*env)->SetObjectArrayElement(env, res, i, item.ListItem);
    }
    return res;
}
/* Set index of the selected action */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_setSelActIndex(JNIEnv *env, jobject api, jint ind, jboolean toRefreshUI)
{
    if (ind >= 0 && ind < qspCurActsCount && ind != qspCurSelAction)
    {
        qspPrepareExecution(QSP_FALSE);
        qspCurSelAction = ind;
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_ACTSELECTED), 0, 0);
        if (qspErrorNum) return JNI_FALSE;
        if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    }
    return JNI_TRUE;
}
/* Execute the selected action */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_execSelAction(JNIEnv *env, jobject api, jboolean toRefreshUI)
{
    if (qspCurSelAction >= 0)
    {
        qspPrepareExecution(QSP_FALSE);
        qspExecAction(qspCurSelAction);
        if (qspErrorNum) return JNI_FALSE;
        if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    }
    return JNI_TRUE;
}
/* Get index of the selected action */
JNIEXPORT jint JNICALL Java_com_libqsp_jni_QSPLib_getSelActIndex(JNIEnv *env, jobject api)
{
    return qspCurSelAction;
}
/* ------------------------------------------------------------ */
/* Objects */

/* Get current objects */
JNIEXPORT jobjectArray JNICALL Java_com_libqsp_jni_QSPLib_getObjects(JNIEnv *env, jobject api)
{
    int i;
    JNIListItem item;
    QSPObjectItem obj;
    jobjectArray res = (*env)->NewObjectArray(env, qspCurObjsCount, qspObjectItemClass, 0);
    for (i = 0; i < qspCurObjsCount; ++i)
    {
        if (qspGetObjectInfoByIndex(i, &obj))
        {
            item = qspToJavaObjectItem(env, obj.Name, obj.Title, obj.Image);
            (*env)->SetObjectArrayElement(env, res, i, item.ListItem);
        }
    }
    return res;
}
/* Set index of the selected object */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_setSelObjIndex(JNIEnv *env, jobject api, jint ind, jboolean toRefreshUI)
{
    if (ind >= 0 && ind < qspCurObjsCount && ind != qspCurSelObject)
    {
        qspPrepareExecution(QSP_FALSE);
        qspCurSelObject = ind;
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_OBJSELECTED), 0, 0);
        if (qspErrorNum) return JNI_FALSE;
        if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    }
    return JNI_TRUE;
}
/* Get index of the selected object */
JNIEXPORT jint JNICALL Java_com_libqsp_jni_QSPLib_getSelObjIndex(JNIEnv *env, jobject api)
{
    return qspCurSelObject;
}
/* ------------------------------------------------------------ */
/* Get the state of changes per window */
JNIEXPORT jint JNICALL Java_com_libqsp_jni_QSPLib_getWindowsChangedState(JNIEnv *env, jobject api)
{
    return qspCurWindowsChangedState;
}
/* Synchronize visibility of a region of the UI */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_showWindow(JNIEnv *env, jobject api, jint type, jboolean toShow)
{
    qspSetWindowState(type, toShow);
}
/* ------------------------------------------------------------ */
/* Variables */

/* Get the number of items in an array */
JNIEXPORT jint JNICALL Java_com_libqsp_jni_QSPLib_getVarValuesCount(JNIEnv *env, jobject api, jstring name)
{
    QSPString varName = qspFromJavaString(env, name);
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    qspFreeString(&varName);
    if (var) return var->ValsCount;
    return 0;
}
/* Get index of an item by string */
JNIEXPORT jint JNICALL Java_com_libqsp_jni_QSPLib_getVarIndexByString(JNIEnv *env, jobject api, jstring name, jstring str)
{
    QSPString varName = qspFromJavaString(env, name);
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    qspFreeString(&varName);
    if (var)
    {
        int arrIndex;
        QSPString indexStr = qspFromJavaString(env, str);
        QSPVariant index = qspStrVariant(indexStr, QSP_TYPE_STR);
        arrIndex = qspGetVarIndex(var, index, QSP_FALSE);
        qspFreeString(&indexStr);
        return arrIndex;
    }
    return -1;
}
/* Get numeric value of the specified array item */
JNIEXPORT jlong JNICALL Java_com_libqsp_jni_QSPLib_getNumVarValue(JNIEnv *env, jobject api, jstring name, jint ind)
{
    QSPString varName = qspFromJavaString(env, name);
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    qspFreeString(&varName);
    if (var && ind >= 0 && ind < var->ValsCount)
    {
        QSPVariant *val = var->Values + ind;
        if (QSP_ISNUM(val->Type)) return QSP_PNUM(val);
    }
    return 0;
}
/* Get string value of the specified array item */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getStrVarValue(JNIEnv *env, jobject api, jstring name, jint ind)
{
    QSPString varName = qspFromJavaString(env, name);
    QSPVar *var = qspVarReference(varName, QSP_FALSE);
    qspFreeString(&varName);
    if (var && ind >= 0 && ind < var->ValsCount)
    {
        QSPVariant *val = var->Values + ind;
        if (QSP_ISSTR(val->Type)) return qspToJavaString(env, QSP_PSTR(val));
    }
    return qspToJavaString(env, qspNullString);
}
/* ------------------------------------------------------------ */
/* Code execution */

/* Execute a line of code */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_execString(JNIEnv *env, jobject api, jstring s, jboolean toRefreshUI)
{
    QSPString codeStr;
    qspPrepareExecution(QSP_FALSE);
    codeStr = qspFromJavaString(env, s);
    qspExecStringAsCode(codeStr);
    qspFreeString(&codeStr);
    if (qspErrorNum) return JNI_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return JNI_TRUE;
}
/* Calculate string value of an expression (causes execution of code) */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_calculateStrExpr(JNIEnv *env, jobject api, jstring s, jboolean toRefreshUI)
{
    jstring res;
    QSPString codeStr;
    QSPVariant value;
    qspPrepareExecution(QSP_FALSE);
    codeStr = qspFromJavaString(env, s);
    qspPrepareStringToExecution(&codeStr);
    value = qspCalculateExprValue(codeStr);
    qspFreeString(&codeStr);
    if (qspErrorNum) return qspToJavaString(env, qspNullString);
    qspConvertVariantTo(&value, QSP_TYPE_STR);
    res = qspToJavaString(env, QSP_STR(value));
    qspFreeVariant(&value);
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return res;
}
/* Calculate numeric value of an expression (causes execution of code) */
JNIEXPORT jlong JNICALL Java_com_libqsp_jni_QSPLib_calculateNumExpr(JNIEnv *env, jobject api, jstring s, jboolean toRefreshUI)
{
    QSP_BIGINT res;
    QSPString codeStr;
    QSPVariant value;
    qspPrepareExecution(QSP_FALSE);
    codeStr = qspFromJavaString(env, s);
    qspPrepareStringToExecution(&codeStr);
    value = qspCalculateExprValue(codeStr);
    qspFreeString(&codeStr);
    if (qspErrorNum) return 0;
    if (!qspConvertVariantTo(&value, QSP_TYPE_NUM))
    {
        qspFreeVariant(&value);
        return 0;
    }
    res = QSP_NUM(value);
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return res;
}
/* Execute code of the specified location */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_execLocationCode(JNIEnv *env, jobject api, jstring name, jboolean toRefreshUI)
{
    QSPString locName;
    qspPrepareExecution(QSP_FALSE);
    locName = qspFromJavaString(env, name);
    qspExecLocByNameWithArgs(locName, 0, 0, QSP_TRUE, 0);
    qspFreeString(&locName);
    if (qspErrorNum) return JNI_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return JNI_TRUE;
}
/* Execute code of the special "COUNTER" location */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_execCounter(JNIEnv *env, jobject api, jboolean toRefreshUI)
{
    if (!qspIsInCallback)
    {
        qspPrepareExecution(QSP_FALSE);
        qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_COUNTER), 0, 0);
        if (qspErrorNum) return JNI_FALSE;
        if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    }
    return JNI_TRUE;
}
/* Execute code of the special "USERCOM" location */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_execUserInput(JNIEnv *env, jobject api, jboolean toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    qspExecLocByVarNameWithArgs(QSP_STATIC_STR(QSP_LOC_USERCOMMAND), 0, 0);
    if (qspErrorNum) return JNI_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return JNI_TRUE;
}
/* ------------------------------------------------------------ */
/* Errors */

/* Get details of the last error */
JNIEXPORT jobject JNICALL Java_com_libqsp_jni_QSPLib_getLastErrorData(JNIEnv *env, jobject api)
{
    jfieldID fieldId;
    jobject jniErrorInfo = (*env)->AllocObject(env, qspErrorInfoClass);

    fieldId = (*env)->GetFieldID(env, qspErrorInfoClass , "errorNum", "I");
    (*env)->SetIntField(env, jniErrorInfo, fieldId, qspLastError.ErrorNum);

    fieldId = (*env)->GetFieldID(env, qspErrorInfoClass , "errorDesc", "Ljava/lang/String;");
    (*env)->SetObjectField(env, jniErrorInfo, fieldId, qspToJavaString(env, qspLastError.ErrorDesc));

    fieldId = (*env)->GetFieldID(env, qspErrorInfoClass , "locName", "Ljava/lang/String;");
    (*env)->SetObjectField(env, jniErrorInfo, fieldId, qspToJavaString(env, qspLastError.LocName));

    fieldId = (*env)->GetFieldID(env, qspErrorInfoClass , "actIndex", "I");
    (*env)->SetIntField(env, jniErrorInfo, fieldId, qspLastError.ActIndex);

    fieldId = (*env)->GetFieldID(env, qspErrorInfoClass , "topLineNum", "I");
    (*env)->SetIntField(env, jniErrorInfo, fieldId, qspLastError.TopLineNum);

    fieldId = (*env)->GetFieldID(env, qspErrorInfoClass , "intLineNum", "I");
    (*env)->SetIntField(env, jniErrorInfo, fieldId, qspLastError.IntLineNum);

    fieldId = (*env)->GetFieldID(env, qspErrorInfoClass , "intLine", "Ljava/lang/String;");
    (*env)->SetObjectField(env, jniErrorInfo, fieldId, qspToJavaString(env, qspLastError.IntLine));

    return jniErrorInfo;
}
/* Get error description by numeric code */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getErrorDesc(JNIEnv *env, jobject api, jint errorNum)
{
    return qspToJavaString(env, qspGetErrorDesc(errorNum));
}
/* ------------------------------------------------------------ */
/* Game controls */

/* Load game from data */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_loadGameWorldFromData(JNIEnv *env, jobject api, jbyteArray data, jboolean isNewGame)
{
    /* We don't execute any game code here */
    jint dataSize = (*env)->GetArrayLength(env, data);
    jbyte *arrayData = (*env)->GetByteArrayElements(env, data, 0);
    QSP_BOOL res = qspOpenGame(arrayData, dataSize, isNewGame);
    (*env)->ReleaseByteArrayElements(env, data, arrayData, JNI_ABORT);
    return res;
}
/* Save game state to a buffer */
JNIEXPORT jbyteArray JNICALL Java_com_libqsp_jni_QSPLib_saveGameAsData(JNIEnv *env, jobject api, jboolean toRefreshUI)
{
    jbyteArray res;
    void *dataBuf;
    int dataBufSize = 64 * 1024;
    qspPrepareExecution(QSP_FALSE);
    dataBuf = malloc(dataBufSize);
    while (1)
    {
        if (qspSaveGameStatus(dataBuf, &dataBufSize, QSP_TRUE))
            break;
        if (!dataBufSize)
        {
            free(dataBuf);
            return 0;
        }
        /* Happens when we passed insufficient buffer, the new value contains required buffer size */
        /* We have to reserve some extra space to account for game updates during subsequent calls */
        dataBufSize += QSP_SAVEDGAMEDATAEXTRASPACE;
        dataBuf = realloc(dataBuf, dataBufSize);
    }
    res = (*env)->NewByteArray(env, dataBufSize);
    (*env)->SetByteArrayRegion(env, res, 0, dataBufSize, (jbyte *)dataBuf);
    free(dataBuf);
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return res;
}
/* Load game state from data */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_openSavedGameFromData(JNIEnv *env, jobject api, jbyteArray data, jboolean toRefreshUI)
{
    QSP_BOOL res;
    jint dataSize;
    jbyte *arrayData;
    qspPrepareExecution(QSP_FALSE);
    dataSize = (*env)->GetArrayLength(env, data);
    arrayData = (*env)->GetByteArrayElements(env, data, 0);
    res = qspOpenGameStatus(arrayData, dataSize);
    (*env)->ReleaseByteArrayElements(env, data, arrayData, JNI_ABORT);
    if (!res) return JNI_FALSE;
    if (qspErrorNum) return JNI_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return JNI_TRUE;
}
/* Restart current game */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_restartGame(JNIEnv *env, jobject api, jboolean toRefreshUI)
{
    qspPrepareExecution(QSP_FALSE);
    if (!qspRestartGame(QSP_TRUE)) return JNI_FALSE;
    if (qspErrorNum) return JNI_FALSE;
    if (toRefreshUI) qspCallRefreshInt(QSP_FALSE);
    return JNI_TRUE;
}
/* ------------------------------------------------------------ */
/* Initialization of the engine */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_init(JNIEnv *env, jobject api)
{
    jclass clazz;

    qspInitRuntime();

    /* Get JVM references */
    (*env)->GetJavaVM(env, &qspJvm);

    clazz = (*env)->FindClass(env, "com/libqsp/jni/QSPLib");
    qspApiClass = (jclass)(*env)->NewGlobalRef(env, clazz);
    qspApiObject = (jobject)(*env)->NewGlobalRef(env, api);

    clazz = (*env)->FindClass(env, "com/libqsp/jni/QSPLib$ListItem");
    qspListItemClass = (jclass)(*env)->NewGlobalRef(env, clazz);

    clazz = (*env)->FindClass(env, "com/libqsp/jni/QSPLib$ObjectItem");
    qspObjectItemClass = (jclass)(*env)->NewGlobalRef(env, clazz);

    clazz = (*env)->FindClass(env, "com/libqsp/jni/QSPLib$ExecutionState");
    qspExecutionStateClass = (jclass)(*env)->NewGlobalRef(env, clazz);

    clazz = (*env)->FindClass(env, "com/libqsp/jni/QSPLib$ErrorInfo");
    qspErrorInfoClass = (jclass)(*env)->NewGlobalRef(env, clazz);

    /* Get references to callbacks */
    qspSetCallback(QSP_CALL_DEBUG, (*env)->GetMethodID(env, qspApiClass, "onDebug", "(Ljava/lang/String;)V"));
    qspSetCallback(QSP_CALL_ISPLAYINGFILE, (*env)->GetMethodID(env, qspApiClass, "onIsPlayingFile", "(Ljava/lang/String;)Z"));
    qspSetCallback(QSP_CALL_PLAYFILE, (*env)->GetMethodID(env, qspApiClass, "onPlayFile", "(Ljava/lang/String;I)V"));
    qspSetCallback(QSP_CALL_CLOSEFILE, (*env)->GetMethodID(env, qspApiClass, "onCloseFile", "(Ljava/lang/String;)V"));
    qspSetCallback(QSP_CALL_SHOWIMAGE, (*env)->GetMethodID(env, qspApiClass, "onShowImage", "(Ljava/lang/String;)V"));
    qspSetCallback(QSP_CALL_SHOWWINDOW, (*env)->GetMethodID(env, qspApiClass, "onShowWindow", "(IZ)V"));
    qspSetCallback(QSP_CALL_SHOWMENU, (*env)->GetMethodID(env, qspApiClass, "onShowMenu", "([Lcom/libqsp/jni/QSPLib$ListItem;)I"));
    qspSetCallback(QSP_CALL_SHOWMSGSTR, (*env)->GetMethodID(env, qspApiClass, "onShowMessage", "(Ljava/lang/String;)V"));
    qspSetCallback(QSP_CALL_REFRESHINT, (*env)->GetMethodID(env, qspApiClass, "onRefreshInt", "(ZZ)V"));
    qspSetCallback(QSP_CALL_SETTIMER, (*env)->GetMethodID(env, qspApiClass, "onSetTimer", "(I)V"));
    qspSetCallback(QSP_CALL_SETINPUTSTRTEXT, (*env)->GetMethodID(env, qspApiClass, "onSetInputStrText", "(Ljava/lang/String;)V"));
    qspSetCallback(QSP_CALL_SYSTEM, (*env)->GetMethodID(env, qspApiClass, "onSystem", "(Ljava/lang/String;)V"));
    qspSetCallback(QSP_CALL_OPENGAME, (*env)->GetMethodID(env, qspApiClass, "onOpenGame", "(Ljava/lang/String;Z)V"));
    qspSetCallback(QSP_CALL_INITGAME, (*env)->GetMethodID(env, qspApiClass, "onInitGame", "(Z)V"));
    qspSetCallback(QSP_CALL_OPENGAMESTATUS, (*env)->GetMethodID(env, qspApiClass, "onOpenGameStatus", "(Ljava/lang/String;)V"));
    qspSetCallback(QSP_CALL_SAVEGAMESTATUS, (*env)->GetMethodID(env, qspApiClass, "onSaveGameStatus", "(Ljava/lang/String;)V"));
    qspSetCallback(QSP_CALL_SLEEP, (*env)->GetMethodID(env, qspApiClass, "onSleep", "(I)V"));
    qspSetCallback(QSP_CALL_GETMSCOUNT, (*env)->GetMethodID(env, qspApiClass, "onGetMsCount", "()I"));
    qspSetCallback(QSP_CALL_INPUTBOX, (*env)->GetMethodID(env, qspApiClass, "onInputBox", "(Ljava/lang/String;)Ljava/lang/String;"));
    qspSetCallback(QSP_CALL_VERSION, (*env)->GetMethodID(env, qspApiClass, "onVersion", "(Ljava/lang/String;)Ljava/lang/String;"));
}
/* Deallocate all resources */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_terminate(JNIEnv *env, jobject api)
{
    qspTerminateRuntime();

    /* Release references */
    (*env)->DeleteGlobalRef(env, qspApiObject);
    (*env)->DeleteGlobalRef(env, qspApiClass);
    (*env)->DeleteGlobalRef(env, qspListItemClass);
    (*env)->DeleteGlobalRef(env, qspObjectItemClass);
    (*env)->DeleteGlobalRef(env, qspExecutionStateClass);
    (*env)->DeleteGlobalRef(env, qspErrorInfoClass);
}

#endif
