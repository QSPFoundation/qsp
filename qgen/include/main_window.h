#ifndef __QGEN_MAIN_WINDOW_H__
#define __QGEN_MAIN_WINDOW_H__

#include <QtGui/QMainWindow>

namespace QGen
{

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

private:
	
};

} // namespace QGen

#endif // __QGEN_MAIN_WINDOW_H__
