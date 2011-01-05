#include "stdafx.h"
#include "main_window.h"

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	application.setApplicationName("Quest Soft Player");
	application.setApplicationVersion("5.0.0");

	Ui::MainWindow window;
	window.show();

	return application.exec();
}
