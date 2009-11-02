#ifndef DATASET_HH
#define DATASET_HH

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

typedef QVector<SelectedFeature> SelectedFeatures;
typedef QSharedPointer<SelectedFeatures> SelectedFeaturesPtr;

#endif // DATASET_HH
