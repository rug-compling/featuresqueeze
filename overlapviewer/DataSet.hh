#ifndef DATASET_HH
#define DATASET_HH

#include <QHash>
#include <QVector>
#include <QSharedPointer>

struct SelectedFeature
{
	SelectedFeature(size_t newFeature, double newAlpha, double newGain) :
		feature(newFeature), alpha(newAlpha), gain(newGain) {}
	SelectedFeature() { SelectedFeature(0, 0.0, 0.0); }
	size_t feature;
	double alpha;
	double gain;
};

struct OverlappingFeature
{
	OverlappingFeature(size_t newFeature, double newDelta) :
			feature(newFeature), delta(newDelta) {}
	OverlappingFeature() : feature(0), delta(0.0) {}
	size_t feature;
	double delta;
};

typedef QVector<SelectedFeature> SelectedFeatures;
typedef QHash<size_t, QVector<OverlappingFeature> > OverlappingFeatures;

struct DataSet {
	DataSet(SelectedFeatures newSelected, OverlappingFeatures newOverlapping) :
		selectedFeatures(newSelected), overlappingFeatures(newOverlapping) {}

	SelectedFeatures selectedFeatures;
	OverlappingFeatures overlappingFeatures;
};

typedef QSharedPointer<DataSet> DataSetPtr;

#endif // DATASET_HH
