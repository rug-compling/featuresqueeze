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

#include "maxent.ih"

// Active features in at least one context.
FeatureSet fsqueeze::activeFeatures(vector<FeatureSet> const &contextActiveFeatures)
{
	FeatureSet active;
	
	for (vector<FeatureSet>::const_iterator ctxIter = contextActiveFeatures.begin();
			ctxIter != contextActiveFeatures.end(); ++ctxIter)
		active.insert(ctxIter->begin(), ctxIter->end());
	
	return active;
}

void fsqueeze::adjustModel(DataSet const &dataSet, size_t feature,
	double alpha, vector<vector<double> > *sums, vector<double> *zs)
{
	ContextVector::const_iterator ctxIter = dataSet.contexts().begin();
	Sums::iterator ctxSumIter = sums->begin();
	Zs::iterator zIter = zs->begin();
	while (ctxIter != dataSet.contexts().end())
	{
		EventVector::const_iterator evtIter = ctxIter->events().begin();
		Sum::iterator sumIter = ctxSumIter->begin();
		while (evtIter != ctxIter->events().end())
		{
			double fVal = evtIter->features().coeff(feature);
			if (fVal != 0.0)
			{
				*zIter -= *sumIter;
				*sumIter *= exp(alpha * fVal);
				*zIter += *sumIter;
			}
			
			++evtIter; ++sumIter;
		}
		
		++ctxIter; ++ctxSumIter; ++zIter;
	}
}

double fsqueeze::calcGain(DataSet const &dataSet,
	ExpectedValues const &expFeatureValues,
	Sums const &sums,
	Zs const &zs,
	size_t feature,
	double alpha
)
{
	double gainSum = 0.0;
	ContextVector::const_iterator ctxIter = dataSet.contexts().begin();
	Sums::const_iterator ctxSumIter = sums.begin();
	Zs::const_iterator zIter = zs.begin();
	while(ctxIter != dataSet.contexts().end())
	{
		double newZ = zf(ctxIter->events(), *ctxSumIter, *zIter, feature, alpha);
		double lg = ctxIter->prob() * log(newZ / *zIter);
		gainSum -= lg;
		
		++ctxIter; ++ctxSumIter; ++zIter;
	}
	
	return gainSum + alpha * expFeatureValues.coeff(feature);
}

// Calculate the gain of adding each feature.
OrderedGains fsqueeze::calcGains(DataSet const &dataSet,
	vector<FeatureSet> const &contextActiveFeatures,
	ExpectedValues const &expFeatureValues,
	Sums const &sums,
	Zs const &zs,
	FeatureWeights const &alphas
)
{
	GainMap gainSum;
	
	ContextVector::const_iterator ctxIter = dataSet.contexts().begin();
	vector<FeatureSet>::const_iterator ctxFsIter = contextActiveFeatures.begin();
	Sums::const_iterator ctxSumIter = sums.begin();
	Zs::const_iterator zIter = zs.begin();
	while (ctxIter != dataSet.contexts().end())
	{
		for (FeatureSet::const_iterator fsIter = ctxFsIter->begin();
			fsIter != ctxFsIter->end(); ++fsIter)
		{
			int f = *fsIter;

			double newZ = zf(ctxIter->events(), *ctxSumIter, *zIter, f,
				alphas[f]);
			
			double lg = ctxIter->prob() * log(newZ / *zIter);
			
			gainSum[f] -= lg;
		}
		
		++ctxIter; ++ctxFsIter; ++ctxSumIter; ++zIter;
	}
	
	OrderedGains gains;
	for (int f = 0; f < alphas.rows(); ++f)
		gains.insert(make_pair(f, gainSum[f] + alphas[f] *
			expFeatureValues[f]));
	
	return gains;
}

vector<FeatureSet> fsqueeze::contextActiveFeatures(DataSet const &dataSet,
	FeatureSet const &excludedFeatures, Sums const &sums, Zs const &zs)
{
	vector<FeatureSet> ctxActive;
	
	ContextVector::const_iterator ctxIter = dataSet.contexts().begin();
	Sums::const_iterator ctxSumIter = sums.begin();
	Zs::const_iterator zIter = zs.begin();
	while (ctxIter != dataSet.contexts().end())
	{
		FeatureSet active;

		// This context can not have active features if its probability is zero.
		if (ctxIter->prob() == 0.0)
		{
			ctxActive.push_back(active);
			++ctxIter; ++ctxSumIter; ++zIter;	
			continue;
		}

		EventVector::const_iterator evtIter = ctxIter->events().begin();
		Sum::const_iterator sumIter = ctxSumIter->begin();
		while (evtIter != ctxIter->events().end())
		{
			// This event can not have active features if its probability is zero.
			if (p_yx(*sumIter, *zIter) == 0.0) {
				++evtIter; ++sumIter;
				continue;
			}

			for (FeatureVector::InnerIterator fIter(evtIter->features());
					fIter; ++fIter)
				if (excludedFeatures.find(fIter.index()) == excludedFeatures.end() &&
						fIter.value() != 0.0)
					active.insert(fIter.index());
			
			++evtIter; ++sumIter;
		}
		
		ctxActive.push_back(active);
		
		++ctxIter; ++ctxSumIter; ++zIter;
	}
	
	return ctxActive;
}

ExpectedValues fsqueeze::expFeatureValues(DataSet const &dataSet)
{
	ExpectedValues expVals(dataSet.nFeatures());
	
	for (DsFeatureMap::const_iterator fIter = dataSet.features().begin();
		fIter != dataSet.features().end(); ++fIter)
	{
		double expVal = 0.0;
		for (std::vector<std::pair<Event const *, double> >::const_iterator occIter =
				fIter->second.begin(); occIter != fIter->second.end();
				++occIter)
			expVal += occIter->first->prob() * occIter->second;
		expVals.coeffRef(fIter->first) = expVal;
	}
	
	return expVals;
}

ExpectedValues fsqueeze::expModelFeatureValues(
	DataSet const &dataSet,
	Sums const &sums, Zs const &zs)
{
	ExpectedValues expVals(dataSet.nFeatures());

	ContextVector::const_iterator ctxIter = dataSet.contexts().begin();
	Sums::const_iterator ctxSumIter = sums.begin();
	Zs::const_iterator zIter = zs.begin();
	while (ctxIter != dataSet.contexts().end())
	{
		EventVector::const_iterator evtIter = ctxIter->events().begin();
		Sum::const_iterator sumIter = ctxSumIter->begin();
		while (evtIter != ctxIter->events().end())
		{
			double pyx = p_yx(*sumIter, *zIter);
			
			for (FeatureVector::InnerIterator fIter(evtIter->features());
					fIter; ++fIter)
				expVals.coeffRef(fIter.index()) += ctxIter->prob() * pyx * fIter.value();
			
			++evtIter; ++sumIter;
		}
		
		++ctxIter; ++ctxSumIter; ++zIter;
	}
	
	return expVals;
}

vector<double> fsqueeze::initialZs(DataSet const &ds)
{
	Zs zs;

	transform(ds.contexts().begin(), ds.contexts().end(), back_inserter(zs),
		compose_f_gx(
			mem_fun_ref(&EventVector::size),
			mem_fun_ref<EventVector const &>(&Context::events)
		)
	);
	
	return zs;
}

vector<vector<double> > fsqueeze::initialSums(DataSet const &ds)
{
	vector<vector<double> > sums;

	transform(ds.contexts().begin(), ds.contexts().end(), back_inserter(sums),
		makeSumVector());
	
	return sums;
}

double fsqueeze::zf(EventVector const &events, Sum const &ctxSums, double z,
	size_t feature, double alpha)
{
	double newZ = z;

	EventVector::const_iterator evtIter = events.begin();
	Sum::const_iterator sumIter = ctxSums.begin();
	while (evtIter != events.end())
	{
		double fVal = evtIter->features().coeff(feature);
		if (fVal != 0.0)
			newZ = newZ - *sumIter + *sumIter * exp(alpha * fVal);

		++evtIter; ++sumIter;
	}
	
	return newZ;
}
