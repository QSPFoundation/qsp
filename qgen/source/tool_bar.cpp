#include "precompiled.h"
#include "tool_bar.h"

namespace QGen
{

ToolBar::ToolBar( QWidget* parent, Settings* settings )
	:	QToolBar(parent),
		_settings(settings)
{

// 	ToolBar::ToolBar( wxWindow *parent, wxWindowID id, IControls *controls ) : wxAuiToolBar( parent, id )
// 	{
// 		_controls = controls;
// 		SetToolBitmapSize(wxSize(24, 24));
// 		AddTool(ID_LOC_CREATE, wxEmptyString, wxBitmap(toolbar_location_new_xpm));
// 		AddTool(ID_LOC_RENAME, wxEmptyString, wxBitmap(toolbar_location_rename_xpm));
// 		AddTool(ID_LOC_DEL, wxEmptyString, wxBitmap(toolbar_location_delete_xpm));
// 		AddSeparator();
// 		AddTool(ID_GAME_OPEN, wxEmptyString, wxBitmap(toolbar_file_open_xpm));
// 		AddTool(ID_GAME_SAVE, wxEmptyString, wxBitmap(toolbar_file_save_xpm));
// 		AddTool(ID_GAME_SAVEAS, wxEmptyString, wxBitmap(toolbar_file_saveas_xpm));
// 		AddSeparator();
// 		AddTool(ID_GAME_PLAY, wxEmptyString, wxBitmap(toolbar_game_play_xpm));
// 		AddTool(ID_UTIL_INF, wxEmptyString, wxBitmap(toolbar_game_info_xpm));
// 		AddSeparator();
// 		AddTool(ID_TEXT_UNDO, wxEmptyString, wxBitmap(toolbar_undo_xpm));
// 		AddTool(ID_TEXT_REDO, wxEmptyString, wxBitmap(toolbar_redo_xpm));
// 		AddSeparator();
// 		AddTool(ID_LOC_COPY, wxEmptyString, wxBitmap(toolbar_location_copy_xpm));
// 		AddTool(ID_LOC_PASTE, wxEmptyString, wxBitmap(toolbar_location_paste_xpm));
// 		AddTool(ID_LOC_CLEAR, wxEmptyString, wxBitmap(toolbar_location_clear_xpm));
// 		AddSeparator();
// 		AddTool(ID_UTIL_FIND, wxEmptyString, wxBitmap(toolbar_text_search_xpm));
// 		AddTool(ID_UTIL_OPTIONS, wxEmptyString, wxBitmap(toolbar_options_xpm));
// 		Realize();
// 		Update();
// 		_controls->GetSettings()->AddObserver(this);
// 	}
// 
// 	ToolBar::~ToolBar()
// 	{
// 		_controls->GetSettings()->RemoveObserver(this);
// 	}
// 
// 	void ToolBar::Update( bool isFromObservable /*= false*/ )
// 	{
// 		SetToolShortHelp(ID_LOC_CREATE, _("Create location... (F7)"));
// 		SetToolShortHelp(ID_LOC_RENAME, _("Rename selected location... (F6)"));
// 		SetToolShortHelp(ID_LOC_DEL, _("Delete selected location (F8)"));
// 		SetToolShortHelp(ID_GAME_OPEN, _("Open game... (Ctrl+O)"));
// 		SetToolShortHelp(ID_GAME_SAVE, _("Save game (Ctrl+S)"));
// 		SetToolShortHelp(ID_GAME_SAVEAS, _("Save game into another file... (Ctrl+W)"));
// 		SetToolShortHelp(ID_GAME_PLAY, _("Run game (F5)"));
// 		SetToolShortHelp(ID_UTIL_INF, _("Show game statistics (Ctrl+I)"));
// 		SetToolShortHelp(ID_TEXT_UNDO, _("Undo (Ctrl+Z)"));
// 		SetToolShortHelp(ID_TEXT_REDO, _("Redo (Ctrl+Y)"));
// 		SetToolShortHelp(ID_LOC_COPY, _("Copy selected location (Ctrl+Shift+C)"));
// 		SetToolShortHelp(ID_LOC_PASTE, _("Paste location (Ctrl+Shift+V)"));
// 		SetToolShortHelp(ID_LOC_CLEAR, _("Clear selected location (Ctrl+Shift+D)"));
// 		SetToolShortHelp(ID_UTIL_FIND, _("Find / Replace... (Ctrl+F)"));
// 		SetToolShortHelp(ID_UTIL_OPTIONS, _("Settings... (Ctrl+P)"));
// 	}

	Create();
}

void ToolBar::Create()
{
	_createLocAct = new QAction(QIcon(":/main_window/toolbar_location_new.xpm"), tr("Create location... (F7)"), this);
	_createLocAct->setShortcut(QKeySequence(Qt::Key_F7));

	_renameLocAct = new QAction(QIcon(":/main_window/toolbar_location_delete.xpm"), tr("Delete selected location (F8)"), this);
	_renameLocAct->setShortcut((QKeySequence(Qt::Key_F8)));

	_deleteLocAct = new QAction(QIcon(":/main_window/toolbar_location_rename.xpm"), tr("Rename selected location... (F6)"), this);
	_deleteLocAct->setShortcut(QKeySequence(Qt::Key_F6));

	_openGameAct = new QAction(QIcon(":/main_window/toolbar_file_open.xpm"), tr("Open game... (Ctrl+O)"), this);
	_openGameAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));

	_saveGameAct = new QAction(QIcon(":/main_window/toolbar_file_save.xpm"), tr("Save game (Ctrl+S)"), this);
	_saveGameAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));

	_saveGameAsAct = new QAction(QIcon(":/main_window/toolbar_file_saveas.xpm"), tr("Save game into another file... (Ctrl+W)"), this);
	_saveGameAsAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));

	_runGameAct = new QAction(QIcon(":/main_window/toolbar_game_play.xpm"), tr("Run game (F5)"), this);
	_runGameAct->setShortcut(QKeySequence(Qt::Key_F5));

	_gameInfoAct = new QAction(QIcon(":/main_window/toolbar_game_info.xpm"), tr("Show game statistics (Ctrl+I)"), this);
	_gameInfoAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));

	_undoTextAct = new QAction(QIcon(":/main_window/toolbar_undo.xpm"), tr("Undo (Ctrl+Z)"), this);
	_undoTextAct->setShortcut(QKeySequence::Undo);

	_redoTextAct = new QAction(QIcon(":/main_window/toolbar_redo.xpm"), tr("Redo (Ctrl+Y)"), this);
	_redoTextAct->setShortcut(QKeySequence::Redo);

	_copyLocAct = new QAction(QIcon(":/main_window/toolbar_location_copy.xpm"), tr("Copy selected location (Ctrl+Shift+C)"), this);
	_copyLocAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));

	_pasteLocAct = new QAction(QIcon(":/main_window/toolbar_location_paste.xpm"), tr("Paste location (Ctrl+Shift+V)"), this);
	_pasteLocAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V));

	_clearLocAct = new QAction(QIcon(":/main_window/toolbar_location_clear.xpm"), tr("Clear selected location (Ctrl+Shift+D)"), this);
	_clearLocAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));

	_findAct = new QAction(QIcon(":/main_window/toolbar_text_search.xpm"), tr("Find / Replace... (Ctrl+F)"), this);
	_findAct->setShortcut(QKeySequence::Find);

	_settingsAct = new QAction(QIcon(":/main_window/toolbar_options.xpm"), tr("Settings... (Ctrl+P)"), this);
	_settingsAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
	
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
