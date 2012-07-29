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
};

} // namespace QGen

#endif // __QGEN_MENU_BAR_H__
