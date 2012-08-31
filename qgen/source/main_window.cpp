#include "precompiled.h"
#include "main_window.h"

namespace QGen
{

MainWindow::MainWindow(Settings* settings, QWidget *parent, Qt::WFlags flags)
	:	QMainWindow(parent, flags),
		_settings(settings)
{
	setMenuBar(new MenuBar(parent, _settings));
	addToolBar(new ToolBar(parent, _settings));
	setStatusBar(new QStatusBar());
	createDockWindows();
}

MainWindow::~MainWindow()
{

}

void MainWindow::createDockWindows()
{
	QDockWidget* dock = new QDockWidget(tr("Locations"), this);
	_locsListBox = new LocationsListBox(dock, _settings);

	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock->setWidget(_locsListBox);
	addDockWidget(Qt::RightDockWidgetArea, dock);
}

} // namespace QGen

