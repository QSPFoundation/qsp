#include "stdafx.h"
#include "main_window.h"

namespace Ui
{

MainWindow::MainWindow(QWidget* parent, Qt::WFlags flags) :
	QMainWindow(parent, flags)
{
	setMinimumSize(QSize(300, 200));
	setWindowTitle(TITLE);
	setDockNestingEnabled(true);

	resize( 640, 480 );

	// Set QMainWindow in the center of desktop
	QRect rect = geometry();
	rect.moveCenter(QApplication::desktop()->availableGeometry().center());
	setGeometry(rect);

	_mainDescTextBox = new QspTextBox(this);
	setCentralWidget(_mainDescTextBox);

	CreateDockWindows();
	CreateMenuBar();
}

MainWindow::~MainWindow()
{

}

void MainWindow::CreateMenuBar()
{
	//------------------------------------------------------------------
	// File menu
	_fileMenu = menuBar()->addMenu(tr("&Quest"));

	// Open item
	_fileMenu->addAction(QIcon(":/menu/open"), tr("Open game..."),
		this, SLOT(OnOpenGame()), QKeySequence(Qt::ALT + Qt::Key_O));

	// New game item
	_fileMenu->addAction(QIcon(":/menu/new"),tr("Restart game"),
		this, SLOT(OnRestartGame()), QKeySequence(Qt::ALT + Qt::Key_N));

	_fileMenu->addSeparator();

	// Exit item
	_fileMenu->addAction(QIcon(":/menu/exit"), tr("Exit"),
		this, SLOT(OnExit()), QKeySequence(Qt::ALT + Qt::Key_X));
	//------------------------------------------------------------------
	// Game menu
	_gameMenu = menuBar()->addMenu(tr("&Game"));

	// Open saved game item
	_gameMenu->addAction(QIcon(":/menu/statusopen"), tr("Open saved game..."),
		this, SLOT(OnOpenSavedGame()), QKeySequence(Qt::CTRL + Qt::Key_O));

	// Save game item
	_gameMenu->addAction(QIcon(":/menu/statussave"), tr("Save game..."),
		this, SLOT(OnSaveGame()), QKeySequence(Qt::CTRL + Qt::Key_S));
	//------------------------------------------------------------------
	// Settings menu
	_settingsMenu = menuBar()->addMenu(tr("&Settings"));
	
	// Show / Hide submenu
	_showHideMenu = _settingsMenu->addMenu(tr("Show / Hide"));

	// Objects item 
	QAction* action = _objectsWidget->toggleViewAction();
	action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
	_showHideMenu->addAction(action);

	// Actions item 
	action = _actionsWidget->toggleViewAction();
	action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
	_showHideMenu->addAction(action);

	// Additional desc item 
	action = _descWidget->toggleViewAction();
	action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
	_showHideMenu->addAction(action);

	// Input area item 
	action = _inputWidget->toggleViewAction();
	action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_4));
	_showHideMenu->addAction(action);

	_showHideMenu->addSeparator();

	// Captions item
	_showHideMenu->addAction(tr("Captions"), this, SLOT(OnToggleCaptions()),
		QKeySequence(Qt::CTRL + Qt::Key_5));

	// Hotkeys for actions item
	_showHideMenu->addAction(tr("Hotkeys for actions"), this, SLOT(OnToggleHotkeys()),
		QKeySequence(Qt::CTRL + Qt::Key_6));
		
	// Sound volume item
	_settingsMenu->addAction(tr("Sound volume..."),
		this, SLOT(OnChangeSoundVolume()), QKeySequence(Qt::ALT + Qt::Key_V));

	// Window / Fullscreen mode item
	_settingsMenu->addAction(QIcon(":/menu/windowmode"), tr("Window / Fullscreen mode"),
		this, SLOT(OnToggleWinMode()), QKeySequence(Qt::ALT + Qt::Key_Enter));

	_settingsMenu->addSeparator();

	// Options item
	_settingsMenu->addAction(tr("Options..."),
		this, SLOT(OnOptions()), QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_O));
	//------------------------------------------------------------------
	// Help menu
	QMenu* helpMenu(menuBar()->addMenu(tr("&Help")));

	// About item
	helpMenu->addAction(QIcon(":/menu/about"), tr("About..."),
		this, SLOT(OnAbout()), QKeySequence(Qt::CTRL + Qt::Key_H));
}

void MainWindow::CreateDockWindows()
{
	// "Objects" widget
	_objectsWidget = new QDockWidget(tr("Objects"), this);
	addDockWidget(Qt::RightDockWidgetArea, _objectsWidget, Qt::Vertical);
	_objectsListBox = new QspListBox(this);
	_objectsWidget->setWidget(_objectsListBox);

	// "Actions" widget
	_actionsWidget = new QDockWidget(tr("Actions"), this);
	addDockWidget(Qt::RightDockWidgetArea, _actionsWidget, Qt::Vertical);
	_actionsListBox = new QspListBox(this);
	_actionsWidget->setWidget(_actionsListBox);

	// "Additional desc" widget
	_descWidget = new QDockWidget(tr("Additional desc"), this);
	addDockWidget(Qt::BottomDockWidgetArea, _descWidget, Qt::Horizontal);
	_descTextBox = new QspTextBox(this);
	_descWidget->setWidget(_descTextBox);

	// "Input area" widget
	_inputWidget = new QDockWidget(tr("Input area"), this);
	addDockWidget(Qt::TopDockWidgetArea, _inputWidget, Qt::Vertical);
	_inputTextBox = new QspInputBox(this);
	_inputWidget->setWidget(_inputTextBox);
}


void MainWindow::OnOpenGame()
{

}

void MainWindow::OnRestartGame()
{

}

void MainWindow::OnExit()
{
	close();
}

void MainWindow::OnAbout()
{

}

void MainWindow::OnOptions()
{

}

void MainWindow::OnOpenSavedGame()
{

}

void MainWindow::OnSaveGame()
{

}

void MainWindow::OnToggleCaptions()
{

}

void MainWindow::OnToggleHotkeys()
{

}

void MainWindow::OnToggleWinMode()
{

}

void MainWindow::OnChangeSoundVolume()
{

}

} // namespace Ui
