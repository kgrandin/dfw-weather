#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QPixmap>

#include "weather.h"

int main(int argc, char** argv)
{
	QApplication app(argc,argv);
    weather mainwindow;

	mainwindow.show();

	app.exec();

	return 0;
}

