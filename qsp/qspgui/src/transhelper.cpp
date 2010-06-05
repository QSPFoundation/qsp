// Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru)
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

QSPTranslationHelper::QSPTranslationHelper(const wxString &appName, const wxString &path) :
	m_appName(appName), m_path(path), m_locale(0)
{
}

QSPTranslationHelper::~QSPTranslationHelper()
{
	if (m_locale) delete m_locale;
}

void QSPTranslationHelper::Load(wxConfigBase &config, const wxString &key)
{
	wxString langName;
	config.Read(key, &langName, wxEmptyString);
	if (langName.IsEmpty())
		UpdateLocale(wxLANGUAGE_DEFAULT);
	else
	{
		const wxLanguageInfo *langinfo = wxLocale::FindLanguageInfo(langName);
		if (langinfo)
			UpdateLocale(langinfo->Language);
		else
			UpdateLocale(wxLANGUAGE_DEFAULT);
	}
}

void QSPTranslationHelper::Save(wxConfigBase &config, const wxString &key) const
{
	config.Write(key, m_locale ? m_locale->GetCanonicalName() : wxEmptyString);
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
		for (bool cont = dir.GetFirst(&filename, wxT("*"), wxDIR_DEFAULT); cont; cont = dir.GetNext(&filename))
		{
			if (langinfo = wxLocale::FindLanguageInfo(filename))
			{
				names.Add(langinfo->Description);
				identifiers.Add(langinfo->Language);
			}
		}
	}
	int index = wxGetSingleChoiceIndex(_("Select language"), _("Language"), names);
	if (index >= 0)
	{
		UpdateLocale(identifiers[index]);
		return true;
	}
	return false;
}

void QSPTranslationHelper::UpdateLocale(int lang)
{
	if (m_locale) delete m_locale;
	m_locale = new wxLocale;
	m_locale->Init(lang);
	m_locale->AddCatalogLookupPathPrefix(m_path);
	if (!m_locale->AddCatalog(m_appName))
		m_locale->AddCatalog(m_appName + wxT('_') + m_locale->GetCanonicalName().BeforeFirst(wxT('_')));
}
