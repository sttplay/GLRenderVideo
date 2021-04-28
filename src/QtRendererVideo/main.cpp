#include "QtRendererVideo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QtRendererVideo w;
	w.show();
	return a.exec();
}
