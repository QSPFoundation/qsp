#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include "qsp_textbox.h"
#include "qsp_listbox.h"
#include "qsp_inputbox.h"
#include "qsp_imgcanvas.h"

#define TITLE "Quest Soft Player 5"
#define SETTINGS_FILENAME "settings.ini"

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
	void CreateDockWindows();
	void LoadSettings();
	void SaveSettings();
	void closeEvent(QCloseEvent *event);
private:

	QMenu*			_fileMenu;
	QMenu*			_gameMenu;
	QMenu*			_settingsMenu;
	QMenu*			_showHideMenu;
	QspTextBox*		_mainDescTextBox;
	QspListBox*		_objectsListBox;
	QspListBox*		_actionsListBox;
	QspTextBox*		_descTextBox;
	QspInputBox*	_inputTextBox;
	QDockWidget*	_objectsWidget;
	QDockWidget*	_actionsWidget;
	QDockWidget*	_descWidget;
	QDockWidget*	_inputWidget;
private slots:
	void OnOpenGame();
	void OnRestartGame();
	void OnOpenSavedGame();
	void OnSaveGame();
	void OnOptions();
	void OnAbout();
	void OnToggleCaptions();
	void OnToggleHotkeys();
	void OnToggleWinMode();
	void OnChangeSoundVolume();
};

} // namespace Ui

#endif // _MAIN_WINDOW_H_
