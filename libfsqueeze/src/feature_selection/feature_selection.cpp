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

#include "feature_selection.ih"

// R(f)
R_f r_f(FeatureSet const &features,
	ExpectedValues const &expFeatureValues,
	ExpectedValues const &expModelFeatureValues)
{
	R_f r;
	
	for (FeatureSet::const_iterator iter = features.begin(); iter != features.end(); ++iter)
		r[*iter] = expFeatureValues[*iter] <=
			expModelFeatureValues[*iter] ? 1 : -1;

	return r;
}

double r_f(size_t feature, ExpectedValues const &expFeatureValues,
	ExpectedValues const &expModelFeatureValues)
{
	return expFeatureValues[feature] <= expModelFeatureValues[feature] ?
		1 : -1;
}

void updateGradient(DataSet const &dataSet,
	size_t feature,
	Sums const &sums,
	Zs const &zs,
	double alpha,
	double *gp,
	double *gpp)
{
	ContextVector::const_iterator ctxIter = dataSet.contexts().begin();
	Sums::const_iterator ctxSumIter = sums.begin();
	Zs::const_iterator zIter = zs.begin();
	while(ctxIter != dataSet.contexts().end())
	{
		double newZ = zf(ctxIter->events(), *ctxSumIter, *zIter, feature, alpha);
		
		Sum newSums(ctxSumIter->size(), 0);
		double p_fx = 0.0;
		EventVector::const_iterator evtIter = ctxIter->events().begin();
		Sum::const_iterator sumIter = ctxSumIter->begin();
		Sum::iterator newSumIter = newSums.begin();
		while(evtIter != ctxIter->events().end())
		{
			double fVal = evtIter->features().coeff(feature);
			
			double newSum = *sumIter * exp(alpha * fVal);
			*newSumIter = newSum;
			
			p_fx += p_yx(newSum, newZ) * fVal;
			
			++evtIter; ++sumIter; ++newSumIter;
		}
		
		double gppSum = 0.0;
		evtIter = ctxIter->events().begin();
		newSumIter = newSums.begin();
		while (evtIter != ctxIter->events().end())
		{
			double fVal = evtIter->features().coeff(feature);
			
			double newSum = *newSumIter;
			
			gppSum += p_yx(newSum, newZ) * (pow(fVal, 2) - 2 * fVal * p_fx + pow(p_fx, 2));
			
			++evtIter; ++newSumIter;
		}
		
		*gp = *gp - ctxIter->prob() * p_fx;
		*gpp = *gpp - ctxIter->prob() * gppSum;
		
		++ctxIter; ++ctxSumIter; ++zIter;
	}
}

void updateGradients(DataSet const &dataSet,
	FeatureSet const &unconvergedFeatures,
	vector<FeatureSet> const &activeFeatures,
	Sums const &sums,
	Zs const &zs,
	FeatureWeights const &alphas,
	Gp *gp,
	Gpp *gpp)
{
	ContextVector::const_iterator ctxIter = dataSet.contexts().begin();
	Sums::const_iterator ctxSumIter = sums.begin();
	Zs::const_iterator zIter = zs.begin();
	vector<FeatureSet>::const_iterator activeFsIter = activeFeatures.begin();
	while (ctxIter != dataSet.contexts().end())
	{
		for (FeatureSet::const_iterator fsIter = activeFsIter->begin(); fsIter != activeFsIter->end();
			++fsIter)
		{
			if (unconvergedFeatures.find(*fsIter) == unconvergedFeatures.end())
				continue;

			double newZ = zf(ctxIter->events(), *ctxSumIter, *zIter, *fsIter,
				alphas[*fsIter]);
			
			Sum newSums(ctxSumIter->size(), 0);
			double p_fx = 0.0;
			EventVector::const_iterator evtIter = ctxIter->events().begin();
			Sum::const_iterator sumIter = ctxSumIter->begin();
			Sum::iterator newSumIter = newSums.begin();
			while (evtIter != ctxIter->events().end())
			{
				double fVal = evtIter->features().coeff(*fsIter);
				
				double newSum = *sumIter * exp(alphas[*fsIter] * fVal);
				*newSumIter = newSum;
				
				p_fx += p_yx(newSum, newZ) * fVal;
				
				++evtIter; ++sumIter; ++newSumIter;
			}
			
			double gppSum = 0.0;
			evtIter = ctxIter->events().begin();
			sumIter = ctxSumIter->begin();
			newSumIter = newSums.begin();
			while (evtIter != ctxIter->events().end())
			{
				double fVal = evtIter->features().coeff(*fsIter);
				
				double newSum = *newSumIter;
				
				gppSum += p_yx(newSum, newZ) * (pow(fVal, 2) - 2 * fVal * p_fx + pow(p_fx, 2));
				
				++evtIter; ++sumIter; ++newSumIter;
			}
			
			(*gp)[*fsIter] = (*gp)[*fsIter] - ctxIter->prob() * p_fx;
			(*gpp)[*fsIter] = (*gpp)[*fsIter] - ctxIter->prob() * gppSum;
		}
		
		++ctxIter; ++ctxSumIter; ++zIter; ++activeFsIter;
	}
}

// Calculate weight of a single feature for the current model, given G', G'' and R(f).
// Returns true if the feature has converged.
bool updateAlpha(double rF, double gp, double gpp, double *alpha,
	double alphaThreshold)
{
	double newAlpha = *alpha + rF * log(1 - rF * (gp / gpp));
	double delta = fabs(*alpha - newAlpha);
	*alpha = newAlpha;
	
	if (delta < alphaThreshold || isnan(delta))
		return true;
	else
		return false;
}

// Calculate feature weights for the current model, given G', G'' and R(f).
FeatureSet updateAlphas(FeatureSet const &unconvergedFeatures,
	R_f const &r,
	Gp const &gp,
	Gpp const &gpp,
	FeatureWeights *alphas,
	double alphaThreshold)
{
	FeatureSet newUnconvergedFs = unconvergedFeatures;
	for (FeatureSet::const_iterator fIter = unconvergedFeatures.begin();
		fIter != unconvergedFeatures.end(); ++fIter)
	{
		int f = *fIter;
		double rF = r.find(f)->second;
		double oldAlpha = (*alphas)[f];
		double newAlpha = oldAlpha + rF * log(1 - rF * (gp[f] / gpp[f]));
		double delta = fabs(oldAlpha - newAlpha);
		(*alphas)[f] = newAlpha;
		if (delta < alphaThreshold || isnan(delta))
			newUnconvergedFs.erase(f);
	}
	
	return newUnconvergedFs;
}

// Initial feature weights (0.0).
FeatureWeights a_f(size_t nFeatures)
{
	return FeatureWeights::Zero(nFeatures);		
}

// Hmpf...
GainMap orderedGainsToMap(OrderedGains const &gains)
{
	GainMap gainMap;

	for (OrderedGains::const_iterator iter = gains.begin(); iter != gains.end(); ++iter)
		gainMap[iter->first] = iter->second;
	
	return gainMap;
}

// Calculate (un)normalized deltas of gains.
GainDeltas gainDeltas(OrderedGains const &prevGains, OrderedGains const &gains,
	double gainThreshold, bool normalize)
{
	GainMap gainMap = orderedGainsToMap(gains);
	GainMap prevGainMap = orderedGainsToMap(prevGains);
	
	GainDeltas gainDeltas;
	
	for (GainMap::const_iterator iter = gainMap.begin(); iter != gainMap.end(); ++iter)
	{
		GainMap::const_iterator prevIter = prevGainMap.find(iter->first);
		if (prevIter != prevGainMap.end() && !isnan(iter->second) &&
			!isnan(prevIter->second) && prevIter->second > gainThreshold)
		{
			double gainDelta = prevIter->second - iter->second;

			if (normalize)
			{
				if (gainDelta > 0.0)
					gainDelta /= prevIter->second;
				else
					gainDelta /= iter->second;
			}

			gainDeltas[iter->first] = gainDelta;
		}
	}
	
	return gainDeltas;
}

OrderedGains findOverlappingFeatures(OrderedGains const &prevGains,
	OrderedGains const &gains, double gainThreshold, bool normalizeDeltas)
{
	OrderedGains overlappingFs;

	GainDeltas deltas = gainDeltas(prevGains, gains, gainThreshold, normalizeDeltas);
	
	// Average
	double sum = 0.0;
	for (GainDeltas::const_iterator iter = deltas.begin();
			iter != deltas.end(); ++iter)
		sum += iter->second;
	double avg = sum / deltas.size();
	
	// Standard deviation
	double sqDiffSum = 0.0;
	for (GainDeltas::const_iterator iter = deltas.begin();
			iter != deltas.end(); ++iter)
		sqDiffSum += pow(iter->second - avg, 2);
	double sd = sqrt(sqDiffSum / deltas.size());
	
	double const SE99 = 2.68;
	for (GainDeltas::const_iterator iter = deltas.begin();
		iter != deltas.end(); ++iter)
	{
		double zIndex = (iter->second - avg) / sd;
		if (fabs(zIndex) > SE99)
			overlappingFs.insert(make_pair(iter->first, iter->second));
	}
	
	return overlappingFs;
}

OrderedGains fullSelectionStage(DataSet const &dataSet,
	double alphaThreshold,
	ExpectedValues const &expVals,
	Sums *sums,
	Zs *zs,
	FeatureSet *selectedFeatures,
	SelectedFeatureAlphas *selectedFeatureAlphas)
{
	ExpectedValues expModelVals = expModelFeatureValues(dataSet, *sums, *zs);

	vector<FeatureSet> ctxActiveFs = contextActiveFeatures(dataSet, *selectedFeatures, *sums, *zs);
	FeatureSet unconvergedFs = activeFeatures(ctxActiveFs);

	R_f r = r_f(unconvergedFs, expVals, expModelVals);
	
	FeatureWeights a = a_f(dataSet.nFeatures());

	while (unconvergedFs.size() != 0)
	{
		Gp gp = expVals;
		Gpp gpp = a_f(dataSet.nFeatures());
	
		updateGradients(dataSet, unconvergedFs, ctxActiveFs, *sums, *zs, a, &gp, &gpp);
		unconvergedFs = updateAlphas(unconvergedFs, r, gp, gpp, &a, alphaThreshold);
	}

	OrderedGains gains = calcGains(dataSet, ctxActiveFs, expVals, *sums, *zs, a);

	size_t maxF = gains.begin()->first;
	double maxGain = gains.begin()->second;
	double maxAlpha = a.coeff(maxF);

	adjustModel(dataSet, maxF, maxAlpha, sums, zs);
		
	selectedFeatures->insert(maxF);
	selectedFeatureAlphas->push_back(makeTriple(maxF, maxAlpha, maxGain));
	
	return gains;
}

SelectedFeatureAlphas fsqueeze::featureSelection(DataSet const &dataSet,
	Logger logger, double alphaThreshold, double gainThreshold,
	size_t nFeatures, bool detectOverlap)
{
	FeatureSet selectedFeatures;
	SelectedFeatureAlphas selectedFeatureAlphas;
	
	Zs zs = initialZs(dataSet);
	Sums sums = initialSums(dataSet);
	
	ExpectedValues expVals = expFeatureValues(dataSet);
	
	OrderedGains prevGains;
	while(selectedFeatures.size() < nFeatures)	
	{
		OrderedGains gains;
		if (detectOverlap)
			gains = fullSelectionStage(dataSet, alphaThreshold, expVals, &sums, &zs,
				&selectedFeatures, &selectedFeatureAlphas);
		else
			fullSelectionStage(dataSet, alphaThreshold, expVals, &sums, &zs,
				&selectedFeatures, &selectedFeatureAlphas);
		
		if (selectedFeatureAlphas.size() == 0)
			break;
			
		if (detectOverlap)
		{
			if (prevGains.size() != 0)
			{
				OrderedGains overlappingFs = findOverlappingFeatures(prevGains, gains,
					gainThreshold, true);
				copy(overlappingFs.begin(), overlappingFs.end(),
					ostream_iterator<pair<size_t, double> >(logger.message(), "\t"));
				logger.message() << "\n";
			}

			prevGains = gains;
		}

		Triple<size_t, double, double> selected = selectedFeatureAlphas.back();
		
		if (selected.third < gainThreshold)
		{
			selectedFeatureAlphas.pop_back();
			break;
		}
		
		logger.message() << selected.first << "\t" << selected.second <<
			"\t" << selected.third;
		
		logger.message() << "\n";
	}
	
	return selectedFeatureAlphas;
}

void fastSelectionStage(DataSet const &dataSet,
	double alphaThreshold,
	ExpectedValues const &expVals,
	Sums *sums,
	Zs *zs,
	FeatureSet *selectedFeatures,
	SelectedFeatureAlphas *selectedFeatureAlphas,
	OrderedGains *gains)
{
	ExpectedValues expModelVals = expModelFeatureValues(dataSet, *sums, *zs);

	while (true)
	{
		size_t feature = gains->begin()->first;
		double a = 0.0;
		double r = r_f(feature, expVals, expModelVals);

		bool converged = false;
		while (!converged)
		{
			double gp = expVals[feature];
			double gpp = 0.0;
			
			updateGradient(dataSet, feature, *sums, *zs, a, &gp, &gpp);
			converged = updateAlpha(r, gp, gpp, &a, alphaThreshold);
		}	

		double gain = calcGain(dataSet, expVals, *sums, *zs, feature, a);	
		
		OrderedGains::const_iterator gainIter = gains->begin();		
		++gainIter;
		
		if (isnan(gain) && isnan(gainIter->second))
			throw runtime_error("Refusing to select NaNs");

		if (gains->size() == 1 || gainIter->second <= gain || isnan(gainIter->second))
		{
			// The current feature has a higher recalculated gain than the
			// second-highest feature. Select the current feature, and remove
			// it for further analyses.
			adjustModel(dataSet, feature, a, sums, zs);
			selectedFeatures->insert(feature);
			selectedFeatureAlphas->push_back(makeTriple(feature, a, gain));
			gains->erase(gains->begin(), gainIter);
			
			break; // Done for this stage.
		}
		else
		{
			gains->erase(gains->begin(), gainIter);
			gains->insert(make_pair(feature, gain));
		}
 	}
}

SelectedFeatureAlphas fsqueeze::fastFeatureSelection(DataSet const &dataSet,
	Logger logger, double alphaThreshold, double gainThreshold, size_t nFeatures)
{
	FeatureSet selectedFeatures;
	SelectedFeatureAlphas selectedFeatureAlphas;
	
	Zs zs = initialZs(dataSet);
	Sums sums = initialSums(dataSet);
	
	ExpectedValues expVals = expFeatureValues(dataSet);

	// Start with a full selection stage to calculate the stage 2 model and gains.
	OrderedGains gains = fullSelectionStage(dataSet, alphaThreshold, expVals, &sums, &zs,
		&selectedFeatures, &selectedFeatureAlphas);
	OrderedGains::const_iterator gainIter = gains.begin();
	++gainIter;
	gains.erase(gains.begin(), gainIter);
	
	Triple<size_t, double, double> selected = selectedFeatureAlphas.back();
	logger.message() << selected.first << "\t" << selected.second <<
		"\t" << selected.third << "\n";
	
	while(selectedFeatures.size() < nFeatures)	
	{
		fastSelectionStage(dataSet, alphaThreshold, expVals, &sums, &zs,
			&selectedFeatures, &selectedFeatureAlphas, &gains);

		if (selectedFeatureAlphas.back().third < gainThreshold)
		{
			selectedFeatureAlphas.pop_back();
			break;
		}
		
		Triple<size_t, double, double> selected = selectedFeatureAlphas.back();
		logger.message() << selected.first << "\t" << selected.second <<
			"\t" << selected.third << "\n";
	}
	
	return selectedFeatureAlphas;
}
