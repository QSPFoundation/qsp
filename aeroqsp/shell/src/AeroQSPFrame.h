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

#ifndef _AERO_QSP_FRAME_H_
	#define _AERO_QSP_FRAME_H_
	
	#include <wx/wx.h>
	#include <wx/aboutdlg.h>
	#include "AeroQSPFlash.h"

	enum
	{
		ID_OPENGAME,
		ID_SCREEN_MODE,
		ID_ABOUT,
		ID_EXIT
	};

	class AeroQSPFrame : public wxFrame
	{
		DECLARE_EVENT_TABLE()
		wxDECLARE_NO_COPY_CLASS(AeroQSPFrame);
	public:
		AeroQSPFrame(const wxString &currentPath, const wxString &filename, bool isFullScreen);
		~AeroQSPFrame();
		void LoadFile(const wxString &filename);

	private:
		wxTimer			_errorTimer;
		wxString		_currentPath;
		wxString		_titleFrame;
		wxString		_filename;
		AeroQSPFlash	*_flash;
		wxMenu			*_fileMenu;
		wxMenu			*_helpMenu;
		bool			_isFullScreen;
		FlashState		_lastState;

		void Init();
		void OnLoadFile(wxCommandEvent &event);
		void OnShowFullScreen(wxCommandEvent &event);
		void OnFlashEvent(wxActiveXEvent &event);
		void OnAbout(wxCommandEvent &event);
		void OnExit(wxCommandEvent &event);
		void OnTimer(wxTimerEvent &event);
	};

#endif
