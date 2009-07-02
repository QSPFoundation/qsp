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

#include "msgdlg.h"

IMPLEMENT_CLASS(QSPMsgDlg, wxDialog)

BEGIN_EVENT_TABLE(QSPMsgDlg, wxDialog)
	EVT_HTML_LINK_CLICKED(ID_MSG_DESC, QSPMsgDlg::OnLinkClicked)
END_EVENT_TABLE()

QSPMsgDlg::QSPMsgDlg(wxWindow* parent,
					 wxWindowID id,
					 const wxColour& backColor,
					 const wxColour& fontColor,
					 const wxFont& font,
					 const wxString& caption,
					 const wxString& text,
					 bool isHtml,
					 const wxString& gamePath)
{
	if (!Create(parent, id, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)) return;
	// ----------
	SetBackgroundColour(backColor);
	wxSizer *sizerUp = new wxBoxSizer(wxVERTICAL);
	m_desc = new QSPTextBox(this, ID_MSG_DESC);
	m_desc->SetGamePath(gamePath);
	m_desc->SetIsHtml(isHtml);
	m_desc->SetBackgroundColour(backColor);
	m_desc->SetForegroundColour(fontColor);
	m_desc->SetTextFont(font);
	m_desc->SetText(text);
	wxStaticLine* line = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	sizerUp->Add(m_desc, 1, wxALL | wxGROW, 2);
	sizerUp->Add(line, 0, wxALL | wxGROW, 2);
	// ----------
	wxSizer *sizerBottom = new wxBoxSizer(wxHORIZONTAL);
	wxButton *btnOk = new wxButton(this, wxID_OK, _("OK"));
	btnOk->SetDefault();
	btnOk->SetFont(font);
	#ifdef __WXMSW__
		btnOk->SetBackgroundColour(backColor);
		btnOk->SetForegroundColour(fontColor);
	#endif
	sizerBottom->Add(btnOk, 0, wxALL, 2);
	// ----------
	wxSizer *sizerMain = new wxBoxSizer(wxVERTICAL);
	sizerMain->Add(sizerUp, 1, wxGROW, 0);
	sizerMain->Add(sizerBottom, 0, wxALIGN_RIGHT, 0);
	// ----------
	sizerMain->SetMinSize(300, 200);
	SetAutoLayout(true);
	SetSizer(sizerMain);
	sizerMain->Fit(this);
	sizerMain->SetSizeHints(this);
	SetSize(520, 350);
	Center();
	btnOk->SetFocus();
}

void QSPMsgDlg::OnLinkClicked(wxHtmlLinkEvent& event)
{
	wxString href;
	wxHtmlLinkInfo info(event.GetLinkInfo());
	if (info.GetEvent()->LeftUp())
	{
		href = info.GetHref();
		if (href[0] == wxT('#'))
			m_desc->LoadPage(href);
		else
			wxLaunchDefaultBrowser(href);
	}
	else
		event.Skip();
}
