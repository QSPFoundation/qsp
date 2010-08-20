// Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru)
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

#include "callbacks_gui.h"

wxString QSPCallBacks::m_gamePath;
QSPFrame *QSPCallBacks::m_frame;
bool QSPCallBacks::m_isHtml;
FMOD_SYSTEM *QSPCallBacks::m_sys;
QSPSounds QSPCallBacks::m_sounds;
float QSPCallBacks::m_volumeCoeff;

void QSPCallBacks::Init(QSPFrame *frame)
{
	m_frame = frame;
	m_volumeCoeff = 1.0;

	FMOD_System_Create(&m_sys);
	wxString soundPath(QSPTools::GetAppPath() + QSP_SOUNDPLUGINS);
	FMOD_System_SetPluginPath(m_sys, wxConvFile.cWX2MB(soundPath.c_str()));
	#ifdef __WXMSW__
		FMOD_System_SetOutput(m_sys, FMOD_OUTPUTTYPE_DSOUND);
	#elif __WXOSX__
		FMOD_System_SetOutput(m_sys, FMOD_OUTPUTTYPE_COREAUDIO);
	#else
		FMOD_System_SetOutput(m_sys, FMOD_OUTPUTTYPE_ALSA);
	#endif
	FMOD_System_Init(m_sys, 32, FMOD_INIT_NORMAL, 0);

	QSPSetCallBack(QSP_CALL_SETTIMER, (QSP_CALLBACK)&SetTimer);
	QSPSetCallBack(QSP_CALL_REFRESHINT, (QSP_CALLBACK)&RefreshInt);
	QSPSetCallBack(QSP_CALL_SETINPUTSTRTEXT, (QSP_CALLBACK)&SetInputStrText);
	QSPSetCallBack(QSP_CALL_ISPLAYINGFILE, (QSP_CALLBACK)&IsPlay);
	QSPSetCallBack(QSP_CALL_PLAYFILE, (QSP_CALLBACK)&PlayFile);
	QSPSetCallBack(QSP_CALL_CLOSEFILE, (QSP_CALLBACK)&CloseFile);
	QSPSetCallBack(QSP_CALL_SHOWMSGSTR, (QSP_CALLBACK)&Msg);
	QSPSetCallBack(QSP_CALL_SLEEP, (QSP_CALLBACK)&Sleep);
	QSPSetCallBack(QSP_CALL_GETMSCOUNT, (QSP_CALLBACK)&GetMSCount);
	QSPSetCallBack(QSP_CALL_DELETEMENU, (QSP_CALLBACK)&DeleteMenu);
	QSPSetCallBack(QSP_CALL_ADDMENUITEM, (QSP_CALLBACK)&AddMenuItem);
	QSPSetCallBack(QSP_CALL_SHOWMENU, (QSP_CALLBACK)&ShowMenu);
	QSPSetCallBack(QSP_CALL_INPUTBOX, (QSP_CALLBACK)&Input);
	QSPSetCallBack(QSP_CALL_SHOWIMAGE, (QSP_CALLBACK)&ShowImage);
	QSPSetCallBack(QSP_CALL_SHOWWINDOW, (QSP_CALLBACK)&ShowPane);
	QSPSetCallBack(QSP_CALL_OPENGAMESTATUS, (QSP_CALLBACK)&OpenGameStatus);
	QSPSetCallBack(QSP_CALL_SAVEGAMESTATUS, (QSP_CALLBACK)&SaveGameStatus);
}

void QSPCallBacks::DeInit()
{
	CloseFile(0);
	FMOD_System_Close(m_sys);
	FMOD_System_Release(m_sys);
}

void QSPCallBacks::SetTimer(int msecs)
{
	if (m_frame->GetIsQuit()) return;
	if (msecs)
		m_frame->GetTimer()->Start(msecs);
	else
		m_frame->GetTimer()->Stop();
}

void QSPCallBacks::RefreshInt(QSP_BOOL isRedraw)
{
	static int oldFullRefreshCount = 0;
	int i, numVal;
	bool isScroll, isCanSave;
	QSP_CHAR *strVal, *imgPath;
	if (m_frame->GetIsQuit()) return;
	// -------------------------------
	UpdateGamePath();
	// -------------------------------
	const QSP_CHAR *mainDesc = QSPGetMainDesc();
	const QSP_CHAR *varsDesc = QSPGetVarsDesc();
	// -------------------------------
	isScroll = !(QSPGetVarValues(QSP_FMT("DISABLESCROLL"), 0, &numVal, &strVal) && numVal);
	isCanSave = !(QSPGetVarValues(QSP_FMT("NOSAVE"), 0, &numVal, &strVal) && numVal);
	m_isHtml = QSPGetVarValues(QSP_FMT("USEHTML"), 0, &numVal, &strVal) && numVal;
	// -------------------------------
	m_frame->GetVars()->SetIsHtml(m_isHtml, isScroll);
	if (QSPIsVarsDescChanged())
		m_frame->GetVars()->SetText(wxString(varsDesc), isScroll);
	// -------------------------------
	int fullRefreshCount = QSPGetFullRefreshCount();
	if (oldFullRefreshCount != fullRefreshCount)
	{
		isScroll = false;
		oldFullRefreshCount = fullRefreshCount;
	}
	m_frame->GetDesc()->SetIsHtml(m_isHtml, isScroll);
	if (QSPIsMainDescChanged())
		m_frame->GetDesc()->SetText(wxString(mainDesc), isScroll);
	// -------------------------------
	m_frame->GetActions()->SetIsHtml(m_isHtml);
	m_frame->GetActions()->SetIsShowNums(m_frame->GetIsShowHotkeys());
	if (QSPIsActionsChanged())
	{
		int actionsCount = QSPGetActionsCount();
		m_frame->GetActions()->BeginItems();
		for (i = 0; i < actionsCount; ++i)
		{
			QSPGetActionData(i, &imgPath, &strVal);
			m_frame->GetActions()->AddItem(wxString(imgPath), wxString(strVal));
		}
		m_frame->GetActions()->EndItems();
	}
	m_frame->GetActions()->SetSelection(QSPGetSelActionIndex());
	m_frame->GetObjects()->SetIsHtml(m_isHtml);
	if (QSPIsObjectsChanged())
	{
		int objectsCount = QSPGetObjectsCount();
		m_frame->GetObjects()->BeginItems();
		for (i = 0; i < objectsCount; ++i)
		{
			QSPGetObjectData(i, &imgPath, &strVal);
			m_frame->GetObjects()->AddItem(wxString(imgPath), wxString(strVal));
		}
		m_frame->GetObjects()->EndItems();
	}
	m_frame->GetObjects()->SetSelection(QSPGetSelObjectIndex());
	// -------------------------------
	if (QSPGetVarValues(QSP_FMT("BACKIMAGE"), 0, &numVal, &strVal) && strVal && *strVal)
		m_frame->GetDesc()->LoadBackImage(m_gamePath + strVal);
	else
		m_frame->GetDesc()->LoadBackImage(wxEmptyString);
	// -------------------------------
	m_frame->ApplyParams();
	if (isRedraw)
	{
		m_frame->EnableControls(false, true);
		m_frame->Update();
		wxTheApp->Yield(true);
		if (m_frame->GetIsQuit()) return;
		m_frame->EnableControls(true, true);
	}
	m_frame->GetGameMenu()->Enable(ID_SAVEGAMESTAT, isCanSave);
}

void QSPCallBacks::SetInputStrText(const QSP_CHAR *text)
{
	if (m_frame->GetIsQuit()) return;
	m_frame->GetInput()->SetText(wxString(text));
}

QSP_BOOL QSPCallBacks::IsPlay(const QSP_CHAR *file)
{
	FMOD_BOOL playing = FALSE;
	QSPSounds::iterator elem = m_sounds.find(wxFileName(file, wxPATH_DOS).GetFullPath().Upper());
	if (elem != m_sounds.end())
		FMOD_Channel_IsPlaying(((QSPSound *)(&elem->second))->Channel, &playing);
	return (playing == TRUE);
}

void QSPCallBacks::CloseFile(const QSP_CHAR *file)
{
	if (file)
	{
		QSPSounds::iterator elem = m_sounds.find(wxFileName(file, wxPATH_DOS).GetFullPath().Upper());
		if (elem != m_sounds.end())
		{
			((QSPSound *)(&elem->second))->Free();
			m_sounds.erase(elem);
		}
	}
	else
	{
		for (QSPSounds::iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
			((QSPSound *)(&i->second))->Free();
		m_sounds.clear();
	}
}

void QSPCallBacks::PlayFile(const QSP_CHAR *file, int volume)
{
	FMOD_SOUND *newSound;
	FMOD_CHANNEL *newChannel;
	QSPSound snd;
	if (SetVolume(file, volume)) return;
	CloseFile(file);
	wxString strFile(wxFileName(file, wxPATH_DOS).GetFullPath());
	#if defined(__WXMSW__) || defined(__WXOSX__)
	if (!FMOD_System_CreateSound(m_sys, wxConvFile.cWX2MB(strFile.c_str()), FMOD_SOFTWARE | FMOD_CREATESTREAM, 0, &newSound))
	#else
	FMOD_CREATESOUNDEXINFO exInfo;
	memset(&exInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	wxString dlsPath(QSPTools::GetAppPath() + QSP_MIDIDLS);
	wxCharBuffer dlsCharPath(wxConvFile.cWX2MB(dlsPath.c_str()));
	exInfo.dlsname = dlsCharPath;
	if (!FMOD_System_CreateSound(m_sys, wxConvFile.cWX2MB(strFile.c_str()), FMOD_SOFTWARE | FMOD_CREATESTREAM, &exInfo, &newSound))
	#endif
	{
		UpdateSounds();
		FMOD_System_PlaySound(m_sys, FMOD_CHANNEL_FREE, newSound, FALSE, &newChannel);
		snd.Channel = newChannel;
		snd.Sound = newSound;
		snd.Volume = volume;
		m_sounds.insert(QSPSounds::value_type(strFile.Upper(), snd));
		SetVolume(file, volume);
	}
}

void QSPCallBacks::ShowPane(int type, QSP_BOOL isShow)
{
	if (m_frame->GetIsQuit()) return;
	switch (type)
	{
	case QSP_WIN_ACTS:
		m_frame->ShowPane(ID_ACTIONS, isShow != QSP_FALSE);
		break;
	case QSP_WIN_OBJS:
		m_frame->ShowPane(ID_OBJECTS, isShow != QSP_FALSE);
		break;
	case QSP_WIN_VARS:
		m_frame->ShowPane(ID_VARSDESC, isShow != QSP_FALSE);
		break;
	case QSP_WIN_INPUT:
		m_frame->ShowPane(ID_INPUT, isShow != QSP_FALSE);
		break;
	}
}

void QSPCallBacks::Sleep(int msecs)
{
	if (m_frame->GetIsQuit()) return;
	bool isSave = m_frame->GetGameMenu()->IsEnabled(ID_SAVEGAMESTAT);
	m_frame->EnableControls(false, true);
	int i, count = msecs / 50;
	for (i = 0; i < count; ++i)
	{
		wxThread::Sleep(50);
		m_frame->Update();
		wxTheApp->Yield(true);
		if (m_frame->GetIsQuit()) return;
	}
	wxThread::Sleep(msecs % 50);
	m_frame->Update();
	wxTheApp->Yield(true);
	if (m_frame->GetIsQuit()) return;
	m_frame->EnableControls(true, true);
	m_frame->GetGameMenu()->Enable(ID_SAVEGAMESTAT, isSave);
}

int QSPCallBacks::GetMSCount()
{
	static wxStopWatch stopWatch;
	int ret = stopWatch.Time();
	stopWatch.Start();
	return ret;
}

void QSPCallBacks::Msg(const QSP_CHAR *str)
{
	if (m_frame->GetIsQuit()) return;
	RefreshInt(QSP_FALSE);
	QSPMsgDlg dialog(m_frame,
		wxID_ANY,
		m_frame->GetDesc()->GetBackgroundColour(),
		m_frame->GetDesc()->GetForegroundColour(),
		m_frame->GetDesc()->GetTextFont(),
		_("Info"),
		wxString(str),
		m_isHtml,
		m_gamePath
	);
	m_frame->EnableControls(false);
	dialog.ShowModal();
	m_frame->EnableControls(true);
}

void QSPCallBacks::DeleteMenu()
{
	if (m_frame->GetIsQuit()) return;
	m_frame->DeleteMenu();
}

void QSPCallBacks::AddMenuItem(const QSP_CHAR *name, const QSP_CHAR *imgPath)
{
	if (m_frame->GetIsQuit()) return;
	m_frame->AddMenuItem(wxString(name), wxString(imgPath));
}

void QSPCallBacks::ShowMenu()
{
	if (m_frame->GetIsQuit()) return;
	m_frame->EnableControls(false);
	m_frame->ShowMenu();
	m_frame->EnableControls(true);
}

void QSPCallBacks::Input(const QSP_CHAR *text, QSP_CHAR *buffer, int maxLen)
{
	if (m_frame->GetIsQuit()) return;
	RefreshInt(QSP_FALSE);
	QSPInputDlg dialog(m_frame,
		wxID_ANY,
		m_frame->GetDesc()->GetBackgroundColour(),
		m_frame->GetDesc()->GetForegroundColour(),
		m_frame->GetDesc()->GetTextFont(),
		_("Input data"),
		wxString(text),
		m_isHtml,
		m_gamePath
	);
	m_frame->EnableControls(false);
	dialog.ShowModal();
	m_frame->EnableControls(true);
	#ifdef _UNICODE
		wcsncpy(buffer, dialog.GetText().c_str(), maxLen);
	#else
		strncpy(buffer, dialog.GetText().c_str(), maxLen);
	#endif
}

void QSPCallBacks::ShowImage(const QSP_CHAR *file)
{
	if (m_frame->GetIsQuit()) return;
	m_frame->ShowPane(ID_VIEWPIC, m_frame->GetImgView()->OpenFile(wxString(file)));
}

void QSPCallBacks::OpenGameStatus(const QSP_CHAR *file)
{
	if (m_frame->GetIsQuit()) return;
	if (file)
	{
		if (wxFileExists(wxString(file))) QSPOpenSavedGame(file, QSP_FALSE);
	}
	else
	{
		wxFileDialog dialog(m_frame, _("Select saved game file"), wxEmptyString, wxEmptyString, _("Saved game files (*.sav)|*.sav"), wxFD_OPEN);
		m_frame->EnableControls(false);
		int res = dialog.ShowModal();
		m_frame->EnableControls(true);
		if (res == wxID_OK)
			QSPOpenSavedGame((const QSP_CHAR *)dialog.GetPath().c_str(), QSP_FALSE);
	}
}

void QSPCallBacks::SaveGameStatus(const QSP_CHAR *file)
{
	if (m_frame->GetIsQuit()) return;
	if (file)
		QSPSaveGame(file, QSP_FALSE);
	else
	{
		wxFileDialog dialog(m_frame, _("Select file to save"), wxEmptyString, wxEmptyString, _("Saved game files (*.sav)|*.sav"), wxFD_SAVE);
		m_frame->EnableControls(false);
		int res = dialog.ShowModal();
		m_frame->EnableControls(true);
		if (res == wxID_OK)
			QSPSaveGame((const QSP_CHAR *)dialog.GetPath().c_str(), QSP_FALSE);
	}
}

void QSPCallBacks::UpdateGamePath()
{
	wxFileName file(QSPGetQstFullPath());
	m_gamePath = file.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	m_frame->GetDesc()->SetGamePath(m_gamePath);
	m_frame->GetObjects()->SetGamePath(m_gamePath);
	m_frame->GetActions()->SetGamePath(m_gamePath);
	m_frame->GetVars()->SetGamePath(m_gamePath);
}

bool QSPCallBacks::SetVolume(const QSP_CHAR *file, int volume)
{
	if (!IsPlay(file)) return false;
	QSPSounds::iterator elem = m_sounds.find(wxFileName(file, wxPATH_DOS).GetFullPath().Upper());
	QSPSound *snd = (QSPSound *)&elem->second;
	snd->Volume = volume;
	FMOD_Channel_SetVolume(snd->Channel, (float)(m_volumeCoeff * volume) / 100);
	return true;
}

void QSPCallBacks::SetOverallVolume(float coeff)
{
	QSPSound *snd;
	FMOD_BOOL playing = FALSE;
	if (coeff < 0.0)
		coeff = 0.0;
	else if (coeff > 1.0)
		coeff = 1.0;
	m_volumeCoeff = coeff;
	for (QSPSounds::iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
	{
		snd = (QSPSound *)&i->second;
		FMOD_Channel_IsPlaying(snd->Channel, &playing);
		if (playing)
			FMOD_Channel_SetVolume(snd->Channel, (float)(m_volumeCoeff * snd->Volume) / 100);
	}
}

void QSPCallBacks::UpdateSounds()
{
	QSPSound *snd;
	FMOD_BOOL playing = FALSE;
	QSPSounds::iterator i = m_sounds.begin();
	while (i != m_sounds.end())
	{
		snd = (QSPSound *)&i->second;
		FMOD_Channel_IsPlaying(snd->Channel, &playing);
		if (playing)
			++i;
		else
		{
			snd->Free();
			m_sounds.erase(i++);
		}
	}
}
