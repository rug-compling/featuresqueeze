#ifndef CORR_SELECTION_HH
#define CORR_SELECTION_HH

#include <limits>

#include "DataSet.hh"
#include "Logger.hh"
#include "selection.hh"

namespace fsqueeze {

SelectedFeatureAlphas corrFeatureSelection(DataSet const &ds, Logger logger,
	double minCorrelation = 0.9, size_t nFeatures = std::numeric_limits<size_t>::max());

}

#endif // CORR_SELECTION_HH