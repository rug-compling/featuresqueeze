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

double zf(vector<Event> const &events, vector<double> const &ctxSums, double z,
	size_t feature, double alpha);

vector<FeatureSet> contextActiveFeatures(DataSet const &dataSet,
	FeatureSet const &selectedFeatures, bool includeSelected,
	Sums const &sums, Zs const &zs)
{
	vector<FeatureSet> ctxActive;
	
	for (auto ctxIter = dataSet.contexts().begin(), ctxSumIter = sums.begin(), zIter = zs.begin();
	ctxIter != dataSet.contexts().end(); ++ctxIter, ++ctxSumIter, ++zIter)
	{
		FeatureSet active;

		// This context can not have active features if its probability is zero.
		if (ctxIter->prob() == 0.0)
		{
			ctxActive.push_back(active);
			continue;
		}

		for (auto evtIter = ctxIter->events().begin(), sumIter = ctxSumIter->begin();
			evtIter != ctxIter->events().end(); ++evtIter, ++sumIter)
		{
			// This event can not have active features if its probability is zero.
			if (p_yx(*sumIter, *zIter) == 0.0)
				continue;

			auto &features = evtIter->features();

			// Iterate over the provided feature set if it is small...
			if (includeSelected && selectedFeatures.size() < evtIter->features().size())
				for (auto fIter = selectedFeatures.begin(); fIter != selectedFeatures.end();
						++fIter)
				{
					auto f = features.find(*fIter);
					if (f != features.end() && f->second.value() != 0.0)
						active.insert(*fIter);
				}
			// ...otherwise iterate over all features of the current event.
			else
				for (auto fIter = evtIter->features().begin();
						fIter != evtIter->features().end();
						++fIter)
					if (includeSelected)
					{
						if (selectedFeatures.find(fIter->first) != selectedFeatures.end() &&
								fIter->second.value() != 0.0)
							active.insert(fIter->first);
					}	
					else
						if (selectedFeatures.find(fIter->first) == selectedFeatures.end() &&
								fIter->second.value() != 0.0)
							active.insert(fIter->first);
		}
		
		ctxActive.push_back(active);
	}
	
	return ctxActive;
}

// Active features in at least one context.
FeatureSet activeFeatures(vector<FeatureSet> const &contextActiveFeatures)
{
	FeatureSet active;
	
	for (auto ctxIter = contextActiveFeatures.begin(); ctxIter != contextActiveFeatures.end();
			++ctxIter)
		active.insert(ctxIter->begin(), ctxIter->end());
	
	return active;
}

// R(f)
unordered_map<size_t, double> r_f(FeatureSet const &features,
	unordered_map<size_t, double> const &expFeatureValues,
	unordered_map<size_t, double> const &expModelFeatureValues)
{
	unordered_map<size_t, double> r;
	
	for (auto iter = features.begin(); iter != features.end(); ++iter)
	{
		auto expIter = expFeatureValues.find(*iter);
		r[*iter] = expIter->second <=
			expModelFeatureValues.find(expIter->first)->second ? 1 : -1;
	}

	return r;
}

double r_f(size_t feature, unordered_map<size_t, double> const &expFeatureValues,
	unordered_map<size_t, double> const &expModelFeatureValues)
{
	auto expIter = expFeatureValues.find(feature);
	return expIter->second <= expModelFeatureValues.find(expIter->first)->second ?
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
	for (auto ctxIter = dataSet.contexts().begin(), ctxSumIter = sums.begin(), zIter = zs.begin();
	ctxIter != dataSet.contexts().end(); ++ctxIter, ++ctxSumIter, ++zIter)
	{
		auto newZ = zf(ctxIter->events(), *ctxSumIter, *zIter, feature, alpha);
		
		vector<double> newSums(ctxSumIter->size(), 0);
		auto p_fx = 0.0;
		for (auto evtIter = ctxIter->events().begin(), sumIter = ctxSumIter->begin(),
			newSumIter = newSums.begin(); evtIter != ctxIter->events().end();
			++evtIter, ++sumIter, ++newSumIter)
		{
			auto fVal = 0.0;
			auto iter = evtIter->features().find(feature);
			if (iter != evtIter->features().end())
				fVal = iter->second.value();
			
			auto newSum = *sumIter * exp(alpha * fVal);
			*newSumIter = newSum;
			
			p_fx += p_yx(newSum, newZ) * fVal;
		}
		
		auto gppSum = 0.0;
		for (auto evtIter = ctxIter->events().begin(), newSumIter = newSums.begin();
			evtIter != ctxIter->events().end();
			++evtIter, ++newSumIter)
		{
			auto fVal = 0.0;
			auto iter = evtIter->features().find(feature);
			if (iter != evtIter->features().end())
				fVal = iter->second.value();
			
			auto newSum = *newSumIter;
			
			gppSum += p_yx(newSum, newZ) * (pow(fVal, 2) - 2 * fVal * p_fx + pow(p_fx, 2));
		}
		
		*gp = *gp - ctxIter->prob() * p_fx;
		*gpp = *gpp - ctxIter->prob() * gppSum;
	}
}

void updateGradients(DataSet const &dataSet,
	FeatureSet const &unconvergedFeatures,
	vector<FeatureSet> const &activeFeatures,
	Sums const &sums,
	Zs const &zs,
	unordered_map<size_t, double> const &alphas,
	unordered_map<size_t, double> *gp,
	unordered_map<size_t, double> *gpp)
{
	for (auto ctxIter = dataSet.contexts().begin(), ctxSumIter = sums.begin(), zIter = zs.begin(),
	activeFsIter = activeFeatures.begin(); ctxIter != dataSet.contexts().end();
	++ctxIter, ++ctxSumIter, ++zIter, ++activeFsIter)
	{
		for (auto fsIter = activeFsIter->begin(); fsIter != activeFsIter->end();
			++fsIter)
		{
			if (unconvergedFeatures.find(*fsIter) == unconvergedFeatures.end())
				continue;

			auto newZ = zf(ctxIter->events(), *ctxSumIter, *zIter, *fsIter,
				alphas.find(*fsIter)->second);
			
			vector<double> newSums(ctxSumIter->size(), 0);
			auto p_fx = 0.0;
			for (auto evtIter = ctxIter->events().begin(), sumIter = ctxSumIter->begin(),
				newSumIter = newSums.begin(); evtIter != ctxIter->events().end();
				++evtIter, ++sumIter, ++newSumIter)
			{
				auto fVal = 0.0;
				auto iter = evtIter->features().find(*fsIter);
				if (iter != evtIter->features().end())
					fVal = iter->second.value();
				
				auto newSum = *sumIter * exp(alphas.find(*fsIter)->second * fVal);
				*newSumIter = newSum;
				
				p_fx += p_yx(newSum, newZ) * fVal;
			}
			
			auto gppSum = 0.0;
			for (auto evtIter = ctxIter->events().begin(), sumIter = ctxSumIter->begin(),
				newSumIter = newSums.begin(); evtIter != ctxIter->events().end();
				++evtIter, ++sumIter, ++newSumIter)
			{
				auto fVal = 0.0;
				auto iter = evtIter->features().find(*fsIter);
				if (iter != evtIter->features().end())
					fVal = iter->second.value();
				
				auto newSum = *newSumIter;
				
				gppSum += p_yx(newSum, newZ) * (pow(fVal, 2) - 2 * fVal * p_fx + pow(p_fx, 2));
			}
			
			(*gp)[*fsIter] = (*gp)[*fsIter] - ctxIter->prob() * p_fx;
			(*gpp)[*fsIter] = (*gpp)[*fsIter] - ctxIter->prob() * gppSum;
		}
	}
}

// Calculate weight of a single feature for the current model, given G', G'' and R(f).
// Returns true if the feature has converged.
bool updateAlpha(double rF, double gp, double gpp, double *alpha,
	double alphaThreshold)
{
	auto newAlpha = *alpha + rF * log(1 - rF * (gp / gpp));
	auto delta = fabs(*alpha - newAlpha);
	*alpha = newAlpha;
	
	if (delta < alphaThreshold || isnan(delta))
		return true;
	else
		return false;
}

// Calculate feature weights for the current model, given G', G'' and R(f).
FeatureSet updateAlphas(FeatureSet const &unconvergedFeatures,
	unordered_map<size_t, double> const &r,
	unordered_map<size_t, double> const &gp,
	unordered_map<size_t, double> const &gpp,
	unordered_map<size_t, double> *alphas,
	double alphaThreshold)
{
	FeatureSet newUnconvergedFs = unconvergedFeatures;
	for (auto fIter = unconvergedFeatures.begin(); fIter != unconvergedFeatures.end();
		++fIter)
	{
		size_t f = *fIter;
		double rF = r.find(f)->second;
		auto newAlpha = (*alphas)[f] + rF * log(1 - rF * (gp.find(f)->second / gpp.find(f)->second));
		auto delta = fabs((*alphas)[f] - newAlpha);
		(*alphas)[f] = newAlpha;
		if (delta < alphaThreshold || isnan(delta))
			newUnconvergedFs.erase(f);
	}
	
	return newUnconvergedFs;
}

// Initial feature weights (0.0).
unordered_map<size_t, double> a_f(FeatureSet const &features)
{
	unordered_map<size_t, double> a;
	
	for (auto iter = features.begin(); iter != features.end();
			++iter)
		a[*iter] = 0.0;
		
	return a;
}

// Calculate the gain of adding each feature.
OrderedGains calcGains(DataSet const &dataSet,
	vector<FeatureSet> const &contextActiveFeatures,
	unordered_map<size_t, double> const &expFeatureValues,
	unordered_map<size_t, double> const &alphas,
	vector<vector<double>> const &sums,
	vector<double> const &zs)
{
	unordered_map<size_t, double> gainSum;
	
	for (auto ctxIter = dataSet.contexts().begin(), fsIter = contextActiveFeatures.begin(),
		ctxSumIter = sums.begin(), zIter = zs.begin(); ctxIter != dataSet.contexts().end();
		++ctxIter, ++fsIter, ++ctxSumIter, ++zIter)
	{
		for (auto alphaIter = alphas.begin(); alphaIter != alphas.end();
			++alphaIter)
		{
			auto newZ = zf(ctxIter->events(), *ctxSumIter, *zIter, alphaIter->first,
				alphaIter->second);
			
			auto lg = ctxIter->prob() * log(newZ / *zIter);
			
			gainSum[alphaIter->first] -= lg;
		}
	}
	
	OrderedGains gains;
	for (auto alphaIter = alphas.begin(); alphaIter != alphas.end();
			++alphaIter)
		gains.insert(make_pair(alphaIter->first,
			gainSum[alphaIter->first] + alphaIter->second *
			expFeatureValues.find(alphaIter->first)->second
		));
	
	return gains;
}

// Calculate the gain of adding a feature to the model.
double calcGain(DataSet const &dataSet,
	size_t feature,
	unordered_map<size_t, double> const &expFeatureValues,
	double alpha,
	vector<vector<double>> const &sums,
	vector<double> const &zs)
{
	double gainSum = 0.0;
	
	for (auto ctxIter = dataSet.contexts().begin(), ctxSumIter = sums.begin(),
		zIter = zs.begin(); ctxIter != dataSet.contexts().end();
		++ctxIter, ++ctxSumIter, ++zIter)
	{
		auto newZ = zf(ctxIter->events(), *ctxSumIter, *zIter, feature, alpha);
		auto lg = ctxIter->prob() * log(newZ / *zIter);
		gainSum -= lg;
	}
	
	return gainSum + alpha * expFeatureValues.find(feature)->second;
}

// Hmpf...
unordered_map<int, double> orderedGainsToMap(OrderedGains const &gains)
{
	unordered_map<int, double> gainMap;

	for (auto iter = gains.begin(); iter != gains.end(); ++iter)
		gainMap[iter->first] = iter->second;
	
	return gainMap;
}

// Calculate (un)normalized deltas of gains.
unordered_map<int, double> gainDeltas(OrderedGains const &prevGains, OrderedGains const &gains,
	double gainThreshold, bool normalize)
{
	auto gainMap = orderedGainsToMap(gains);
	auto prevGainMap = orderedGainsToMap(prevGains);
	
	unordered_map<int, double> gainDeltas;
	
	for (auto iter = gainMap.begin(); iter != gainMap.end(); ++iter)
	{
		auto prevIter = prevGainMap.find(iter->first);
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

	auto deltas = gainDeltas(prevGains, gains, gainThreshold, normalizeDeltas);
	
	// Average
	double sum = 0.0;
	for (auto iter = deltas.begin(); iter != deltas.end(); ++iter)
		sum += iter->second;
	double avg = sum / deltas.size();
	
	// Standard deviation
	double sqDiffSum = 0.0;
	for (auto iter = deltas.begin(); iter != deltas.end(); ++iter)
		sqDiffSum += pow(iter->second - avg, 2);
	double sd = sqrt(sqDiffSum / deltas.size());
	
	double const SE99 = 2.68;
	for (auto iter = deltas.begin(); iter != deltas.end(); ++iter)
	{
		double zIndex = (iter->second - avg) / sd;
		if (fabs(zIndex) > SE99)
			overlappingFs.insert(make_pair(iter->first, iter->second));
	}
	
	return overlappingFs;
}

OrderedGains fullSelectionStage(DataSet const &dataSet,
	double alphaThreshold,
	unordered_map<size_t, double> const &expVals,
	vector<double> *zs,
	vector<vector<double>> *sums,
	FeatureSet *selectedFeatures,
	SelectedFeatureAlphas *selectedFeatureAlphas)
{
	auto expModelVals = expModelFeatureValues(dataSet, *sums, *zs);

	auto ctxActiveFs = contextActiveFeatures(dataSet, *selectedFeatures, false, *sums, *zs);
	auto unconvergedFs = activeFeatures(ctxActiveFs);

	auto r = r_f(unconvergedFs, expVals, expModelVals);
	
	auto a = a_f(unconvergedFs);

	while (unconvergedFs.size() != 0)
	{
		auto gp = expVals;
		auto gpp = a_f(unconvergedFs);
	
		updateGradients(dataSet, unconvergedFs, ctxActiveFs, *sums, *zs, a, &gp, &gpp);
		unconvergedFs = updateAlphas(unconvergedFs, r, gp, gpp, &a, alphaThreshold);
	}

	auto gains = calcGains(dataSet, ctxActiveFs, expVals, a, *sums, *zs);

	size_t maxF = gains.begin()->first;
	auto maxGain = gains.begin()->second;
	auto maxAlpha = a[maxF];

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
	
	auto zs = initialZs(dataSet);
	auto sums = initialSums(dataSet);
	
	auto expVals = expFeatureValues(dataSet);
	
	OrderedGains prevGains;
	while(selectedFeatures.size() < nFeatures)	
	{
		OrderedGains gains;
		if (detectOverlap)
			gains = fullSelectionStage(dataSet, alphaThreshold, expVals, &zs, &sums,
				&selectedFeatures, &selectedFeatureAlphas);
		else
			fullSelectionStage(dataSet, alphaThreshold, expVals, &zs, &sums,
				&selectedFeatures, &selectedFeatureAlphas);
		
		if (selectedFeatureAlphas.size() == 0)
			break;
			
		if (detectOverlap)
		{
			if (prevGains.size() != 0)
			{
				auto overlappingFs = findOverlappingFeatures(prevGains, gains,
					gainThreshold, true);
				copy(overlappingFs.begin(), overlappingFs.end(),
					ostream_iterator<pair<size_t, double>>(logger.message(), "\t"));
				logger.message() << "\n";
			}

			prevGains = gains;
		}

		auto selected = selectedFeatureAlphas.back();
		
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
	unordered_map<size_t, double> const &expVals,
	vector<double> *zs,
	vector<vector<double>> *sums,
	FeatureSet *selectedFeatures,
	SelectedFeatureAlphas *selectedFeatureAlphas,
	OrderedGains *gains)
{
	auto expModelVals = expModelFeatureValues(dataSet, *sums, *zs);

	while (true)
	{
		auto feature = gains->begin()->first;
		double a = 0.0;
		double r = r_f(feature, expVals, expModelVals);

		bool converged = false;
		while (!converged)
		{
			auto gp = expVals.find(feature)->second;
			double gpp = 0.0;
			
			updateGradient(dataSet, feature, *sums, *zs, a, &gp, &gpp);
			converged = updateAlpha(r, gp, gpp, &a, alphaThreshold);
		}	

		auto gain = calcGain(dataSet, feature, expVals, a, *sums, *zs);	
		
		auto gainIter = gains->begin();		
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
	
	auto zs = initialZs(dataSet);
	auto sums = initialSums(dataSet);
	
	auto expVals = expFeatureValues(dataSet);

	// Start with a full selection stage to calculate the stage 2 model and gains.
	auto gains = fullSelectionStage(dataSet, alphaThreshold, expVals, &zs, &sums,
		&selectedFeatures, &selectedFeatureAlphas);
	auto gainIter = gains.begin();
	++gainIter;
	gains.erase(gains.begin(), gainIter);
	
	auto selected = selectedFeatureAlphas.back();
	logger.message() << selected.first << "\t" << selected.second <<
		"\t" << selected.third << "\n";
	
	while(selectedFeatures.size() < nFeatures)	
	{
		fastSelectionStage(dataSet, alphaThreshold, expVals, &zs, &sums,
			&selectedFeatures, &selectedFeatureAlphas, &gains);

		if (selectedFeatureAlphas.back().third < gainThreshold)
		{
			selectedFeatureAlphas.pop_back();
			break;
		}
		
		auto selected = selectedFeatureAlphas.back();
		logger.message() << selected.first << "\t" << selected.second <<
			"\t" << selected.third << "\n";
	}
	
	return selectedFeatureAlphas;
}


double zf(vector<Event> const &events, vector<double> const &ctxSums, double z,
	size_t feature, double alpha)
{
	auto newZ = z;

	for (auto evtIter = events.begin(), sumIter = ctxSums.begin();
		evtIter != events.end(); ++evtIter, ++sumIter)
	{
		auto iter = evtIter->features().find(feature);
		if (iter != evtIter->features().end())
			newZ = newZ - *sumIter + *sumIter * exp(alpha * iter->second.value());
	}
	
	return newZ;
}
