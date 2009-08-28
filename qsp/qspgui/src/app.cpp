// Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru)
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

#include "app.h"

#define QSP_CONFIG wxT("qspgui.cfg")

IMPLEMENT_APP(QSPApp)

bool QSPApp::OnInit()
{
	QSPFrame *frame;
	wxInitEvent initEvent;
	wxString configPath;
	// ----------------------
	wxLog::EnableLogging(false);
	wxInitAllImageHandlers();
	QSPInit();
	// ----------------------
	wxFileName appPath(argv[0]);
	appPath.MakeAbsolute();
	wxString appPathString(appPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR));
	m_transhelper = new QSPTranslationHelper(*this, appPathString + wxT("langs"));
	configPath = appPathString + QSP_CONFIG;
	if (!(wxFileExists(configPath) || wxFileName::IsDirWritable(appPathString)))
		configPath = wxFileName(wxStandardPaths::Get().GetUserConfigDir(), QSP_CONFIG).GetFullPath();
	// ----------------------
	frame = new QSPFrame(configPath, m_transhelper);
	frame->LoadSettings();
	frame->EnableControls(false);
	QSPCallBacks::Init(frame);
	// ----------------------
	wxCmdLineParser cmdParser(argc, argv);
	if (argc > 1)
	{
		cmdParser.AddParam();
		cmdParser.Parse(false);
		wxFileName path(cmdParser.GetParam());
		path.MakeAbsolute();
		initEvent.SetInitString(path.GetFullPath());
		wxPostEvent(frame, initEvent);
	}
	else
	{
		wxFileName autoPath(wxT("auto.qsp"));
		autoPath.MakeAbsolute();
		wxString autoPathString(autoPath.GetFullPath());
		if (wxFileExists(autoPathString))
		{
			initEvent.SetInitString(autoPathString);
			wxPostEvent(frame, initEvent);
		}
	}
	return true;
}

int QSPApp::OnExit()
{
	QSPDeInit();
	QSPCallBacks::DeInit();
	delete m_transhelper;
	return wxApp::OnExit();
}
