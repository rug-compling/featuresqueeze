#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

#include "DataSet.hh"
#include "OverlapMainWindow.hh"

using namespace std;

SelectedFeaturesPtr readSelectedFeatures(char *filename)
{
	SelectedFeaturesPtr selected(new SelectedFeatures);

	QFile featureFile(filename);
	if (!featureFile.open(QFile::ReadOnly))
	{
		qCritical() << "Could not open" << filename << "for reading!";
		return selected;
	}

	QTextStream featureStream(&featureFile);

	while(!featureStream.atEnd())
	{
		QString featureLine(featureStream.readLine());
		QString overlapLine(featureStream.readLine());
		QStringList lineParts = featureLine.split("\t");

		SelectedFeature feature(lineParts[0].toULong(), lineParts[1].toDouble(),
			lineParts[2].toDouble());
		selected->push_back(feature);
	}

	return selected;
}

void usage(char *programName)
{
	qCritical() << "Usage:" << programName << "features selection";
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		usage(argv[0]);
		return 1;
	}

	QApplication app(argc, argv);

	SelectedFeaturesPtr features(readSelectedFeatures(argv[2]));

	overlapviewer::OverlapMainWindow mainWindow(features);
	mainWindow.show();
	return app.exec();
}
