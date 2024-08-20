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

#include "textbox.h"
#include "comtools.h"

wxIMPLEMENT_CLASS(QSPTextBox, wxHtmlWindow);

BEGIN_EVENT_TABLE(QSPTextBox, wxHtmlWindow)
    EVT_SIZE(QSPTextBox::OnSize)
    EVT_ERASE_BACKGROUND(QSPTextBox::OnEraseBackground)
    EVT_KEY_UP(QSPTextBox::OnKeyUp)
    EVT_MOUSEWHEEL(QSPTextBox::OnMouseWheel)
    EVT_LEFT_DOWN(QSPTextBox::OnMouseClick)
END_EVENT_TABLE()

wxHtmlOpeningStatus QSPTextBox::OnHTMLOpeningURL(wxHtmlURLType WXUNUSED(type), const wxString& url, wxString *redirect) const
{
    if (m_pathProvider)
    {
        if (m_pathProvider->IsValidFullPath(url)) return wxHTML_OPEN;
        *redirect = m_pathProvider->ComposeGamePath(url);
        return wxHTML_REDIRECT;
    }
    return wxHTML_OPEN;
}

QSPTextBox::QSPTextBox(wxWindow *parent, wxWindowID id) : wxHtmlWindow(parent, id)
{
    SetBorders(5);
    m_toUseHtml = false;
    m_font = *wxNORMAL_FONT;
    m_pathProvider = NULL;
    m_outFormat = wxString::Format(
        wxT("<HTML><META HTTP-EQUIV = \"Content-Type\" CONTENT = \"text/html; charset=%s\">")
        wxT("<BODY><FONT COLOR = #%%s>%%s</FONT></BODY></HTML>"),
        wxFontMapper::GetEncodingName(wxLocale::GetSystemEncoding()).wx_str()
    );
    wxString fontName(m_font.GetFaceName());
    SetStandardFonts(m_font.GetPointSize(), fontName, fontName);
}

void QSPTextBox::SetIsHtml(bool isHtml)
{
    if (m_toUseHtml != isHtml)
    {
        m_toUseHtml = isHtml;
        RefreshUI();
    }
}

void QSPTextBox::RefreshUI(bool toScroll)
{
    wxString color(QSPTools::GetHexColor(GetForegroundColour()));
    wxString text(QSPTools::HtmlizeWhitespaces(m_toUseHtml ? m_text : QSPTools::ProceedAsPlain(m_text)));
    wxON_BLOCK_EXIT_THIS0(QSPTextBox::Thaw);
    Freeze();
    SetPage(wxString::Format(m_outFormat, color.wx_str(), text.wx_str()));
    if (toScroll) Scroll(0, 0x7FFFFFFF);
}

void QSPTextBox::LoadBackImage(const wxString& imagePath)
{
    if (m_imagePath != imagePath)
    {
        m_imagePath = imagePath;
        wxString fullImagePath;
        if (m_pathProvider)
            fullImagePath = m_pathProvider->ComposeGamePath(imagePath);
        else
            fullImagePath = imagePath;

        if (wxFileExists(fullImagePath))
        {
            wxImage image;
            if (image.LoadFile(fullImagePath))
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

void QSPTextBox::SetText(const wxString& text, bool toScroll)
{
    if (m_text != text)
    {
        if (toScroll)
        {
            if (m_text.IsEmpty() || !text.StartsWith(m_text))
                toScroll = false;
        }
        m_text = text;
        RefreshUI(toScroll);
    }
}

void QSPTextBox::SetTextFont(const wxFont& font)
{
    int fontSize = font.GetPointSize();
    wxString fontName(font.GetFaceName());
    if (!m_font.GetFaceName().IsSameAs(fontName, false) || m_font.GetPointSize() != fontSize)
    {
        m_font = font;
        wxON_BLOCK_EXIT_THIS0(QSPTextBox::Thaw);
        Freeze();
        SetStandardFonts(fontSize, fontName, fontName);
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

void QSPTextBox::OnMouseClick(wxMouseEvent& event)
{
    event.Skip();
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
