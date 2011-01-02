#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

namespace Ui
{

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = NULL, Qt::WFlags flags = 0);
	~MainWindow();

private:
	void CreateMenuBar();
private:
	QMenu*		_fileMenu;
	QMenu*		_gameMenu;
	QMenu*		_settingsMenu;
};

} // namespace Ui

#endif // MAIN_WINDOW_H
