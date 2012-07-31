#include "precompiled.h"
#include "menu_bar.h"

namespace QGen
{

MenuBar::MenuBar(QWidget* parent, Settings* settings )
	:	QMenuBar(parent),
		_settings(settings)
{
	createGameMenu();
	createUtilsMenu();
	createLocsMenu();
	createTextMenu();
	createViewMenu();
	createHelpMenu();
}

void MenuBar::createGameMenu()
{
	_newGameAct = new QAction(QIcon(":/main_window/menu_game_new.xpm"), tr("&New"), this);
	_newGameAct->setShortcuts(QKeySequence::New);
	_newGameAct->setStatusTip(tr("Create a new game"));
	connect(_newGameAct, SIGNAL(triggered()), this, SIGNAL(createNewGame()));

	_openGameAct = new QAction(QIcon(":/main_window/menu_file_open.xpm"), tr("&Open..."), this);
	_openGameAct->setShortcuts(QKeySequence::Open);
	_openGameAct->setStatusTip(tr("Open game file"));
	connect(_newGameAct, SIGNAL(triggered()), this, SIGNAL(openGame()));

	_mergeGameAct = new QAction(tr("&Merge game..."), this);
	_mergeGameAct->setShortcut(Qt::CTRL + Qt::Key_M);
	_mergeGameAct->setStatusTip(tr("Add locations from another game"));
	connect(_mergeGameAct, SIGNAL(triggered()), this, SIGNAL(mergeGame()));

	_saveGameAct = new QAction(QIcon(":/main_window/menu_file_save.xpm"), tr("&Save"), this);
	_saveGameAct->setShortcuts(QKeySequence::Save);
	_saveGameAct->setStatusTip(tr("Save game"));
	connect(_saveGameAct, SIGNAL(triggered()), this, SIGNAL(saveGame()));

	_saveAsGameAct = new QAction(tr("Save &as..."), this);
	_saveAsGameAct->setShortcuts(QKeySequence::SaveAs);
	_saveAsGameAct->setStatusTip(tr("Save game into another file..."));
	connect(_saveAsGameAct, SIGNAL(triggered()), this, SIGNAL(saveAsGame()));

	_exportTextAct = new QAction(tr("Text file..."), this);
	connect(_exportTextAct, SIGNAL(triggered()), this, SIGNAL(exportText()));

	_exportText2GamAct = new QAction(tr("Text file in TXT2GAM format..."), this);
	connect(_exportText2GamAct, SIGNAL(triggered()), this, SIGNAL(exportText2Gam()));

	_importText2GamAct = new QAction(tr("Text file in TXT2GAM format..."), this);
	connect(_importText2GamAct, SIGNAL(triggered()), this, SIGNAL(importText2Gam()));

	_exitAppAct = new QAction(QIcon(":/main_window/menu_exit.xpm"), tr("&Exit"), this);
	_exitAppAct->setShortcuts(QKeySequence::Quit);
	_exitAppAct->setStatusTip(tr("Close program"));
	connect(_exitAppAct, SIGNAL(triggered()), this, SIGNAL(exitApp()));

	_gameMenu	= addMenu(tr("&Game"));

	_exportSubMenu	= new QMenu(tr("&Export"));
	_importSubMenu	= new QMenu(tr("&Import"));

	_exportSubMenu->addAction(_exportTextAct);
	_exportSubMenu->addAction(_exportText2GamAct);
	_importSubMenu->addAction(_importText2GamAct);

	_gameMenu->addAction(_newGameAct);
	_gameMenu->addAction(_openGameAct);
	_gameMenu->addAction(_mergeGameAct);
	_gameMenu->addAction(_saveGameAct);
	_gameMenu->addAction(_saveAsGameAct);
	_gameMenu->addSeparator();
	_gameMenu->addMenu(_exportSubMenu);
	_gameMenu->addMenu(_importSubMenu);
	_gameMenu->addSeparator();
	_gameMenu->addAction(_exitAppAct);
}

void MenuBar::createUtilsMenu()
{
	_runGameAct = new QAction(QIcon(":/main_window/menu_game_play.xpm"), tr("&Run game"), this);
	_runGameAct->setShortcuts(QKeySequence::Refresh);
	_runGameAct->setStatusTip(tr("Run current game"));
	connect(_runGameAct, SIGNAL(triggered()), this, SIGNAL(runGame()));

	_findAct = new QAction(QIcon(":/main_window/menu_text_search.xpm"), tr("&Find / Replace"), this);
	_findAct->setShortcuts(QKeySequence::Find);
	_findAct->setStatusTip(tr("Find / replace some text"));
	connect(_findAct, SIGNAL(triggered()), this, SIGNAL(find()));

	_spellcheckAct = new QAction(tr("&Check spelling..."), this);
	_spellcheckAct->setStatusTip(tr("Check spelling of some text"));
	connect(_spellcheckAct, SIGNAL(triggered()), this, SIGNAL(spellcheck()));

	_gameInfoAct = new QAction(QIcon(":/main_window/menu_game_info.xpm"), tr("&Game info"), this);
	_gameInfoAct->setShortcut(Qt::CTRL + Qt::Key_I);
	_gameInfoAct->setStatusTip(tr("Show short statistics"));
	connect(_gameInfoAct, SIGNAL(triggered()), this, SIGNAL(gameInfo()));
	
	_settingsAct = new QAction(tr("&Settings..."), this);
	_settingsAct->setShortcut(Qt::CTRL + Qt::Key_P);
	connect(_settingsAct, SIGNAL(triggered()), this, SIGNAL(setSettings()));

	_utilsMenu	= addMenu(tr("&Utilities"));

	_utilsMenu->addAction(_runGameAct);
	_utilsMenu->addAction(_findAct);
	_utilsMenu->addAction(_spellcheckAct);
	_utilsMenu->addAction(_gameInfoAct);
	_utilsMenu->addSeparator();
	_utilsMenu->addAction(_settingsAct);
}

void MenuBar::createLocsMenu()
{
	_createLocAct = new QAction(tr("&Create..."), this);
	_createLocAct->setShortcut(Qt::Key_F7);
	_createLocAct->setStatusTip(tr("Create location"));
	connect(_createLocAct, SIGNAL(triggered()), this, SIGNAL(createLocation()));

	_renameLocAct = new QAction(tr("&Rename..."), this);
	_renameLocAct->setShortcut(Qt::Key_F6);
	_renameLocAct->setStatusTip(tr("Rename location"));
	connect(_renameLocAct, SIGNAL(triggered()), this, SIGNAL(renameLocation()));

	_deleteLocAct = new QAction(tr("&Delete"), this);
	_deleteLocAct->setShortcut(Qt::Key_F8);
	_deleteLocAct->setStatusTip(tr("Delete location"));
	connect(_deleteLocAct, SIGNAL(triggered()), this, SIGNAL(deleteLocation()));

	_createFolderAct = new QAction(tr("Create folder..."), this);
	_createFolderAct->setStatusTip(tr("Create folder for locations"));
	connect(_createFolderAct, SIGNAL(triggered()), this, SIGNAL(createFolder()));
	
	_renameFolderAct = new QAction(tr("Rename folder..."), this);
	_renameFolderAct->setStatusTip(tr("Rename selected folder"));
	connect(_renameFolderAct, SIGNAL(triggered()), this, SIGNAL(renameFolder()));

	_deleteFolderAct = new QAction(tr("Delete folder"), this);
	_deleteFolderAct->setStatusTip(tr("Delete folder, but keep locations"));
	connect(_deleteFolderAct, SIGNAL(triggered()), this, SIGNAL(deleteFolder()));

	_copyLocAct = new QAction(tr("&Copy"), this);
	_copyLocAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_C);
	_copyLocAct->setStatusTip(tr("Copy selected location to clipboard"));
	connect(_copyLocAct, SIGNAL(triggered()), this, SIGNAL(copyLocation()));

	_pasteLocAct = new QAction(tr("&Paste"), this);
	_pasteLocAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_V);
	_pasteLocAct->setStatusTip(tr("Paste location from clipboard"));
	connect(_pasteLocAct, SIGNAL(triggered()), this, SIGNAL(pasteLocation()));

	_replaceLocAct = new QAction(tr("&Replace"), this);
	_replaceLocAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_R);
	_replaceLocAct->setStatusTip(tr("Replace selected location with clipboard data"));
	connect(_replaceLocAct, SIGNAL(triggered()), this, SIGNAL(replaceLocation()));

	_pasteInLocAct = new QAction(tr("P&aste in..."), this);
	_pasteInLocAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_N);
	_pasteInLocAct->setStatusTip(tr("Paste clipboard data to the new location"));
	connect(_pasteInLocAct, SIGNAL(triggered()), this, SIGNAL(pasteInLocation()));

	_clearLocAct = new QAction(tr("C&lear"), this);
	_clearLocAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_D);
	_clearLocAct->setStatusTip(tr("Clear location"));
	connect(_clearLocAct, SIGNAL(triggered()), this, SIGNAL(clearLocation()));

	_createActAct = new QAction(tr("&Create"), this);
	_createActAct->setShortcut(Qt::ALT + Qt::Key_F7);
	_createActAct->setStatusTip(tr("Create action on selected location"));
	connect(_createActAct, SIGNAL(triggered()), this, SIGNAL(createAction()));
	
	_renameActAct = new QAction(tr("&Rename..."), this);
	_renameActAct->setShortcut(Qt::ALT + Qt::Key_F6);
	_renameActAct->setStatusTip(tr("Rename selected action"));
	connect(_renameActAct, SIGNAL(triggered()), this, SIGNAL(renameAction()));

	_deleteActAct = new QAction(tr("&Delete"), this);
	_deleteActAct->setShortcut(Qt::ALT + Qt::Key_F8);
	_deleteActAct->setStatusTip("Delete selected action");
	connect(_deleteActAct, SIGNAL(triggered()), this, SIGNAL(deleteAction()));

	_deleteAllActAct = new QAction(tr("D&elete all"), this);
	_deleteAllActAct->setShortcut(Qt::ALT + Qt::Key_F10);
	_deleteAllActAct->setStatusTip("Delete all actions");
	connect(_deleteAllActAct, SIGNAL(triggered()), this, SIGNAL(deleteAction()));

	_sortLocAscAct = new QAction(tr("So&rt ascending"), this);
	_sortLocAscAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_O);
	connect(_sortLocAscAct, SIGNAL(triggered()), this, SIGNAL(sortLocationAsc()));
	
	_sortLocDescAct = new QAction(tr("Sor&t descending"), this);
	_sortLocDescAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_P);
	connect(_sortLocDescAct, SIGNAL(triggered()), this, SIGNAL(sortLocationDesc()));

	_jumpLocAct = new QAction(tr("G&o to selected location"), this);
	_jumpLocAct->setShortcut(Qt::CTRL + Qt::Key_G);
	connect(_jumpLocAct, SIGNAL(triggered()), this, SIGNAL(jumpLocation()));

	_locsMenu = addMenu(tr("&Locations"));

	_actionsSubMenu = new QMenu(tr("&Actions"));
	_actionsSubMenu->addAction(_createActAct);
	_actionsSubMenu->addAction(_renameActAct);
	_actionsSubMenu->addAction(_deleteActAct);
	_actionsSubMenu->addAction(_deleteAllActAct);

	_locsMenu->addAction(_createLocAct);
	_locsMenu->addAction(_renameLocAct);
	_locsMenu->addAction(_deleteLocAct);
	_locsMenu->addSeparator();
	_locsMenu->addAction(_createFolderAct);
	_locsMenu->addAction(_renameFolderAct);
	_locsMenu->addAction(_deleteFolderAct);
	_locsMenu->addSeparator();
	_locsMenu->addAction(_copyLocAct);
	_locsMenu->addAction(_pasteLocAct);
	_locsMenu->addAction(_replaceLocAct);
	_locsMenu->addAction(_pasteInLocAct);
	_locsMenu->addAction(_clearLocAct);
	_locsMenu->addSeparator();
	_locsMenu->addMenu(_actionsSubMenu);
	_locsMenu->addSeparator();
	_locsMenu->addAction(_sortLocAscAct);
	_locsMenu->addAction(_sortLocDescAct);
	_locsMenu->addSeparator();
	_locsMenu->addAction(_jumpLocAct);
}

void MenuBar::createTextMenu()
{
	_undoTextAct = new QAction(QIcon(":/main_window/menu_undo.xpm"), tr("&Undo"), this);
	_undoTextAct->setShortcut(QKeySequence::Undo);
	connect(_undoTextAct, SIGNAL(triggered()), this, SIGNAL(undoText()));
	
	_redoTextAct = new QAction(QIcon(":/main_window/menu_redo.xpm"), tr("&Redo"), this);
	_redoTextAct->setShortcut(QKeySequence::Redo);
	connect(_redoTextAct, SIGNAL(triggered()), this, SIGNAL(redoText()));

	_cutTextAct = new QAction(QIcon(":/main_window/menu_text_cut.xpm"), tr("&Cut"), this);
	_cutTextAct->setShortcut(QKeySequence::Cut);
	connect(_cutTextAct, SIGNAL(triggered()), this, SIGNAL(cutText()));

	_copyTextAct = new QAction(QIcon(":/main_window/menu_text_copy.xpm"), tr("C&opy"), this);
	_copyTextAct->setShortcut(QKeySequence::Copy);
	connect(_copyTextAct, SIGNAL(triggered()), this, SIGNAL(copyText()));

	_pasteTextAct = new QAction(QIcon(":/main_window/menu_text_paste.xpm"), tr("&Paste"), this);
	_pasteTextAct->setShortcut(QKeySequence::Paste);
	connect(_pasteTextAct, SIGNAL(triggered()), this, SIGNAL(pasteText()));

	_deleteTextAct = new QAction(QIcon(":/main_window/menu_text_delete.xpm"), tr("&Delete"), this);
	_deleteTextAct->setShortcut(QKeySequence::Delete);
	connect(_deleteTextAct, SIGNAL(triggered()), this, SIGNAL(deleteText()));

	_selectAllTextAct = new QAction(tr("S&elect all"), this);
	_selectAllTextAct->setShortcut(QKeySequence::SelectAll);
	connect(_selectAllTextAct, SIGNAL(triggered()), this, SIGNAL(selectAllText()));

	_textMenu = addMenu(tr("&Text"));

	_textMenu->addAction(_undoTextAct);
	_textMenu->addAction(_redoTextAct);
	_textMenu->addSeparator();
	_textMenu->addAction(_cutTextAct);
	_textMenu->addAction(_copyTextAct);
	_textMenu->addAction(_pasteTextAct);
	_textMenu->addAction(_deleteTextAct);
	_textMenu->addSeparator();
	_textMenu->addAction(_selectAllTextAct);
}

void MenuBar::createViewMenu()
{
	_toolBarAct = new QAction(tr("&Toolbar"), this);
	_toolBarAct->setCheckable(true);
	connect(_toolBarAct, SIGNAL(triggered(bool)), this, SIGNAL(showToolBar(bool)));

	_locsListAct = new QAction(tr("&Locations list"), this);
	_locsListAct->setCheckable(true);
	connect(_locsListAct, SIGNAL(triggered(bool)), this, SIGNAL(showLocsList(bool)));

	_statusBarAct = new QAction(tr("&Statusbar"), this);
	_statusBarAct->setCheckable(true);
	connect(_statusBarAct, SIGNAL(triggered(bool)), this, SIGNAL(showStatusBar(bool)));

	_closeAllTabsAct = new QAction(tr("&Close all tabs"), this);
	_closeAllTabsAct->setCheckable(true);
	_closeAllTabsAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_F4);
	connect(_closeAllTabsAct, SIGNAL(triggered()), this, SIGNAL(closeAllTabs()));
	
	_closeAllTabsExcCurAct = new QAction(tr("Close all tabs &except current"), this);
	_closeAllTabsExcCurAct->setCheckable(true);
	connect(_closeAllTabsExcCurAct, SIGNAL(triggered()), this, SIGNAL(closeAllTabsExcCur()));

	_closeCurrentTabAct = new QAction(tr("Close c&urrent tab"), this);
	_closeCurrentTabAct->setShortcut(Qt::CTRL + Qt::Key_F4);
	connect(_closeCurrentTabAct, SIGNAL(triggered()), this, SIGNAL(closeCurrentTab()));

	_pinUnpinTabAct = new QAction(tr("Pin/Unpin &tab"), this);
	connect(_pinUnpinTabAct, SIGNAL(triggered()), this, SIGNAL(pinTab()));
		
	_hideLocsDescAct = new QAction(tr("Show/Hide location's &description"), this);
	_hideLocsDescAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_D);
	connect(_hideLocsDescAct, SIGNAL(triggered()), this, SIGNAL(hideLocsActs()));

	_hideLocsActsAct = new QAction(tr("Show/Hide location's &actions"), this);
	_hideLocsActsAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_A);
	connect(_hideLocsActsAct, SIGNAL(triggered()), this, SIGNAL(hideLocsDesc()));
	
	_windowsListMenu = new QMenu(tr("&Windows list"));
	_windowsListMenu->addAction(_toolBarAct);
	_windowsListMenu->addAction(_locsListAct);
	_windowsListMenu->addAction(_statusBarAct);

	_viewMenu = addMenu(tr("&View"));
	_viewMenu->addMenu(_windowsListMenu);
	_viewMenu->addSeparator();
	_viewMenu->addAction(_closeAllTabsAct);
	_viewMenu->addAction(_closeAllTabsExcCurAct);
	_viewMenu->addAction(_closeCurrentTabAct);
	_viewMenu->addSeparator();
	_viewMenu->addAction(_pinUnpinTabAct);
	_viewMenu->addSeparator();
	_viewMenu->addAction(_hideLocsDescAct);
	_viewMenu->addAction(_hideLocsActsAct);
}

void MenuBar::createHelpMenu()
{
	_helpAct = new QAction(QIcon(":/main_window/menu_help.xpm"), tr("&Help"), this);
	_helpAct->setShortcut(QKeySequence::HelpContents);
	connect(_helpAct, SIGNAL(triggered()), this, SIGNAL(showHelp()));
	
	_helpByKeywordAct = new QAction(QIcon(":/main_window/menu_help_search.xpm"), tr("Help by &keyword"), this);
	_helpByKeywordAct->setShortcut(QKeySequence::WhatsThis);
	connect(_helpByKeywordAct, SIGNAL(triggered()), this, SIGNAL(showHelpByKeyword()));

	_aboutAct = new QAction(tr("&About..."), this);
	connect(_aboutAct, SIGNAL(triggered()), this, SIGNAL(showAbout()));

	_helpMenu = addMenu(tr("&Help"));

	_helpMenu->addAction(_helpAct);
	_helpMenu->addAction(_helpByKeywordAct);
	_helpMenu->addSeparator();
	_helpMenu->addAction(_aboutAct);
}

} // namespace QGen

