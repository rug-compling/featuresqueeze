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

#include "FeatureSelection.ih"

struct GainLess
{
	bool operator()(pair<size_t, double> const &f1, pair<size_t, double> const &f2)
	{
		// Treat NaN as no gain.
		double g1 = isnan(f1.second) ? 0.0 : f1.second;
		double g2 = isnan(f2.second) ? 0.0 : f2.second;
		
		if (g1 == g2)
			return f1.first < f2.first;
		
		return g1 > g2;
	}
};

double zf(vector<Event> const &events, vector<double> const &ctxSums, double z,
	size_t feature, double alpha);

unordered_map<size_t, double> expFeatureValues(DataSet const &dataSet)
{
	unordered_map<size_t, double> expVals;
	
	for (auto fIter = dataSet.features().begin(); fIter != dataSet.features().end();
		++fIter)
	{
		auto expVal = 0.0;
		for (auto occIter = fIter->second.begin(); occIter != fIter->second.end();
				++occIter)
			expVal += occIter->first->prob() * occIter->second->value();
		expVals[fIter->first] = expVal;
	}
	
	return expVals;
}

unordered_map<size_t, double> expModelFeatureValues(DataSet const &dataSet,
	Sums const &sums, Zs const &zs)
{
	unordered_map<size_t, double> expVals;

	for (auto ctxIter = dataSet.contexts().begin(), ctxSumIter = sums.begin(), zIter = zs.begin();
	ctxIter != dataSet.contexts().end(); ++ctxIter, ++ctxSumIter, ++zIter)
	{
		for (auto evtIter = ctxIter->events().begin(), sumIter = ctxSumIter->begin();
			evtIter != ctxIter->events().end(); ++evtIter, ++sumIter)
		{
			auto pyx = p_yx(*sumIter, *zIter);
			
			for (auto fIter = evtIter->features().begin(); fIter != evtIter->features().end();
					++fIter)
				expVals[fIter->first] += ctxIter->prob() * pyx * fIter->second.value();
		}
	}
	
	return expVals;
}

vector<unordered_set<size_t>> contextActiveFeatures(DataSet const &dataSet,
	unordered_set<size_t> const &selectedFeatures, bool includeSelected,
	Sums const &sums, Zs const &zs)
{
	vector<unordered_set<size_t>> ctxActive;
	
	for (auto ctxIter = dataSet.contexts().begin(), ctxSumIter = sums.begin(), zIter = zs.begin();
	ctxIter != dataSet.contexts().end(); ++ctxIter, ++ctxSumIter, ++zIter)
	{
		unordered_set<size_t> active;

		for (auto evtIter = ctxIter->events().begin(), sumIter = ctxSumIter->begin();
			evtIter != ctxIter->events().end(); ++evtIter, ++sumIter)
		{
			auto pyx = p_yx(*sumIter, *zIter);
			
			for (auto fIter = evtIter->features().begin();
					fIter != evtIter->features().end();
					++fIter)
				if (includeSelected)
				{
					if (selectedFeatures.find(fIter->first) != selectedFeatures.end() &&
							active.find(fIter->first) == active.end() &&
							fIter->second.value() * pyx * ctxIter->prob() != 0.0)
						active.insert(fIter->first);
				}	
				else
					if (selectedFeatures.find(fIter->first) == selectedFeatures.end() &&
							active.find(fIter->first) == active.end() &&
							fIter->second.value() * pyx * ctxIter->prob() != 0.0)
						active.insert(fIter->first);
		}
		
		ctxActive.push_back(active);
	}
	
	return ctxActive;
}

unordered_set<size_t> activeFeatures(vector<unordered_set<size_t>> const &contextActiveFeatures)
{
	unordered_set<size_t> active;
	
	for (auto ctxIter = contextActiveFeatures.begin(); ctxIter != contextActiveFeatures.end();
			++ctxIter)
		active.insert(ctxIter->begin(), ctxIter->end());
	
	return active;
}

vector<double> initialZs(DataSet const &ds)
{
	vector<double> zs;

	auto evtFun = mem_fun_ref<vector<Event> const &>(&Context::events);
	auto evtSizeFun = mem_fun_ref(&vector<Event>::size);
	transform(ds.contexts().begin(), ds.contexts().end(), back_inserter(zs),
		compose_f_gx(evtSizeFun, evtFun));
	
	return zs;
}

struct makeSumVector
{
	vector<double> operator()(Context const &context) const
	{
		return vector<double>(context.events().size(), 1.0);
	}	
};

vector<vector<double>> initialSums(DataSet const &ds)
{
	vector<vector<double>> sums;

	transform(ds.contexts().begin(), ds.contexts().end(), back_inserter(sums),
		makeSumVector());
	
	return sums;
}

unordered_map<size_t, double> r_f(unordered_map<size_t, double> const &expFeatureValues,
	unordered_map<size_t, double> const &expModelFeatureValues)
{
	unordered_map<size_t, double> r;
	
	for (auto iter = expFeatureValues.begin(); iter != expFeatureValues.end();
			++iter)
		r[iter->first] = iter->second <=
			expModelFeatureValues.find(iter->first)->second ? 1 : -1;
	
	return r;
}

void updateGradients(DataSet const &dataSet,
	unordered_set<size_t> const &unconvergedFeatures,
	vector<unordered_set<size_t>> const &activeFeatures,
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
			
			auto p_fx = 0.0;
			for (auto evtIter = ctxIter->events().begin(), sumIter = ctxSumIter->begin();
				evtIter != ctxIter->events().end(); ++evtIter, ++sumIter)
			{
				auto fVal = 0.0;
				auto iter = evtIter->features().find(*fsIter);
				if (iter != evtIter->features().end())
					fVal = iter->second.value();
				
				auto newSum = *sumIter * exp(alphas.find(*fsIter)->second * fVal);
				
				p_fx += p_yx(newSum, newZ) * fVal;
			}
			
			auto gppSum = 0.0;
			for (auto evtIter = ctxIter->events().begin(), sumIter = ctxSumIter->begin();
				evtIter != ctxIter->events().end(); ++evtIter, ++sumIter)
			{
				auto fVal = 0.0;
				auto iter = evtIter->features().find(*fsIter);
				if (iter != evtIter->features().end())
					fVal = iter->second.value();
				
				auto newSum = *sumIter * exp(alphas.find(*fsIter)->second * fVal);
				
				gppSum += p_yx(newSum, newZ) * (fVal * fVal - fVal * p_fx);
			}
			
			(*gp)[*fsIter] = (*gp)[*fsIter] - ctxIter->prob() * p_fx;
			(*gpp)[*fsIter] = (*gpp)[*fsIter] - ctxIter->prob() * gppSum;
		}
	}
}

unordered_set<size_t> updateAlphas(unordered_set<size_t> const &unconvergedFeatures,
	unordered_map<size_t, double> const &r,
	unordered_map<size_t, double> const &gp,
	unordered_map<size_t, double> const &gpp,
	unordered_map<size_t, double> *alphas,
	double alphaThreshold)
{
	unordered_set<size_t> newUnconvergedFs = unconvergedFeatures;
	for (auto fIter = unconvergedFeatures.begin(); fIter != unconvergedFeatures.end();
		++fIter)
	{
		size_t f = *fIter;
		double rF = r.find(f)->second;
		auto newAlpha = (*alphas)[f] + rF * log(1 - rF * (gp.find(f)->second / gpp.find(f)->second));
		auto delta = abs((*alphas)[f] - newAlpha);
		(*alphas)[f] = newAlpha;
		if (delta < alphaThreshold || isnan(delta))
			newUnconvergedFs.erase(f);
	}
	
	return newUnconvergedFs;
}

unordered_map<size_t, double> a_f(unordered_set<size_t> &features)
{
	unordered_map<size_t, double> a;
	
	for (auto iter = features.begin(); iter != features.end();
			++iter)
		a[*iter] = 0.0;
		
	return a;
}

set<pair<size_t, double>, GainLess> calcGains(DataSet const &dataSet,
	vector<unordered_set<size_t>> const &contextActiveFeatures,
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
	
	set<pair<size_t, double>, GainLess> gains;
	for (auto alphaIter = alphas.begin(); alphaIter != alphas.end();
			++alphaIter)
		gains.insert(make_pair(alphaIter->first,
			gainSum[alphaIter->first] + alphaIter->second *
			expFeatureValues.find(alphaIter->first)->second
		));
	
	return gains;
}

void adjustModel(DataSet const &dataSet, size_t feature, double alpha,
	vector<vector<double>> *sums, vector<double> *zs)
{
	for (auto ctxIter = dataSet.contexts().begin(), ctxSumIter = sums->begin(), zIter = zs->begin();
		ctxIter != dataSet.contexts().end(); ++ctxIter, ++ctxSumIter, ++zIter)
	{
		for (auto evtIter = ctxIter->events().begin(), sumIter = ctxSumIter->begin();
			evtIter != ctxIter->events().end(); ++evtIter, ++sumIter)
		{
			auto fIter = evtIter->features().find(feature);
			if (fIter != evtIter->features().end())
			{
				*zIter -= *sumIter;
				*sumIter *= exp(alpha * fIter->second.value());
				*zIter += *sumIter;
			}
		}
	}
}

set<pair<size_t, double>, GainLess> fullSelectionStage(DataSet const &dataSet,
	double alphaThreshold,
	unordered_map<size_t, double> const &expVals,
	vector<double> *zs,
	vector<vector<double>> *sums,
	unordered_set<size_t> *selectedFeatures,
	SelectedFeatureAlphas *selectedFeatureAlphas)
{
	auto expModelVals = expModelFeatureValues(dataSet, *sums, *zs);

	auto ctxActiveFs = contextActiveFeatures(dataSet, *selectedFeatures, false, *sums, *zs);
	auto unconvergedFs = activeFeatures(ctxActiveFs);

	auto r = r_f(expVals, expModelVals);
	
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
	double alphaThreshold, double gainThreshold, size_t nFeatures)
{
	unordered_set<size_t> selectedFeatures;
	SelectedFeatureAlphas selectedFeatureAlphas;
	
	auto zs = initialZs(dataSet);
	auto sums = initialSums(dataSet);
	
	auto expVals = expFeatureValues(dataSet);

	while(selectedFeatures.size() < nFeatures)	
	{
		fullSelectionStage(dataSet, alphaThreshold, expVals, &zs, &sums,
			&selectedFeatures, &selectedFeatureAlphas);
		
		if (selectedFeatureAlphas.back().third < gainThreshold)
		{
			selectedFeatureAlphas.pop_back();
			break;
		}
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
