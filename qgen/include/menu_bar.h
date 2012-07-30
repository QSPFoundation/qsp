#ifndef __QGEN_MENU_BAR_H__
#define __QGEN_MENU_BAR_H__

#include "settings.h"

namespace QGen
{

class MenuBar : public QMenuBar
{
	Q_OBJECT
public:
	MenuBar(QWidget* parent, Settings* settings);

signals:
	void createNewGame();
	void openGame();
	void mergeGame();
	void saveGame();
	void saveAsGame();
	void exitApp();
	void exportText();
	void exportText2Gam();
	void importText2Gam();
	void runGame();
	void find();
	void spellcheck();
	void gameInfo();
	void setSettings();
	void createLocation();
	void renameLocation();
	void deleteLocation();
	void createFolder();
	void renameFolder();
	void deleteFolder();
	void copyLocation();
	void pasteLocation();
	void replaceLocation();
	void pasteInLocation();
	void clearLocation();
	void createAction();
	void renameAction();
	void deleteAction();
	void deleteAllAction();
	void sortLocationAsc();
	void sortLocationDesc();
	void jumpLocation();
	void undoText();
	void redoText();
	void cutText();
	void copyText();
	void pasteText();
	void deleteText();
	void selectAllText();
	void showToolBar(bool show);
	void showLocsList(bool show);
	void showStatusBar(bool show);
	void closeAllTabs();
	void closeAllTabsExcCur();
	void closeCurrentTab();
	void pinTab();
	void hideLocsDesc();
	void hideLocsActs();
	void showHelp();
	void showHelpByKeyword();
	void showAbout();

private:
	void createMenus();
	void createGameMenu();
	void createUtilsMenu();
	void createLocsMenu();
	void createTextMenu();
	void createViewMenu();
	void createHelpMenu();

private:
	Settings*	_settings;
	QMenu*		_gameMenu;
	QMenu*		_utilsMenu;
	QMenu*		_locsMenu;
	QMenu*		_textMenu;
	QMenu*		_viewMenu;
	QMenu*		_helpMenu;
	QMenu*		_exportSubMenu;
	QMenu*		_importSubMenu;
	QMenu*		_actionsSubMenu;
	QMenu*		_windowsListMenu;

	QAction*	_newGameAct;
	QAction*	_openGameAct;
	QAction*	_mergeGameAct;
	QAction*	_saveGameAct;
	QAction*	_saveAsGameAct;
	QAction*	_exitAppAct;
	QAction*	_exportTextAct;
	QAction*	_exportText2GamAct;
	QAction*	_importText2GamAct;

	QAction*	_runGameAct;
	QAction*	_findAct;
	QAction*	_spellcheckAct;
	QAction*	_gameInfoAct;
	QAction*	_settingsAct;

	QAction*	_createLocAct;
	QAction*	_renameLocAct;
	QAction*	_deleteLocAct;
	QAction*	_createFolderAct;
	QAction*	_renameFolderAct;
	QAction*	_deleteFolderAct;
	QAction*	_copyLocAct;
	QAction*	_pasteLocAct;
	QAction*	_replaceLocAct;
	QAction*	_pasteInLocAct;
	QAction*	_clearLocAct;

	QAction*	_createActAct;
	QAction*	_renameActAct;
	QAction*	_deleteActAct;
	QAction*	_deleteAllActAct;

	QAction*	_sortLocAscAct;
	QAction*	_sortLocDescAct;
	QAction*	_jumpLocAct;

	QAction*	_undoTextAct;
	QAction*	_redoTextAct;
	QAction*	_cutTextAct;
	QAction*	_copyTextAct;
	QAction*	_pasteTextAct;
	QAction*	_deleteTextAct;
	QAction*	_selectAllTextAct;

	QAction*	_toolBarAct;
	QAction*	_locsListAct;
	QAction*	_statusBarAct;
	QAction*	_closeAllTabsAct;
	QAction*	_closeAllTabsExcCurAct;
	QAction*	_closeCurrentTabAct;
	QAction*	_pinUnpinTabAct;
	QAction*	_hideLocsDescAct;
	QAction*	_hideLocsActsAct;

	QAction*	_helpAct;
	QAction*	_helpByKeywordAct;
	QAction*	_aboutAct;
};

} // namespace QGen

#endif // __QGEN_MENU_BAR_H__
