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

#include "imgcanvas.h"

IMPLEMENT_CLASS(QSPImgCanvas, wxWindow)

BEGIN_EVENT_TABLE(QSPImgCanvas, wxWindow)
	EVT_SIZE(QSPImgCanvas::OnSize)
	EVT_PAINT(QSPImgCanvas::OnPaint)
	EVT_KEY_UP(QSPImgCanvas::OnKeyUp)
	EVT_MOUSEWHEEL(QSPImgCanvas::OnMouseWheel)
END_EVENT_TABLE()

QSPImgCanvas::QSPImgCanvas(wxWindow *parent, wxWindowID id) : wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxFULL_REPAINT_ON_RESIZE)
{
	m_animation = new QSPAnimWin(this);
	m_posX = m_posY = m_width = m_height = 0;
	m_isAnim = false;
	m_animation->Move(0, 0);
	m_animation->Hide();
}

QSPImgCanvas::~QSPImgCanvas()
{
	delete m_animation;
}

bool QSPImgCanvas::OpenFile(const wxString& fileName)
{
	bool ret;
	wxString path(fileName);
	path.Replace(wxT("\\"), wxT("/"));
	if (m_path != path || path.IsEmpty())
	{
		if (wxFileExists(path))
		{
			if (m_isAnim = m_animation->LoadFile(path))
			{
				m_animation->Show();
				ret = true;
			}
			else
			{
				m_animation->Hide();
				ret = m_image.LoadFile(path);
			}
			if (ret)
			{
				wxSizeEvent e;
				OnSize(e);
				if (m_isAnim)
					m_animation->Play();
				else
					Refresh();
				m_path = path;
			}
			return ret;
		}
		return false;
	}
	return true;
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
		m_width = srcW * h / srcH;
		m_height = srcH * w / srcW;
		if (m_width > w)
			m_width = w;
		else
			m_height = h;
		m_posX = (w - m_width) / 2;
		m_posY = (h - m_height) / 2;
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
	wxBitmap bitmap(m_image.Scale(m_width, m_height));
	dc.DrawBitmap(bitmap, m_posX, m_posY, true);
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
