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

	_exportMenu	= new QMenu(tr("&Export"));
	_importMenu	= new QMenu(tr("&Import"));

	_exportMenu->addAction(_exportTextAct);
	_exportMenu->addAction(_exportText2GamAct);
	_importMenu->addAction(_importText2GamAct);

	_gameMenu->addAction(_newGameAct);
	_gameMenu->addAction(_openGameAct);
	_gameMenu->addAction(_mergeGameAct);
	_gameMenu->addAction(_saveGameAct);
	_gameMenu->addAction(_saveAsGameAct);
	_gameMenu->addSeparator();
	_gameMenu->addMenu(_exportMenu);
	_gameMenu->addMenu(_importMenu);
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
	_locsMenu = addMenu(tr("&Locations"));
}

void MenuBar::createTextMenu()
{
	_textMenu = addMenu(tr("&Text"));
}

void MenuBar::createViewMenu()
{
	_viewMenu = addMenu(tr("&View"));
}

void MenuBar::createHelpMenu()
{
	_helpMenu = addMenu(tr("&Help"));
}

} // namespace QGen
