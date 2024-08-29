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

#include "app.h"
#include "comtools.h"

wxIMPLEMENT_APP(QSPApp);

bool QSPApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    wxLog::EnableLogging(false);
    wxInitAllImageHandlers();
    QSPInit();
    InitUI();
    return true;
}

int QSPApp::OnExit()
{
    QSPTerminate();
    QSPCallbacks::DeInit();
    delete m_transHelper;
    wxTheClipboard->Flush();
    return wxApp::OnExit();
}

void QSPApp::OnInitCmdLine(wxCmdLineParser &parser)
{
    wxApp::OnInitCmdLine(parser);

    parser.AddParam("game file",
                    wxCMD_LINE_VAL_STRING,
                    wxCMD_LINE_PARAM_OPTIONAL);
}

bool QSPApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
    if (!wxApp::OnCmdLineParsed(parser))
        return false;

    if (parser.GetParamCount() > 0)
        m_gameFile = parser.GetParam();
    return true;
}

void QSPApp::InitUI()
{
    wxString configPath = QSPTools::GetAppPath(wxEmptyString, QSP_CONFIG);
    if (!wxFileExists(configPath) && !wxFileName::IsDirWritable(QSPTools::GetAppPath()))
        configPath = QSPTools::GetConfigPath(wxEmptyString, QSP_CONFIG);

    wxString langsPath = QSPTools::GetResourcePath(QSP_TRANSLATIONS);
    m_transHelper = new QSPTranslationHelper(QSP_APPNAME, langsPath);

    // ----------------------
    QSPFrame * frame = new QSPFrame(configPath, m_transHelper);
    frame->LoadSettings();
    frame->EnableControls(false);
    QSPCallbacks::Init(frame);
    // ----------------------
    wxInitEvent initEvent;
    if (GetAutoRunEvent(initEvent))
        wxPostEvent(frame, initEvent);
}

bool QSPApp::GetAutoRunEvent(wxInitEvent& initEvent)
{
    if (!m_gameFile.IsEmpty())
    {
        wxFileName path(m_gameFile);
        path.MakeAbsolute();
        initEvent.SetInitString(path.GetFullPath());
        return true;
    }
    else
    {
        wxFileName autoPath(wxT("auto.qsp"));
        autoPath.MakeAbsolute();
        wxString autoPathString(autoPath.GetFullPath());
        if (wxFileExists(autoPathString))
        {
            initEvent.SetInitString(autoPathString);
            return true;
        }
    }
    return false;
}
