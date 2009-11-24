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
			FeatureMap::const_iterator fIter = evtIter->features().find(feature);
			if (fIter != evtIter->features().end())
			{
				*zIter -= *sumIter;
				*sumIter *= exp(alpha * fIter->second.value());
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
	
	return gainSum + alpha * expFeatureValues.find(feature)->second;
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
	vector<FeatureSet>::const_iterator fsIter = contextActiveFeatures.begin();
	Sums::const_iterator ctxSumIter = sums.begin();
	Zs::const_iterator zIter = zs.begin();
	while (ctxIter != dataSet.contexts().end())
	{
		for (FeatureWeights::const_iterator alphaIter =
			alphas.begin(); alphaIter != alphas.end();
			++alphaIter)
		{
			double newZ = zf(ctxIter->events(), *ctxSumIter, *zIter, alphaIter->first,
				alphaIter->second);
			
			double lg = ctxIter->prob() * log(newZ / *zIter);
			
			gainSum[alphaIter->first] -= lg;
		}
		
		++ctxIter; ++fsIter; ++ctxSumIter; ++zIter;
	}
	
	OrderedGains gains;
	for (FeatureWeights::const_iterator alphaIter = alphas.begin();
			alphaIter != alphas.end(); ++alphaIter)
		gains.insert(make_pair(alphaIter->first,
			gainSum[alphaIter->first] + alphaIter->second *
			expFeatureValues.find(alphaIter->first)->second
		));
	
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
			continue;
		}

		EventVector::const_iterator evtIter = ctxIter->events().begin();
		Sum::const_iterator sumIter = ctxSumIter->begin();
		while (evtIter != ctxIter->events().end())
		{
			// This event can not have active features if its probability is zero.
			if (p_yx(*sumIter, *zIter) == 0.0)
				continue;

			for (FeatureMap::const_iterator fIter = evtIter->features().begin();
					fIter != evtIter->features().end();
					++fIter)
				if (excludedFeatures.find(fIter->first) == excludedFeatures.end() &&
						fIter->second.value() != 0.0)
					active.insert(fIter->first);
			
			++evtIter; ++sumIter;
		}
		
		ctxActive.push_back(active);
		
		++ctxIter; ++ctxSumIter; ++zIter;
	}
	
	return ctxActive;
}

ExpectedValues fsqueeze::expFeatureValues(DataSet const &dataSet)
{
	ExpectedValues expVals;
	
	for (DsFeatureMap::const_iterator fIter = dataSet.features().begin();
		fIter != dataSet.features().end(); ++fIter)
	{
		double expVal = 0.0;
		for (std::vector<std::pair<Event const *, Feature const *> >::const_iterator occIter =
				fIter->second.begin(); occIter != fIter->second.end();
				++occIter)
			expVal += occIter->first->prob() * occIter->second->value();
		expVals[fIter->first] = expVal;
	}
	
	return expVals;
}

ExpectedValues fsqueeze::expModelFeatureValues(
	DataSet const &dataSet,
	Sums const &sums, Zs const &zs)
{
	ExpectedValues expVals;

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
			
			for (FeatureMap::const_iterator fIter = evtIter->features().begin();
					fIter != evtIter->features().end(); ++fIter)
				expVals[fIter->first] += ctxIter->prob() * pyx * fIter->second.value();
			
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
		FeatureMap::const_iterator iter = evtIter->features().find(feature);
		if (iter != evtIter->features().end())
			newZ = newZ - *sumIter + *sumIter * exp(alpha * iter->second.value());

		++evtIter; ++sumIter;
	}
	
	return newZ;
}