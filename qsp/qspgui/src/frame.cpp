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

#include "frame.h"
#include "callbacks_gui.h"

#define QSP_LOGO _("Quest Soft Player 5")

BEGIN_EVENT_TABLE(QSPFrame, wxFrame)
	EVT_INIT(QSPFrame::OnInit)
	EVT_CLOSE(QSPFrame::OnClose)
	EVT_TIMER(ID_TIMER, QSPFrame::OnTimer)
	EVT_MENU(wxID_EXIT, QSPFrame::OnQuit)
	EVT_MENU(ID_OPENGAME, QSPFrame::OnOpenGame)
	EVT_MENU(ID_NEWGAME, QSPFrame::OnNewGame)
	EVT_MENU(ID_OPENGAMESTAT, QSPFrame::OnOpenGameStat)
	EVT_MENU(ID_SAVEGAMESTAT, QSPFrame::OnSaveGameStat)
	EVT_MENU(ID_SELECTFONT, QSPFrame::OnSelectFont)
	EVT_MENU(ID_SELECTFONTCOLOR, QSPFrame::OnSelectFontColor)
	EVT_MENU(ID_SELECTBACKCOLOR, QSPFrame::OnSelectBackColor)
	EVT_MENU(ID_SELECTLANG, QSPFrame::OnSelectLang)
	EVT_MENU(ID_TOGGLEWINMODE, QSPFrame::OnToggleWinMode)
	EVT_MENU(ID_TOGGLEOBJS, QSPFrame::OnToggleObjs)
	EVT_MENU(ID_TOGGLEACTS, QSPFrame::OnToggleActs)
	EVT_MENU(ID_TOGGLEDESC, QSPFrame::OnToggleDesc)
	EVT_MENU(ID_TOGGLEINPUT, QSPFrame::OnToggleInput)
	EVT_MENU(ID_TOGGLECAPTIONS, QSPFrame::OnToggleCaptions)
	EVT_MENU(ID_TOGGLEHOTKEYS, QSPFrame::OnToggleHotkeys)
	EVT_MENU(ID_ABOUT, QSPFrame::OnAbout)
	EVT_HTML_LINK_CLICKED(ID_MAINDESC, QSPFrame::OnLinkClicked)
	EVT_HTML_LINK_CLICKED(ID_VARSDESC, QSPFrame::OnLinkClicked)
	EVT_LISTBOX(ID_OBJECTS, QSPFrame::OnObjectChange)
	EVT_LISTBOX(ID_ACTIONS, QSPFrame::OnActionChange)
	EVT_LISTBOX_DCLICK(ID_ACTIONS, QSPFrame::OnActionDblClick)
	EVT_TEXT(ID_INPUT, QSPFrame::OnInputTextChange)
	EVT_ENTER(ID_INPUT, QSPFrame::OnInputTextEnter)
	EVT_KEY_UP(QSPFrame::OnKey)
	EVT_MOUSEWHEEL(QSPFrame::OnWheel)
	EVT_AUI_PANE_CLOSE(QSPFrame::OnPaneClose)
END_EVENT_TABLE()

IMPLEMENT_CLASS(QSPFrame, wxFrame)

QSPFrame::QSPFrame(const wxString &configPath, QSPTranslationHelper *transhelper) :
	wxFrame(0, wxID_ANY, wxEmptyString),
	m_configPath(configPath),
	m_transhelper(transhelper)
{
	SetIcon(wxIcon(logo_xpm));
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
	wxMenuItem *gameSaveItem = new wxMenuItem(m_gameMenu, ID_SAVEGAMESTAT, wxT("-"));
	gameSaveItem->SetBitmap(wxBitmap(statussave_xpm));
	m_gameMenu->Append(gameSaveItem);
	// ------------
	m_wndsMenu = new wxMenu;
	m_wndsMenu->Append(ID_TOGGLEOBJS, wxT("-"));
	m_wndsMenu->Append(ID_TOGGLEACTS, wxT("-"));
	m_wndsMenu->Append(ID_TOGGLEDESC, wxT("-"));
	m_wndsMenu->Append(ID_TOGGLEINPUT, wxT("-"));
	m_wndsMenu->AppendSeparator();
	m_wndsMenu->Append(ID_TOGGLECAPTIONS, wxT("-"));
	m_wndsMenu->Append(ID_TOGGLEHOTKEYS, wxT("-"));
	// ------------
	wxMenu *settingsMenu = new wxMenu;
	settingsMenu->Append(ID_SHOWHIDE, wxT("-"), m_wndsMenu);
	settingsMenu->Append(ID_SELECTFONT, wxT("-"));
	settingsMenu->Append(ID_SELECTFONTCOLOR, wxT("-"));
	settingsMenu->Append(ID_SELECTBACKCOLOR, wxT("-"));
	settingsMenu->AppendSeparator();
	wxMenuItem *settingsWinModeItem = new wxMenuItem(settingsMenu, ID_TOGGLEWINMODE, wxT("-"));
	settingsWinModeItem->SetBitmap(wxBitmap(windowmode_xpm));
	settingsMenu->Append(settingsWinModeItem);
	settingsMenu->AppendSeparator();
	settingsMenu->Append(ID_SELECTLANG, wxT("-"));
	// ------------
	wxMenu *helpMenu = new wxMenu;
	wxMenuItem *helpAboutItem = new wxMenuItem(helpMenu, ID_ABOUT, wxT("-"));
	helpAboutItem->SetBitmap(wxBitmap(about_xpm));
	helpMenu->Append(helpAboutItem);
	// ------------
	menuBar->Append(m_fileMenu, wxEmptyString);
	menuBar->Append(m_gameMenu, wxEmptyString);
	menuBar->Append(settingsMenu, wxEmptyString);
	menuBar->Append(helpMenu, wxEmptyString);
	SetMenuBar(menuBar);
	// --------------------------------------
	m_manager = new wxAuiManager(this);
	m_manager->SetDockSizeConstraint(0.5, 0.5);
	m_imgBack = new QSPImgCanvas(this, ID_BACKPIC);
	m_manager->AddPane(m_imgBack, wxAuiPaneInfo().Name(wxT("imgback")).MinSize(50, 50).BestSize(150, 150).CloseButton(false).MaximizeButton().Top().Hide());
	m_imgView = new QSPImgCanvas(this, ID_VIEWPIC);
	m_manager->AddPane(m_imgView, wxAuiPaneInfo().Name(wxT("imgview")).MinSize(50, 50).BestSize(150, 150).MaximizeButton().Top().Hide());
	m_desc = new QSPTextBox(this, ID_MAINDESC);
	m_manager->AddPane(m_desc, wxAuiPaneInfo().Name(wxT("desc")).CenterPane());
	m_objects = new QSPListBox(this, ID_OBJECTS);
	m_manager->AddPane(m_objects, wxAuiPaneInfo().Name(wxT("objs")).MinSize(50, 50).BestSize(100, 100).MaximizeButton().Right());
	m_actions = new QSPListBox(this, ID_ACTIONS, LB_EXTENDED);
	m_manager->AddPane(m_actions, wxAuiPaneInfo().Name(wxT("acts")).MinSize(50, 50).BestSize(100, 100).MaximizeButton().Bottom());
	m_vars = new QSPTextBox(this, ID_VARSDESC);
	m_manager->AddPane(m_vars, wxAuiPaneInfo().Name(wxT("vars")).MinSize(50, 50).BestSize(100, 100).MaximizeButton().Bottom());
	m_input = new QSPInputBox(this, ID_INPUT);
	m_manager->AddPane(m_input, wxAuiPaneInfo().Name(wxT("input")).MinSize(50, 20).BestSize(100, 20).Layer(1).Bottom());
	// --------------------------------------
	SetMinSize(wxSize(450, 300));
	m_isQuit = false;
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
	bool isMaximized;
	if (IsFullScreen()) ShowFullScreen(false);
	if (IsIconized()) Iconize(false);
	if (isMaximized = IsMaximized()) Maximize(false);
	wxFileConfig cfg(wxEmptyString, wxEmptyString, m_configPath);
	cfg.Write(wxT("General/BackColor"), m_backColor.Blue() << 16 | m_backColor.Green() << 8 | m_backColor.Red());
	cfg.Write(wxT("General/FontColor"), m_fontColor.Blue() << 16 | m_fontColor.Green() << 8 | m_fontColor.Red());
	cfg.Write(wxT("General/FontSize"), m_fontSize);
	cfg.Write(wxT("General/FontName"), m_fontName);
	cfg.Write(wxT("General/ShowHotkeys"), m_isShowHotkeys);
	cfg.Write(wxT("General/Panels"), m_manager->SavePerspective());
	m_transhelper->Save(cfg, wxT("General/Language"));
	wxRect r = GetRect();
	cfg.Write(wxT("Pos/Left"), r.GetLeft());
	cfg.Write(wxT("Pos/Top"), r.GetTop());
	cfg.Write(wxT("Pos/Width"), r.GetWidth());
	cfg.Write(wxT("Pos/Height"), r.GetHeight());
	cfg.Write(wxT("Pos/Maximize"), isMaximized);
}

void QSPFrame::LoadSettings()
{
	bool isMaximize;
	int x, y, w, h, temp;
	wxFileConfig cfg(wxEmptyString, wxEmptyString, m_configPath);
	cfg.Read(wxT("General/BackColor"), &temp, 0xE0E0E0);
	m_backColor = wxColour(temp);
	cfg.Read(wxT("General/FontColor"), &temp, 0x000000);
	m_fontColor = wxColour(temp);
	wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	cfg.Read(wxT("General/FontSize"), &m_fontSize, font.GetPointSize());
	cfg.Read(wxT("General/FontName"), &m_fontName, font.GetFaceName());
	cfg.Read(wxT("General/ShowHotkeys"), &m_isShowHotkeys, false);
	cfg.Read(wxT("Pos/Left"), &x, 10);
	cfg.Read(wxT("Pos/Top"), &y, 10);
	cfg.Read(wxT("Pos/Width"), &w, 850);
	cfg.Read(wxT("Pos/Height"), &h, 650);
	cfg.Read(wxT("Pos/Maximize"), &isMaximize, false);
	wxString panels(wxT("layout2|") \
		wxT("name=imgback;state=4196350;dir=1;layer=0;row=0;pos=0;prop=102431;bestw=413;besth=178;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("name=imgview;state=6293503;dir=1;layer=0;row=0;pos=1;prop=102431;bestw=413;besth=178;minw=50;minh=50;maxw=-1;maxh=-1;floatx=182;floaty=161;floatw=494;floath=368|") \
		wxT("name=desc;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=610;besth=330;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("name=objs;state=6293500;dir=2;layer=0;row=0;pos=0;prop=100000;bestw=216;besth=313;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("name=acts;state=6293500;dir=3;layer=0;row=0;pos=0;prop=124833;bestw=461;besth=196;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("name=vars;state=6293500;dir=3;layer=0;row=0;pos=1;prop=95618;bestw=365;besth=196;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("name=input;state=2099196;dir=3;layer=1;row=0;pos=0;prop=100000;bestw=832;besth=20;minw=50;minh=20;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|") \
		wxT("dock_size(5,0,0)=22|dock_size(3,0,0)=215|dock_size(2,0,0)=218|dock_size(3,1,0)=39|"));
	cfg.Read(wxT("General/Panels"), &panels);
	m_transhelper->Load(cfg, wxT("General/Language"));
	// -------------------------------------------------
	ApplyBackColor(m_backColor);
	ApplyFontColor(m_fontColor);
	ApplyFontSize(m_fontSize);
	ApplyFontName(m_fontName);
	RefreshUI();
	m_manager->LoadPerspective(panels);
	m_manager->RestoreMaximizedPane();
	SetSize(x, y, w, h);
	ShowPane(ID_VIEWPIC, false);
	ShowPane(ID_BACKPIC, false);
	ShowPane(ID_ACTIONS, true);
	ShowPane(ID_OBJECTS, true);
	ShowPane(ID_VARSDESC, true);
	ShowPane(ID_INPUT, true);
	ReCreateGUI();
	Show();
	if (isMaximize) Maximize();
}

void QSPFrame::EnableControls(bool status, bool isExtended)
{
	if (isExtended) m_fileMenu->Enable(ID_OPENGAME, status);
	m_fileMenu->Enable(ID_NEWGAME, status);
	m_gameMenu->Enable(ID_OPENGAMESTAT, status);
	m_gameMenu->Enable(ID_SAVEGAMESTAT, status);
	m_wndsMenu->Enable(ID_TOGGLEOBJS, status);
	m_wndsMenu->Enable(ID_TOGGLEACTS, status);
	m_wndsMenu->Enable(ID_TOGGLEDESC, status);
	m_wndsMenu->Enable(ID_TOGGLEINPUT, status);
	m_objects->Enable(status);
	m_actions->Enable(status);
	m_input->SetEditable(status);
	m_isProcessEvents = status;
}

void QSPFrame::ShowPane(wxWindowID id, bool isShow)
{
	long i;
	wxAuiPaneInfo &pane = m_manager->GetPane(FindWindow(id));
	wxAuiPaneInfoArray& allPanes = m_manager->GetAllPanes();
	for (i = (long)allPanes.GetCount() - 1; i >= 0; --i)
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
	long numVal;
	QSP_CHAR *strVal;
	wxColour setBackColor, setFontColor;
	wxString setFontName;
	int setFontSize;
	bool isRefresh = false;
	// --------------
	setBackColor = ((QSPCallBacks::GetVarValue(QSP_FMT("BCOLOR"), &numVal, &strVal) && numVal) ? numVal : m_backColor);
	if (setBackColor != m_desc->GetBackgroundColour())
	{
		ApplyBackColor(setBackColor);
		isRefresh = true;
	}
	// --------------
	setFontColor = ((QSPCallBacks::GetVarValue(QSP_FMT("FCOLOR"), &numVal, &strVal) && numVal) ? numVal : m_fontColor);
	if (setFontColor != m_desc->GetForegroundColour())
	{
		ApplyFontColor(setFontColor);
		isRefresh = true;
	}
	// --------------
	setFontSize = ((QSPCallBacks::GetVarValue(QSP_FMT("FSIZE"), &numVal, &strVal) && numVal) ? numVal : m_fontSize);
	if (setFontSize != m_desc->GetTextFont().GetPointSize())
	{
		ApplyFontSize(setFontSize);
		isRefresh = true;
	}
	// --------------
	setFontName = ((QSPCallBacks::GetVarValue(QSP_FMT("FNAME"), &numVal, &strVal) && strVal && *strVal) ? wxString(strVal) : m_fontName);
	if (setFontName != m_desc->GetTextFont().GetFaceName())
	{
		ApplyFontName(setFontName);
		isRefresh = true;
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
	wxString itemName(name);
	if (itemName == wxT("-"))
		m_menu->AppendSeparator();
	else
	{
		wxMenuItem *item = new wxMenuItem(m_menu, m_menuItemId, itemName);
		wxString itemPath(imgPath);
		itemPath.Replace(wxT("\\"), wxT("/"));
		if (wxFileExists(itemPath))
		{
			wxBitmap itemBmp(itemPath, wxBITMAP_TYPE_ANY);
			if (itemBmp.Ok()) item->SetBitmap(itemBmp);
		}
		m_menu->Append(item);
	}
	++m_menuItemId;
}

void QSPFrame::ShowMenu()
{
	PopupMenu(m_menu);
}

void QSPFrame::ShowError()
{
	bool oldIsProcessEvents;
	wxString wxMessage;
	QSP_CHAR *loc, *desc;
	long code, where, line;
	QSPGetLastErrorData(&code, &loc, &where, &line);
	desc = QSPGetErrorDesc(code);
	if (loc)
		wxMessage = wxString::Format(
			_("Location: %s\nArea: %s\nLine: %ld\nCode: %ld\nDesc: %s"),
			loc,
			(where == QSP_AREA_ONLOCVISIT ? _("on visit") : _("on action")),
			line,
			code,
			wxGetTranslation(desc)
		);
	else
		wxMessage = wxString::Format(
			_("Code: %ld\nDesc: %s"),
			code,
			wxGetTranslation(desc)
		);
	wxMessageDialog wxMsgDialog(this, wxMessage, _("Error"), wxOK | wxICON_ERROR);
	oldIsProcessEvents = m_isProcessEvents;
	m_isProcessEvents = false;
	wxMsgDialog.ShowModal();
	m_isProcessEvents = oldIsProcessEvents;
	QSPCallBacks::RefreshInt(QSP_FALSE);
}

void QSPFrame::ReCreateGUI()
{
	wxMenuBar *menuBar = GetMenuBar();
	SetTitle(QSP_LOGO);
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
	menuBar->SetLabel(ID_SAVEGAMESTAT, _("&Save game...\tCtrl-S"));
	menuBar->SetLabel(ID_TOGGLEOBJS, _("&Objects\tCtrl-1"));
	menuBar->SetLabel(ID_TOGGLEACTS, _("&Actions\tCtrl-2"));
	menuBar->SetLabel(ID_TOGGLEDESC, _("A&dditional desc\tCtrl-3"));
	menuBar->SetLabel(ID_TOGGLEINPUT, _("&Input area\tCtrl-4"));
	menuBar->SetLabel(ID_TOGGLECAPTIONS, _("&Captions\tCtrl-5"));
	menuBar->SetLabel(ID_TOGGLEHOTKEYS, _("&Hotkeys for actions\tCtrl-6"));
	menuBar->SetLabel(ID_SHOWHIDE, _("&Show / Hide"));
	menuBar->SetLabel(ID_SELECTFONT, _("Select &font...\tAlt-F"));
	menuBar->SetLabel(ID_SELECTFONTCOLOR, _("Select font's &color...\tAlt-C"));
	menuBar->SetLabel(ID_SELECTBACKCOLOR, _("Select &background's color...\tAlt-B"));
	menuBar->SetLabel(ID_TOGGLEWINMODE, _("Window / Fullscreen &mode\tAlt-Enter"));
	menuBar->SetLabel(ID_SELECTLANG, _("Select &language...\tAlt-L"));
	menuBar->SetLabel(ID_ABOUT, _("&About...\tCtrl-H"));
	// --------------------------------------
	m_manager->GetPane(wxT("imgback")).Caption(_("Image"));
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
	m_imgBack->RefreshUI();
}

void QSPFrame::ApplyFont(const wxFont& font)
{
	m_desc->SetTextFont(font);
	m_objects->SetTextFont(font);
	m_actions->SetTextFont(font);
	m_vars->SetTextFont(font);
	m_input->SetFont(font);
}

void QSPFrame::ApplyFontSize(int size)
{
	wxFont font(m_desc->GetTextFont());
	font.SetPointSize(size);
	ApplyFont(font);
}

void QSPFrame::ApplyFontName(const wxString& name)
{
	wxFont font(m_desc->GetTextFont());
	font.SetFaceName(name);
	ApplyFont(font);
}

void QSPFrame::ApplyFontColor(const wxColour& color)
{
	m_desc->SetForegroundColour(color);
	m_objects->SetForegroundColour(color);
	m_actions->SetForegroundColour(color);
	m_vars->SetForegroundColour(color);
	m_input->SetForegroundColour(color);
}

void QSPFrame::ApplyBackColor(const wxColour& color)
{
	m_desc->SetBackgroundColour(color);
	m_objects->SetBackgroundColour(color);
	m_actions->SetBackgroundColour(color);
	m_vars->SetBackgroundColour(color);
	m_input->SetBackgroundColour(color);
	m_imgBack->SetBackgroundColour(color);
	m_imgView->SetBackgroundColour(color);
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

void QSPFrame::TogglePane(wxWindowID id)
{
	bool isShow = !m_manager->GetPane(FindWindow(id)).IsShown();
	CallPaneFunc(id, (QSP_BOOL)isShow);
	ShowPane(id, isShow);
}

void QSPFrame::OnInit(wxInitEvent& event)
{
	wxCommandEvent dummy;
	if (QSPLoadGameWorld((QSP_CHAR *)event.GetInitString().wx_str()))
	{
		OnNewGame(dummy);
		EnableControls(true);
	}
	else
		ShowError();
}

void QSPFrame::OnClose(wxCloseEvent& event)
{
	event.Skip();
	SaveSettings();
	EnableControls(false, true);
	m_isQuit = true;
}

void QSPFrame::OnTimer(wxTimerEvent& event)
{
	if (m_isProcessEvents && !QSPExecCounter(QSP_TRUE))
		ShowError();
}

void QSPFrame::OnMenu(wxCommandEvent& event)
{
	QSPSelectMenuItem(event.GetId() - ID_BEGOFDYNMENU);
}

void QSPFrame::OnQuit(wxCommandEvent& event)
{
	Close();
}

void QSPFrame::OnOpenGame(wxCommandEvent& event)
{
	wxCommandEvent dummy;
	wxFileDialog dialog(this, _("Select game file"), wxEmptyString, wxEmptyString, _("QSP games (*.gam)|*.gam"), wxFD_OPEN);
	if (dialog.ShowModal() == wxID_OK)
	{
		if (QSPLoadGameWorld((QSP_CHAR *)dialog.GetPath().wx_str()))
		{
			OnNewGame(dummy);
			EnableControls(true);
		}
		else
			ShowError();
	}
}

void QSPFrame::OnNewGame(wxCommandEvent& event)
{
	if (!QSPRestartGame(QSP_TRUE))
		ShowError();
}

void QSPFrame::OnOpenGameStat(wxCommandEvent& event)
{
	wxFileDialog dialog(this, _("Select saved game file"), wxEmptyString, wxEmptyString, _("Saved game files (*.sav)|*.sav"), wxFD_OPEN);
	if (dialog.ShowModal() == wxID_OK && !QSPOpenSavedGame((QSP_CHAR *)dialog.GetPath().wx_str(), QSP_TRUE))
		ShowError();
}

void QSPFrame::OnSaveGameStat(wxCommandEvent& event)
{
	wxFileDialog dialog(this, _("Select file to save"), wxEmptyString, wxEmptyString, _("Saved game files (*.sav)|*.sav"), wxFD_SAVE);
	if (dialog.ShowModal() == wxID_OK && !QSPSaveGame((QSP_CHAR *)dialog.GetPath().wx_str(), QSP_TRUE))
		ShowError();
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

void QSPFrame::OnSelectLang(wxCommandEvent& event)
{
	if (m_transhelper->AskUserForLanguage()) ReCreateGUI();
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
	long i;
	bool isShow = !m_manager->GetPane(m_objects).HasCaption();
	wxAuiPaneInfoArray& allPanes = m_manager->GetAllPanes();
	for (i = (long)allPanes.GetCount() - 1; i >= 0; --i)
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
	info.SetCopyright(wxT("Byte Soft, 2001-2008"));
	info.SetDescription(wxString::Format(
		_("Version: %s\nEngine Compiled: %s\nGUI Compiled: %s"),
		QSPGetVersion(),
		QSPGetCompiledDateTime(),
		wxT(__DATE__) wxT(", ") wxT(__TIME__)
	));
	info.SetWebSite(wxT("http://qsp.org.ru"));
	// ----
	info.AddDeveloper(wxT("Byte [nporep@mail.ru]"));
	info.AddDocWriter(wxT("Korwin [tightbow@yandex.ru]"));
	info.AddArtist(wxT("3dEyes [3deyes@gmail.com]"));
	info.AddArtist(wxT("Alex [dogmar88@mail.ru]"));
	info.AddArtist(wxT("BalleR [vsevolod_kremyan@mail.ru]"));
	info.AddArtist(wxT("BaxZzZz [bauer_v@mail.ru]"));
	info.AddArtist(wxT("Belial [belgame@bk.ru]"));
	info.AddArtist(wxT("DzafT [dzaft@mail.ru]"));
	info.AddArtist(wxT("Fireton [fireton@mail.ru]"));
	info.AddArtist(wxT("Gilving [mrgilving@gmail.com]"));
	info.AddArtist(wxT("HIman [himan@rambler.ru]"));
	info.AddArtist(wxT("Lostas [lostas@mail.ru]"));
	info.AddArtist(wxT("Mioirel [mioirel@rambler.ru]"));
	info.AddArtist(wxT("Morgan [gorgonyte@mail.ru]"));
	info.AddArtist(wxT("Mortem [reijii@darthman.com]"));
	info.AddArtist(wxT("Nex [nex@otaku.ru]"));
	info.AddArtist(wxT("Ondoo [ondoo@mail.ru]"));
	info.AddArtist(wxT("RB [qsp1@narod.ru]"));
	// ----
	wxAboutBox(info);
}

void QSPFrame::OnLinkClicked(wxHtmlLinkEvent& event)
{
	long ind;
	wxString href;
	wxHtmlLinkInfo info(event.GetLinkInfo());
	if (info.GetEvent()->LeftUp())
	{
		href = info.GetHref();
		if (href[0] == wxT('#'))
		{
			if (event.GetId() == m_desc->GetId())
				m_desc->LoadPage(href);
			else
				m_vars->LoadPage(href);
		}
		else if (href.Upper().StartsWith(wxT("EXEC:")))
		{
			if (m_isProcessEvents && !QSPExecString((QSP_CHAR *)href.Mid(5).wx_str(), QSP_TRUE)) ShowError();
		}
		else if (href.ToLong(&ind))
		{
			if (m_isProcessEvents && ind > 0 && ind <= QSPGetActionsCount())
			{
				wxCommandEvent e;
				if (QSPSetSelActionIndex(ind - 1, QSP_TRUE))
					OnActionDblClick(e);
				else
					ShowError();
			}
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
	QSPSetInputStrText((QSP_CHAR *)text.wx_str());
}

void QSPFrame::OnInputTextEnter(wxCommandEvent& event)
{
	if (!QSPExecUserInput(QSP_TRUE))
		ShowError();
}

void QSPFrame::OnKey(wxKeyEvent& event)
{
	int ind;
	event.Skip();
	if (m_isProcessEvents && !event.HasModifiers() && wxWindow::FindFocus() != m_input)
	{
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
		default: return;
		}
		if (ind < QSPGetActionsCount())
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

void QSPFrame::OnPaneClose(wxAuiManagerEvent& event)
{
	if (m_isProcessEvents)
		CallPaneFunc(event.GetPane()->window->GetId(), QSP_FALSE);
	else
		event.Veto();
}
