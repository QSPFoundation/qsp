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
	_newGameAct = new QAction(QIcon(":/main_window/menu_game_new.png"), tr("&New"), this);
	_newGameAct->setShortcuts(QKeySequence::New);
	_newGameAct->setStatusTip(tr("Create a new game"));

	_openGameAct = new QAction(QIcon(":/main_window/menu_game_open.png"), tr("&Open..."), this);
	_openGameAct->setShortcuts(QKeySequence::Open);
	_openGameAct->setStatusTip(tr("Open game file"));

	_mergeGameAct = new QAction(tr("&Merge game..."), this);
	_mergeGameAct->setShortcut(Qt::CTRL + Qt::Key_M);
	_mergeGameAct->setStatusTip(tr("Add locations from another game"));

	_saveGameAct = new QAction(QIcon(":/main_window/menu_game_save.png"), tr("&Save"), this);
	_saveGameAct->setShortcuts(QKeySequence::Save);
	_saveGameAct->setStatusTip(tr("Save game"));

	_saveAsGameAct = new QAction(tr("Save &as..."), this);
	_saveAsGameAct->setShortcuts(QKeySequence::SaveAs);
	_saveAsGameAct->setStatusTip(tr("Save game into another file..."));

	_exportTextAct = new QAction(tr("Text file..."), this);
	_exportText2GamAct = new QAction(tr("Text file in TXT2GAM format..."), this);

	_importText2GamAct = new QAction(tr("Text file in TXT2GAM format..."), this);

	_exitAppAct = new QAction(QIcon(":/main_window/menu_game_exit.png"), tr("&Exit"), this);
	_exitAppAct->setShortcuts(QKeySequence::Quit);
	_exitAppAct->setStatusTip(tr("Close program"));

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
	_runGameAct = new QAction(QIcon(""), tr("&Run game"), this);
	_runGameAct->setShortcuts(QKeySequence::Refresh);
	_runGameAct->setStatusTip(tr("Run current game"));

	_findAct = new QAction(QIcon(""), tr("&Find / Replace"), this);
	_findAct->setShortcuts(QKeySequence::Find);
	_findAct->setStatusTip(tr("Find / replace some text"));

	_spellcheckAct = new QAction(QIcon(), tr("&Check spelling..."), this);
	_spellcheckAct->setStatusTip(tr("Check spelling of some text"));

	_gameInfoAct = new QAction(QIcon(), tr("&Game info"), this);

	_gameInfoAct->setShortcut(Qt::CTRL + Qt::Key_I);
	_gameInfoAct->setStatusTip(tr("Show short statistics"));

	_settingsAct = new QAction(tr("&Settings..."), this);
	_settingsAct->setShortcut(Qt::CTRL + Qt::Key_P);

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

	_renameLocAct = new QAction(tr("&Rename..."), this);
	_renameLocAct->setShortcut(Qt::Key_F6);
	_renameLocAct->setStatusTip(tr("Rename location"));

	_deleteLocAct = new QAction(tr("&Delete"), this);
	_deleteLocAct->setShortcut(Qt::Key_F8);
	_deleteLocAct->setStatusTip(tr("Delete location"));

	_createFolderAct = new QAction(tr("Create folder..."), this);
	_createFolderAct->setStatusTip(tr("Create folder for locations"));
	
	_renameFolderAct = new QAction(tr("Rename folder..."), this);
	_renameFolderAct->setStatusTip(tr("Rename selected folder"));

	_deleteFolderAct = new QAction(tr("Delete folder"), this);
	_deleteFolderAct->setStatusTip(tr("Delete folder, but keep locations"));

	_copyLocAct = new QAction(tr("&Copy"), this);
	_copyLocAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_C);
	_copyLocAct->setStatusTip(tr("Copy selected location to clipboard"));

	_pasteLocAct = new QAction(tr("&Paste"), this);
	_pasteLocAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_V);
	_pasteLocAct->setStatusTip(tr("Paste location from clipboard"));

	_replaceLocAct = new QAction(tr("&Replace"), this);
	_replaceLocAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_R);
	_replaceLocAct->setStatusTip(tr("Replace selected location with clipboard data"));

	_pasteInLocAct = new QAction(tr("P&aste in..."), this);
	_pasteInLocAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_N);
	_pasteInLocAct->setStatusTip(tr("Paste clipboard data to the new location"));

	_clearLocAct = new QAction(tr("C&lear"), this);
	_clearLocAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_D);
	_clearLocAct->setStatusTip(tr("Clear location"));

	_createActAct = new QAction(tr("&Create"), this);
	_createActAct->setShortcut(Qt::ALT + Qt::Key_F7);
	_createActAct->setStatusTip(tr("Create action on selected location"));
	
	_renameActAct = new QAction(tr("&Rename..."), this);
	_renameActAct->setShortcut(Qt::ALT + Qt::Key_F6);
	_renameActAct->setStatusTip(tr("Rename selected action"));

	_deleteActAct = new QAction(tr("&Delete"), this);
	_deleteActAct->setShortcut(Qt::ALT + Qt::Key_F8);
	_deleteActAct->setStatusTip("Delete selected action");

	_deleteAllActAct = new QAction(tr("D&elete all"), this);
	_deleteAllActAct->setShortcut(Qt::ALT + Qt::Key_F10);
	_deleteAllActAct->setStatusTip("Delete all actions");

	_sortLocAscAct = new QAction(tr("So&rt ascending"), this);
	_sortLocAscAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_O);
	
	_sortLocDescAct = new QAction(tr("Sor&t descending"), this);
	_sortLocDescAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_P);

	_jumpLocAct = new QAction(tr("G&o to selected location"), this);
	_jumpLocAct->setShortcut(Qt::CTRL + Qt::Key_G);

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
	_undoTextAct = new QAction(QIcon(""), tr("&Undo"), this);
	_undoTextAct->setShortcut(QKeySequence::Undo);
	
	_redoTextAct = new QAction(QIcon(""), tr("&Redo"), this);
	_redoTextAct->setShortcut(QKeySequence::Redo);

	_cutTextAct = new QAction(QIcon(""), tr("&Cut"), this);
	_cutTextAct->setShortcut(QKeySequence::Cut);

	_copyTextAct = new QAction(QIcon(""), tr("C&opy"), this);
	_copyTextAct->setShortcut(QKeySequence::Copy);

	_pasteTextAct = new QAction(QIcon(""), tr("&Paste"), this);
	_pasteTextAct->setShortcut(QKeySequence::Paste);

	_deleteTextAct = new QAction(QIcon(""), tr("&Delete"), this);
	_deleteTextAct->setShortcut(QKeySequence::Delete);

	_selectAllTextAct = new QAction(QIcon(""), tr("S&elect all"), this);
	_selectAllTextAct->setShortcut(QKeySequence::SelectAll);

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
	_locsListAct = new QAction(tr("&Locations list"), this);
	_statusBarAct = new QAction(tr("&Statusbar"), this);

	_closeAllTabsAct = new QAction(tr("&Close all tabs"), this);
	_closeAllTabsAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_F4);
	
	_closeAllTabsExCurrAct = new QAction(tr("Close all tabs &except current"), this);

	_closeCurrentTabAct = new QAction(tr("Close c&urrent tab"), this);
	_closeCurrentTabAct->setShortcut(Qt::CTRL + Qt::Key_F4);

	_pinUnpinTabAct = new QAction(tr("Pin/Unpin &tab"), this);
	
	_showHideLocsDescAct = new QAction(tr("Show/Hide location's &description"), this);
	_showHideLocsDescAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_D);

	_showHideLocsActsAct = new QAction(tr("Show/Hide location's &actions"), this);
	_showHideLocsActsAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_A);
	
	_windowsListMenu = new QMenu(tr("&Windows list"));
	_windowsListMenu->addAction(_toolBarAct);
	_windowsListMenu->addAction(_locsListAct);
	_windowsListMenu->addAction(_statusBarAct);

	_viewMenu = addMenu(tr("&View"));
	_viewMenu->addMenu(_windowsListMenu);
	_viewMenu->addSeparator();
	_viewMenu->addAction(_closeAllTabsAct);
	_viewMenu->addAction(_closeAllTabsExCurrAct);
	_viewMenu->addAction(_closeCurrentTabAct);
	_viewMenu->addSeparator();
	_viewMenu->addAction(_pinUnpinTabAct);
	_viewMenu->addSeparator();
	_viewMenu->addAction(_showHideLocsDescAct);
	_viewMenu->addAction(_showHideLocsActsAct);
}

void MenuBar::createHelpMenu()
{
	_helpMenu = addMenu(tr("&Help"));
}

} // namespace QGen
