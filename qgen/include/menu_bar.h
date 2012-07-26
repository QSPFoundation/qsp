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
	Settings*	_settins;
	QMenu*		_gameMenu;
	QMenu*		_utilsMenu;
	QMenu*		_locsMenu;
	QMenu*		_textMenu;
	QMenu*		_viewMenu;
	QMenu*		_helpMenu;
};

} // namespace QGen

#endif // __QGEN_MENU_BAR_H__
