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

#include "imgcanvas.h"

wxIMPLEMENT_CLASS(QSPImgCanvas, wxWindow);

BEGIN_EVENT_TABLE(QSPImgCanvas, wxWindow)
    EVT_SIZE(QSPImgCanvas::OnSize)
    EVT_PAINT(QSPImgCanvas::OnPaint)
    EVT_KEY_UP(QSPImgCanvas::OnKeyUp)
    EVT_MOUSEWHEEL(QSPImgCanvas::OnMouseWheel)
    EVT_LEFT_DOWN(QSPImgCanvas::OnMouseClick)
END_EVENT_TABLE()

QSPImgCanvas::QSPImgCanvas(wxWindow *parent, wxWindowID id) : wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxFULL_REPAINT_ON_RESIZE)
{
    m_animation = new QSPAnimWin(this);
    m_posX = m_posY = 0;
    m_isAnim = false;
    m_animation->Move(0, 0);
    m_animation->Hide();
}

QSPImgCanvas::~QSPImgCanvas()
{
    delete m_animation;
}

bool QSPImgCanvas::OpenFile(const wxString& fullPath)
{
    bool ret;
    if (m_path == fullPath)
        return true;

    m_animation->Stop();
    m_isAnim = false;

    if (wxFileExists(fullPath))
    {
        if (m_isAnim = m_animation->LoadFile(fullPath))
        {
            m_animation->Show();
            ret = true;
        }
        else
        {
            m_animation->Hide();
            ret = m_image.LoadFile(fullPath);
        }
        if (ret)
        {
            wxSizeEvent e;
            OnSize(e);
            if (m_isAnim)
                m_animation->Play();
            else
                Refresh();
            m_path = fullPath;
            return true;
        }
    }
    return false;
}

void QSPImgCanvas::RefreshUI()
{
    if (m_isAnim)
        m_animation->RefreshUI();
    else
        Refresh();
}

bool QSPImgCanvas::SetBackgroundColour(const wxColour& color)
{
    wxWindow::SetBackgroundColour(color);
    m_animation->SetBackgroundColour(color);
    return true;
}

void QSPImgCanvas::OnSize(wxSizeEvent& event)
{
    int w, h;
    if (!m_isAnim && !m_image.Ok())
    {
        event.Skip();
        return;
    }
    GetClientSize(&w, &h);
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    if (m_isAnim)
        m_animation->SetSize(w, h);
    else
    {
        int srcW = m_image.GetWidth(), srcH = m_image.GetHeight();
        int destW = srcW * h / srcH, destH = srcH * w / srcW;
        if (destW > w)
            destW = w;
        else
            destH = h;
        m_posX = (w - destW) / 2;
        m_posY = (h - destH) / 2;
        if (destW > 0 && destH > 0)
            m_cachedBitmap = wxBitmap(m_image.Scale(destW, destH, wxIMAGE_QUALITY_BILINEAR));
        else
            m_cachedBitmap = wxNullBitmap;
    }
}

void QSPImgCanvas::OnPaint(wxPaintEvent& event)
{
    if (m_isAnim || !m_image.Ok())
    {
        event.Skip();
        return;
    }
    wxPaintDC dc(this);
    if (m_cachedBitmap.Ok())
        dc.DrawBitmap(m_cachedBitmap, m_posX, m_posY, true);
}

void QSPImgCanvas::OnKeyUp(wxKeyEvent& event)
{
    event.Skip();
    event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
}

void QSPImgCanvas::OnMouseWheel(wxMouseEvent& event)
{
    event.Skip();
    event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
}

void QSPImgCanvas::OnMouseClick(wxMouseEvent& event)
{
    event.Skip();
    event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
}
