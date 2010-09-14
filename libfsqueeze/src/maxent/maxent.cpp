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

double fsqueeze::calcGain(DataSet const &dataSet,
	Sums const &sums,
	Zs const &zs,
	size_t feature,
	double alpha
)
{
	double gainSum = 0.0;
	ContextVector const &contexts = dataSet.contexts();

	#pragma omp parallel for
	for (int i = 0; i < static_cast<int>(dataSet.contexts().size()); ++i)
	{
		double newZ = zf(contexts[i].featureValues(), sums[i], zs[i], feature, alpha);
		double lg = contexts[i].prob() * log(newZ / zs[i]);
		
		#pragma omp atomic
		gainSum -= lg;
	}
	
	return gainSum + alpha * dataSet.expFeatureValues()[feature];
}

// Calculate the gain of adding each feature.
OrderedGains fsqueeze::calcGains(DataSet const &dataSet,
	vector<FeatureSet> const &contextActiveFeatures,
	Sums const &sums,
	Zs const &zs,
	FeatureWeights const &alphas
)
{
	GainMap gainSum;
	
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
			
			gainSum[f] -= lg;
		}		
	}
	
	OrderedGains gains;
	for (int f = 0; f < alphas.rows(); ++f)
		gains.insert(make_pair(f, gainSum[f] + alphas[f] *
			dataSet.expFeatureValues()[f]));
	
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

ExpectedValues fsqueeze::expFeatureValues(DsFeatureMap const &features, int nFeatures)
{
	ExpectedValues expVals(nFeatures);
	
	for (DsFeatureMap::const_iterator fIter = features.begin();
		fIter != features.end(); ++fIter)
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

lbfgsfloatval_t lbfgs_maxent_evaluate(void *instance, lbfgsfloatval_t const *x, lbfgsfloatval_t *g,
	int const n, lbfgsfloatval_t const step);
	
int lbfgs_maxent_progress(void *instance, const lbfgsfloatval_t *x, const lbfgsfloatval_t *g,
	const lbfgsfloatval_t fx, const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm,
	const lbfgsfloatval_t step, int n, int k, int ls);

struct EvaluateData
{
	DataSet const *dataSet;
	FeatureSet const *featureSet;
};

Eigen::VectorXd fsqueeze::lbfgs_maxent(DataSet const &dataSet, FeatureSet const &featureSet)
{
	lbfgsfloatval_t *x = lbfgs_malloc(dataSet.nFeatures());

	lbfgs_parameter_t param;
	lbfgs_parameter_init(&param);
	
	EvaluateData evalData = {&dataSet, &featureSet};
	int r = lbfgs(dataSet.nFeatures(), x, 0, lbfgs_maxent_evaluate, lbfgs_maxent_progress,
		const_cast<void *>(reinterpret_cast<void const *>(&evalData)), &param);

	cerr << "r: " << r << endl;
//	cerr << LBFGSERR_ROUNDING_ERROR << endl;

	Eigen::VectorXd weights(dataSet.nFeatures());
	
	for (int i = 0; i < weights.size(); ++i)
		weights[i] = x[i];
	
	cerr << "ngram: " << x[3833] << endl;
	
	lbfgs_free(x);
	
	return weights;
}

lbfgsfloatval_t lbfgs_maxent_evaluate(void *instance, lbfgsfloatval_t const *x, lbfgsfloatval_t *g,
	int const n, lbfgsfloatval_t const step)
{
	/*
	for (int i = 0; i < n; ++i)
		if (x[i] != 0.0)
			cerr << "x[" << i << "] = " << x[i] << endl;
	*/
	EvaluateData const *evalData = reinterpret_cast<EvaluateData const *>(instance);
	DataSet const *dataSet = evalData->dataSet;
	FeatureSet const *featureSet = evalData->featureSet;

	Eigen::VectorXd const &expVals = dataSet->expFeatureValues();
	for (int i = 0; i < n; ++ i)
		if (featureSet->find(i) != featureSet->end())
			g[i] = -expVals[i];

	//Eigen::VectorXd expModelVals = Eigen::VectorXd::Zero(dataSet->nFeatures());
	lbfgsfloatval_t ll = 0.0;
		
	ContextVector::const_iterator ctxIter = dataSet->contexts().begin();
	size_t i = 0;
	while (ctxIter != dataSet->contexts().end())
	{
		// Skip contexts that have a probability of zero. If we allow such
		// contexts, we can not calculate empirical p(y|x).
		if (ctxIter->prob() == 0.0) {
			++ctxIter; ++i;
			continue;
		}

		FeatureValues const &featureVals = ctxIter->featureValues();
		int nEvents = ctxIter->eventProbs().size();
		
		Eigen::VectorXd sums = Eigen::VectorXd::Zero(nEvents);
		double z = 0.0;
		
		// Calculate unnormalized probabilities, and the normalizer (Z(x)).
		for (int j = 0; j < featureVals.outerSize(); ++j)
		{
			for (FeatureValues::InnerIterator fIter(featureVals, j);
					fIter; ++fIter)
				if (featureSet->find(fIter.index()) != featureSet->end())
					sums[j] += x[fIter.index()] * fIter.value();

			sums[j] = exp(sums[j]);
			z += sums[j];
		}
		
		for (int j = 0; j < featureVals.outerSize(); ++j)
		{
			// Conditional probability of the event y, given the context x.
			double pyx = p_yx(sums[j], z);
			
			ll += ctxIter->eventProbs()[j] * log(pyx);
			
			// Contribution of this context to p(f).
			for (FeatureValues::InnerIterator fIter(featureVals, j);
					fIter; ++fIter)
				if (featureSet->find(fIter.index()) != featureSet->end())
					g[fIter.index()] += ctxIter->prob() * pyx * fIter.value();
		}
		
		++ctxIter; ++i;
	}
		
	// Calculate gradients of the log-likelihood functions for a feature,
	// given the parameter x[i].
	/*
	Eigen::VectorXd const &expVals = dataSet->expFeatureValues();
	for (int i = 0; i < expModelVals.size(); ++i)
		if (featureSet->find(i) != featureSet->end()) {
			cerr << "expVals[i] = " << expVals[i] << ", expModelVals[i] = " << expModelVals[i] << endl;
			g[i] = expVals[i] - expModelVals[i];
			cerr << "g[" << i << "] = " << g[i] << endl;
		}
    */

	//cerr << "-log_likelihood: " << -ll << endl;
	
	return -ll;
}

int lbfgs_maxent_progress(void *instance, const lbfgsfloatval_t *x, const lbfgsfloatval_t *g,
	const lbfgsfloatval_t fx, const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm,
	const lbfgsfloatval_t step, int n, int k, int ls)
{
	cerr << "lbfgs iteration: " << k << endl;
	cerr << "fx = " << fx << endl;
	
	return 0;
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
