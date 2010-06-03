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

// NOTE
//
// Currently, only one loop here has an OpenMP annotation. Most other
// cases would require a critical section in an inner loop. So, let's
// not do it without proper benchmarking.

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
	double alpha, Sums *sums, Zs *zs)
{
	ContextVector::const_iterator ctxIter = dataSet.contexts().begin();
	size_t i = 0;
	while (ctxIter != dataSet.contexts().end())
	{
		FeatureValues const &featureVals = ctxIter->featureValues();
		for (int j = 0; j < featureVals.outerSize(); ++j)
		{
			double fVal = featureVals.coeff(j, feature);
			if (fVal != 0.0)
			{
				(*zs)[i] -= (*sums)[i][j];
				(*sums)[i][j] *= exp(alpha * fVal);
				(*zs)[i] += (*sums)[i][j];
			}			
		}
		
		++ctxIter; ++i;
	}
}

double fsqueeze::calcGain(double gaussianVariance,
	DataSet const &dataSet,
	ExpectedValues const &expFeatureValues,
	Sums const &sums,
	Zs const &zs,
	size_t feature,
	double alpha
)
{
	double modelLL = 0.0;
	ContextVector const &contexts = dataSet.contexts();

	#pragma omp parallel for
	for (int i = 0; i < static_cast<int>(dataSet.contexts().size()); ++i)
	{
		double newZ = zf(contexts[i].featureValues(), sums[i], zs[i], feature, alpha);
		double lg = contexts[i].prob() * log(newZ / zs[i]);
		
		#pragma omp atomic
		modelLL += lg;
	}
	
	if (gaussianVariance != 0.0)
		modelLL -= pow(alpha, 2.0) * gaussianVariance;
	
	return -modelLL + alpha * expFeatureValues[feature];
}

// Calculate the gain of adding each feature.
OrderedGains fsqueeze::calcGains(double gaussianVariance,
	DataSet const &dataSet,
	vector<FeatureSet> const &contextActiveFeatures,
	ExpectedValues const &expFeatureValues,
	Sums const &sums,
	Zs const &zs,
	FeatureWeights const &alphas
)
{
	GainMap modelLLs;
	
	ContextVector const &contexts = dataSet.contexts();
	
	for (int i = 0; i < static_cast<int>(dataSet.contexts().size()); ++i)
	{
		for (FeatureSet::const_iterator fsIter = contextActiveFeatures[i].begin();
			fsIter != contextActiveFeatures[i].end(); ++fsIter)
		{
			int f = *fsIter;

			double newZ = zf(contexts[i].featureValues(), sums[i], zs[i], f,
				alphas[f]);
			
			double lg = contexts[i].prob() * log(newZ / zs[i]);
			
			modelLLs[f] += lg;
		}		
	}
	
	OrderedGains gains;
	for (int f = 0; f < alphas.rows(); ++f) {
		double modelLL = modelLLs[f];

		if (gaussianVariance != 0.0)
			 modelLL -= pow(alphas[f], 2.0) * gaussianVariance;

		gains.insert(make_pair(f, -modelLL + alphas[f] *
			expFeatureValues[f]));
	}
	
	return gains;
}

vector<FeatureSet> fsqueeze::contextActiveFeatures(DataSet const &dataSet,
	FeatureSet const &excludedFeatures, Sums const &sums, Zs const &zs)
{
	vector<FeatureSet> ctxActive;
	
	ContextVector::const_iterator ctxIter = dataSet.contexts().begin();
	size_t i = 0;
	while (ctxIter != dataSet.contexts().end())
	{
		FeatureSet active;

		// This context can not have active features if its probability is zero.
		if (ctxIter->prob() == 0.0)
		{
			ctxActive.push_back(active);
			++ctxIter; ++i;	
			continue;
		}

		FeatureValues const &featureVals = ctxIter->featureValues();
		for (int j = 0; j < featureVals.outerSize(); ++j)
		{
			// This event can not have active features if its probability is zero.
			if (p_yx(sums[i][j], zs[i]) == 0.0)
				continue;

			for (FeatureValues::InnerIterator fIter(featureVals, j);
					fIter; ++fIter)
				if (excludedFeatures.find(fIter.index()) == excludedFeatures.end() &&
						fIter.value() != 0.0)
					active.insert(fIter.index());
		}
		
		ctxActive.push_back(active);
		
		++ctxIter; ++i;
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
		for (std::vector<std::pair<double, double> >::const_iterator occIter =
				fIter->second.begin(); occIter != fIter->second.end();
				++occIter)
			expVal += occIter->first * occIter->second;
		expVals[fIter->first] = expVal;
	}
	
	return expVals;
}

ExpectedValues fsqueeze::expModelFeatureValues(
	DataSet const &dataSet,
	Sums const &sums, Zs const &zs)
{
	ExpectedValues expVals(dataSet.nFeatures());

	ContextVector::const_iterator ctxIter = dataSet.contexts().begin();
	size_t i = 0;
	while (ctxIter != dataSet.contexts().end())
	{
		FeatureValues const &featureVals = ctxIter->featureValues();
		
		for (int j = 0; j < featureVals.outerSize(); ++j)
		{
			double pyx = p_yx(sums[i][j], zs[i]);
			
			for (FeatureValues::InnerIterator fIter(featureVals, j);
					fIter; ++fIter)
				expVals[fIter.index()] += ctxIter->prob() * pyx * fIter.value();
		}
		
		++ctxIter; ++i;
	}
	
	return expVals;
}

Zs fsqueeze::initialZs(DataSet const &ds)
{
	size_t nContexts = ds.contexts().size();
	ContextVector const &contexts = ds.contexts();
	
	Zs zs(nContexts);
	for (size_t i = 0; i < nContexts; ++i)
		zs[i] = contexts[i].eventProbs().size();
	
	return zs;
}

Sums fsqueeze::initialSums(DataSet const &ds)
{
	Sums sums;

	transform(ds.contexts().begin(), ds.contexts().end(), back_inserter(sums),
		makeSumVector());
	
	return sums;
}

double fsqueeze::zf(FeatureValues const &featureValues, Sum const &ctxSums,
	double z, size_t feature, double alpha)
{
	for (int i = 0; i < featureValues.outerSize(); ++i)
	{
		double fVal = featureValues.coeff(i, feature);
		if (fVal != 0.0)
			z = z - ctxSums[i] + ctxSums[i] * exp(alpha * fVal);
	}
	
	return z;
}
