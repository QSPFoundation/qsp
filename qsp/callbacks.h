/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
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

#include "declarations.h"

#ifndef QSP_CALLSDEFINES
	#define QSP_CALLSDEFINES

	typedef struct
	{
		QSP_BOOL IsInCallBack;
		QSP_BOOL IsDisableCodeExec;
		QSP_BOOL IsExitOnError;
		QSP_BOOL IsMainDescChanged;
		QSP_BOOL IsVarsDescChanged;
		QSP_BOOL IsObjectsChanged;
		QSP_BOOL IsActionsChanged;
	} QSPCallState;

	extern QSP_CALLBACK qspCallBacks[QSP_CALL_DUMMY];
	extern QSP_BOOL qspIsInCallBack;
	extern QSP_BOOL qspIsDisableCodeExec;
	extern QSP_BOOL qspIsExitOnError;

	/* External functions */
	void qspSaveCallState(QSPCallState *, QSP_BOOL, QSP_BOOL);
	void qspRestoreCallState(QSPCallState *);

	void qspInitCallBacks();
	void qspSetCallBack(int, QSP_CALLBACK);
	void qspCallDebug(QSPString str);
	void qspCallSetTimer(int);
	void qspCallRefreshInt(QSP_BOOL);
	void qspCallSetInputStrText(QSPString text);
	void qspCallAddMenuItem(QSPString name, QSPString imgPath);
	void qspCallSystem(QSPString cmd);
	void qspCallOpenGame(QSPString file);
	void qspCallSaveGame(QSPString file);
	void qspCallShowMessage(QSPString text);
	int qspCallShowMenu();
	void qspCallShowPicture(QSPString file);
	void qspCallShowWindow(int, QSP_BOOL);
	void qspCallPlayFile(QSPString file, int volume);
	QSP_BOOL qspCallIsPlayingFile(QSPString file);
	void qspCallSleep(int);
	void qspCallCloseFile(QSPString file);
	void qspCallDeleteMenu();
	QSPString qspCallInputBox(QSPString text);

#endif
