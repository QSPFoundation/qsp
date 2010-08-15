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

#ifndef _AERO_QSP_FLASH_H_
	#define _AERO_QSP_FLASH_H_
	
	#include <wx/wx.h>
	#include <wx/msw/ole/activex.h>
	
	#import "libid:D27CDB6B-AE6D-11CF-96B8-444553540000" no_auto_exclude
	
	using namespace ShockwaveFlashObjects;

	const CLSID CLSID_ShockwaveFlash = __uuidof(ShockwaveFlash);
	const IID IID_IShockwaveFlash = __uuidof(IShockwaveFlash);

	inline wxString bstr2wx(const _bstr_t& bstr)
	{
		return wxString(static_cast<const wchar_t *>(bstr));
	}

	inline _bstr_t wx2bstr(const wxString& str)
	{
		return _bstr_t(str.wc_str());
	}

	const int FLASH_DISPID_ONREADYSTATECHANGE = -609; // DISPID_ONREADYSTATECHANGE
	const int FLASH_DISPID_ONPROGRESS = 0x7a6;
	const int FLASH_DISPID_FSCOMMAND = 0x96;
	const int FLASH_DISPID_FLASHCALL = 0xc5;

	enum FlashState
	{
		FlashState_Unknown = -1,
		FlashState_Loading,
		FlashState_Uninitialized,
		FlashState_Loaded,
		FlashState_Interactive,
		FlashState_Complete,
		FlashState_Max
	};

	typedef UINT (CALLBACK *CALL_PTR)(REFCLSID, REFIID, LPVOID *);

	class AeroQSPFlash
	{
	public:
		AeroQSPFlash(wxWindow *owner, const wxString &flashPath);
		~AeroQSPFlash();
		void LoadEngine(const wxString &movie);
		void LoadGame(const wxString &filename);
		wxString GetVersion() const;
		bool IsLoaded() const;
		bool IsOk() const;
		void SetFocus();

	private:
		IShockwaveFlash		*_flashInterface;
		wxActiveXContainer	*_container;
		wxString CallFlashFunc(const wxString& func, const wxString& argtype = wxEmptyString, const wxString& arg = wxEmptyString) const;
	};

#endif
