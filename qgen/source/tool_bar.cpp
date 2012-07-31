#include "precompiled.h"
#include "tool_bar.h"

namespace QGen
{

ToolBar::ToolBar( QWidget* parent, Settings* settings )
	:	QToolBar(parent),
		_settings(settings)
{
	Create();
}

void ToolBar::Create()
{
	_createLocAct = new QAction(QIcon(":/main_window/toolbar_location_new.xpm"), tr("Create location... (F7)"), this);
	_createLocAct->setShortcut(QKeySequence(Qt::Key_F7));
	connect(_createLocAct, SIGNAL(triggered()), this, SIGNAL(createLocation()));

	_renameLocAct = new QAction(QIcon(":/main_window/toolbar_location_delete.xpm"), tr("Delete selected location (F8)"), this);
	_renameLocAct->setShortcut((QKeySequence(Qt::Key_F8)));
	connect(_renameLocAct, SIGNAL(triggered()), this, SIGNAL(renameLocation()));

	_deleteLocAct = new QAction(QIcon(":/main_window/toolbar_location_rename.xpm"), tr("Rename selected location... (F6)"), this);
	_deleteLocAct->setShortcut(QKeySequence(Qt::Key_F6));
	connect(_deleteLocAct, SIGNAL(triggered()), this, SIGNAL(deleteLocation()));

	_openGameAct = new QAction(QIcon(":/main_window/toolbar_file_open.xpm"), tr("Open game... (Ctrl+O)"), this);
	_openGameAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
	connect(_openGameAct, SIGNAL(triggered()), this, SIGNAL(openGame()));

	_saveGameAct = new QAction(QIcon(":/main_window/toolbar_file_save.xpm"), tr("Save game (Ctrl+S)"), this);
	_saveGameAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	connect(_saveGameAct, SIGNAL(triggered()), this, SIGNAL(saveGame()));

	_saveGameAsAct = new QAction(QIcon(":/main_window/toolbar_file_saveas.xpm"), tr("Save game into another file... (Ctrl+W)"), this);
	_saveGameAsAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
	connect(_saveGameAsAct, SIGNAL(triggered()), this, SIGNAL(saveGameAs()));

	_runGameAct = new QAction(QIcon(":/main_window/toolbar_game_play.xpm"), tr("Run game (F5)"), this);
	_runGameAct->setShortcut(QKeySequence(Qt::Key_F5));
	connect(_runGameAct, SIGNAL(triggered()), this, SIGNAL(runGame()));

	_gameInfoAct = new QAction(QIcon(":/main_window/toolbar_game_info.xpm"), tr("Show game statistics (Ctrl+I)"), this);
	_gameInfoAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
	connect(_gameInfoAct, SIGNAL(triggered()), this, SIGNAL(gameInfo()));

	// TODO: shortcut can be changed
	_undoTextAct = new QAction(QIcon(":/main_window/toolbar_undo.xpm"), tr("Undo (Ctrl+Z)"), this);
	_undoTextAct->setShortcut(QKeySequence::Undo);
	connect(_undoTextAct, SIGNAL(triggered()), this, SIGNAL(undoText()));

	// TODO: shortcut can be changed
	_redoTextAct = new QAction(QIcon(":/main_window/toolbar_redo.xpm"), tr("Redo (Ctrl+Y)"), this);
	_redoTextAct->setShortcut(QKeySequence::Redo);
	connect(_redoTextAct, SIGNAL(triggered()), this, SIGNAL(redoText()));

	_copyLocAct = new QAction(QIcon(":/main_window/toolbar_location_copy.xpm"), tr("Copy selected location (Ctrl+Shift+C)"), this);
	_copyLocAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
	connect(_copyLocAct, SIGNAL(triggered()), this, SIGNAL(copyLocation()));

	_pasteLocAct = new QAction(QIcon(":/main_window/toolbar_location_paste.xpm"), tr("Paste location (Ctrl+Shift+V)"), this);
	_pasteLocAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V));
	connect(_pasteLocAct, SIGNAL(triggered()), this, SIGNAL(pasteLocation()));

	_clearLocAct = new QAction(QIcon(":/main_window/toolbar_location_clear.xpm"), tr("Clear selected location (Ctrl+Shift+D)"), this);
	_clearLocAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
	connect(_clearLocAct, SIGNAL(triggered()), this, SIGNAL(clearLocation()));

	// TODO: shortcut can be changed
	_findAct = new QAction(QIcon(":/main_window/toolbar_text_search.xpm"), tr("Find / Replace... (Ctrl+F)"), this);
	_findAct->setShortcut(QKeySequence::Find);
	connect(_findAct, SIGNAL(triggered()), this, SIGNAL(find()));

	_settingsAct = new QAction(QIcon(":/main_window/toolbar_options.xpm"), tr("Settings... (Ctrl+P)"), this);
	_settingsAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
	connect(_settingsAct, SIGNAL(triggered()), this, SIGNAL(setSettings()));
	
	addAction(_createLocAct);
	addAction(_renameLocAct);
	addAction(_deleteLocAct);
	addSeparator();
	addAction(_openGameAct);
	addAction(_saveGameAct);
	addAction(_saveGameAsAct);
	addSeparator();
	addAction(_runGameAct);
	addAction(_gameInfoAct);
	addSeparator();
	addAction(_undoTextAct);
	addAction(_redoTextAct);
	addSeparator();
	addAction(_copyLocAct);
	addAction(_pasteLocAct);
	addAction(_clearLocAct);
	addSeparator();
	addAction(_findAct);
	addAction(_settingsAct);
}

} // namespace QGen
