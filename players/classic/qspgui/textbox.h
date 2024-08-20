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

#ifndef TEXTBOX_H
    #define TEXTBOX_H

    #include <wx/wx.h>
    #include <wx/fontmap.h>
    #include <wx/html/htmlwin.h>
    #include "path_provider.h"

    class QSPTextBox : public wxHtmlWindow
    {
        DECLARE_CLASS(QSPTextBox)
        DECLARE_EVENT_TABLE()
    public:
        // C-tors / D-tor
        QSPTextBox(wxWindow *parent, wxWindowID id);

        // Methods
        void RefreshUI(bool toScroll = false);
        void LoadBackImage(const wxString& imagePath);
        void SetPathProvider(PathProvider *provider) { m_pathProvider = provider; }

        // Accessors
        void SetIsHtml(bool isHtml);
        void SetText(const wxString& text, bool toScroll = false);
        void SetTextFont(const wxFont& font);
        wxFont GetTextFont() const { return m_font; }
        wxString GetText() const { return m_text; }
        void SetLinkColor(const wxColour& clr);
        const wxColour& GetLinkColor() const { return m_Parser->GetLinkColor(); }
        void SetBackgroundImage(const wxBitmap& bmpBg);
    protected:
        // Internal methods
        void CalcImageSize();
        virtual wxHtmlOpeningStatus OnHTMLOpeningURL(wxHtmlURLType type, const wxString& url, wxString *redirect) const;

        // Events
        void OnSize(wxSizeEvent& event);
        void OnEraseBackground(wxEraseEvent& event);
        void OnKeyUp(wxKeyEvent& event);
        void OnMouseWheel(wxMouseEvent& event);
        void OnMouseClick(wxMouseEvent& event);

        // Fields
        PathProvider *m_pathProvider;
        bool m_toUseHtml;
        wxString m_outFormat;
        wxString m_imagePath;
        wxFont m_font;
        wxString m_text;
        wxBitmap m_bmpBg;
        wxBitmap m_bmpRealBg;
        int m_posX;
        int m_posY;
    };

#endif
