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

#ifndef _AERO_QSP_APP_H_
	#define _AERO_QSP_APP_H_

	#include <wx/wx.h>
	#include <wx/filename.h>
	#include <wx/cmdline.h>
	#include "AeroQSPFrame.h"

	class AeroQSPApp : public wxApp
	{
	public:
		AeroQSPApp() { }
		virtual bool OnInit();
		virtual void OnInitCmdLine(wxCmdLineParser& parser);
		virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
		virtual bool OnExceptionInMainLoop();

	private:
		AeroQSPFrame	*_frame;
		wxString		_filename;
		bool			_isFullScreen;
	};

#endif
