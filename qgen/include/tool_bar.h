#ifndef __QGEN_TOOL_BAR_H__
#define __QGEN_TOOL_BAR_H__

#include "settings.h"

namespace QGen
{

class ToolBar : public QToolBar
{
	Q_OBJECT
public:
	ToolBar(QWidget* parent, Settings* settings);

signals:

private:
	void Create();

private:
	Settings*	_settings;
	QAction*	_createLocAct;
	QAction*	_renameLocAct;
	QAction*	_deleteLocAct;
	QAction*	_openGameAct;
	QAction*	_saveGameAct;
	QAction*	_saveGameAsAct;
	QAction*	_runGameAct;
	QAction*	_gameInfoAct;
	QAction*	_undoTextAct;
	QAction*	_redoTextAct;
	QAction*	_copyLocAct;
	QAction*	_pasteLocAct;
	QAction*	_clearLocAct;
	QAction*	_findAct;
	QAction*	_settingsAct;
};

} // namespace QGen

#endif // __QGEN_TOOL_BAR_H__
