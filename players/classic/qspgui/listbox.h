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

#ifndef LISTBOX_H
    #define LISTBOX_H

    #include <wx/wx.h>
    #include <wx/fontmap.h>
    #include <wx/htmllbox.h>
    #include "path_provider.h"

    enum ListBoxType
    {
        LB_NORMAL,
        LB_EXTENDED
    };

    class QSPListBox : public wxHtmlListBox
    {
        DECLARE_CLASS(QSPListBox)
        DECLARE_EVENT_TABLE()
    public:
        // C-tors / D-tor
        QSPListBox(wxWindow *parent, wxWindowID id, ListBoxType type = LB_NORMAL);

        // Methods
        void SetStandardFonts(int size = -1,
            const wxString& normal_face = wxEmptyString,
            const wxString& fixed_face = wxEmptyString);
        void RefreshUI();
        void BeginItems();
        void AddItem(const wxString& image, const wxString& desc);
        void EndItems();
        void SetPathProvider(PathProvider *provider) { m_pathProvider = provider; }

        // Accessors
        void SetIsHtml(bool isHtml);
        void SetToShowNums(bool toShow);
        void SetTextFont(const wxFont& font);
        wxFont GetTextFont() const { return m_font; }
        void SetLinkColor(const wxColour& clr);
        const wxColour& GetLinkColor() const;
    protected:
        // Internal methods
        virtual wxString OnGetItem(size_t n) const;
        virtual wxHtmlOpeningStatus OnHTMLOpeningURL(wxHtmlURLType type, const wxString& url, wxString *redirect) const;
        void CreateHTMLParser() const;

        // Events
        void OnMouseMove(wxMouseEvent& event);
        void OnMouseClick(wxMouseEvent& event);
        void OnChar(wxKeyEvent& event);
        void OnKeyUp(wxKeyEvent& event);
        void OnMouseWheel(wxMouseEvent& event);

        // Fields
        PathProvider *m_pathProvider;
        wxString m_outFormat;
        wxString m_outFormatNums;
        wxString m_outFormatImage;
        wxString m_outFormatImageNums;
        ListBoxType m_type;
        bool m_toUseHtml;
        bool m_toShowNums;
        wxFont m_font;
        wxArrayString m_images;
        wxArrayString m_descs;
        wxArrayString m_newImages;
        wxArrayString m_newDescs;
    };

#endif
