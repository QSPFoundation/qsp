// Copyright (C) 2010 BaxZzZz (bauer_v AT mail DOT ru)
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

#include "AeroQSPFlash.h"

AeroQSPFlash::AeroQSPFlash(wxWindow *owner)
{
	_flashInterface = NULL;
	HRESULT hr = ::CoCreateInstance(CLSID_ShockwaveFlash, NULL,
		CLSCTX_INPROC_SERVER, IID_IShockwaveFlash, (void **)&_flashInterface);
	if (SUCCEEDED(hr))
	{
		_flashInterface->PutBackgroundColor(0x00000000);
		_flashInterface->PutAllowScriptAccess(wx2bstr(wxT("always")));
		_flashInterface->DisableLocalSecurity();
		_container = new wxActiveXContainer(owner, IID_IShockwaveFlash, _flashInterface);
	}
}

AeroQSPFlash::~AeroQSPFlash()
{
	if (_flashInterface) _flashInterface->Release();
}

void AeroQSPFlash::LoadEngine( const wxString &movie )
{
	_flashInterface->LoadMovie(0, wx2bstr(movie));
	_flashInterface->Play();
}

void AeroQSPFlash::LoadGame( const wxString &filename )
{
	CallFlashFunc(wxT("run"), wxT("string"), filename);
}

wxString AeroQSPFlash::CallFlashFunc( const wxString& func, const wxString& argtype /*= wxEmptyString*/, const wxString& arg /*= wxEmptyString*/ ) const
{
	wxString args;
	if (!argtype.empty()) args = wxString::Format("<%s>%s</%s>", argtype, arg, argtype);
	wxString request = wxString::Format
		(
			"<invoke name=\"%s\" returntype=\"xml\">"
				"<arguments>"
					"%s"
				"</arguments>"
			"</invoke>",
			func,
			args
		);
	return bstr2wx(_flashInterface->CallFunction(wx2bstr(request)));
}

wxString AeroQSPFlash::GetVersion() const
{
	wxString version = CallFlashFunc(wxT("getVersion"));
	version.Replace(wxT("<string>"), wxEmptyString);
	version.Replace(wxT("</string>"), wxEmptyString);
	return version;
}

bool AeroQSPFlash::IsLoaded() const
{
	if (_flashInterface->FrameNum >= 0) return true;
	return false;
}

bool AeroQSPFlash::IsOk() const
{
	return (_flashInterface != NULL);
}
