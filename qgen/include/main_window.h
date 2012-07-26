#ifndef __QGEN_MAIN_WINDOW_H__
#define __QGEN_MAIN_WINDOW_H__

#include "i_main_window_view.h"

namespace QGen
{

class MainWindow : public QMainWindow, public IMainWindowView
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

private slots:
	
private:
	void createMenus();
	void createActions();
	void createToolBar();
	void createStatusBar();

private:
	QMenu*		_gameMenu;
	QMenu*		_utilsMenu;
	QMenu*		_locsMenu;
	QMenu*		_textMenu;
	QMenu*		_viewMenu;
	QMenu*		_helpMenu;

};

} // namespace QGen

#endif // __QGEN_MAIN_WINDOW_H__
