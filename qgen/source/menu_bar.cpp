#include "precompiled.h"
#include "menu_bar.h"

namespace QGen
{

MenuBar::MenuBar(QWidget* parent, Settings* settings )
	:	QMenuBar(parent)
{
	_gameMenu	= addMenu(tr("&Game"));
	_utilsMenu	= addMenu(tr("&Utilities"));
	_locsMenu	= addMenu(tr("&Locations"));
	_textMenu	= addMenu(tr("&Text"));
	_viewMenu	= addMenu(tr("&View"));
	_helpMenu	= addMenu(tr("&Help"));
}

} // namespace QGen
