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

#include "textbox.h"

IMPLEMENT_CLASS(QSPTextBox, wxHtmlWindow)

BEGIN_EVENT_TABLE(QSPTextBox, wxHtmlWindow)
	EVT_SIZE(QSPTextBox::OnSize)
	EVT_ERASE_BACKGROUND(QSPTextBox::OnEraseBackground)
	EVT_KEY_UP(QSPTextBox::OnKeyUp)
	EVT_MOUSEWHEEL(QSPTextBox::OnMouseWheel)
END_EVENT_TABLE()

wxHtmlOpeningStatus QSPTextBox::OnHTMLOpeningURL(wxHtmlURLType type, const wxString& url, wxString *redirect) const
{
	if (wxFileName(url).IsAbsolute()) return wxHTML_OPEN;
	*redirect = wxFileName(m_path + url, wxPATH_DOS).GetFullPath();
	return wxHTML_REDIRECT;
}

QSPTextBox::QSPTextBox(wxWindow *parent, wxWindowID id) : wxHtmlWindow(parent, id)
{
	SetBorders(5);
	m_isUseHtml = false;
	m_font = *wxNORMAL_FONT;
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

void QSPTextBox::LoadBackImage(const wxString& fileName)
{
	wxString path(wxFileName(fileName, wxPATH_DOS).GetFullPath());
	if (m_imagePath != path)
	{
		m_imagePath = path;
		if (wxFileExists(path))
		{
			wxImage image;
			if (image.LoadFile(path))
			{
				SetBackgroundImage(wxBitmap(image));
				Refresh();
				return;
			}
		}
		SetBackgroundImage(wxNullBitmap);
		Refresh();
	}
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
	if (!m_font.GetFaceName().IsSameAs(fontName, false) || m_font.GetPointSize() != fontSize)
	{
		m_font = font;
		Freeze();
		SetStandardFonts(fontSize, fontName, fontName);
		Thaw();
	}
}

void QSPTextBox::SetLinkColor(const wxColour& clr)
{
	m_Parser->SetLinkColor(clr);
	RefreshUI();
}

void QSPTextBox::OnKeyUp(wxKeyEvent& event)
{
	event.Skip();
	wxKeyEvent keyEvent(event);
	keyEvent.ResumePropagation(wxEVENT_PROPAGATE_MAX);
	TryAfter(keyEvent);
}

void QSPTextBox::OnMouseWheel(wxMouseEvent& event)
{
	event.Skip();
	if (wxFindWindowAtPoint(wxGetMousePosition()) != this)
		event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
}

void QSPTextBox::OnSize(wxSizeEvent& event)
{
	CalcImageSize();
	wxHtmlWindow::OnSize(event);
}

void QSPTextBox::OnEraseBackground(wxEraseEvent& event)
{
	wxDC *dc = event.GetDC();
	dc->SetBackground(wxBrush(GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
	dc->Clear();
	if (m_bmpBg.Ok() && m_bmpRealBg.Ok())
	{
		wxPoint pt = dc->GetDeviceOrigin();
		dc->DrawBitmap(m_bmpRealBg, m_posX - pt.x, m_posY - pt.y, true);
	}
}

void QSPTextBox::SetBackgroundImage(const wxBitmap& bmpBg)
{
	m_bmpBg = bmpBg;
	CalcImageSize();
}

void QSPTextBox::CalcImageSize()
{
	if (m_bmpBg.Ok())
	{
		int w, h;
		GetClientSize(&w, &h);
		if (w < 1) w = 1;
		if (h < 1) h = 1;
		int srcW = m_bmpBg.GetWidth(), srcH = m_bmpBg.GetHeight();
		int destW = srcW * h / srcH, destH = srcH * w / srcW;
		if (destW > w)
			destW = w;
		else
			destH = h;
		m_posX = (w - destW) / 2;
		m_posY = (h - destH) / 2;
		if (destW > 0 && destH > 0)
			m_bmpRealBg = wxBitmap(m_bmpBg.ConvertToImage().Scale(destW, destH));
		else
			m_bmpRealBg = wxNullBitmap;
	}
}
