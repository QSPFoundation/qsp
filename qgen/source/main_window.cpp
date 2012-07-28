#include "precompiled.h"
#include "main_window.h"

namespace QGen
{

MainWindow::MainWindow(Settings* settings, QWidget *parent, Qt::WFlags flags)
	:	QMainWindow(parent, flags),
		_settings(settings)
{
	createMenuBar();
}

MainWindow::~MainWindow()
{

}

void MainWindow::createMenuBar()
{
	_menuBar = new MenuBar(NULL, _settings);
	QMainWindow::setMenuBar(_menuBar);
}

void MainWindow::createToolBar()
{

}

void MainWindow::createStatusBar()
{

}

} // namespace QGen
