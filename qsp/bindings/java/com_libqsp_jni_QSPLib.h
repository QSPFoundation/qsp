/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_libqsp_jni_QSPLib */

#ifndef _Included_com_libqsp_jni_QSPLib
#define _Included_com_libqsp_jni_QSPLib
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_init
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    terminate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_terminate
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    useCallback
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_useCallback
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    enableDebugMode
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_enableDebugMode
  (JNIEnv *, jobject, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getCurrentState
 * Signature: ()Lcom/libqsp/jni/QSPLib/ExecutionState;
 */
JNIEXPORT jobject JNICALL Java_com_libqsp_jni_QSPLib_getCurrentState
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getVersion
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getCompiledDateTime
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getCompiledDateTime
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getFullRefreshCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_libqsp_jni_QSPLib_getFullRefreshCount
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getMainDesc
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getMainDesc
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    isMainDescChanged
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_isMainDescChanged
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getVarDesc
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getVarDesc
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    isVarDescChanged
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_isVarDescChanged
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    setInputStrText
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_setInputStrText
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getActions
 * Signature: ()[Lcom/libqsp/jni/QSPLib/ListItem;
 */
JNIEXPORT jobjectArray JNICALL Java_com_libqsp_jni_QSPLib_getActions
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    setSelActIndex
 * Signature: (IZ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_setSelActIndex
  (JNIEnv *, jobject, jint, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    executeSelAction
 * Signature: (Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_executeSelAction
  (JNIEnv *, jobject, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getSelActIndex
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_libqsp_jni_QSPLib_getSelActIndex
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    isActsChanged
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_isActsChanged
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getObjects
 * Signature: ()[Lcom/libqsp/jni/QSPLib/ListItem;
 */
JNIEXPORT jobjectArray JNICALL Java_com_libqsp_jni_QSPLib_getObjects
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    setSelObjIndex
 * Signature: (IZ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_setSelObjIndex
  (JNIEnv *, jobject, jint, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getSelObjIndex
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_libqsp_jni_QSPLib_getSelObjIndex
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    isObjsChanged
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_isObjsChanged
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    showWindow
 * Signature: (IZ)V
 */
JNIEXPORT void JNICALL Java_com_libqsp_jni_QSPLib_showWindow
  (JNIEnv *, jobject, jint, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getVarValuesCount
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_libqsp_jni_QSPLib_getVarValuesCount
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getVarIndexByString
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_libqsp_jni_QSPLib_getVarIndexByString
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getNumVarValue
 * Signature: (Ljava/lang/String;I)J
 */
JNIEXPORT jlong JNICALL Java_com_libqsp_jni_QSPLib_getNumVarValue
  (JNIEnv *, jobject, jstring, jint);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getStrVarValue
 * Signature: (Ljava/lang/String;I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getStrVarValue
  (JNIEnv *, jobject, jstring, jint);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    execString
 * Signature: (Ljava/lang/String;Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_execString
  (JNIEnv *, jobject, jstring, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    calculateStrExpr
 * Signature: (Ljava/lang/String;Z)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_calculateStrExpr
  (JNIEnv *, jobject, jstring, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    calculateNumExpr
 * Signature: (Ljava/lang/String;Z)J
 */
JNIEXPORT jlong JNICALL Java_com_libqsp_jni_QSPLib_calculateNumExpr
  (JNIEnv *, jobject, jstring, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    execLocationCode
 * Signature: (Ljava/lang/String;Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_execLocationCode
  (JNIEnv *, jobject, jstring, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    execCounter
 * Signature: (Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_execCounter
  (JNIEnv *, jobject, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    execUserInput
 * Signature: (Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_execUserInput
  (JNIEnv *, jobject, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getLastErrorData
 * Signature: ()Lcom/libqsp/jni/QSPLib/ErrorInfo;
 */
JNIEXPORT jobject JNICALL Java_com_libqsp_jni_QSPLib_getLastErrorData
  (JNIEnv *, jobject);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    getErrorDesc
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_libqsp_jni_QSPLib_getErrorDesc
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    loadGameWorldFromData
 * Signature: ([BZ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_loadGameWorldFromData
  (JNIEnv *, jobject, jbyteArray, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    saveGameAsData
 * Signature: (Z)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_libqsp_jni_QSPLib_saveGameAsData
  (JNIEnv *, jobject, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    openSavedGameFromData
 * Signature: ([BZ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_openSavedGameFromData
  (JNIEnv *, jobject, jbyteArray, jboolean);

/*
 * Class:     com_libqsp_jni_QSPLib
 * Method:    restartGame
 * Signature: (Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_libqsp_jni_QSPLib_restartGame
  (JNIEnv *, jobject, jboolean);

#ifdef __cplusplus
}
#endif
#endif
