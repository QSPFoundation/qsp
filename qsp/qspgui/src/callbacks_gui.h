// Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru)
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

#ifndef CALLBACKS_GUI_H
	#define CALLBACKS_GUI_H

	#include <map>
	#include "frame.h"
	#include "inputdlg.h"
	#include "fmod.h"

	typedef struct
	{
		FMOD_CHANNEL *Channel;
		FMOD_SOUND *Sound;

		void Free() const
		{
			FMOD_Sound_Release(Sound);
		}
	} QSPSound;

	typedef std::map<wxString, QSPSound> QSPSounds;

	class QSPCallBacks
	{
	public:
		// Methods
		static void Init(QSPFrame *frame);
		static void DeInit();
		static bool GetVarValue(QSP_CHAR *name, long *num, QSP_CHAR **str);

		// CallBacks
		static void RefreshInt(QSP_BOOL isRedraw);
		static void SetTimer(long msecs);
		static void SetInputStrText(QSP_CHAR *text);
		static QSP_BOOL IsPlay(QSP_CHAR *file);
		static void CloseFile(QSP_CHAR *file);
		static void PlayFile(QSP_CHAR *file, long volume);
		static void ShowPane(long type, QSP_BOOL isShow);
		static void Sleep(long msecs);
		static long GetMSCount();
		static void Msg(QSP_CHAR *str);
		static void DeleteMenu();
		static void AddMenuItem(QSP_CHAR *name, QSP_CHAR *imgPath);
		static void ShowMenu();
		static void Input(QSP_CHAR *text, QSP_CHAR *buffer, long maxLen);
		static void ShowImage(QSP_CHAR *file);
		static void OpenGameStatus();
		static void SaveGameStatus();
	private:
		// Internal methods
		static void UpdateGamePath();
		static bool SetVolume(QSP_CHAR *file, long volume);

		// Fields
		static wxString m_gamePath;
		static QSPFrame *m_frame;
		static bool m_isHtml;
		static FMOD_SYSTEM *m_sys;
		static QSPSounds m_sounds;
	};

#endif
