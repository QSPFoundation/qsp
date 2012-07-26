#include "precompiled.h"
#include "main_window.h"

namespace QGen
{

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	createActions();
	createMenus();
}

MainWindow::~MainWindow()
{

}

void MainWindow::createActions()
{

}

void MainWindow::createMenus()
{
	_gameMenu	= menuBar()->addMenu(tr("&Game"));
	_utilsMenu	= menuBar()->addMenu(tr("&Utilities"));
	_locsMenu	= menuBar()->addMenu(tr("&Locations"));
	_textMenu	= menuBar()->addMenu(tr("&Text"));
	_viewMenu	= menuBar()->addMenu(tr("&View"));
	_helpMenu	= menuBar()->addMenu(tr("&Help"));
}

void MainWindow::createToolBar()
{

}

void MainWindow::createStatusBar()
{

}

} // namespace QGen
