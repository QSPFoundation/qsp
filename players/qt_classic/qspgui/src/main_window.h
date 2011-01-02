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
private slots:
	void OnOpenGame();
	void OnRestartGame();
	void OnExit();
	void OnOpenSavedGame();
	void OnSaveGame();
	void OnOptions();
	void OnAbout();
	void OnToggleObjs();
	void OnToggleActs();
	void OnToggleDesc();
	void OnToggleInput();
	void OnToggleCaptions();
	void OnToggleHotkeys();
	void OnToggleWinMode();
	void OnChangeSoundVolume();
};

} // namespace Ui

#endif // MAIN_WINDOW_H
