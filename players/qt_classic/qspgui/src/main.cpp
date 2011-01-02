#include "stdafx.h"
#include "main_window.h"

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);

// 	application.setStyle("plastique"); 
// 	application.setPalette(application.style()->standardPalette());

	Ui::MainWindow window;
	window.show();

	return application.exec();
}
