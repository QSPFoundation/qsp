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

#include "listbox.h"

IMPLEMENT_CLASS(QSPListBox, wxHtmlListBox)

BEGIN_EVENT_TABLE(QSPListBox, wxHtmlListBox)
	EVT_MOTION(QSPListBox::OnMouseMove)
	EVT_LEFT_DOWN(QSPListBox::OnLeftDown)
	EVT_CHAR(QSPListBox::OnChar)
	EVT_KEY_UP(QSPListBox::OnKeyUp)
	EVT_MOUSEWHEEL(QSPListBox::OnMouseWheel)
END_EVENT_TABLE()

wxHtmlOpeningStatus QSPListBox::OnHTMLOpeningURL(wxHtmlURLType type, const wxString& url, wxString *redirect) const
{
	if (wxFileName(url).IsAbsolute()) return wxHTML_OPEN;
	*redirect = wxFileName(m_path + url, wxPATH_DOS).GetFullPath();
	return wxHTML_REDIRECT;
}

QSPListBox::QSPListBox(wxWindow *parent, wxWindowID id, ListBoxType type) : wxHtmlListBox(parent, id, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
	m_type = type;
	m_isUseHtml = false;
	m_isShowNums = false;
	m_font = wxFont(-1, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	wxString commonPart(wxString::Format(
		wxT("<META HTTP-EQUIV = \"Content-Type\" CONTENT = \"text/html; charset=%s\">")
		wxT("<FONT COLOR = #%%%%s><TABLE CELLSPACING = 4 CELLPADDING = 0><TR>%%s</TR></TABLE></FONT>"),
		wxFontMapper::GetEncodingName(m_font.GetEncoding()).wx_str()
	));
	m_outFormat = wxString::Format(commonPart, wxT("<TD WIDTH = 100%%>%s</TD>"));
	m_outFormatNums = wxString::Format(commonPart, wxT("<TD>[%ld]</TD><TD WIDTH = 100%%>%s</TD>"));
	m_outFormatImage = wxString::Format(commonPart, wxT("<TD><IMG SRC=\"%s\"></TD><TD WIDTH = 100%%>%s</TD>"));
	m_outFormatImageNums = wxString::Format(commonPart, wxT("<TD>[%ld]</TD><TD><IMG SRC=\"%s\"></TD><TD WIDTH = 100%%>%s</TD>"));
	wxString fontName(m_font.GetFaceName());
	SetStandardFonts(m_font.GetPointSize(), fontName, fontName);
	SetSelectionBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
}

void QSPListBox::SetStandardFonts(int size, const wxString& normal_face, const wxString& fixed_face)
{
	CreateHTMLParser();
	m_htmlParser->SetStandardFonts(size, normal_face, fixed_face);
	RefreshUI();
}

void QSPListBox::RefreshUI()
{
	Freeze();
	RefreshAll();
	Thaw();
}

void QSPListBox::BeginItems()
{
	m_newImages.Clear();
	m_newDescs.Clear();
}

void QSPListBox::AddItem(const wxString& image, const wxString& desc)
{
	m_newImages.Add(image);
	m_newDescs.Add(desc);
}

void QSPListBox::EndItems()
{
	size_t count;
	if (m_images != m_newImages || m_descs != m_newDescs)
	{
		m_images = m_newImages;
		m_descs = m_newDescs;
		Freeze();
		count = m_descs.GetCount();
		SetItemCount(count);
		RefreshAll();
		if (count) ScrollToRow(0);
		Thaw();
	}
}

void QSPListBox::SetIsHtml(bool isHtml)
{
	if (m_isUseHtml != isHtml)
	{
		m_isUseHtml = isHtml;
		RefreshUI();
	}
}

void QSPListBox::SetIsShowNums(bool isShow)
{
	if (m_isShowNums != isShow)
	{
		m_isShowNums = isShow;
		RefreshUI();
	}
}

void QSPListBox::SetTextFont(const wxFont& font)
{
	int fontSize = font.GetPointSize();
	wxString fontName(font.GetFaceName());
	if (!m_font.GetFaceName().IsSameAs(fontName, false) || m_font.GetPointSize() != fontSize)
	{
		m_font = font;
		SetStandardFonts(fontSize, fontName, fontName);
	}
}

void QSPListBox::SetLinkColor(const wxColour& clr)
{
	CreateHTMLParser();
	m_htmlParser->SetLinkColor(clr);
	RefreshUI();
}

const wxColour& QSPListBox::GetLinkColor() const
{
	CreateHTMLParser();
	return m_htmlParser->GetLinkColor();
}

void QSPListBox::CreateHTMLParser() const
{
	if (!m_htmlParser)
	{
		QSPListBox *self = wxConstCast(this, QSPListBox);
		self->m_htmlParser = new wxHtmlWinParser(self);
		m_htmlParser->SetDC(new wxClientDC(self));
		m_htmlParser->SetFS(&self->m_filesystem);
		#if !wxUSE_UNICODE
			if (GetFont().Ok()) m_htmlParser->SetInputEncoding(GetFont().GetEncoding());
		#endif
		m_htmlParser->SetStandardFonts();
	}
}

wxString QSPListBox::OnGetItem(size_t n) const
{
	wxString image(wxFileName(m_images[n], wxPATH_DOS).GetFullPath());
	wxString color(QSPTools::GetHexColor(GetForegroundColour()));
	wxString text(QSPTools::HtmlizeWhitespaces(m_isUseHtml ? m_descs[n] : QSPTools::ProceedAsPlain(m_descs[n])));
	if (m_isShowNums && n < 9)
	{
		if (wxFileExists(image))
			return wxString::Format(m_outFormatImageNums, color.wx_str(), n + 1, image.wx_str(), text.wx_str());
		else
			return wxString::Format(m_outFormatNums, color.wx_str(), n + 1, text.wx_str());
	}
	else
	{
		if (wxFileExists(image))
			return wxString::Format(m_outFormatImage, color.wx_str(), image.wx_str(), text.wx_str());
		else
			return wxString::Format(m_outFormat, color.wx_str(), text.wx_str());
	}
}

void QSPListBox::OnMouseMove(wxMouseEvent& event)
{
	int item;
	event.Skip();
	if (m_type == LB_EXTENDED)
	{
		item = HitTest(event.GetPosition());
		if (item != wxNOT_FOUND) DoHandleItemClick(item, 0);
	}
}

void QSPListBox::OnLeftDown(wxMouseEvent& event)
{
	event.Skip();
	if (m_type == LB_EXTENDED) OnLeftDClick(event);
}

void QSPListBox::OnChar(wxKeyEvent& event)
{
	event.Skip();
	if (m_type == LB_EXTENDED && event.GetKeyCode() == WXK_RETURN && GetSelection() != wxNOT_FOUND)
	{
		wxCommandEvent clickEvent(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, GetId());
		clickEvent.SetEventObject(this);
		clickEvent.SetInt(GetSelection());
		ProcessEvent(clickEvent);
		SetFocus();
	}
}

void QSPListBox::OnKeyUp(wxKeyEvent& event)
{
	event.Skip();
	event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
}

void QSPListBox::OnMouseWheel(wxMouseEvent& event)
{
	if (wxFindWindowAtPoint(wxGetMousePosition()) != this)
	{
		event.Skip();
		event.m_wheelRotation = 0;
	}
}
