// Copyright (C) 2010 BaxZzZz (bauer_v AT mail DOT ru)
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

#include "AeroQSPFrame.h"

BEGIN_EVENT_TABLE(AeroQSPFrame, wxFrame)
	EVT_MENU(ID_OPENGAME, AeroQSPFrame::OnLoadFile)
	EVT_MENU(ID_SCREEN_MODE, AeroQSPFrame::OnShowFullScreen)
	EVT_MENU(ID_EXIT, AeroQSPFrame::OnExit)
	EVT_MENU(ID_ABOUT, AeroQSPFrame::OnAbout)
	EVT_TIMER(wxID_ANY, AeroQSPFrame::OnTimer)
	EVT_ACTIVEX(wxID_ANY, AeroQSPFrame::OnFlashEvent)
END_EVENT_TABLE()

AeroQSPFrame::AeroQSPFrame( const wxString &currentPath, const wxString &filename, bool isFullScreen) :
	_errorTimer(this)
{
	_currentPath = currentPath;
	_isFullScreen = isFullScreen;
	_filename = filename;

	if (Create(NULL, wxID_ANY, wxT("AeroQSP"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE))
	{
		_flash = new AeroQSPFlash(this, _currentPath + wxT("Flash.dll"));
		if (!_flash->IsOk())
		{
			wxMessageBox("Error: Flash not found");
			Close();
			return;
		}
		_lastState = FlashState_Unknown;
		SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
		SetIcon(wxICON(logo));
		wxMenuBar *menuBar = new wxMenuBar;
		_fileMenu = new wxMenu;
		_helpMenu = new wxMenu;
		wxMenuItem *fileOpenItem = new wxMenuItem(_fileMenu, ID_OPENGAME, wxT("&Открыть...\tCtrl+O"));
		wxMenuItem *exitItem = new wxMenuItem(_fileMenu, ID_EXIT, wxT("&Выход\tAlt+X"));
		wxMenuItem *screenModeItem = new wxMenuItem(_fileMenu, ID_SCREEN_MODE, wxT("&Полноэкранный режим\tAlt+Enter"));
		_fileMenu->Append(fileOpenItem);
		_fileMenu->Append(screenModeItem);
		_fileMenu->AppendSeparator();
		_fileMenu->Append(exitItem);
		wxMenuItem *about = new wxMenuItem(_helpMenu, ID_ABOUT, wxT("&О программе..."));
		_helpMenu->Append(about);
		menuBar->Append(_fileMenu, wxT("&Игра"));
		menuBar->Append(_helpMenu, wxT("&Помощь"));
		SetMenuBar(menuBar);
		SetClientSize(800, 600);
		CenterOnScreen();
		_flash->LoadEngine(_currentPath + wxT("AeroQSP.swf"));
		_errorTimer.Start(300, true);
	}
}

AeroQSPFrame::~AeroQSPFrame()
{
	delete _flash;
}

void AeroQSPFrame::Init()
{
	Show();
	if (wxFileExists(_filename))
		LoadFile(_filename);
	else
	{
		wxCommandEvent dummy;
		OnLoadFile(dummy);
	}
	ShowFullScreen(_isFullScreen);
}

void AeroQSPFrame::SetUserSize( const wxString &size )
{
	long width = 800, height = 600;
	int pos = size.Index(wxT('x'));
	if (pos > 0 && pos + 1 < size.Length())
	{
		size.Mid(0, pos).ToLong(&width);
		size.Mid(pos + 1).ToLong(&height);
	}
	if (width > 0 && height > 0)
	{
		int curW, curH;
		if (_isFullScreen)
		{
			_isFullScreen = false;
			ShowFullScreen(false);
		}
		GetClientSize(&curW, &curH);
		if (width != curW && height != curH)
		{
			SetClientSize(width, height);
			CenterOnScreen();
		}
	}
}

void AeroQSPFrame::SetUserTitle( const wxString &title )
{
	if (title.IsEmpty())
		SetTitle(wxT("AeroQSP"));
	else
		SetTitle(title);
}

void AeroQSPFrame::OnLoadFile( wxCommandEvent &event )
{
	wxFileDialog dialog(this, wxT("Открыть игру"), wxEmptyString, wxEmptyString,
		wxT("Архивы игр QSP (*.zip)|*.zip"), wxFD_OPEN);
	dialog.CenterOnParent();
	if (dialog.ShowModal() == wxID_OK)
		LoadFile(dialog.GetPath());
}

void AeroQSPFrame::LoadFile(const wxString &filename)
{
	_flash->LoadGame(filename);
}

void AeroQSPFrame::OnShowFullScreen( wxCommandEvent &event )
{
	_isFullScreen = !_isFullScreen;
	ShowFullScreen(_isFullScreen);
}

void AeroQSPFrame::OnFlashEvent( wxActiveXEvent &event )
{
	wxString str, val;
	switch (event.GetDispatchId())
	{
	case FLASH_DISPID_ONREADYSTATECHANGE:
		{
			const int state = event[0].GetInteger();
			if (state != _lastState)
			{
				if ( state >= 0 && state < FlashState_Max )
					_lastState = static_cast<FlashState>(state);
				else
					_lastState = FlashState_Unknown;

				if (_lastState == FlashState_Complete)
				{
					if (_flash->IsLoaded())
						Init();
					else
					{
						wxMessageBox("Error: AeroQSP can't be loaded");
						Close();
					}
				}
			}
		}
		break;
	case FLASH_DISPID_ONPROGRESS:
		break;
	case FLASH_DISPID_FSCOMMAND:
		str = event[0].GetString();
		val = event[1].GetString();
		if (str == wxT("setTitle"))
			SetUserTitle(val);
		else if (str == wxT("setSize"))
			SetUserSize(val);
		break;
	case FLASH_DISPID_FLASHCALL:
		break;
	}
	event.Skip();
}

void AeroQSPFrame::OnAbout( wxCommandEvent &event )
{
	wxString verQSP = _flash->GetVersion();
	wxAboutDialogInfo info;
	info.SetIcon(wxICON(logo));
	info.SetName(wxT("AeroQSP"));
	info.SetCopyright(wxT("Quest Soft, 2001-2009"));
	info.SetDescription(wxString::Format("Shell version: %s\nQSP version: %s", wxT("0.0.1"), verQSP));
	info.SetWebSite(wxT("http://qsp.su"));
	info.AddDeveloper(wxT("BaxZzZz [baxzzzz@gmail.com]"));
	info.AddDeveloper(wxT("Byte [nporep@mail.ru]"));
	wxAboutBox(info);
}

void AeroQSPFrame::OnExit( wxCommandEvent &event )
{
	Close();
}

void AeroQSPFrame::OnTimer( wxTimerEvent &event )
{
	if (!_flash->IsLoaded())
	{
		wxMessageBox("Error: AeroQSP can't be loaded");
		Close();
	}
}
