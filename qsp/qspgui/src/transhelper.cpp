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

#include "transhelper.h"

QSPTranslationHelper::QSPTranslationHelper(wxApp &app, const wxString &path) : m_app(app), m_path(path), m_locale(0)
{
}

QSPTranslationHelper::~QSPTranslationHelper()
{
	if (m_locale) delete m_locale;
}

void QSPTranslationHelper::Load(wxConfigBase &config, const wxString &key)
{
	long lang;
	config.Read(key, &lang, wxLANGUAGE_DEFAULT);
	if (m_locale) delete m_locale;
	m_locale = new wxLocale;
	m_locale->Init(lang);
	m_locale->AddCatalogLookupPathPrefix(m_path);
	m_locale->AddCatalog(m_app.GetAppName());
}

void QSPTranslationHelper::Save(wxConfigBase &config, const wxString &key) const
{
	config.Write(key, m_locale ? m_locale->GetLanguage() : wxLANGUAGE_UNKNOWN);
}

bool QSPTranslationHelper::AskUserForLanguage()
{
	wxArrayString names;
	wxArrayInt identifiers;
	wxString filename;
	const wxLanguageInfo *langinfo;
	wxString name(wxLocale::GetLanguageName(wxLANGUAGE_DEFAULT));
	if (!name.IsEmpty())
	{
		names.Add(_("Default"));
		identifiers.Add(wxLANGUAGE_DEFAULT);
	}
	wxDir dir(m_path);
	if (dir.IsOpened())
	{
		wxString anyFileMask;
		#ifdef __WXMSW__
			anyFileMask = wxT("*.*");
		#else
			anyFileMask = wxT("*");
		#endif
		for (bool cont = dir.GetFirst(&filename, anyFileMask, wxDIR_DEFAULT); cont; cont = dir.GetNext(&filename))
		{
			langinfo = wxLocale::FindLanguageInfo(filename);
			if (langinfo &&
				wxFileExists(dir.GetName() + wxFileName::GetPathSeparator() +
				filename + wxFileName::GetPathSeparator() +
				m_app.GetAppName() + wxT(".mo")))
			{
				names.Add(langinfo->Description);
				identifiers.Add(langinfo->Language);
			}
		}
	}
	int index = wxGetSingleChoiceIndex(_("Select language"), _("Language"), names);
	if (index >= 0)
	{
		if (m_locale) delete m_locale;
		m_locale = new wxLocale;
		m_locale->Init(identifiers[index]);
		m_locale->AddCatalogLookupPathPrefix(m_path);
		m_locale->AddCatalog(m_app.GetAppName());
		return true;
	}
	return false;
}
