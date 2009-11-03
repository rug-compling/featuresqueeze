#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

#include "DataSet.hh"
#include "OverlapMainWindow.hh"

using namespace std;

DataSetPtr readSelectedFeatures(char *filename)
{
	SelectedFeatures selectedFeatures;
	OverlappingFeatures overlappingFeatures;

	QFile featureFile(filename);
	if (!featureFile.open(QFile::ReadOnly))
	{
		qCritical() << "Could not open" << filename << "for reading!";
		return QSharedPointer<DataSet>(new DataSet(selectedFeatures, overlappingFeatures));
	}

	QTextStream featureStream(&featureFile);

	while(!featureStream.atEnd())
	{
		QString featureLine(featureStream.readLine());		
		QStringList lineParts = featureLine.split("\t");
		size_t featureNr = lineParts[0].toULong();

		SelectedFeature feature(featureNr, lineParts[1].toDouble(),
			lineParts[2].toDouble());
		selectedFeatures.push_back(feature);

		QString overlapLine(featureStream.readLine());
		QStringList overlapParts = overlapLine.split("\t", QString::SkipEmptyParts);

		QVector<OverlappingFeature> overlaps;
		for (QStringList::const_iterator iter = overlapParts.begin();
			iter != overlapParts.end(); ++iter)
		{
			QStringList overlap = iter->split('#');
			//qDebug(overlap[0]);
			//qDebug(overlap[1]));
			OverlappingFeature overlappingFeature(overlap[1].toULong(),
				overlap[0].toDouble());
			overlaps.push_back(overlappingFeature);
		}

		overlappingFeatures[featureNr] = overlaps;
	}

	return QSharedPointer<DataSet>(new DataSet(selectedFeatures, overlappingFeatures));
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

	DataSetPtr features(readSelectedFeatures(argv[2]));

	overlapviewer::OverlapMainWindow mainWindow(features);
	mainWindow.show();
	return app.exec();
}
