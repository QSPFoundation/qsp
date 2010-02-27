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

#include "AeroQSPApp.h"

IMPLEMENT_APP(AeroQSPApp)

bool AeroQSPApp::OnInit()
{
	wxFileName appPath(wxStandardPaths::Get().GetExecutablePath());
	_isFullScreen = false;
	if (!wxApp::OnInit()) return false;
	_frame = new AeroQSPFrame(appPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR), _filename, _isFullScreen);
	return true;
}

void AeroQSPApp::OnInitCmdLine( wxCmdLineParser& parser )
{
	parser.DisableLongOptions();
	parser.AddSwitch(wxT("f"), wxT("f"), wxT("Run in fullscreen mode."), wxCMD_LINE_PARAM_OPTIONAL);
	parser.AddSwitch(wxT("h"), wxT("h"), wxT("Get this help."), wxCMD_LINE_PARAM_OPTIONAL);
	parser.AddParam(wxT("game_file.zip"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
}

bool AeroQSPApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
	if (parser.GetParamCount())
	{
		wxFileName path(parser.GetParam(0));
		path.MakeAbsolute();
		_filename = path.GetFullPath();
	}
	if (parser.Found(wxT("f")))
		_isFullScreen = true;
	if (parser.Found(wxT("h")))
		parser.Usage();
	return true;
}

bool AeroQSPApp::OnExceptionInMainLoop()
{
	try
	{
		throw;
	}
	catch (_com_error& ce)
	{
		wxMessageBox(wxString::Format("Error: %s", ce.ErrorMessage()));
		return false;
	}
	catch (...)
	{
		throw;
	}
}
