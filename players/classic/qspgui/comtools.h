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

#ifndef TOOLS_H
    #define TOOLS_H

    #include <wx/wx.h>
    #include <wx/filename.h>
    #include <wx/stdpaths.h>
    #include <wx/scopeguard.h>
    #include <wx/filefn.h>
    #include <wx/uri.h>

    #define QSP_APPNAME wxT("qspgui")
    #define QSP_CONFIG wxT("qspgui.cfg")
    #define QSP_TRANSLATIONS wxT("langs")
    #define QSP_SOUNDPLUGINS wxT("sound")
    #define QSP_MIDIDLS wxT("midi.dls")

    class QSPTools
    {
    public:
        static void LaunchDefaultBrowser(const wxString& url);
        static wxString GetHexColor(const wxColour& color);
        static wxString HtmlizeWhitespaces(const wxString& str);
        static wxString ProceedAsPlain(const wxString& str);
        static wxString GetAppPath(const wxString &path = wxEmptyString, const wxString &file = wxEmptyString);
        static wxString GetResourcePath(const wxString &path = wxEmptyString, const wxString &file = wxEmptyString);
        static wxString GetConfigPath(const wxString &path = wxEmptyString, const wxString &file = wxEmptyString);
        static wxString GetPlatform();
        static wxString GetVersion(const wxString& libVersion);
    };

#endif
