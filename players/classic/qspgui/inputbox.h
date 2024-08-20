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

#ifndef INPUTBOX_H
    #define INPUTBOX_H

    #include <wx/wx.h>

    wxDECLARE_EVENT(wxEVT_ENTER, wxCommandEvent);

    #define EVT_ENTER(winid, func) \
        wx__DECLARE_EVT1(wxEVT_ENTER, winid, wxCommandEventHandler(func))

    class QSPInputBox : public wxTextCtrl
    {
        DECLARE_CLASS(QSPInputBox)
        DECLARE_EVENT_TABLE()
    public:
        // C-tors / D-tor
        QSPInputBox(wxWindow *parent, wxWindowID id);

        // Accessors
        void SetText(const wxString& text, bool toChangeValue = true);
        wxString GetText() const { return m_text; }
    protected:
        // Events
        void OnChar(wxKeyEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        void OnMouseWheel(wxMouseEvent& event);

        // Fields
        wxString m_text;
        wxArrayString m_strings;
        int m_selIndex;
    };

#endif
