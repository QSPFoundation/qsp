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

#include "inputbox.h"

wxIMPLEMENT_CLASS(QSPInputBox, wxTextCtrl);

wxDEFINE_EVENT(wxEVT_ENTER, wxCommandEvent);

BEGIN_EVENT_TABLE(QSPInputBox, wxTextCtrl)
    EVT_CHAR(QSPInputBox::OnChar)
    EVT_KEY_DOWN(QSPInputBox::OnKeyDown)
    EVT_MOUSEWHEEL(QSPInputBox::OnMouseWheel)
END_EVENT_TABLE()

QSPInputBox::QSPInputBox(wxWindow *parent, wxWindowID id) : wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER)
{
    m_selIndex = -1;
}

void QSPInputBox::SetText(const wxString& text, bool toChangeValue)
{
    if (m_text != text)
    {
        m_text = text;
        if (toChangeValue) ChangeValue(m_text);
    }
}

void QSPInputBox::OnChar(wxKeyEvent& event)
{
    if (!IsEditable() || event.GetKeyCode() != WXK_RETURN)
    {
        event.Skip();
        return;
    }
    int count = (int)m_strings.GetCount();
    wxString curText(GetValue());
    m_selIndex = count;
    if (!count || curText != m_strings.Last())
    {
        m_strings.Add(curText);
        ++m_selIndex;
    }
    wxCommandEvent enterEvent(wxEVT_ENTER, GetId());
    enterEvent.SetEventObject(this);
    ProcessEvent(enterEvent);
}

void QSPInputBox::OnKeyDown(wxKeyEvent& event)
{
    if (!IsEditable())
    {
        event.Skip();
        return;
    }
    int count = (int)m_strings.GetCount();
    wxString curText(GetValue());
    switch (event.GetKeyCode())
    {
    case WXK_UP:
        if (m_selIndex > 0)
        {
            --m_selIndex;
            if (curText == m_strings[m_selIndex]) --m_selIndex;
        }
        break;
    case WXK_DOWN:
        if (m_selIndex < count - 1)
        {
            ++m_selIndex;
            if (curText == m_strings[m_selIndex]) ++m_selIndex;
        }
        break;
    default:
        event.Skip();
        return;
    }
    if (m_selIndex >= 0 && m_selIndex < count)
    {
        SetValue(m_strings[m_selIndex]);
        SetSelection(GetLastPosition(), -1);
    }
}

void QSPInputBox::OnMouseWheel(wxMouseEvent& event)
{
    event.Skip();
    if (wxFindWindowAtPoint(wxGetMousePosition()) != this)
        event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
}
