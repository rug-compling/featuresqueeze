#include <QApplication>

#include "OverlapMainWindow.hh"

using namespace std;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	overlapviewer::OverlapMainWindow mainWindow;
	mainWindow.show();
	return app.exec();
}
