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
}

MainWindow::~MainWindow()
{

}

} // namespace QGen
