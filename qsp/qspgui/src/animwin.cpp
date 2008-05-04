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

#include "animwin.h"

IMPLEMENT_CLASS(QSPAnimWin, wxAnimationCtrl)

BEGIN_EVENT_TABLE(QSPAnimWin, wxAnimationCtrl)
	EVT_KEY_UP(QSPAnimWin::OnKeyUp)
	EVT_MOUSEWHEEL(QSPAnimWin::OnMouseWheel)
END_EVENT_TABLE()

QSPAnimWin::QSPAnimWin(wxWindow *parent) :
	wxAnimationCtrl(parent, wxID_ANY, wxNullAnimation, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxAC_NO_AUTORESIZE)
{
}

void QSPAnimWin::RefreshUI()
{
	IncrementalUpdateBackingStore();
	Refresh();
}

void QSPAnimWin::PrepareEventForParent(wxEvent& event)
{
	wxWindow *wnd = GetParent();
	if (wnd)
	{
		event.SetEventObject(wnd);
		event.SetId(wnd->GetId());
	}
}

void QSPAnimWin::OnKeyUp(wxKeyEvent& event)
{
	event.Skip();
	wxKeyEvent keyEvent(event);
	keyEvent.ResumePropagation(wxEVENT_PROPAGATE_MAX);
	keyEvent.SetEventType(wxEVT_KEY);
	PrepareEventForParent(keyEvent);
	ProcessEvent(keyEvent);
}

void QSPAnimWin::OnMouseWheel(wxMouseEvent& event)
{
	wxMouseEvent mouseEvent(event);
	mouseEvent.ResumePropagation(wxEVENT_PROPAGATE_MAX);
	mouseEvent.SetEventType(wxEVT_WHEEL);
	PrepareEventForParent(mouseEvent);
	ProcessEvent(mouseEvent);
}
