#ifndef FSQUEEZE_MAXENT_HH
#define FSQUEEZE_MAXENT_HH

#include <vector>

#include <tr1/unordered_map>

#include "DataSet.hh"

namespace fsqueeze
{

typedef std::vector<std::vector<double> > Sums;
typedef std::vector<double> Sum;
typedef std::vector<double> Zs;

struct makeSumVector
{
	std::vector<double> operator()(Context const &context) const;
};

/**
 * Adjust a model's sums and zs by assigning the weight alpha to feature. Here
 * we assume that the previous value of alpha was zero.
 */
void adjustModel(DataSet const &dataSet, size_t feature, double alpha,
	std::vector<std::vector<double> > *sums, std::vector<double> *zs);

/**
 * Calculate the expected value of each feature in a data set.
 */
std::tr1::unordered_map<size_t, double> expFeatureValues(DataSet const &dataSet);

/**
 * Calculate the expected value of each feature according to the model represented
 * by zs and sums.
 */
std::tr1::unordered_map<size_t, double> expModelFeatureValues(DataSet const &dataSet,
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

inline std::vector<double> makeSumVector::operator()(Context const &context) const
{
	return std::vector<double>(context.events().size(), 1.0);
}	


}

#endif // FSQUEEZE_MAXENT_HH