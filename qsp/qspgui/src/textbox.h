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

#ifndef TEXTBOX_H
	#define TEXTBOX_H

	#include <wx/wx.h>
	#include <wx/fontmap.h>
	#include <wx/html/htmlwin.h>
	#include "comtools.h"

	class QSPTextBox : public wxHtmlWindow
	{
		DECLARE_CLASS(QSPTextBox)
		DECLARE_EVENT_TABLE()
	public:
		// C-tors / D-tor
		QSPTextBox(wxWindow *parent, wxWindowID id);

		// Methods
		void RefreshUI(bool isScroll = false);

		// Accessors
		void SetIsHtml(bool isHtml, bool isScroll = false);
		void SetText(const wxString& text, bool isScroll = false);
		void SetTextFont(const wxFont& font);
		wxFont GetTextFont() const { return m_font; }
		wxString GetText() const { return m_text; }
		void SetGamePath(const wxString& path) { m_path = path; }
	protected:
		// Internal methods
		virtual wxHtmlOpeningStatus OnOpeningURL(wxHtmlURLType type, const wxString& url, wxString *redirect) const;

		// Events
		void OnKeyUp(wxKeyEvent& event);
		void OnMouseWheel(wxMouseEvent& event);

		// Fields
		bool m_isUseHtml;
		wxString m_outFormat;
		wxString m_path;
		wxFont m_font;
		wxString m_text;
	};

#endif
