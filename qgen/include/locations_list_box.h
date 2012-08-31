#ifndef __QGEN_LOCATIONS_LIST_BOX_H__
#define __QGEN_LOCATIONS_LIST_BOX_H__

#include "settings.h"

namespace QGen
{

class LocationsListBox : public QListWidget
{
	Q_OBJECT
public:
	LocationsListBox(QWidget* parent, Settings* settings);

private:
	Settings*	_settings;
};

} // namespace QGen

#endif // __QGEN_LOCATIONS_LIST_BOX_H__
