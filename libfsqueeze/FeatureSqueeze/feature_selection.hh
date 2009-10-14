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

#include <limits>
#include <utility>
#include <vector>

#include "DataSet.hh"
#include "Logger.hh"

#include "util.hh"

namespace fsqueeze {

typedef std::vector<Triple<size_t, double, double>> SelectedFeatureAlphas;

SelectedFeatureAlphas fastFeatureSelection(DataSet const &ds, Logger logger, 
	double alphaThreshold = 1e-10, double gainThreshold = 1e-10,
	size_t nFeatures = std::numeric_limits<size_t>::max());

/**
 * Select features based on a dataset.
 *
 * @ds The dataset to mine for features
 * @alphaThreshold Threshold to determine convergence of alpha values
 * @gainThreshold Threshold to determine the stopping point of the feature 
 *	selection cycle.
 */
SelectedFeatureAlphas featureSelection(DataSet const &ds, Logger logger,
	double alphaThreshold = 1e-10, double gainThreshold = 1e-10,
	size_t nFeatures = std::numeric_limits<size_t>::max());

}

#endif // FEATURE_SELECTION_HH