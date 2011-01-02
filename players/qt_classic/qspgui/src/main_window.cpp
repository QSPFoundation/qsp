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
	// file menu
	_fileMenu = menuBar()->addMenu(tr("&Quest"));

	// game menu
	_gameMenu = menuBar()->addMenu(tr("&Game"));

	// settings menu
	_settingsMenu = menuBar()->addMenu(tr("&Settings"));

	// help menu
	QMenu* helpMenu(menuBar()->addMenu(tr("&Help")));

}

} // namespace Ui
