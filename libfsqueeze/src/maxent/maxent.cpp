#include "maxent.ih"

void fsqueeze::adjustModel(DataSet const &dataSet, size_t feature,
	double alpha, vector<vector<double>> *sums, vector<double> *zs)
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

unordered_map<size_t, double> fsqueeze::expFeatureValues(DataSet const &dataSet)
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

unordered_map<size_t, double> fsqueeze::expModelFeatureValues(
	DataSet const &dataSet,
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

vector<double> fsqueeze::initialZs(DataSet const &ds)
{
	vector<double> zs;

	auto evtFun = mem_fun_ref<vector<Event> const &>(&Context::events);
	auto evtSizeFun = mem_fun_ref(&vector<Event>::size);
	transform(ds.contexts().begin(), ds.contexts().end(), back_inserter(zs),
		compose_f_gx(evtSizeFun, evtFun));
	
	return zs;
}

vector<vector<double>> fsqueeze::initialSums(DataSet const &ds)
{
	vector<vector<double>> sums;

	transform(ds.contexts().begin(), ds.contexts().end(), back_inserter(sums),
		makeSumVector());
	
	return sums;
}

