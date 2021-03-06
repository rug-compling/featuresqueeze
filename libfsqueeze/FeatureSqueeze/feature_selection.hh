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

#ifndef FEATURE_SELECTION_HH
#define FEATURE_SELECTION_HH

#include <cmath>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

#include <tr1/unordered_map>

#include <Eigen/Core>

#include "DataSet.hh"
#include "Logger.hh"
#include "selection.hh"

namespace std {

inline std::ostream &operator<<(std::ostream &out, std::pair<size_t, double> const &p)
{
  return out << p.second << "#" << p.first;
}

}

namespace fsqueeze {

typedef Eigen::VectorXd R_f;
typedef Eigen::VectorXd Gp;
typedef Eigen::VectorXd Gpp;
typedef std::tr1::unordered_map<size_t, double> GainDeltas;

struct SelectionParameters {
  SelectionParameters() : alphaThreshold(1e-10), gainThreshold(1e-20),
    nFeatures(std::numeric_limits<size_t>::max()),
    detectOverlap(false), fullOptimizationCycles(0),
    fullOptimizationExpBase(0.0) {}
  double alphaThreshold;
  double gainThreshold;
  size_t nFeatures;
  bool detectOverlap;
  size_t fullOptimizationCycles;
  double fullOptimizationExpBase;
};

/**
 * Select features based on a dataset using the fast selection algorithm.
 * This algorithm assumes that gains of candidate features rarely increase
 * as a result of adding a feature.
 *
 * @ds The dataset to mine for features
 * @logger Logger for printing output
 * @alphaThreshold Threshold to determine convergence of alpha values
 * @gainThreshold Threshold to determine the stopping point of the feature 
 *  selection cycle.
 * @nFeatures Maximum number of features.
 */
SelectedFeatureAlphas fastFeatureSelection(DataSet const &ds, Logger logger, 
    SelectionParameters const &param);

/**
 * Select features based on a dataset.
 *
 * @ds The dataset to mine for features
 * @logger Logger for printing output
 * @alphaThreshold Threshold to determine convergence of alpha values
 * @gainThreshold Threshold to determine the stopping point of the feature 
 *  selection cycle.
 * @nFeatures Maximum number of features.
 */
SelectedFeatureAlphas featureSelection(DataSet const &ds, Logger logger,
    SelectionParameters const &param);

}

#endif // FEATURE_SELECTION_HH
