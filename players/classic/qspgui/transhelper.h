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

#ifndef TRANSHELPER_H
    #define TRANSHELPER_H

    #include <wx/wx.h>
    #include <wx/dir.h>
    #include <wx/filename.h>
    #include <wx/confbase.h>

    class QSPTranslationHelper
    {
    public:
        // C-tors / D-tor
        QSPTranslationHelper(const wxString &appName, const wxString &path);
        ~QSPTranslationHelper();

        // Methods
        void Load(wxConfigBase &config, const wxString &key);
        void Save(wxConfigBase &config, const wxString &key) const;
        bool AskUserForLanguage();
        void UpdateLocale(int lang);
    private:
        // Fields
        wxString m_appName;
        wxString m_path;
        wxLocale *m_locale;
    };

#endif
