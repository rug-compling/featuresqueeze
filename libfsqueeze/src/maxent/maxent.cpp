#include "maxent.ih"

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

