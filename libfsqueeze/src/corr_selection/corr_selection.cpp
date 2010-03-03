#include "corr_selection.ih"

VectorXd calcAverages(DataSet const &ds)
{
	VectorXd avgs(VectorXd::Zero(ds.nFeatures()));
	size_t nEvents = 0;
	
	for (ContextVector::const_iterator ctxIter = ds.contexts().begin();
		ctxIter != ds.contexts().end(); ++ctxIter)
	{
		FeatureValues const &fVals = ctxIter->featureValues();
		
		// Find all non-zero features for the current context.
		for (int i = 0; i < fVals.outerSize(); ++i) {
			++nEvents;
			
			for (FeatureValues::InnerIterator fIter(fVals, i); fIter; ++fIter)
				avgs[fIter.index()] += fIter.value();
		}
	}
	
	for (int i = 0; i < avgs.rows(); ++i)
		avgs[i] /= nEvents;
	
	return avgs;
}

VectorXd calcSDs(DataSet const &ds, VectorXd const &avgs,
	set<pair<int, int>, PairReverseLess<int> > const &orderedFeatures)
{
	VectorXd sds(VectorXd::Zero(ds.nFeatures()));
	size_t nEvents = 0;
	
	for (ContextVector::const_iterator ctxIter = ds.contexts().begin();
		ctxIter != ds.contexts().end(); ++ctxIter)
	{
		FeatureValues const &fVals = ctxIter->featureValues();
		
		for (int i = 0; i < fVals.outerSize(); ++i) {
			++nEvents;
			
			for (set<pair<int, int>, PairReverseLess<int> >::const_iterator
					iter = orderedFeatures.begin(); iter != orderedFeatures.end();
					++iter)
				sds[iter->first] += pow(fVals.coeff(i, iter->first) - avgs[iter->first], 2);
		}
	}

	for (set<pair<int, int>, PairReverseLess<int> >::const_iterator iter = orderedFeatures.begin();
			iter != orderedFeatures.end(); ++iter)
		sds[iter->first] = sqrt(sds[iter->first] / nEvents);
	
	return sds;
}

double featureCorrelation(DataSet const &ds, VectorXd const &avgs, VectorXd const &sds,
	int f1, int f2)
{
	double r = 0.0;
	size_t nEvents = 0;
	
	for (ContextVector::const_iterator ctxIter = ds.contexts().begin();
		ctxIter != ds.contexts().end(); ++ctxIter)
	{
		FeatureValues const &fVals = ctxIter->featureValues();
		
		for (int i = 0; i < fVals.outerSize(); ++i) {
			++nEvents;
			
			r += (fVals.coeff(i, f1) - avgs[f1]) * (fVals.coeff(i, f2) - avgs[f2]);
		}
	}

	r /= (nEvents - 1) * sds(f1) * sds(f2);
	
	return r;
}

VectorXd featureCorrelations(DataSet const &ds, VectorXd const &avgs, VectorXd const &sds,
	int feature, set<pair<int, int> >::const_iterator begin,
	set<pair<int, int> >::const_iterator end)
{
	VectorXd rs(VectorXd::Zero(ds.nFeatures()));
	vector<int> fs;
	for (; begin != end; ++begin)
		fs.push_back(begin->first);
	
	#pragma omp parallel for
	for (int i = 0; i < static_cast<int>(fs.size()); ++i)
	{
		double r = featureCorrelation(ds, avgs, sds, feature, fs[i]);

		#pragma omp critical
		rs[fs[i]] = r;
	}

	return rs;
}

SelectedFeatureAlphas fsqueeze::corrFeatureSelection(DataSet const &ds, Logger logger,
	double minCorrelation, size_t nFeatures)
{
	FeatureChangeFreqs changeFreqs = ds.dynamicFeatureFreqs();
	VectorXd avgs = calcAverages(ds);
	
	// Prepare a frequency-ordered set.
	set<pair<int, int>, PairReverseLess<int> > orderedFeatures;
	for (int i = 0; i < changeFreqs.rows(); ++i)
		if (changeFreqs[i] > 1)
			orderedFeatures.insert(make_pair(i, changeFreqs[i]));

	VectorXd sds = calcSDs(ds, avgs, orderedFeatures);
	
	unordered_set<int> excludedFeatures;
	size_t nSelected = 0;
	for (set<pair<int, int> >::const_iterator iter = orderedFeatures.begin();
			iter != orderedFeatures.end() && nSelected < nFeatures; ++iter)
	{
		if (excludedFeatures.find(iter->first) != excludedFeatures.end())
			continue;

		logger.message() << iter->first << "\t" << iter->second <<
			"\t" << iter->second << "\n";
		++nSelected;

		set<pair<int, int> >::const_iterator iterCopy = iter;
		++iterCopy;
		
		// Exclude overlapping features.
		VectorXd rs = featureCorrelations(ds, avgs, sds, iter->first, iterCopy,
			orderedFeatures.end());
		for (int i = 0; i < rs.rows(); ++i)
			if (rs[i] >= minCorrelation || rs[i] <= -minCorrelation)
				excludedFeatures.insert(i);
	}
	
	SelectedFeatureAlphas selected;
		
	return selected;
}
