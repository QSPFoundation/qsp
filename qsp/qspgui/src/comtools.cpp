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

#include "comtools.h"

wxString QSPTools::GetHexColor(const wxColour& color)
{
	return wxString::Format(wxT("%.2X%.2X%.2X"), (int)color.Red(), (int)color.Green(), (int)color.Blue());
}

wxString QSPTools::HtmlizeWhitespaces(const wxString& str)
{
	wxString::const_iterator i;
	wxChar ch, quote;
	wxString out;
	size_t j, linepos = 0;
	bool isLastSpace = true;
	for (i = str.begin(); i != str.end(); ++i)
	{
		switch (ch = *i)
		{
		case wxT('<'):
			quote = 0;
			while (i != str.end())
			{
				ch = *i;
				if (quote)
				{
					if (ch == wxT('&'))
						out << wxT("&amp;");
					else
					{
						if (ch == wxT('\\'))
						{
							out << ch;
							if (++i == str.end()) break;
							ch = *i;
						}
						else if (ch == quote)
							quote = 0;
						out << ch;
					}
				}
				else
				{
					out << ch;
					if (ch == wxT('>'))
						break;
					else if (ch == wxT('"') || ch == wxT('\''))
						quote = ch;
				}
				++i;
			}
			if (i == str.end()) return out;
			isLastSpace = true;
			break;
		case wxT(' '):
			if (isLastSpace)
				out << wxT("&nbsp;");
			else
				out << wxT(' ');
			isLastSpace = !isLastSpace;
			++linepos;
			break;
		case wxT('\r'):
			break;
		case wxT('\n'):
			out << wxT("<br />");
			isLastSpace = true;
			linepos = 0;
			break;
		case wxT('\t'):
			for (j = 4 - linepos % 4; j > 0; --j)
			{
				if (isLastSpace)
					out << wxT("&nbsp;");
				else
					out << wxT(' ');
				isLastSpace = !isLastSpace;
			}
			linepos += 4 - linepos % 4;
			break;
		default:
			out << ch;
			isLastSpace = false;
			++linepos;
			break;
		}
	}
	return out;
}

wxString QSPTools::ProceedAsPlain(const wxString& str)
{
	wxString::const_iterator i;
	wxChar ch;
	wxString out;
	for (i = str.begin(); i != str.end(); ++i)
	{
		switch (ch = *i)
		{
		case wxT('<'):
			out << wxT("&lt;");
			break;
		case wxT('>'):
			out << wxT("&gt;");
			break;
		case wxT('&'):
			out << wxT("&amp;");
			break;
		default:
			out << ch;
			break;
		}
	}
	return out;
}

wxString QSPTools::GetAppPath()
{
	wxFileName appPath(wxStandardPaths::Get().GetExecutablePath());
	return appPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}
