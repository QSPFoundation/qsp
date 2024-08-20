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

#ifndef APP_H
    #define APP_H

    #include <wx/wx.h>
    #include <wx/stdpaths.h>
    #include <wx/clipbrd.h>
    #include "frame.h"
    #include "callbacks_gui.h"
    #include "transhelper.h"

    class QSPApp : public wxApp
    {
    public:
        // Overloaded methods
        virtual bool OnInit();
        virtual int OnExit();
        virtual void OnInitCmdLine(wxCmdLineParser &parser);
        virtual bool OnCmdLineParsed(wxCmdLineParser &parser);
    protected:
        void InitUI();
        bool GetAutoRunEvent(wxInitEvent& initEvent);
        // Fields
        QSPTranslationHelper *m_transHelper;
        wxString m_gameFile;
    };

#endif
