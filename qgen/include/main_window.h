#ifndef __QGEN_MAIN_WINDOW_H__
#define __QGEN_MAIN_WINDOW_H__

#include "i_main_window_view.h"
#include "menu_bar.h"
#include "tool_bar.h"
#include "locations_list_box.h"

namespace QGen
{

class MainWindow : public QMainWindow, public IMainWindowView
{
	Q_OBJECT
public:
	MainWindow(Settings* settings, QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

private slots:
	
private:
	void createDockWindows();

private:
	Settings*			_settings;
	MenuBar*			_menuBar;
	ToolBar*			_toolBar;
	QStatusBar*			_statusBar;
	LocationsListBox*	_locsListBox;
};

} // namespace QGen

#endif // __QGEN_MAIN_WINDOW_H__

