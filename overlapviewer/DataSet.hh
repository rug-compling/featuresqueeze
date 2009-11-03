#ifndef DATASET_HH
#define DATASET_HH

#include <QHash>
#include <QVector>
#include <QSharedPointer>

struct SelectedFeature
{
	SelectedFeature(QString const &newFeature, double newAlpha, double newGain) :
		feature(newFeature), alpha(newAlpha), gain(newGain) {}
	SelectedFeature() { SelectedFeature(0, 0.0, 0.0); }
	QString feature;
	double alpha;
	double gain;
};

struct OverlappingFeature
{
	OverlappingFeature(QString const &newFeature, double newDelta) :
			feature(newFeature), delta(newDelta) {}
	OverlappingFeature() : feature(0), delta(0.0) {}
	QString feature;
	double delta;
};

typedef QVector<SelectedFeature> SelectedFeatures;
typedef QHash<QString, QVector<OverlappingFeature> > OverlappingFeatures;

struct DataSet {
	DataSet(SelectedFeatures newSelected, OverlappingFeatures newOverlapping) :
		selectedFeatures(newSelected), overlappingFeatures(newOverlapping) {}

	SelectedFeatures selectedFeatures;
	OverlappingFeatures overlappingFeatures;
};

typedef QSharedPointer<DataSet> DataSetPtr;

typedef QHash<size_t, QString> FeatureMapping;
typedef QSharedPointer<FeatureMapping> FeatureMappingPtr;

#endif // DATASET_HH
