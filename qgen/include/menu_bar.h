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
	QMenu*		_exportMenu;
	QMenu*		_importMenu;

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
};

} // namespace QGen

#endif // __QGEN_MENU_BAR_H__
