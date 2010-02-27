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

AeroQSPFlash::AeroQSPFlash(wxWindow *owner, const wxString &flashPath)
{
	_flashInterface = NULL;
	HINSTANCE hInst = ::CoLoadLibrary(wx2bstr(flashPath), TRUE);
	if (!hInst) return;
	CALL_PTR func = (CALL_PTR)::GetProcAddress(hInst, "DllGetClassObject");
	if (!func) return;
	IClassFactory *factory;
	UINT nRetCode = func(CLSID_ShockwaveFlash, IID_IClassFactory, (LPVOID *)&factory);
	if (nRetCode != S_OK) return;
	factory->AddRef();
	HRESULT hr = factory->CreateInstance(NULL, IID_IShockwaveFlash, (LPVOID *)&_flashInterface);
	factory->Release();
	if (SUCCEEDED(hr))
	{
		_flashInterface->AddRef();
		_flashInterface->PutBackgroundColor(0x00000000);
		_flashInterface->PutAllowScriptAccess(wx2bstr(wxT("always")));
		_flashInterface->DisableLocalSecurity();
		_flashInterface->PutMenu(0);
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
