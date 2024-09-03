// Copyright (C) 2001-2024 Val Argunov (byte AT qsp DOT org)
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
#include "comtools.h"

QSPFrame *QSPCallbacks::m_frame;
bool QSPCallbacks::m_isHtml;
FMOD_SYSTEM *QSPCallbacks::m_sys;
QSPSounds QSPCallbacks::m_sounds;
float QSPCallbacks::m_volumeCoeff;
QSPVersionInfoValues QSPCallbacks::m_versionInfo;

void QSPCallbacks::Init(QSPFrame *frame)
{
    m_frame = frame;
    m_volumeCoeff = 1.0;

    FMOD_System_Create(&m_sys);
    wxString soundPath(QSPTools::GetResourcePath(QSP_SOUNDPLUGINS));
    FMOD_System_SetPluginPath(m_sys, wxConvFile.cWX2MB(soundPath.c_str()));
    #ifdef __WXMSW__
        FMOD_System_SetOutput(m_sys, FMOD_OUTPUTTYPE_DSOUND);
    #elif __WXOSX__
        FMOD_System_SetOutput(m_sys, FMOD_OUTPUTTYPE_COREAUDIO);
    #else
        FMOD_System_SetOutput(m_sys, FMOD_OUTPUTTYPE_ALSA);
    #endif
    FMOD_System_Init(m_sys, 32, FMOD_INIT_NORMAL, 0);

    QSPSetCallback(QSP_CALL_SETTIMER, (QSP_CALLBACK)&SetTimer);
    QSPSetCallback(QSP_CALL_REFRESHINT, (QSP_CALLBACK)&RefreshInt);
    QSPSetCallback(QSP_CALL_SETINPUTSTRTEXT, (QSP_CALLBACK)&SetInputStrText);
    QSPSetCallback(QSP_CALL_ISPLAYINGFILE, (QSP_CALLBACK)&IsPlay);
    QSPSetCallback(QSP_CALL_PLAYFILE, (QSP_CALLBACK)&PlayFile);
    QSPSetCallback(QSP_CALL_CLOSEFILE, (QSP_CALLBACK)&CloseFile);
    QSPSetCallback(QSP_CALL_SHOWMSGSTR, (QSP_CALLBACK)&Msg);
    QSPSetCallback(QSP_CALL_SLEEP, (QSP_CALLBACK)&Sleep);
    QSPSetCallback(QSP_CALL_GETMSCOUNT, (QSP_CALLBACK)&GetMSCount);
    QSPSetCallback(QSP_CALL_SHOWMENU, (QSP_CALLBACK)&ShowMenu);
    QSPSetCallback(QSP_CALL_INPUTBOX, (QSP_CALLBACK)&Input);
    QSPSetCallback(QSP_CALL_SHOWIMAGE, (QSP_CALLBACK)&ShowImage);
    QSPSetCallback(QSP_CALL_SHOWWINDOW, (QSP_CALLBACK)&ShowPane);
    QSPSetCallback(QSP_CALL_OPENGAME, (QSP_CALLBACK)&OpenGame);
    QSPSetCallback(QSP_CALL_OPENGAMESTATUS, (QSP_CALLBACK)&OpenGameStatus);
    QSPSetCallback(QSP_CALL_SAVEGAMESTATUS, (QSP_CALLBACK)&SaveGameStatus);
    QSPSetCallback(QSP_CALL_VERSION, (QSP_CALLBACK)&Version);

    /* Prepare version values */
    m_versionInfo["player"] = "Classic";
    m_versionInfo["platform"] = QSPTools::GetPlatform();
}

void QSPCallbacks::DeInit()
{
    CloseFile(qspStringFromPair(0, 0));
    FMOD_System_Close(m_sys);
    FMOD_System_Release(m_sys);
}

int QSPCallbacks::SetTimer(int msecs)
{
    if (m_frame->ToQuit()) return 0;
    if (msecs)
        m_frame->GetTimer()->Start(msecs);
    else
        m_frame->GetTimer()->Stop();
    return 0;
}

int QSPCallbacks::RefreshInt(QSP_BOOL isForced)
{
    static int oldFullRefreshCount = 0;
    QSP_BIGINT numVal;
    QSPString strVal;
    bool toScroll, canSave;
    QSPListItem items[MAX_LIST_ITEMS];
    if (m_frame->ToQuit()) return 0;
    // -------------------------------
    toScroll = !(QSPGetNumVarValue(QSP_STATIC_STR(QSP_FMT("DISABLESCROLL")), 0, &numVal) && numVal);
    canSave = !(QSPGetNumVarValue(QSP_STATIC_STR(QSP_FMT("NOSAVE")), 0, &numVal) && numVal);
    m_isHtml = QSPGetNumVarValue(QSP_STATIC_STR(QSP_FMT("USEHTML")), 0, &numVal) && numVal;
    // -------------------------------
    m_frame->GetVars()->SetIsHtml(m_isHtml);
    if (QSPIsVarsDescChanged())
    {
        QSPString varsDesc = QSPGetVarsDesc();
        m_frame->GetVars()->SetText(wxString(varsDesc.Str, varsDesc.End), toScroll);
    }
    // -------------------------------
    int fullRefreshCount = QSPGetFullRefreshCount();
    if (oldFullRefreshCount != fullRefreshCount)
    {
        toScroll = false;
        oldFullRefreshCount = fullRefreshCount;
    }
    m_frame->GetDesc()->SetIsHtml(m_isHtml);
    if (QSPIsMainDescChanged())
    {
        QSPString mainDesc = QSPGetMainDesc();
        m_frame->GetDesc()->SetText(wxString(mainDesc.Str, mainDesc.End), toScroll);
    }
    // -------------------------------
    m_frame->GetActions()->SetIsHtml(m_isHtml);
    m_frame->GetActions()->SetToShowNums(m_frame->ToShowHotkeys());
    if (QSPIsActionsChanged())
    {
        int i, actionsCount = QSPGetActions(items, MAX_LIST_ITEMS);
        m_frame->GetActions()->BeginItems();
        for (i = 0; i < actionsCount; ++i)
            m_frame->GetActions()->AddItem(wxString(items[i].Image.Str, items[i].Image.End), wxString(items[i].Name.Str, items[i].Name.End));
        m_frame->GetActions()->EndItems();
    }
    m_frame->GetActions()->SetSelection(QSPGetSelActionIndex());
    m_frame->GetObjects()->SetIsHtml(m_isHtml);
    if (QSPIsObjectsChanged())
    {
        int i, objectsCount = QSPGetObjects(items, MAX_LIST_ITEMS);
        m_frame->GetObjects()->BeginItems();
        for (i = 0; i < objectsCount; ++i)
            m_frame->GetObjects()->AddItem(wxString(items[i].Image.Str, items[i].Image.End), wxString(items[i].Name.Str, items[i].Name.End));
        m_frame->GetObjects()->EndItems();
    }
    m_frame->GetObjects()->SetSelection(QSPGetSelObjectIndex());
    // -------------------------------
    if (QSPGetStrVarValue(QSP_STATIC_STR(QSP_FMT("BACKIMAGE")), 0, &strVal) && !qspIsEmpty(strVal))
        m_frame->GetDesc()->LoadBackImage(qspToWxString(strVal));
    else
        m_frame->GetDesc()->LoadBackImage(wxEmptyString);
    // -------------------------------
    m_frame->ApplyParams();
    if (isForced)
    {
        m_frame->EnableControls(false, true);
        m_frame->Update();
        wxTheApp->Yield(true);
        if (m_frame->ToQuit()) return 0;
        m_frame->EnableControls(true, true);
    }
    m_frame->GetGameMenu()->Enable(ID_SAVEGAMESTAT, canSave);
    return 0;
}

int QSPCallbacks::SetInputStrText(QSPString text)
{
    if (m_frame->ToQuit()) return 0;
    m_frame->GetInput()->SetText(wxString(text.Str, text.End));
    return 0;
}

int QSPCallbacks::IsPlay(QSPString file)
{
    FMOD_BOOL playing = FALSE;
    wxString fileName(file.Str, file.End);
    QSPSounds::iterator elem = m_sounds.find(fileName.Upper());
    if (elem != m_sounds.end())
        FMOD_Channel_IsPlaying(((QSPSound *)(&elem->second))->Channel, &playing);
    return (playing == TRUE);
}

int QSPCallbacks::CloseFile(QSPString file)
{
    if (file.Str)
    {
        wxString fileName(file.Str, file.End);
        QSPSounds::iterator elem = m_sounds.find(fileName.Upper());
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
    return 0;
}

int QSPCallbacks::PlayFile(QSPString file, int volume)
{
    FMOD_SOUND *newSound;
    FMOD_CHANNEL *newChannel;
    QSPSound snd;
    if (SetVolume(file, volume)) return 0;
    CloseFile(file);
    wxString fileName(file.Str, file.End);
    wxString filePath(m_frame->ComposeGamePath(fileName));
#if defined(__WXMSW__) || defined(__WXOSX__)
    if (!FMOD_System_CreateSound(m_sys, wxConvFile.cWX2MB(filePath.c_str()), FMOD_SOFTWARE | FMOD_CREATESTREAM, 0, &newSound))
#else
    FMOD_CREATESOUNDEXINFO exInfo;
    memset(&exInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    wxString dlsPath(QSPTools::GetResourcePath(QSP_SOUNDPLUGINS, QSP_MIDIDLS));
    wxCharBuffer dlsCharPath(wxConvFile.cWX2MB(dlsPath.c_str()));
    exInfo.dlsname = dlsCharPath;
    if (!FMOD_System_CreateSound(m_sys, wxConvFile.cWX2MB(filePath.c_str()), FMOD_SOFTWARE | FMOD_CREATESTREAM, &exInfo, &newSound))
#endif
    {
        UpdateSounds();
        FMOD_System_PlaySound(m_sys, FMOD_CHANNEL_FREE, newSound, FALSE, &newChannel);
        snd.Channel = newChannel;
        snd.Sound = newSound;
        snd.Volume = volume;
        m_sounds.insert(QSPSounds::value_type(fileName.Upper(), snd));
        SetVolume(file, volume);
    }
    return 0;
}

int QSPCallbacks::ShowPane(int type, QSP_BOOL toShow)
{
    if (m_frame->ToQuit()) return 0;
    switch (type)
    {
    case QSP_WIN_ACTS:
        m_frame->ShowPane(ID_ACTIONS, toShow != QSP_FALSE);
        break;
    case QSP_WIN_OBJS:
        m_frame->ShowPane(ID_OBJECTS, toShow != QSP_FALSE);
        break;
    case QSP_WIN_VARS:
        m_frame->ShowPane(ID_VARSDESC, toShow != QSP_FALSE);
        break;
    case QSP_WIN_INPUT:
        m_frame->ShowPane(ID_INPUT, toShow != QSP_FALSE);
        break;
    }
    return 0;
}

int QSPCallbacks::Sleep(int msecs)
{
    if (m_frame->ToQuit()) return 0;
    bool canSave = m_frame->GetGameMenu()->IsEnabled(ID_SAVEGAMESTAT);
    bool toBreak = false;
    m_frame->EnableControls(false, true);
    int i, count = msecs / 50;
    for (i = 0; i < count; ++i)
    {
        wxThread::Sleep(50);
        m_frame->Update();
        wxTheApp->Yield(true);
        if (m_frame->ToQuit() ||
            m_frame->IsKeyPressedWhileDisabled())
        {
            toBreak = true;
            break;
        }
    }
    if (!toBreak)
    {
        wxThread::Sleep(msecs % 50);
        m_frame->Update();
        wxTheApp->Yield(true);
    }
    m_frame->EnableControls(true, true);
    m_frame->GetGameMenu()->Enable(ID_SAVEGAMESTAT, canSave);
    return 0;
}

int QSPCallbacks::GetMSCount()
{
    static wxStopWatch stopWatch;
    int ret = stopWatch.Time();
    stopWatch.Start();
    return ret;
}

int QSPCallbacks::Msg(QSPString str)
{
    if (m_frame->ToQuit()) return 0;
    QSPMsgDlg dialog(m_frame,
        wxID_ANY,
        m_frame->GetDesc()->GetBackgroundColour(),
        m_frame->GetDesc()->GetForegroundColour(),
        m_frame->GetDesc()->GetTextFont(),
        _("Info"),
        wxString(str.Str, str.End),
        m_isHtml,
        m_frame
    );
    m_frame->EnableControls(false);
    dialog.ShowModal();
    m_frame->EnableControls(true);
    return 0;
}

int QSPCallbacks::ShowMenu(QSPListItem *items, int count)
{
    if (m_frame->ToQuit()) return -1;
    m_frame->EnableControls(false);
    m_frame->DeleteMenu();
    for (int i = 0; i < count; ++i)
        m_frame->AddMenuItem(wxString(items[i].Name.Str, items[i].Name.End), wxString(items[i].Image.Str, items[i].Image.End));
    int index = m_frame->ShowMenu();
    m_frame->EnableControls(true);
    return index;
}

int QSPCallbacks::Input(QSPString text, QSP_CHAR *buffer, int maxLen)
{
    if (m_frame->ToQuit()) return 0;
    QSPInputDlg dialog(m_frame,
        wxID_ANY,
        m_frame->GetDesc()->GetBackgroundColour(),
        m_frame->GetDesc()->GetForegroundColour(),
        m_frame->GetDesc()->GetTextFont(),
        _("Input data"),
        wxString(text.Str, text.End),
        m_isHtml,
        m_frame
    );
    m_frame->EnableControls(false);
    dialog.ShowModal();
    m_frame->EnableControls(true);
#ifdef _UNICODE
    wcsncpy(buffer, dialog.GetText().c_str(), maxLen);
#else
    strncpy(buffer, dialog.GetText().c_str(), maxLen);
#endif
    return 0;
}

int QSPCallbacks::ShowImage(QSPString file)
{
    if (m_frame->ToQuit()) return 0;
    if (file.Str)
    {
        wxString imgFullPath(m_frame->ComposeGamePath(wxString(file.Str, file.End)));
        m_frame->ShowPane(ID_VIEWPIC, m_frame->GetImgView()->OpenFile(imgFullPath));
    }
    else
    {
        m_frame->ShowPane(ID_VIEWPIC, false);
    }
    return 0;
}

int QSPCallbacks::OpenGame(QSPString file, QSP_BOOL isNewGame)
{
    if (m_frame->ToQuit()) return 0;
    wxString fullPath(m_frame->ComposeGamePath(wxString(file.Str, file.End)));
    if (wxFileExists(fullPath))
    {
        wxFile fileToLoad(fullPath);
        int fileSize = fileToLoad.Length();
        void *fileData = (void *)malloc(fileSize);
        if (fileToLoad.Read(fileData, fileSize) == fileSize)
        {
            if (QSPLoadGameWorldFromData(fileData, fileSize, isNewGame) && isNewGame)
                m_frame->UpdateGamePath(fullPath);
        }
        free(fileData);
    }
    return 0;
}

int QSPCallbacks::OpenGameStatus(QSPString file)
{
    if (m_frame->ToQuit()) return 0;
    wxString fullPath;
    if (file.Str)
    {
        fullPath = m_frame->ComposeGamePath(wxString(file.Str, file.End));
    }
    else
    {
        wxFileDialog dialog(m_frame, _("Select saved game file"), wxEmptyString, wxEmptyString, _("Saved game files (*.sav)|*.sav"), wxFD_OPEN);
        m_frame->EnableControls(false);
        int res = dialog.ShowModal();
        m_frame->EnableControls(true);
        if (res != wxID_OK)
            return 0;
        fullPath = dialog.GetPath();
    }
    if (wxFileExists(fullPath))
    {
        wxFile fileToLoad(fullPath);
        int fileSize = fileToLoad.Length();
        void *fileData = (void *)malloc(fileSize);
        if (fileToLoad.Read(fileData, fileSize) == fileSize)
            QSPOpenSavedGameFromData(fileData, fileSize, QSP_FALSE);
        free(fileData);
    }
    return 0;
}

int QSPCallbacks::SaveGameStatus(QSPString file)
{
    if (m_frame->ToQuit()) return 0;
    wxString fullPath;
    if (file.Str)
    {
        fullPath = m_frame->ComposeGamePath(wxString(file.Str, file.End));
    }
    else
    {
        wxFileDialog dialog(m_frame, _("Select file to save"), wxEmptyString, wxEmptyString, _("Saved game files (*.sav)|*.sav"), wxFD_SAVE);
        m_frame->EnableControls(false);
        int res = dialog.ShowModal();
        m_frame->EnableControls(true);
        if (res != wxID_OK)
            return 0;
        fullPath = dialog.GetPath();
    }
    int fileSize = 64 * 1024;
    void *fileData = (void *)malloc(fileSize);
    if (!QSPSaveGameAsData(fileData, &fileSize, QSP_FALSE))
    {
        while (fileSize)
        {
            fileData = (void *)realloc(fileData, fileSize);
            if (QSPSaveGameAsData(fileData, &fileSize, QSP_FALSE))
                break;
        }
        if (!fileSize)
        {
            free(fileData);
            return 0;
        }
    }
    wxFile fileToSave(fullPath, wxFile::write);
    fileToSave.Write(fileData, fileSize);
    free(fileData);
    return 0;
}

int QSPCallbacks::Version(QSPString param, QSP_CHAR *buffer, int maxLen)
{
    wxString result;
    wxString request(param.Str, param.End);

    if (request.IsEmpty())
    {
        QSPString libVersion = QSPGetVersion();
        result = QSPTools::GetVersion(wxString(libVersion.Str, libVersion.End));
    }
    else
    {
        QSPVersionInfoValues::iterator value = m_versionInfo.find(request.Lower());
        if (value != m_versionInfo.end())
            result = value->second;
    }

#ifdef _UNICODE
    wcsncpy(buffer, result.c_str(), maxLen);
#else
    strncpy(buffer, result.c_str(), maxLen);
#endif
    return 0;
}

bool QSPCallbacks::SetVolume(QSPString file, int volume)
{
    if (!IsPlay(file)) return false;
    wxString fileName(file.Str, file.End);
    QSPSounds::iterator elem = m_sounds.find(fileName.Upper());
    if (elem != m_sounds.end())
    {
        QSPSound *snd = (QSPSound *)&elem->second;
        snd->Volume = volume;
        FMOD_Channel_SetVolume(snd->Channel, (float)(m_volumeCoeff * volume) / 100);
    }
    return true;
}

void QSPCallbacks::SetOverallVolume(float coeff)
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

void QSPCallbacks::UpdateSounds()
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
