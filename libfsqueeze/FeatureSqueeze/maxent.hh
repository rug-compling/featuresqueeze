/*
 * Copyright (c) 2009 Daniël de Kok
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef FSQUEEZE_MAXENT_HH
#define FSQUEEZE_MAXENT_HH

#include <set>
#include <utility>
#include <vector>

#include <tr1/unordered_set>
#include <tr1/unordered_map>

#include <Eigen/Core>

#include "DataSet.hh"
#include "util.hh"

namespace fsqueeze
{

typedef Eigen::VectorXd FeatureWeights;
typedef Eigen::VectorXd ExpectedValues;
typedef Eigen::VectorXd Sum;
typedef std::vector<Sum> Sums;
typedef Eigen::VectorXd Zs;
typedef std::tr1::unordered_set<size_t> FeatureSet;

/*
 * Function object for gain-based orderering (highest gain first).
 */
typedef PairReverseLess<double> GainLess;

typedef std::set<std::pair<size_t, double>, GainLess> OrderedGains;
typedef std::tr1::unordered_map<size_t, double> GainMap;

struct makeSumVector
{
	Sum operator()(Context const &context) const;
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
	Sums *sums, Zs *zs);

/*
 * Calculate the gain of a model after changing a feature weight from zero
 * to non-zero.
 */
double calcGain(double gausianStdDev, DataSet const &dataSet,
	ExpectedValues const &expFeatureValues, Sums const &sums,
	Zs const &zs, size_t feature, double alpha);

/*
 * Calculate the model gains after changing for a set of features and their
 * weights.
 */
OrderedGains calcGains(double gausianStdDev, DataSet const &dataSet,
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
Zs initialZs(DataSet const &ds);

/**
 * Construct a vector of unnormalized event 'probabilities' representing a
 * uniform model.
 */
Sums initialSums(DataSet const &ds);

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
double zf(FeatureValues const &featureValues, Sum const &ctxSums, double z,
	size_t feature, double alpha);

inline Sum makeSumVector::operator()(Context const &context) const
{
	return Sum::Ones(context.eventProbs().size());
}

}

#endif // FSQUEEZE_MAXENT_HH