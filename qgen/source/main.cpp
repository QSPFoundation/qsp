#include "precompiled.h"
#include "main_window.h"
#include "settings.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QGen::Settings settings;
	QGen::MainWindow mainWindow(&settings);

	mainWindow.show();

	return a.exec();
}
