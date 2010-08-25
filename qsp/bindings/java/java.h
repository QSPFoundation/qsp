#include "../../qsp.h"

#ifndef QSP_JAVADEFINES
	#define QSP_JAVADEFINES

#ifdef __cplusplus
	typedef int (*QSP_CALLBACK)(...);
#else
	typedef int (*QSP_CALLBACK)();
#endif

void QSPIsInCallBack(QSP_BOOL *res);
void QSPEnableDebugMode(QSP_BOOL isDebug);
void QSPGetCurStateData(QSP_CHAR **loc, int *actIndex, int *line);
void QSPGetVersion(const QSP_CHAR* *res);
void QSPGetCompiledDateTime(QSP_CHAR *res);
void QSPGetFullRefreshCount(int *res);
void QSPGetQstFullPath(const QSP_CHAR* *res);
void QSPGetCurLoc(const QSP_CHAR* *res);
void QSPGetMainDesc(const QSP_CHAR* *res);
void QSPIsMainDescChanged(QSP_BOOL *res);
void QSPGetVarsDesc(const QSP_CHAR* *res);
void QSPIsVarsDescChanged(QSP_BOOL *res);
void QSPGetExprValue(QSP_BOOL *res, const QSP_CHAR *expr, QSP_BOOL *isString, int *numVal, QSP_CHAR *strVal, int strValBufSize);
void QSPSetInputStrText(const QSP_CHAR *val);
void QSPGetActionsCount(int* res);
void QSPGetActionData(int ind, QSP_CHAR **image, QSP_CHAR **desc);
void QSPExecuteSelActionCode(QSP_BOOL *res, QSP_BOOL isRefresh);
void QSPSetSelActionIndex(QSP_BOOL *res, int ind, QSP_BOOL isRefresh);
void QSPGetSelActionIndex(int *res);
void QSPIsActionsChanged(QSP_BOOL *res);
void QSPGetObjectsCount(int *res);
void QSPGetObjectData(int ind, QSP_CHAR **image, QSP_CHAR **desc);
void QSPSetSelObjectIndex(QSP_BOOL *res, int ind, QSP_BOOL isRefresh);
void QSPGetSelObjectIndex(int* res);
void QSPIsObjectsChanged(QSP_BOOL *res);
void QSPShowWindow(int type, QSP_BOOL isShow);
void QSPGetVarValuesCount(QSP_BOOL *res, const QSP_CHAR *name, int *count);
void QSPGetVarValues(QSP_BOOL *res, const QSP_CHAR *name, int ind, int *numVal, QSP_CHAR **strVal);
void QSPGetMaxVarsCount(int *res);
void QSPGetVarNameByIndex(QSP_BOOL *res, int index, QSP_CHAR **name);
void QSPExecString(QSP_BOOL *res, const QSP_CHAR *s, QSP_BOOL isRefresh);
void QSPExecLocationCode(QSP_BOOL *res, const QSP_CHAR *name, QSP_BOOL isRefresh);
void QSPExecCounter(QSP_BOOL *res, QSP_BOOL isRefresh);
void QSPExecUserInput(QSP_BOOL *res, QSP_BOOL isRefresh);
void QSPGetLastErrorData(int *errorNum, QSP_CHAR **errorLoc, int *errorActIndex, int *errorLine);
void QSPGetErrorDesc(QSP_CHAR *res, int errorNum);
void QSPLoadGameWorld(QSP_BOOL *res, const QSP_CHAR *fileName);
void QSPLoadGameWorldFromData(QSP_BOOL *res, const char *data, int dataSize, const QSP_CHAR *fileName);
void QSPSaveGame(QSP_BOOL *res, const QSP_CHAR *fileName, QSP_BOOL isRefresh);
void QSPSaveGameAsString(QSP_BOOL *res, QSP_CHAR *strBuf, int strBufSize, int *realSize, QSP_BOOL isRefresh);
void QSPOpenSavedGame(QSP_BOOL *res, const QSP_CHAR *fileName, QSP_BOOL isRefresh);
void QSPOpenSavedGameFromString(QSP_BOOL *res, const QSP_CHAR *str, QSP_BOOL isRefresh);
void QSPRestartGame(QSP_BOOL *res, QSP_BOOL isRefresh);
void QSPSelectMenuItem(int index);
//void QSPSetCallBack(int type, QSP_CALLBACK func);
void QSPInit();
void QSPDeInit();

#endif
