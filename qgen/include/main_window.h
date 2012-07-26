#ifndef __QGEN_MAIN_WINDOW_H__
#define __QGEN_MAIN_WINDOW_H__

#include "i_main_window_view.h"
#include "menu_bar.h"

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
	void createMenuBar();
	void createActions();
	void createToolBar();
	void createStatusBar();

private:
	MenuBar*	_menuBar;
	Settings*	_settings;
};

} // namespace QGen

#endif // __QGEN_MAIN_WINDOW_H__
