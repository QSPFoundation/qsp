// Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru)
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

#include "textbox.h"

IMPLEMENT_CLASS(QSPTextBox, wxHtmlWindow)

BEGIN_EVENT_TABLE(QSPTextBox, wxHtmlWindow)
	EVT_KEY_UP(QSPTextBox::OnKeyUp)
	EVT_MOUSEWHEEL(QSPTextBox::OnMouseWheel)
END_EVENT_TABLE()

wxHtmlOpeningStatus QSPTextBox::OnOpeningURL(wxHtmlURLType type, const wxString& url, wxString *redirect) const
{
	wxFileName file(url);
	if (file.IsAbsolute()) return wxHTML_OPEN;
	(*redirect = m_path + url).Replace(wxT("\\"), wxT("/"));
	return wxHTML_REDIRECT;
}

QSPTextBox::QSPTextBox(wxWindow *parent, wxWindowID id) : wxHtmlWindow(parent, id)
{
	SetBorders(5);
	m_isUseHtml = false;
	m_font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	m_outFormat = wxString::Format(
		wxT("<HTML><META HTTP-EQUIV = \"Content-Type\" CONTENT = \"text/html; charset=%s\">")
		wxT("<BODY><FONT COLOR = #%%s>%%s</FONT></BODY></HTML>"),
		wxFontMapper::GetEncodingName(m_font.GetEncoding()).wx_str()
	);
	wxString fontName(m_font.GetFaceName());
	SetStandardFonts(m_font.GetPointSize(), fontName, fontName);
}

void QSPTextBox::SetIsHtml(bool isHtml, bool isScroll)
{
	if (m_isUseHtml != isHtml)
	{
		m_isUseHtml = isHtml;
		RefreshUI(isScroll);
	}
}

void QSPTextBox::RefreshUI(bool isScroll)
{
	Freeze();
	SetPage(wxString::Format(m_outFormat, QSPTools::GetHexColor(GetForegroundColour()).wx_str(), QSPTools::HtmlizeWhitespaces(m_isUseHtml ? m_text : QSPTools::ProceedAsPlain(m_text)).wx_str()));
	if (isScroll) Scroll(0, 0x7FFFFFFF);
	Thaw();
}

void QSPTextBox::SetText(const wxString& text, bool isScroll)
{
	if (m_text != text)
	{
		m_text = text;
		RefreshUI(isScroll);
	}
}

void QSPTextBox::SetTextFont(const wxFont& font)
{
	int fontSize = font.GetPointSize();
	wxString fontName(font.GetFaceName());
	if (m_font.GetFaceName() != fontName || m_font.GetPointSize() != fontSize)
	{
		m_font = font;
		Freeze();
		SetStandardFonts(fontSize, fontName, fontName);
		Thaw();
	}
}

void QSPTextBox::OnKeyUp(wxKeyEvent& event)
{
	event.Skip();
	wxKeyEvent keyEvent(event);
	keyEvent.ResumePropagation(wxEVENT_PROPAGATE_MAX);
	TryParent(keyEvent);
}

void QSPTextBox::OnMouseWheel(wxMouseEvent& event)
{
	event.Skip();
	if (wxFindWindowAtPoint(wxGetMousePosition()) != this)
		event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
}
