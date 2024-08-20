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

#ifndef ANIMWIN_H
    #define ANIMWIN_H

    #include <wx/wx.h>
    #include <wx/animate.h>
    #include <wx/generic/animate.h>

    class QSPAnimWin : public wxGenericAnimationCtrl
    {
        DECLARE_CLASS(QSPAnimWin)
        DECLARE_EVENT_TABLE()
    public:
        // C-tors / D-tor
        QSPAnimWin(wxWindow *parent);

        // Methods
        void RefreshUI();
        virtual bool LoadFile(const wxString& filename, wxAnimationType type = wxANIMATION_TYPE_ANY);
    protected:
        // Events
        void OnKeyUp(wxKeyEvent& event);
        void OnMouseWheel(wxMouseEvent& event);
        void OnMouseClick(wxMouseEvent& event);
    };

#endif
