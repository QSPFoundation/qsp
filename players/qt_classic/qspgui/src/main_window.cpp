#include "stdafx.h"
#include "main_window.h"

namespace Ui
{

MainWindow::MainWindow(QWidget* parent, Qt::WFlags flags) :
	QMainWindow(parent, flags)
{
	setMinimumSize(QSize(300, 200));
	
	CreateMenuBar();
}

MainWindow::~MainWindow()
{

}

void MainWindow::CreateMenuBar()
{
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
	
	// Game menu
	_gameMenu = menuBar()->addMenu(tr("&Game"));
	// Open saved game item
	_gameMenu->addAction(QIcon(":/menu/statusopen"), tr("Open saved game..."),
		this, SLOT(OnOpenSavedGame()), QKeySequence(Qt::CTRL + Qt::Key_O));
	// Save game item
	_gameMenu->addAction(QIcon(":/menu/statussave"), tr("Save game..."),
		this, SLOT(OnSaveGame()), QKeySequence(Qt::CTRL + Qt::Key_S));

	// Settings menu
	_settingsMenu = menuBar()->addMenu(tr("&Settings"));
	
	// Show / Hide submenu
	QMenu* showHide = _settingsMenu->addMenu(tr("Show / Hide"));
	// Objects item
	showHide->addAction(tr("Objects"), this, SLOT(OnToggleObjs()),
		QKeySequence(Qt::CTRL + Qt::Key_1));
	// Actions item
	showHide->addAction(tr("Actions"), this, SLOT(OnToggleActs()),
		QKeySequence(Qt::CTRL + Qt::Key_2));
	// Additional desc item
	showHide->addAction(tr("Additional desc"), this, SLOT(OnToggleDesc()),
		QKeySequence(Qt::CTRL + Qt::Key_3));
	// Input area item
	showHide->addAction(tr("Input area"), this, SLOT(OnToggleInput()),
		QKeySequence(Qt::CTRL + Qt::Key_4));

	showHide->addSeparator();
	
	// Captions item
	showHide->addAction(tr("Captions"), this, SLOT(OnToggleCaptions()),
		QKeySequence(Qt::CTRL + Qt::Key_5));
	// Hotkeys for actions item
	showHide->addAction(tr("Hotkeys for actions"), this, SLOT(OnToggleHotkeys()),
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

	// Help menu
	QMenu* helpMenu(menuBar()->addMenu(tr("&Help")));
	// About item
	helpMenu->addAction(QIcon(":/menu/about"), tr("About..."),
		this, SLOT(OnAbout()), QKeySequence(Qt::CTRL + Qt::Key_H));

}

void MainWindow::OnOpenGame()
{

}

void MainWindow::OnRestartGame()
{

}

void MainWindow::OnExit()
{

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

void MainWindow::OnToggleObjs()
{

}

void MainWindow::OnToggleActs()
{

}

void MainWindow::OnToggleDesc()
{

}

void MainWindow::OnToggleInput()
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
