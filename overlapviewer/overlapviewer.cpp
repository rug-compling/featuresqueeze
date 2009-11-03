#include <stdexcept>
#include <string>

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

#include "DataSet.hh"
#include "OverlapMainWindow.hh"

using namespace std;

FeatureMappingPtr readFeatureMapping(char *filename)
{
	FeatureMappingPtr featureMapping(new FeatureMapping);

	QFile mappingFile(filename);
	if (!mappingFile.open(QFile::ReadOnly))
		throw runtime_error(string("Could not open ") + filename +
			"for reading!");

	QTextStream mappingStream(&mappingFile);

	while(!mappingStream.atEnd())
	{
		QString mappingLine(mappingStream.readLine());
		QRegExp mappingRe;
		mappingRe.setPattern("^([0-9]+)\\|(.*)$");
		mappingRe.indexIn(mappingLine);
		(*featureMapping)[mappingRe.cap(1).toUInt()] =
			mappingRe.cap(2);
	}

	return featureMapping;
}

DataSetPtr readSelectedFeatures(char *filename, FeatureMappingPtr featureMapping)
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
		QString featureName = (*featureMapping)[featureNr];

		SelectedFeature feature(featureName, lineParts[1].toDouble(),
			lineParts[2].toDouble());
		selectedFeatures.push_back(feature);

		QString overlapLine(featureStream.readLine());
		QStringList overlapParts = overlapLine.split("\t", QString::SkipEmptyParts);

		QVector<OverlappingFeature> overlaps;
		for (QStringList::const_iterator iter = overlapParts.begin();
			iter != overlapParts.end(); ++iter)
		{
			QStringList overlap = iter->split('#');
			OverlappingFeature overlappingFeature(
				(*featureMapping)[overlap[1].toUInt()],
				overlap[0].toDouble());
			overlaps.push_back(overlappingFeature);
		}

		overlappingFeatures[featureName] = overlaps;
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

	FeatureMappingPtr mapping = readFeatureMapping(argv[1]);
	DataSetPtr features(readSelectedFeatures(argv[2], mapping));
	mapping.clear();

	overlapviewer::OverlapMainWindow mainWindow(features);
	mainWindow.show();
	return app.exec();
}
