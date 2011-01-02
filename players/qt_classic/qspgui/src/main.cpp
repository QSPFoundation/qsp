#include "stdafx.h"
#include "main_window.h"

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	
	Ui::MainWindow window;
	window.show();

	return application.exec();
}
