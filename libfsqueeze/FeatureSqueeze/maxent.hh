#ifndef FSQUEEZE_MAXENT_HH
#define FSQUEEZE_MAXENT_HH

#include <set>
#include <utility>
#include <vector>

#include <tr1/unordered_set>
#include <tr1/unordered_map>

#include "DataSet.hh"

namespace fsqueeze
{

typedef std::tr1::unordered_map<size_t, double> FeatureWeights;
typedef std::tr1::unordered_map<size_t, double> ExpectedValues;
typedef std::vector<std::vector<double> > Sums;
typedef std::vector<double> Sum;
typedef std::vector<double> Zs;
typedef std::tr1::unordered_set<size_t> FeatureSet;

/*
 * Function object for gain-based orderering (highest gain first).
 */
struct GainLess
{
	bool operator()(std::pair<size_t, double> const &f1, std::pair<size_t, double> const &f2)
	{
		// Treat NaN as no gain.
		double g1 = std::isnan(f1.second) ? 0.0 : f1.second;
		double g2 = std::isnan(f2.second) ? 0.0 : f2.second;
		
		if (g1 == g2)
			return f1.first < f2.first;
		
		return g1 > g2;
	}
};

typedef std::set<std::pair<size_t, double>, GainLess> OrderedGains;
typedef std::tr1::unordered_map<size_t, double> GainMap;

struct makeSumVector
{
	std::vector<double> operator()(Context const &context) const;
};

/*
 * Find features that are active in at least one context.
 */
FeatureSet activeFeatures(std::vector<FeatureSet> const &contextActiveFeatures);

/**
 * Adjust a model's sums and zs by assigning the weight alpha to feature. Here
 * we assume that the previous value of alpha was zero.
 */
void adjustModel(DataSet const &dataSet, size_t feature, double alpha,
	std::vector<std::vector<double> > *sums, std::vector<double> *zs);

/*
 * Calculate the gain of a model after changing a feature weight from zero
 * to non-zero.
 */
double calcGain(DataSet const &dataSet, ExpectedValues const &expFeatureValues,
	Sums const &sums, Zs const &zs, size_t feature, double alpha);

/*
 * Calculate the model gains after changing for a set of features and their
 * weights.
 */
OrderedGains calcGains(DataSet const &dataSet,
	std::vector<FeatureSet> const &contextActiveFeatures,
	ExpectedValues const &expFeatureValues, Sums const &sums, Zs const &zs,
	FeatureWeights const &alphas);

/*
 * Determine active features per context.
 */
std::vector<FeatureSet> contextActiveFeatures(DataSet const &dataSet,
	FeatureSet const &excludedFeatures, Sums const &sums, Zs const &zs);

/**
 * Calculate the expected value of each feature in a data set.
 */
ExpectedValues expFeatureValues(DataSet const &dataSet);

/**
 * Calculate the expected value of each feature according to the model represented
 * by zs and sums.
 */
ExpectedValues expModelFeatureValues(DataSet const &dataSet,
	Sums const &sums, Zs const &zs);

/**
 * Construct a vector of Z(x) normalization values representing a uniform model.
 */
std::vector<double> initialZs(DataSet const &ds);

/**
 * Construct a vector of unnormalized event 'probabilities' representing a
 * uniform model.
 */
std::vector<std::vector<double> > initialSums(DataSet const &ds);

/**
 * Calculate the probability p(y|x) based on sum and normalization z.
 */
inline double p_yx(double sum, double z)
{
	return sum / z;
}

/*
 * Calculate an updated Z(x) value as the result of changing the weight
 * of a feature form a zero to a non-zero value.
 */
double zf(EventVector const &events, Sum const &ctxSums, double z,
	size_t feature, double alpha);

inline std::vector<double> makeSumVector::operator()(Context const &context) const
{
	return std::vector<double>(context.events().size(), 1.0);
}


}

#endif // FSQUEEZE_MAXENT_HH