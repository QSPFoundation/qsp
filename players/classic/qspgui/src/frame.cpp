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

#include "frame.h"
#include "callbacks_gui.h"

BEGIN_EVENT_TABLE(QSPFrame, wxFrame)
	EVT_INIT(QSPFrame::OnInit)
	EVT_CLOSE(QSPFrame::OnClose)
	EVT_IDLE(QSPFrame::OnIdle)
	EVT_TIMER(ID_TIMER, QSPFrame::OnTimer)
	EVT_MENU(wxID_EXIT, QSPFrame::OnQuit)
	EVT_MENU(ID_OPENGAME, QSPFrame::OnOpenGame)
	EVT_MENU(ID_NEWGAME, QSPFrame::OnNewGame)
	EVT_MENU(ID_OPENGAMESTAT, QSPFrame::OnOpenGameStat)
	EVT_MENU(ID_SAVEGAMESTAT, QSPFrame::OnSaveGameStat)
	EVT_MENU(ID_QUICKSAVE, QSPFrame::OnQuickSave)
	EVT_MENU(ID_SELECTFONT, QSPFrame::OnSelectFont)
	EVT_MENU(ID_USEFONTSIZE, QSPFrame::OnUseFontSize)
	EVT_MENU(ID_SELECTFONTCOLOR, QSPFrame::OnSelectFontColor)
	EVT_MENU(ID_SELECTBACKCOLOR, QSPFrame::OnSelectBackColor)
	EVT_MENU(ID_SELECTLINKCOLOR, QSPFrame::OnSelectLinkColor)
	EVT_MENU(ID_SELECTLANG, QSPFrame::OnSelectLang)
	EVT_MENU(ID_TOGGLEWINMODE, QSPFrame::OnToggleWinMode)
	EVT_MENU(ID_TOGGLEOBJS, QSPFrame::OnToggleObjs)
	EVT_MENU(ID_TOGGLEACTS, QSPFrame::OnToggleActs)
	EVT_MENU(ID_TOGGLEDESC, QSPFrame::OnToggleDesc)
	EVT_MENU(ID_TOGGLEINPUT, QSPFrame::OnToggleInput)
	EVT_MENU(ID_TOGGLECAPTIONS, QSPFrame::OnToggleCaptions)
	EVT_MENU(ID_TOGGLEHOTKEYS, QSPFrame::OnToggleHotkeys)
	EVT_MENU(ID_VOLUME0, QSPFrame::OnVolume)
	EVT_MENU(ID_VOLUME20, QSPFrame::OnVolume)
	EVT_MENU(ID_VOLUME40, QSPFrame::OnVolume)
	EVT_MENU(ID_VOLUME60, QSPFrame::OnVolume)
	EVT_MENU(ID_VOLUME80, QSPFrame::OnVolume)
	EVT_MENU(ID_VOLUME100, QSPFrame::OnVolume)
	EVT_MENU(wxID_ABOUT, QSPFrame::OnAbout)
	EVT_HTML_LINK_CLICKED(ID_MAINDESC, QSPFrame::OnLinkClicked)
	EVT_HTML_LINK_CLICKED(ID_VARSDESC, QSPFrame::OnLinkClicked)
	EVT_LISTBOX(ID_OBJECTS, QSPFrame::OnObjectChange)
	EVT_LISTBOX(ID_ACTIONS, QSPFrame::OnActionChange)
	EVT_LISTBOX_DCLICK(ID_ACTIONS, QSPFrame::OnActionDblClick)
	EVT_TEXT(ID_INPUT, QSPFrame::OnInputTextChange)
	EVT_ENTER(ID_INPUT, QSPFrame::OnInputTextEnter)
	EVT_KEY_UP(QSPFrame::OnKey)
	EVT_MOUSEWHEEL(QSPFrame::OnWheel)
	EVT_LEFT_DOWN(QSPFrame::OnMouseClick)
	EVT_AUI_PANE_CLOSE(QSPFrame::OnPaneClose)
	EVT_DROP_FILES(QSPFrame::OnDropFiles)
END_EVENT_TABLE()

IMPLEMENT_CLASS(QSPFrame, wxFrame)

QSPFrame::QSPFrame(const wxString &configPath, QSPTranslationHelper *transhelper) :
	wxFrame(0, wxID_ANY, wxEmptyString),
	m_configDefPath(configPath),
	m_configPath(configPath),
	m_transhelper(transhelper)
{
	wxRegisterId(ID_DUMMY);
	SetIcon(wxICON(logo));
	DragAcceptFiles(true);
	m_timer = new wxTimer(this, ID_TIMER);
	m_menu = new wxMenu;
	// Menu
	wxMenuBar *menuBar = new wxMenuBar;
	m_fileMenu = new wxMenu;
	wxMenuItem *fileOpenItem = new wxMenuItem(m_fileMenu, ID_OPENGAME, wxT("-"));
	fileOpenItem->SetBitmap(wxBitmap(open_xpm));
	m_fileMenu->Append(fileOpenItem);
	wxMenuItem *fileNewItem = new wxMenuItem(m_fileMenu, ID_NEWGAME, wxT("-"));
	fileNewItem->SetBitmap(wxBitmap(new_xpm));
	m_fileMenu->Append(fileNewItem);
	m_fileMenu->AppendSeparator();
	wxMenuItem *fileExitItem = new wxMenuItem(m_fileMenu, wxID_EXIT);
	fileExitItem->SetBitmap(wxBitmap(exit_xpm));
	m_fileMenu->Append(fileExitItem);
	// ------------
	m_gameMenu = new wxMenu;
	wxMenuItem *gameOpenItem = new wxMenuItem(m_gameMenu, ID_OPENGAMESTAT, wxT("-"));
	gameOpenItem->SetBitmap(wxBitmap(statusopen_xpm));
	m_gameMenu->Append(gameOpenItem);
	m_gameMenu->Append(ID_SAVEGAMESTAT, wxT("-"));
	wxMenuItem *gameSaveItem = new wxMenuItem(m_gameMenu, ID_QUICKSAVE, wxT("-"));
	gameSaveItem->SetBitmap(wxBitmap(statussave_xpm));
	m_gameMenu->Append(gameSaveItem);
	// ------------
	wxMenu *wndsMenu = new wxMenu;
	wndsMenu->Append(ID_TOGGLEOBJS, wxT("-"));
	wndsMenu->Append(ID_TOGGLEACTS, wxT("-"));
	wndsMenu->Append(ID_TOGGLEDESC, wxT("-"));
	wndsMenu->Append(ID_TOGGLEINPUT, wxT("-"));
	wndsMenu->AppendSeparator();
	wndsMenu->Append(ID_TOGGLECAPTIONS, wxT("-"));
	wndsMenu->Append(ID_TOGGLEHOTKEYS, wxT("-"));
	// ------------
	wxMenu *fontMenu = new wxMenu;
	fontMenu->Append(ID_SELECTFONT, wxT("-"));
	fontMenu->AppendCheckItem(ID_USEFONTSIZE, wxT("-"));
	// ------------
	wxMenu *colorsMenu = new wxMenu;
	colorsMenu->Append(ID_SELECTFONTCOLOR, wxT("-"));
	colorsMenu->Append(ID_SELECTBACKCOLOR, wxT("-"));
	colorsMenu->Append(ID_SELECTLINKCOLOR, wxT("-"));
	// ------------
	wxMenu *volumeMenu = new wxMenu;
	volumeMenu->AppendRadioItem(ID_VOLUME0, wxT("-"));
	volumeMenu->AppendRadioItem(ID_VOLUME20, wxT("-"));
	volumeMenu->AppendRadioItem(ID_VOLUME40, wxT("-"));
	volumeMenu->AppendRadioItem(ID_VOLUME60, wxT("-"));
	volumeMenu->AppendRadioItem(ID_VOLUME80, wxT("-"));
	volumeMenu->AppendRadioItem(ID_VOLUME100, wxT("-"));
	// ------------
	m_settingsMenu = new wxMenu;
	m_settingsMenu->Append(ID_SHOWHIDE, wxT("-"), wndsMenu);
	m_settingsMenu->Append(ID_FONT, wxT("-"), fontMenu);
	m_settingsMenu->Append(ID_COLORS, wxT("-"), colorsMenu);
	m_settingsMenu->Append(ID_VOLUME, wxT("-"), volumeMenu);
	m_settingsMenu->AppendSeparator();
	wxMenuItem *settingsWinModeItem = new wxMenuItem(m_settingsMenu, ID_TOGGLEWINMODE, wxT("-"));
	settingsWinModeItem->SetBitmap(wxBitmap(windowmode_xpm));
	m_settingsMenu->Append(settingsWinModeItem);
	m_settingsMenu->AppendSeparator();
	m_settingsMenu->Append(ID_SELECTLANG, wxT("-"));
	// ------------
	wxMenu *helpMenu = new wxMenu;
	wxMenuItem *helpAboutItem = new wxMenuItem(helpMenu, wxID_ABOUT, wxT("-"));
	helpAboutItem->SetBitmap(wxBitmap(about_xpm));
	helpMenu->Append(helpAboutItem);
	// ------------
	menuBar->Append(m_fileMenu, wxT("-"));
	menuBar->Append(m_gameMenu, wxT("-"));
	menuBar->Append(m_settingsMenu, wxT("-"));
	menuBar->Append(helpMenu, wxT("-"));
	SetMenuBar(menuBar);
	// --------------------------------------
	m_manager = new wxAuiManager(this);
	m_manager->SetDockSizeConstraint(0.5, 0.5);
	m_imgView = new QSPImgCanvas(this, ID_VIEWPIC);
	m_manager->AddPane(m_imgView, wxAuiPaneInfo().Name(wxT("imgview")).MinSize(50, 50).BestSize(150, 150).Top().MaximizeButton().Hide());
	m_desc = new QSPTextBox(this, ID_MAINDESC);
	m_manager->AddPane(m_desc, wxAuiPaneInfo().Name(wxT("desc")).CenterPane());
	m_objects = new QSPListBox(this, ID_OBJECTS);
	m_manager->AddPane(m_objects, wxAuiPaneInfo().Name(wxT("objs")).MinSize(50, 50).BestSize(100, 100).Right().MaximizeButton());
	m_actions = new QSPListBox(this, ID_ACTIONS, LB_EXTENDED);
	m_manager->AddPane(m_actions, wxAuiPaneInfo().Name(wxT("acts")).MinSize(50, 50).BestSize(100, 100).Bottom().MaximizeButton());
	m_vars = new QSPTextBox(this, ID_VARSDESC);
	m_manager->AddPane(m_vars, wxAuiPaneInfo().Name(wxT("vars")).MinSize(50, 50).BestSize(100, 100).Bottom().MaximizeButton());
	m_input = new QSPInputBox(this, ID_INPUT);
	m_manager->AddPane(m_input, wxAuiPaneInfo().Name(wxT("input")).MinSize(50, 20).BestSize(100, 20).Bottom().Layer(1));
	// --------------------------------------
	SetMinClientSize(wxSize(450, 300));
	SetOverallVolume(100);
	m_savedGamePath.Clear();
	m_isQuit = false;
	m_keyPressedWhileDisabled = false;
	m_isGameOpened = false;
}

QSPFrame::~QSPFrame()
{
	m_manager->UnInit();
	delete m_manager;
	delete m_menu;
	delete m_timer;
}

void QSPFrame::SaveSettings()
{
	int x, y, w, h;
	bool isMaximized;
	if (IsFullScreen()) ShowFullScreen(false);
	if (IsIconized()) Iconize(false);
	if (isMaximized = IsMaximized()) Maximize(false);
	wxFileConfig cfg(wxEmptyString, wxEmptyString, m_configPath);
	cfg.Write(wxT("Colors/BackColor"), m_backColor.Blue() << 16 | m_backColor.Green() << 8 | m_backColor.Red());
	cfg.Write(wxT("Colors/FontColor"), m_fontColor.Blue() << 16 | m_fontColor.Green() << 8 | m_fontColor.Red());
	cfg.Write(wxT("Colors/LinkColor"), m_linkColor.Blue() << 16 | m_linkColor.Green() << 8 | m_linkColor.Red());
	cfg.Write(wxT("Font/FontSize"), m_fontSize);
	cfg.Write(wxT("Font/FontName"), m_fontName);
	cfg.Write(wxT("Font/UseFontSize"), m_isUseFontSize);
	cfg.Write(wxT("General/Volume"), m_volume);
	cfg.Write(wxT("General/ShowHotkeys"), m_isShowHotkeys);
	cfg.Write(wxT("General/Panels"), m_manager->SavePerspective());
	m_transhelper->Save(cfg, wxT("General/Language"));
	GetPosition(&x, &y);
	GetClientSize(&w, &h);
	cfg.Write(wxT("Pos/Left"), x);
	cfg.Write(wxT("Pos/Top"), y);
	cfg.Write(wxT("Pos/Width"), w);
	cfg.Write(wxT("Pos/Height"), h);
	cfg.Write(wxT("Pos/Maximize"), isMaximized);
}

void QSPFrame::LoadSettings()
{
	bool isMaximize;
	int x, y, w, h, temp;
	Hide();
	wxFileConfig cfg(wxEmptyString, wxEmptyString, m_configPath);
	cfg.Read(wxT("Colors/BackColor"), &temp, 0xE0E0E0);
	m_backColor = wxColour(temp);
	cfg.Read(wxT("Colors/FontColor"), &temp, 0x000000);
	m_fontColor = wxColour(temp);
	cfg.Read(wxT("Colors/LinkColor"), &temp, 0xFF0000);
	m_linkColor = wxColour(temp);
	temp = wxNORMAL_FONT->GetPointSize();
	if (temp < 12) temp = 12;
	cfg.Read(wxT("Font/FontSize"), &m_fontSize, temp);
	cfg.Read(wxT("Font/FontName"), &m_fontName, wxNORMAL_FONT->GetFaceName());
	cfg.Read(wxT("Font/UseFontSize"), &m_isUseFontSize, false);
	cfg.Read(wxT("General/ShowHotkeys"), &m_isShowHotkeys, false);
	cfg.Read(wxT("General/Volume"), &m_volume, 100);
	cfg.Read(wxT("Pos/Left"), &x, 10);
	cfg.Read(wxT("Pos/Top"), &y, 10);
	cfg.Read(wxT("Pos/Width"), &w, 850);
	cfg.Read(wxT("Pos/Height"), &h, 650);
	cfg.Read(wxT("Pos/Maximize"), &isMaximize, false);
	wxString panels(wxT("layout2|") \
		wxT("name=imgview;state=1080035327;dir=1;layer=0;row=0;pos=0;prop=100000;bestw=832;besth=150;minw=50;minh=50;maxw=-1;maxh=-1;floatx=175;floaty=148;floatw=518;floath=372|") \
		wxT("name=desc;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=613;besth=341;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("name=objs;state=6293500;dir=2;layer=0;row=0;pos=0;prop=100000;bestw=213;besth=324;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("name=acts;state=6293500;dir=3;layer=0;row=0;pos=0;prop=117349;bestw=475;besth=185;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("name=vars;state=6293500;dir=3;layer=0;row=0;pos=1;prop=82651;bestw=351;besth=185;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("name=input;state=2099196;dir=3;layer=1;row=0;pos=0;prop=100000;bestw=832;besth=22;minw=50;minh=20;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("dock_size(5,0,0)=22|dock_size(2,0,0)=215|dock_size(3,0,0)=204|dock_size(3,1,0)=41|"));
	cfg.Read(wxT("General/Panels"), &panels);
	m_transhelper->Load(cfg, wxT("General/Language"));
	// -------------------------------------------------
	SetOverallVolume(m_volume);
	ApplyBackColor(m_backColor);
	ApplyFontColor(m_fontColor);
	ApplyLinkColor(m_linkColor);
	ApplyFontSize(m_fontSize);
	if (!ApplyFontName(m_fontName))
	{
		m_fontName = wxNORMAL_FONT->GetFaceName();
		ApplyFontName(m_fontName);
	}
	RefreshUI();
	m_settingsMenu->Check(ID_USEFONTSIZE, m_isUseFontSize);
	m_manager->LoadPerspective(panels);
	m_manager->RestoreMaximizedPane();
	// Check for correct position
	wxSize winSize(ClientToWindowSize(wxSize(w, h)));
	w = winSize.GetWidth();
	h = winSize.GetHeight();
	wxRect dispRect(wxGetClientDisplayRect());
	if (w > dispRect.GetWidth()) w = dispRect.GetWidth();
	if (h > dispRect.GetHeight()) h = dispRect.GetHeight();
	if (x < dispRect.GetLeft()) x = dispRect.GetLeft();
	if (y < dispRect.GetTop()) y = dispRect.GetTop();
	if (x + w - 1 > dispRect.GetRight()) x = dispRect.GetRight() - w + 1;
	if (y + h - 1 > dispRect.GetBottom()) y = dispRect.GetBottom() - h + 1;
	// --------------------------
	SetSize(x, y, w, h);
	ShowPane(ID_VIEWPIC, false);
	ShowPane(ID_ACTIONS, true);
	ShowPane(ID_OBJECTS, true);
	ShowPane(ID_VARSDESC, true);
	ShowPane(ID_INPUT, true);
	ReCreateGUI();
	if (isMaximize) Maximize();
	Show();
	m_manager->Update();
}

void QSPFrame::EnableControls(bool status, bool isExtended)
{
	if (isExtended) m_fileMenu->Enable(ID_OPENGAME, status);
	m_fileMenu->Enable(ID_NEWGAME, status);
	m_gameMenu->Enable(ID_OPENGAMESTAT, status);
	m_gameMenu->Enable(ID_SAVEGAMESTAT, status);
	m_gameMenu->Enable(ID_QUICKSAVE, status);
	m_settingsMenu->Enable(ID_TOGGLEOBJS, status);
	m_settingsMenu->Enable(ID_TOGGLEACTS, status);
	m_settingsMenu->Enable(ID_TOGGLEDESC, status);
	m_settingsMenu->Enable(ID_TOGGLEINPUT, status);
	m_objects->Enable(status);
	m_actions->Enable(status);
	m_input->SetEditable(status);
	m_isProcessEvents = status;
	m_keyPressedWhileDisabled = false;
}

void QSPFrame::ShowPane(wxWindowID id, bool isShow)
{
	int i;
	wxAuiPaneInfo &pane = m_manager->GetPane(FindWindow(id));
	wxAuiPaneInfoArray& allPanes = m_manager->GetAllPanes();
	wxON_BLOCK_EXIT_THIS0(QSPFrame::Thaw);
	Freeze();
	for (i = (int)allPanes.GetCount() - 1; i >= 0; --i)
		if (allPanes.Item(i).IsMaximized())
		{
			if (&allPanes.Item(i) == &pane)
			{
				if (!isShow)
				{
					m_manager->RestorePane(pane);
					pane.Hide();
					m_manager->Update();
				}
			}
			else if (pane.HasFlag(wxAuiPaneInfo::savedHiddenState) == isShow)
				pane.SetFlag(wxAuiPaneInfo::savedHiddenState, !isShow);
			return;
		}
	if (pane.IsShown() != isShow)
	{
		pane.Show(isShow);
		m_manager->Update();
	}
}

void QSPFrame::ApplyParams()
{
	int numVal;
	QSPString strVal;
	wxColour setBackColor, setFontColor, setLinkColor;
	wxString setFontName;
	int setFontSize;
	bool isRefresh = false;
	// --------------
	setBackColor = ((QSPGetVarValues(QSP_STATIC_STR(QSP_FMT("BCOLOR")), 0, &numVal, &strVal) && numVal) ? wxColour(numVal) : m_backColor);
	if (setBackColor != m_desc->GetBackgroundColour())
	{
		if (ApplyBackColor(setBackColor)) isRefresh = true;
	}
	// --------------
	setFontColor = ((QSPGetVarValues(QSP_STATIC_STR(QSP_FMT("FCOLOR")), 0, &numVal, &strVal) && numVal) ? wxColour(numVal) : m_fontColor);
	if (setFontColor != m_desc->GetForegroundColour())
	{
		if (ApplyFontColor(setFontColor)) isRefresh = true;
	}
	// --------------
	setLinkColor = ((QSPGetVarValues(QSP_STATIC_STR(QSP_FMT("LCOLOR")), 0, &numVal, &strVal) && numVal) ? wxColour(numVal) : m_linkColor);
	if (setLinkColor != m_desc->GetLinkColor())
	{
		if (ApplyLinkColor(setLinkColor)) isRefresh = true;
	}
	// --------------
	if (m_isUseFontSize)
		setFontSize = m_fontSize;
	else
		setFontSize = ((QSPGetVarValues(QSP_STATIC_STR(QSP_FMT("FSIZE")), 0, &numVal, &strVal) && numVal) ? numVal : m_fontSize);
	if (setFontSize != m_desc->GetTextFont().GetPointSize())
	{
		if (ApplyFontSize(setFontSize)) isRefresh = true;
	}
	// --------------
	setFontName = ((QSPGetVarValues(QSP_STATIC_STR(QSP_FMT("FNAME")), 0, &numVal, &strVal) &&
		strVal.Str && strVal.Str != strVal.End) ? wxString(strVal.Str, strVal.End) : m_fontName);
	if (!setFontName.IsSameAs(m_desc->GetTextFont().GetFaceName(), false))
	{
		if (ApplyFontName(setFontName))
			isRefresh = true;
		else if (!m_fontName.IsSameAs(m_desc->GetTextFont().GetFaceName(), false))
		{
			if (ApplyFontName(m_fontName)) isRefresh = true;
		}
	}
	// --------------
	if (isRefresh) RefreshUI();
}

void QSPFrame::DeleteMenu()
{
	delete m_menu;
	m_menu = new wxMenu;
	m_menuItemId = ID_BEGOFDYNMENU;
}

void QSPFrame::AddMenuItem(const wxString &name, const wxString &imgPath)
{
	Connect(m_menuItemId, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(QSPFrame::OnMenu));
	if (name == wxT("-"))
		m_menu->AppendSeparator();
	else
	{
		wxMenuItem *item = new wxMenuItem(m_menu, m_menuItemId, name);
		wxString itemPath(wxFileName(imgPath, wxPATH_DOS).GetFullPath());
		if (wxFileExists(itemPath))
		{
			wxBitmap itemBmp(itemPath, wxBITMAP_TYPE_ANY);
			if (itemBmp.Ok()) item->SetBitmap(itemBmp);
		}
		m_menu->Append(item);
	}
	++m_menuItemId;
}

int QSPFrame::ShowMenu()
{
	m_menuIndex = -1;
	PopupMenu(m_menu);
	return m_menuIndex;
}

void QSPFrame::UpdateGamePath(const wxString &path)
{
	m_desc->SetGamePath(path);
	m_vars->SetGamePath(path);
	m_actions->SetGamePath(path);
	m_objects->SetGamePath(path);
}

void QSPFrame::ShowError()
{
	bool oldIsProcessEvents;
	wxString wxMessage;
	QSPString loc;
	int code, actIndex, line;
	if (m_isQuit) return;
	QSPGetLastErrorData(&code, &loc, &actIndex, &line);
	QSPString desc = QSPGetErrorDesc(code);
	if (loc.Str)
		wxMessage = wxString::Format(
			_("Location: %s\nArea: %s\nLine: %ld\nCode: %ld\nDesc: %s"),
			wxString(loc.Str, loc.End).wx_str(),
			(actIndex < 0 ? _("on visit").wx_str() : _("on action").wx_str()),
			(size_t)line,
			(size_t)code,
			wxGetTranslation(wxString(desc.Str, desc.End)).wx_str()
		);
	else
		wxMessage = wxString::Format(
			_("Code: %ld\nDesc: %s"),
			(size_t)code,
			wxGetTranslation(wxString(desc.Str, desc.End)).wx_str()
		);
	wxMessageDialog dialog(this, wxMessage, _("Error"), wxOK | wxICON_ERROR);
	oldIsProcessEvents = m_isProcessEvents;
	m_isProcessEvents = false;
	dialog.ShowModal();
	m_isProcessEvents = oldIsProcessEvents;
	if (m_isGameOpened) QSPCallBacks::RefreshInt(QSP_FALSE);
}

void QSPFrame::UpdateTitle()
{
	wxString title(QSP_LOGO);
	#ifdef _DEBUG
		title = wxString::Format(wxT("%s (DEBUG)"), title.wx_str());
	#endif
	if (m_configPath != m_configDefPath)
		title = wxString::Format(wxT("%s [+]"), title.wx_str());
	SetTitle(title);
}

void QSPFrame::ReCreateGUI()
{
	wxMenuBar *menuBar = GetMenuBar();
	UpdateTitle();
	// ------------
	menuBar->SetMenuLabel(0, _("&Quest"));
	menuBar->SetMenuLabel(1, _("&Game"));
	menuBar->SetMenuLabel(2, _("&Settings"));
	menuBar->SetMenuLabel(3, _("&Help"));
	// ------------
	menuBar->SetLabel(ID_OPENGAME, _("&Open game...\tAlt-O"));
	menuBar->SetLabel(ID_NEWGAME, _("&Restart game\tAlt-N"));
	menuBar->SetLabel(wxID_EXIT, _("&Quit\tAlt-X"));
	menuBar->SetLabel(ID_OPENGAMESTAT, _("&Open saved game...\tCtrl-O"));
	menuBar->SetLabel(ID_SAVEGAMESTAT, _("&Save game..."));
	menuBar->SetLabel(ID_QUICKSAVE, _("&Quicksave\tCtrl-S"));
	menuBar->SetLabel(ID_TOGGLEOBJS, _("&Objects\tCtrl-1"));
	menuBar->SetLabel(ID_TOGGLEACTS, _("&Actions\tCtrl-2"));
	menuBar->SetLabel(ID_TOGGLEDESC, _("A&dditional desc\tCtrl-3"));
	menuBar->SetLabel(ID_TOGGLEINPUT, _("&Input area\tCtrl-4"));
	menuBar->SetLabel(ID_TOGGLECAPTIONS, _("&Captions\tCtrl-5"));
	menuBar->SetLabel(ID_TOGGLEHOTKEYS, _("&Hotkeys for actions\tCtrl-6"));
	menuBar->SetLabel(ID_SHOWHIDE, _("&Show / Hide"));
	menuBar->SetLabel(ID_FONT, _("&Font"));
	menuBar->SetLabel(ID_SELECTFONT, _("Select &font...\tAlt-F"));
	menuBar->SetLabel(ID_USEFONTSIZE, _("&Always use selected font's size"));
	menuBar->SetLabel(ID_COLORS, _("&Colors"));
	menuBar->SetLabel(ID_SELECTFONTCOLOR, _("Select font's &color...\tAlt-C"));
	menuBar->SetLabel(ID_SELECTBACKCOLOR, _("Select &background's color...\tAlt-B"));
	menuBar->SetLabel(ID_SELECTLINKCOLOR, _("Select l&inks' color...\tAlt-I"));
	menuBar->SetLabel(ID_VOLUME, _("Sound &volume"));
	menuBar->SetLabel(ID_VOLUME0, _("No sound\tAlt-1"));
	menuBar->SetLabel(ID_VOLUME20, _("20%\tAlt-2"));
	menuBar->SetLabel(ID_VOLUME40, _("40%\tAlt-3"));
	menuBar->SetLabel(ID_VOLUME60, _("60%\tAlt-4"));
	menuBar->SetLabel(ID_VOLUME80, _("80%\tAlt-5"));
	menuBar->SetLabel(ID_VOLUME100, _("Initial volume\tAlt-6"));
	menuBar->SetLabel(ID_TOGGLEWINMODE, _("Window / Fullscreen &mode\tAlt-Enter"));
	menuBar->SetLabel(ID_SELECTLANG, _("Select &language...\tAlt-L"));
	menuBar->SetLabel(wxID_ABOUT, _("&About...\tCtrl-H"));
	// --------------------------------------
	m_manager->GetPane(wxT("imgview")).Caption(_("Preview"));
	m_manager->GetPane(wxT("objs")).Caption(_("Objects"));
	m_manager->GetPane(wxT("acts")).Caption(_("Actions"));
	m_manager->GetPane(wxT("vars")).Caption(_("Additional desc"));
	m_manager->GetPane(wxT("input")).Caption(_("Input area"));
	// --------------------------------------
	m_manager->Update();
}

void QSPFrame::RefreshUI()
{
	m_desc->RefreshUI();
	m_objects->RefreshUI();
	m_actions->RefreshUI();
	m_vars->RefreshUI();
	m_input->Refresh();
	m_imgView->RefreshUI();
}

void QSPFrame::ApplyFont(const wxFont& font)
{
	m_desc->SetTextFont(font);
	m_objects->SetTextFont(font);
	m_actions->SetTextFont(font);
	m_vars->SetTextFont(font);
	m_input->SetFont(font);
}

bool QSPFrame::ApplyFontSize(int size)
{
	wxFont font(m_desc->GetTextFont());
	font.SetPointSize(size);
	ApplyFont(font);
	return true;
}

bool QSPFrame::ApplyFontName(const wxString& name)
{
	if (wxFontEnumerator::IsValidFacename(name))
	{
		wxFont font(m_desc->GetTextFont());
		font.SetFaceName(name);
		ApplyFont(font);
		return true;
	}
	return false;
}

bool QSPFrame::ApplyFontColor(const wxColour& color)
{
	m_desc->SetForegroundColour(color);
	m_objects->SetForegroundColour(color);
	m_actions->SetForegroundColour(color);
	m_vars->SetForegroundColour(color);
	m_input->SetForegroundColour(color);
	return true;
}

bool QSPFrame::ApplyBackColor(const wxColour& color)
{
	m_desc->SetBackgroundColour(color);
	m_objects->SetBackgroundColour(color);
	m_actions->SetBackgroundColour(color);
	m_vars->SetBackgroundColour(color);
	m_input->SetBackgroundColour(color);
	m_imgView->SetBackgroundColour(color);
	return true;
}

bool QSPFrame::ApplyLinkColor(const wxColour& color)
{
	m_desc->SetLinkColor(color);
	m_objects->SetLinkColor(color);
	m_actions->SetLinkColor(color);
	m_vars->SetLinkColor(color);
	return true;
}

void QSPFrame::CallPaneFunc(wxWindowID id, QSP_BOOL isShow) const
{
	switch (id)
	{
	case ID_ACTIONS:
		QSPShowWindow(QSP_WIN_ACTS, isShow);
		break;
	case ID_OBJECTS:
		QSPShowWindow(QSP_WIN_OBJS, isShow);
		break;
	case ID_VARSDESC:
		QSPShowWindow(QSP_WIN_VARS, isShow);
		break;
	case ID_INPUT:
		QSPShowWindow(QSP_WIN_INPUT, isShow);
		break;
	}
}

void QSPFrame::SetOverallVolume(int percents)
{
	int id = wxNOT_FOUND;
	switch (percents)
	{
	case 0: id = ID_VOLUME0; break;
	case 20: id = ID_VOLUME20; break;
	case 40: id = ID_VOLUME40; break;
	case 60: id = ID_VOLUME60; break;
	case 80: id = ID_VOLUME80; break;
	case 100: id = ID_VOLUME100; break;
	}
	if (id >= 0) m_settingsMenu->Check(id, true);
	QSPCallBacks::SetOverallVolume((float)percents / 100);
	m_volume = percents;
}

void QSPFrame::TogglePane(wxWindowID id)
{
	bool isShow = !m_manager->GetPane(FindWindow(id)).IsShown();
	CallPaneFunc(id, (QSP_BOOL)isShow);
	ShowPane(id, isShow);
}

void QSPFrame::OpenGameFile(const wxString& path)
{
	if (QSPLoadGameWorld(qspStringFromLen(path.c_str(), path.Length()), QSP_TRUE))
	{
		m_isGameOpened = true;
		wxCommandEvent dummy;
		wxFileName file(path);
		wxString filePath(file.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR));
		wxString configString(filePath + QSP_CONFIG);
		wxString newPath(wxFileExists(configString) ? configString : m_configDefPath);
		if (newPath != m_configPath)
		{
			SaveSettings();
			m_configPath = newPath;
			LoadSettings();
		}
		UpdateGamePath(filePath);
		OnNewGame(dummy);
		if (m_isQuit) return;
		UpdateTitle();
		EnableControls(true);
		m_savedGamePath.Clear();
	}
	else
		ShowError();
}

void QSPFrame::OnInit(wxInitEvent& event)
{
	OpenGameFile(event.GetInitString());
}

void QSPFrame::OnClose(wxCloseEvent& event)
{
	if (event.CanVeto()) event.Veto();
	SaveSettings();
	EnableControls(false, true);
	Hide();
	m_isQuit = true;
}

void QSPFrame::OnIdle(wxIdleEvent& event)
{
	if (m_isQuit) Destroy();
}

void QSPFrame::OnTimer(wxTimerEvent& event)
{
	if (m_isProcessEvents && !QSPExecCounter(QSP_TRUE))
		ShowError();
}

void QSPFrame::OnMenu(wxCommandEvent& event)
{
	m_menuIndex = event.GetId() - ID_BEGOFDYNMENU;
}

void QSPFrame::OnQuit(wxCommandEvent& event)
{
	Close();
}

void QSPFrame::OnOpenGame(wxCommandEvent& event)
{
	wxFileDialog dialog(this, _("Select game file"), wxEmptyString, wxEmptyString, _("QSP games (*.qsp;*.gam)|*.qsp;*.gam"), wxFD_OPEN);
	if (dialog.ShowModal() == wxID_OK)
		OpenGameFile(dialog.GetPath());
}

void QSPFrame::OnNewGame(wxCommandEvent& event)
{
	if (!QSPRestartGame(QSP_TRUE))
		ShowError();
}

void QSPFrame::OnOpenGameStat(wxCommandEvent& event)
{
	wxFileDialog dialog(this, _("Select saved game file"), wxEmptyString, wxEmptyString, _("Saved game files (*.sav)|*.sav"), wxFD_OPEN);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString path(dialog.GetPath());
		if (!QSPOpenSavedGame(qspStringFromLen(path.c_str(), path.Length()), QSP_TRUE))
			ShowError();
	}
}

void QSPFrame::OnSaveGameStat(wxCommandEvent& event)
{
	wxFileDialog dialog(this, _("Select file to save"), wxEmptyString, wxEmptyString, _("Saved game files (*.sav)|*.sav"), wxFD_SAVE);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString path(dialog.GetPath());
		if (QSPSaveGame(qspStringFromLen(path.c_str(), path.Length()), QSP_TRUE))
			m_savedGamePath = path;
		else
			ShowError();
	}
}

void QSPFrame::OnQuickSave(wxCommandEvent& event)
{
	if (m_savedGamePath.IsEmpty())
		OnSaveGameStat(event);
	else
	{
		if (!QSPSaveGame(qspStringFromLen(m_savedGamePath.c_str(), m_savedGamePath.Length()), QSP_TRUE))
			ShowError();
	}
}

void QSPFrame::OnSelectFont(wxCommandEvent& event)
{
	wxFontData data;
	wxFont font(m_desc->GetTextFont());
	font.SetPointSize(m_fontSize);
	font.SetFaceName(m_fontName);
	data.EnableEffects(false);
	data.SetInitialFont(font);
	wxFontDialog dialog(this, data);
	dialog.SetTitle(_("Select font"));
	if (dialog.ShowModal() == wxID_OK)
	{
		font = dialog.GetFontData().GetChosenFont();
		m_fontSize = font.GetPointSize();
		m_fontName = font.GetFaceName();
		if (m_isProcessEvents)
			ApplyParams();
		else
		{
			ApplyFontSize(m_fontSize);
			ApplyFontName(m_fontName);
			RefreshUI();
		}
	}
}

void QSPFrame::OnUseFontSize(wxCommandEvent& event)
{
	m_isUseFontSize = !m_isUseFontSize;
	if (m_isProcessEvents)
		ApplyParams();
	else
	{
		ApplyFontSize(m_fontSize);
		RefreshUI();
	}
}

void QSPFrame::OnSelectFontColor(wxCommandEvent& event)
{
	wxColourData data;
	data.SetColour(m_fontColor);
	wxColourDialog dialog(this, &data);
	dialog.SetTitle(_("Select font's color"));
	if (dialog.ShowModal() == wxID_OK)
	{
		m_fontColor = dialog.GetColourData().GetColour();
		if (m_isProcessEvents)
			ApplyParams();
		else
		{
			ApplyFontColor(m_fontColor);
			RefreshUI();
		}
	}
}

void QSPFrame::OnSelectBackColor(wxCommandEvent& event)
{
	wxColourData data;
	data.SetColour(m_backColor);
	wxColourDialog dialog(this, &data);
	dialog.SetTitle(_("Select background's color"));
	if (dialog.ShowModal() == wxID_OK)
	{
		m_backColor = dialog.GetColourData().GetColour();
		if (m_isProcessEvents)
			ApplyParams();
		else
		{
			ApplyBackColor(m_backColor);
			RefreshUI();
		}
	}
}

void QSPFrame::OnSelectLinkColor(wxCommandEvent& event)
{
	wxColourData data;
	data.SetColour(m_linkColor);
	wxColourDialog dialog(this, &data);
	dialog.SetTitle(_("Select links' color"));
	if (dialog.ShowModal() == wxID_OK)
	{
		m_linkColor = dialog.GetColourData().GetColour();
		if (m_isProcessEvents)
			ApplyParams();
		else
		{
			ApplyLinkColor(m_linkColor);
			RefreshUI();
		}
	}
}

void QSPFrame::OnSelectLang(wxCommandEvent& event)
{
	if (m_transhelper->AskUserForLanguage()) ReCreateGUI();
}

void QSPFrame::OnVolume(wxCommandEvent& event)
{
	int volume = 100;
	switch (event.GetId())
	{
	case ID_VOLUME0: volume = 0; break;
	case ID_VOLUME20: volume = 20; break;
	case ID_VOLUME40: volume = 40; break;
	case ID_VOLUME60: volume = 60; break;
	case ID_VOLUME80: volume = 80; break;
	}
	SetOverallVolume(volume);
}

void QSPFrame::OnToggleWinMode(wxCommandEvent& event)
{
	ShowFullScreen(!IsFullScreen());
}

void QSPFrame::OnToggleObjs(wxCommandEvent& event)
{
	TogglePane(ID_OBJECTS);
}

void QSPFrame::OnToggleActs(wxCommandEvent& event)
{
	TogglePane(ID_ACTIONS);
}

void QSPFrame::OnToggleDesc(wxCommandEvent& event)
{
	TogglePane(ID_VARSDESC);
}

void QSPFrame::OnToggleInput(wxCommandEvent& event)
{
	TogglePane(ID_INPUT);
}

void QSPFrame::OnToggleCaptions(wxCommandEvent& event)
{
	int i;
	bool isShow = !m_manager->GetPane(m_objects).HasCaption();
	wxAuiPaneInfoArray& allPanes = m_manager->GetAllPanes();
	for (i = (int)allPanes.GetCount() - 1; i >= 0; --i)
		allPanes.Item(i).CaptionVisible(isShow);
	m_manager->GetPane(m_desc).CaptionVisible(false);
	m_manager->Update();
}

void QSPFrame::OnToggleHotkeys(wxCommandEvent& event)
{
	m_isShowHotkeys = !m_isShowHotkeys;
	if (m_isProcessEvents) QSPCallBacks::RefreshInt(QSP_FALSE);
}

void QSPFrame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo info;
	info.SetIcon(wxIcon(logo_big_xpm));
	info.SetName(QSP_LOGO);
	info.SetCopyright(wxT("Byte Soft, 2001-2010"));
	QSPString version = QSPGetVersion();
	QSPString libCompiledDate = QSPGetCompiledDateTime();
	wxString guiCompiledDate(wxT(__DATE__) wxT(", ") wxT(__TIME__));
	info.SetDescription(wxString::Format(
		_("Version: %s\nEngine Compiled: %s\nGUI Compiled: %s"),
		wxString(version.Str, version.End).wx_str(),
		wxString(libCompiledDate.Str, libCompiledDate.End).wx_str(),
		guiCompiledDate.wx_str()
	));
	info.SetWebSite(wxT("http://qsp.su"));
	// ----
	info.AddDeveloper(wxT("Byte [nporep@mail.ru]"));
	info.AddDocWriter(wxT("Korwin [tightbow@yandex.ru]"));
	info.AddArtist(wxT("3dEyes [3deyes@gmail.com]"));
	info.AddArtist(wxT("AI [gribanov_a@mail.ru]"));
	info.AddArtist(wxT("Ajenta [ajenta.arrow@gmail.com]"));
	info.AddArtist(wxT("Alex [dogmar88@mail.ru]"));
	info.AddArtist(wxT("BaxZzZz [bauer_v@mail.ru]"));
	info.AddArtist(wxT("Belial [belgame@bk.ru]"));
	info.AddArtist(wxT("DzafT [dzaft@mail.ru]"));
	info.AddArtist(wxT("Fireton [fireton@mail.ru]"));
	info.AddArtist(wxT("Gilving [mrgilving@gmail.com]"));
	info.AddArtist(wxT("Goraph [goraph@gmail.com]"));
	info.AddArtist(wxT("HIman [himan@rambler.ru]"));
	info.AddArtist(wxT("Lostas [lostas@mail.ru]"));
	info.AddArtist(wxT("Mioirel [mioirel@rambler.ru]"));
	info.AddArtist(wxT("Morgan [gorgonyte@mail.ru]"));
	info.AddArtist(wxT("Mortem [reijii@darthman.com]"));
	info.AddArtist(wxT("Nex [nex@otaku.ru]"));
	info.AddArtist(wxT("Ntropy [ntropy@yandex.ru]"));
	info.AddArtist(wxT("Ondoo [ondoo@mail.ru]"));
	info.AddArtist(wxT("RB [qsp1@narod.ru]"));
	info.AddArtist(wxT("rrock.ru [rrock.ru@gmail.com]"));
	info.AddArtist(wxT("WladySpb [wladyspb@mail.ru]"));
	// ----
	wxAboutBox(info, this);
}

void QSPFrame::OnLinkClicked(wxHtmlLinkEvent& event)
{
	wxString href;
	wxHtmlLinkInfo info(event.GetLinkInfo());
	if (info.GetEvent()->LeftUp())
	{
		href = info.GetHref();
		if (href.StartsWith(wxT("#")))
		{
			if (event.GetId() == m_desc->GetId())
				m_desc->LoadPage(href);
			else
				m_vars->LoadPage(href);
		}
		else if (href.Upper().StartsWith(wxT("EXEC:")))
		{
			wxString string = href.Mid(5);
			if (m_isProcessEvents && !QSPExecString(qspStringFromLen(string.c_str(), string.Length()), QSP_TRUE))
				ShowError();
		}
		else
			wxLaunchDefaultBrowser(href);
	}
	else
		event.Skip();
}

void QSPFrame::OnObjectChange(wxCommandEvent& event)
{
	m_objects->Update();
	wxThread::Sleep(20);
	if (!QSPSetSelObjectIndex(event.GetInt(), QSP_TRUE))
		ShowError();
}

void QSPFrame::OnActionChange(wxCommandEvent& event)
{
	if (!QSPSetSelActionIndex(event.GetInt(), QSP_TRUE))
		ShowError();
}

void QSPFrame::OnActionDblClick(wxCommandEvent& event)
{
	if (!QSPExecuteSelActionCode(QSP_TRUE))
		ShowError();
}

void QSPFrame::OnInputTextChange(wxCommandEvent& event)
{
	wxString text(event.GetString());
	m_input->SetText(text, false);
	QSPSetInputStrText(qspStringFromLen(text.c_str(), text.Length()));
}

void QSPFrame::OnInputTextEnter(wxCommandEvent& event)
{
	if (!QSPExecUserInput(QSP_TRUE))
		ShowError();
}

void QSPFrame::OnKey(wxKeyEvent& event)
{
	event.Skip();
	// Exit fullscreen mode
	if (IsFullScreen() && event.GetKeyCode() == WXK_ESCAPE)
	{
		ShowFullScreen(false);
		return;
	}
	// Process key pressed event
	if (event.GetKeyCode() == WXK_SPACE)
		m_keyPressedWhileDisabled = true;
	// Process action shortcut
	if (m_isProcessEvents && !event.HasModifiers() && wxWindow::FindFocus() != m_input)
	{
		int ind = -1;
		int actsCount = QSPGetActions(NULL, 0);
		switch (event.GetKeyCode())
		{
		case '1': case WXK_NUMPAD1: case WXK_NUMPAD_END: ind = 0; break;
		case '2': case WXK_NUMPAD2: case WXK_NUMPAD_DOWN: ind = 1; break;
		case '3': case WXK_NUMPAD3: case WXK_NUMPAD_PAGEDOWN: ind = 2; break;
		case '4': case WXK_NUMPAD4: case WXK_NUMPAD_LEFT: ind = 3; break;
		case '5': case WXK_NUMPAD5: case WXK_CLEAR: ind = 4; break;
		case '6': case WXK_NUMPAD6: case WXK_NUMPAD_RIGHT: ind = 5; break;
		case '7': case WXK_NUMPAD7: case WXK_NUMPAD_HOME: ind = 6; break;
		case '8': case WXK_NUMPAD8: case WXK_NUMPAD_UP: ind = 7; break;
		case '9': case WXK_NUMPAD9: case WXK_NUMPAD_PAGEUP: ind = 8; break;
		case WXK_SPACE:
			if (actsCount == 1) ind = 0;
			break;
		}
		if (ind >= 0 && ind < actsCount)
		{
			wxCommandEvent e;
			if (QSPSetSelActionIndex(ind, QSP_TRUE))
				OnActionDblClick(e);
			else
				ShowError();
		}
	}
}

void QSPFrame::OnWheel(wxMouseEvent& event)
{
	wxWindow *win = wxFindWindowAtPoint(wxGetMousePosition());
	if (win) win->ScrollLines(-event.GetWheelRotation() / event.GetWheelDelta() * event.GetLinesPerAction());
}

void QSPFrame::OnMouseClick(wxMouseEvent& event)
{
	event.Skip();
	m_keyPressedWhileDisabled = true;
}

void QSPFrame::OnPaneClose(wxAuiManagerEvent& event)
{
	if (m_isProcessEvents)
		CallPaneFunc(event.GetPane()->window->GetId(), QSP_FALSE);
	else
		event.Veto();
}

void QSPFrame::OnDropFiles(wxDropFilesEvent& event)
{
	if (event.GetNumberOfFiles() && (!m_isGameOpened || m_isProcessEvents))
	{
		wxFileName path(*event.GetFiles());
		path.MakeAbsolute();
		OpenGameFile(path.GetFullPath());
	}
}
