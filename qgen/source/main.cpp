#include "precompiled.h"
#include "main_window.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	QGen::MainWindow mainWindow;
	mainWindow.show();

	return a.exec();
}
