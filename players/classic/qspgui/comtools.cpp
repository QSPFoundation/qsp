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

#include "comtools.h"

void QSPTools::LaunchDefaultBrowser(const wxString& url)
{
    /* Validate URLs, don't allow opening files & directories */
    const wxURI uri(url);
    bool hasValidScheme = uri.HasScheme() && uri.GetScheme().length() > 1;

    if (hasValidScheme)
    {
        if (uri.GetScheme() == wxT("file"))
            return;
    }
    else
    {
        if (wxFileExists(url) || wxDirExists(url))
            return;
    }

    wxLaunchDefaultBrowser(url);
}

wxString QSPTools::GetHexColor(const wxColour& color)
{
    return wxString::Format(wxT("%.2X%.2X%.2X"), (int)color.Red(), (int)color.Green(), (int)color.Blue());
}

wxString QSPTools::HtmlizeWhitespaces(const wxString& str)
{
    wxString::const_iterator i;
    wxChar ch, quote;
    wxString out;
    size_t j, linepos = 0;
    bool isLastSpace = true;
    for (i = str.begin(); i != str.end(); ++i)
    {
        switch (ch = *i)
        {
        case wxT('<'):
            quote = 0;
            while (i != str.end())
            {
                ch = *i;
                if (quote)
                {
                    if (ch == wxT('\\'))
                    {
                        if (++i == str.end()) break;
                        ch = *i;
                        if (ch == quote)
                        {
                            switch (ch)
                            {
                            case wxT('"'):
                                out << wxT("&quot;");
                                break;
                            case wxT('\''):
                                out << wxT("&apos;");
                                break;
                            }
                            ++i;
                            continue;
                        }
                        out << wxT('\\');
                    }
                    switch (ch)
                    {
                    case wxT('&'):
                        out << wxT("&amp;");
                        break;
                    case wxT('<'):
                        out << wxT("&lt;");
                        break;
                    case wxT('>'):
                        out << wxT("&gt;");
                        break;
                    default:
                        if (ch == quote)
                            quote = 0;
                        out << ch;
                        break;
                    }
                }
                else
                {
                    out << ch;
                    if (ch == wxT('>'))
                        break;
                    else if (ch == wxT('"') || ch == wxT('\''))
                        quote = ch;
                }
                ++i;
            }
            if (i == str.end()) return out;
            isLastSpace = true;
            break;
        case wxT(' '):
            if (isLastSpace)
                out << wxT("&nbsp;");
            else
                out << wxT(' ');
            isLastSpace = !isLastSpace;
            ++linepos;
            break;
        case wxT('\r'):
            break;
        case wxT('\n'):
            out << wxT("<br />");
            isLastSpace = true;
            linepos = 0;
            break;
        case wxT('\t'):
            for (j = 4 - linepos % 4; j > 0; --j)
            {
                if (isLastSpace)
                    out << wxT("&nbsp;");
                else
                    out << wxT(' ');
                isLastSpace = !isLastSpace;
            }
            linepos += 4 - linepos % 4;
            break;
        default:
            out << ch;
            isLastSpace = false;
            ++linepos;
            break;
        }
    }
    return out;
}

wxString QSPTools::ProceedAsPlain(const wxString& str)
{
    wxString::const_iterator i;
    wxChar ch;
    wxString out;
    for (i = str.begin(); i != str.end(); ++i)
    {
        switch (ch = *i)
        {
        case wxT('<'):
            out << wxT("&lt;");
            break;
        case wxT('>'):
            out << wxT("&gt;");
            break;
        case wxT('&'):
            out << wxT("&amp;");
            break;
        default:
            out << ch;
            break;
        }
    }
    return out;
}

wxString QSPTools::GetAppPath(const wxString &path, const wxString &file)
{
    wxFileName appFullPath(wxStandardPaths::Get().GetExecutablePath());
    wxFileName appPath(appFullPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + path, file);
    return appPath.GetFullPath();
}

wxString QSPTools::GetResourcePath(const wxString &path, const wxString &file)
{
    wxPathList resourcePathList;
    resourcePathList.AddEnvList(wxT("XDG_DATA_DIRS"));
    resourcePathList.Add(wxStandardPaths::Get().GetDataDir());

    wxArrayString prefixes;
    prefixes.Add(QSP_APPNAME);
    prefixes.Add(wxEmptyString);

    for (wxPathList::iterator it = resourcePathList.begin(); it != resourcePathList.end(); ++it)
    {
        for (wxArrayString::iterator prefixIt = prefixes.begin(); prefixIt != prefixes.end(); ++prefixIt)
        {
            wxFileName resourcePath(*it, file); /* directory & file names are separated */
            if (!prefixIt->IsEmpty())
                resourcePath.Assign(resourcePath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + *prefixIt, file);

            if (!path.IsEmpty())
                resourcePath.Assign(resourcePath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + path, file);

            if (resourcePath.Exists())
                return resourcePath.GetFullPath();
        }
    }

    return GetAppPath(path, file);
}

wxString QSPTools::GetConfigPath(const wxString &path, const wxString &file)
{
    wxFileName configPath(wxStandardPaths::Get().GetUserConfigDir(), file);

    if (!path.IsEmpty())
        configPath.Assign(configPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + path, file);

    return configPath.GetFullPath();
}

wxString QSPTools::GetPlatform()
{
    wxOperatingSystemId osId = wxPlatformInfo::Get().GetOperatingSystemId();

    const wxChar* string = wxT("Unknown");
    if (osId & wxOS_WINDOWS)
        string = wxT("Windows");
    else if (osId & wxOS_MAC)
        string = wxT("Macintosh");
    else if (osId & wxOS_UNIX_LINUX)
        string = wxT("Linux");
    else if (osId & wxOS_UNIX)
        string = wxT("Unix");

    return string;
}

wxString QSPTools::GetVersion(const wxString& libVersion)
{
    return wxString::Format(wxT("%s (classic)"), libVersion.wx_str());
}
