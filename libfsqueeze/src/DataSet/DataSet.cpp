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

#include "DataSet.ih"

// Error messages used in readDataSet exceptions.
string const ERR_INCORRECT_NEVENTS =
	string("Could not read the indicated number of events: ");
string const ERR_INCORRECT_NFEATURES =
	string("Incorrect number of features in event line: ");
string const ERR_INCORRECT_EVENT =
	string("Incorrect event line: ");

DataSet::DataSet(ContextVector const &contexts)
	: d_contexts(contexts), d_nFeatures(0)
{
	countFeatures();
	removeStaticFeatures();
	normalize();
	buildFeatureMap();
}

DataSet::DataSet(DataSet const &other)
{
	copy(other);
}

DataSet &DataSet::operator=(DataSet const &other)
{
	if (this != &other)
		copy(other);
	
	return *this;
}

void DataSet::copy(DataSet const &other)
{
	d_contexts = other.d_contexts;
	d_nFeatures = other.d_nFeatures;
	buildFeatureMap();
}

// Build a map of features, where the feature identifiers are keys and Event/Feature
// instance pairs values. Useful for calculating expected feature values.
void DataSet::buildFeatureMap()
{
	d_features.clear();

	for (ContextVector::const_iterator ctxIter = d_contexts.begin();
			ctxIter != d_contexts.end(); ++ctxIter)
		for (int i = 0; i < ctxIter->eventProbs().size(); ++i)
			for (FeatureValues::InnerIterator fIter(ctxIter->featureValues(), i);
					fIter; ++fIter)
				d_features[fIter.index()].push_back(make_pair(ctxIter->eventProbs().coeff(i),
					fIter.value()));
}

void DataSet::countFeatures()
{
	for (ContextVector::const_iterator ctxIter = d_contexts.begin();
			ctxIter != d_contexts.end(); ++ctxIter)
	{
		FeatureValues const &vals = ctxIter->featureValues();
		
		for (int i = 0; i < vals.outerSize(); ++i)
			for (FeatureValues::InnerIterator fIter(vals, i); fIter; ++fIter)
				if (fIter.index() > d_nFeatures)
					d_nFeatures = fIter.index();
	}
	
	++d_nFeatures;
}

FeatureChangeFreqs DataSet::dynamicFeatureFreqs() const
{
	FeatureChangeFreqs freqs(VectorXi::Zero(d_nFeatures));
	
	for (ContextVector::const_iterator ctxIter = d_contexts.begin();
		ctxIter != d_contexts.end(); ++ctxIter)
	{
		FeatureValues const &fVals = ctxIter->featureValues();
		
		// Find all non-zero features for the current context.
		unordered_set<size_t> ctxFs;
		for (int i = 0; i < fVals.outerSize(); ++i)
			for (FeatureValues::InnerIterator fIter(fVals, i); fIter; ++fIter)
				ctxFs.insert(fIter.index());
		
		// Find all feature values
		unordered_map<size_t, unordered_set<double> > ctxFVals;
		for (int i = 0; i < fVals.outerSize(); ++i)
			for (unordered_set<size_t>::const_iterator fIter = ctxFs.begin();
				fIter != ctxFs.end(); ++fIter)
			{
				double coeff = fVals.coeff(i, *fIter);
				ctxFVals[*fIter].insert(coeff);
			}
		
		for (unordered_map<size_t, unordered_set<double> >::const_iterator iter = ctxFVals.begin();
				iter != ctxFVals.end(); ++iter)
			if (iter->second.size() > 1)
				freqs[iter->first] += 1;
	}

	return freqs;
}

// Find 'dynamic' features. Dynamic features are features that do not retain the
// same value within at least one context.
unordered_set<size_t> DataSet::dynamicFeatures() const
{
	unordered_set<size_t> changing;
	
	for (ContextVector::const_iterator ctxIter = d_contexts.begin();
		ctxIter != d_contexts.end(); ++ctxIter)
	{
		FeatureValues const &fVals = ctxIter->featureValues();
		
		// Find all (non-proven) features for the current context.
		unordered_set<size_t> ctxFs;
		for (int i = 0; i < fVals.outerSize(); ++i)
			for (FeatureValues::InnerIterator fIter(fVals, i); fIter; ++fIter)
				if (changing.find(fIter.index()) == changing.end())
					ctxFs.insert(fIter.index());
		
		// Find all feature values
		unordered_map<size_t, unordered_set<double> > ctxFVals;
		for (int i = 0; i < fVals.outerSize(); ++i)
			for (unordered_set<size_t>::const_iterator fIter = ctxFs.begin();
				fIter != ctxFs.end(); ++fIter)
			{
				double coeff = fVals.coeff(i, *fIter);
				ctxFVals[*fIter].insert(coeff);
			}
		
		for (unordered_map<size_t, unordered_set<double> >::const_iterator iter = ctxFVals.begin();
				iter != ctxFVals.end(); ++iter)
			if (iter->second.size() > 1)
				changing.insert(iter->first);
	}
	
	return changing;
}

// Normalize context probabilities and context,event joint probabilities.
// Each event has a weighting/frequency (e.g. a fluency quality estimation) -
// we normalize over the sum of all weights. As a result, contexts that
// provide more information about qualitative realizations are weighted
// more heavily. (In line with Van Noord & Malouf, 2005.)
void DataSet::normalize()
{
	sumContexts();
	double ctxSum = contextSum();
	normalizeContexts(ctxSum);
	normalizeEvents(ctxSum);
}

void DataSet::normalizeContexts(double ctxSum)
{
	for (ContextVector::iterator ctxIter = d_contexts.begin();
			ctxIter != d_contexts.end(); ++ctxIter)
		ctxIter->prob(ctxIter->prob() / ctxSum);
}

void DataSet::normalizeEvents(double ctxSum)
{
	for (ContextVector::iterator ctxIter = d_contexts.begin();
		ctxIter != d_contexts.end(); ++ctxIter)
	{	
		EventProbs normEvtProbs(ctxIter->eventProbs());
		for (int i = 0; i < normEvtProbs.size(); ++i)
			normEvtProbs[i] /= ctxSum;		
		ctxIter->eventProbs(normEvtProbs);
	}
}

// Read an event line. An event line consists of:
//
// - The event frequency/weight.
// - The number of non-zero features.
// - Feature/value pairs.
//
pair<double, SparseVector<double> > DataSet::readEvent(string const &eventLine)
{
	std::vector<std::string> lineParts = stringSplit(eventLine);
	
	if (lineParts.size() < 2)
		throw runtime_error(ERR_INCORRECT_EVENT + eventLine);
	
	double eventProb = parseString<double>(lineParts[0]);
	size_t nFeatures = parseString<size_t>(lineParts[1]);

	if ((nFeatures * 2) + 2 != lineParts.size())
		throw runtime_error(ERR_INCORRECT_NFEATURES + eventLine);

	SparseVector<double> fVals;
	for (size_t i = 0; i < (2 * nFeatures); i += 2)
	{
		size_t fId = parseString<size_t>(lineParts[i + 2]);
		double fVal = parseString<double>(lineParts[i + 3]);
		fVals.coeffRef(fId) = fVal;
	}
	
	return make_pair(eventProb, fVals);
}

// Read a context, a context consists of:
//
// - A line indicating the number of events within the context.
// - Event lines.
//
Context DataSet::readContext(istream &iss)
{
	string nEventStr;
	getline(iss, nEventStr);
	size_t nEvents = parseString<size_t>(nEventStr);
	
	EventProbs evtProbs(nEvents);
	FeatureValues fVals(nEvents, 0);
	
	for (size_t i = 0; i < nEvents; ++i)
	{
		string line;
		if (!getline(iss, line))
			throw runtime_error(ERR_INCORRECT_NEVENTS + nEventStr);
		
		pair<double, SparseVector<double> > evt = readEvent(line);

		evtProbs[i] = evt.first;
		
		for (SparseVector<double>::InnerIterator fIter(evt.second); fIter;
				++fIter)
			fVals.coeffRef(i, fIter.index()) = fIter.value();		
	}
	
	return Context(0.0, evtProbs, fVals);
}

DataSet DataSet::readTADMDataSet(istream &iss)
{
	string line;
	ContextVector contexts;
	
	while (iss)
	{
		// Would really like to avoid reading the context here,
		// so try to detect EOF.
		if (iss.peek() == EOF)
			break;

		contexts.push_back(readContext(iss));
	}
	
	return DataSet(contexts);
}

// Remove all features that are not dynamic.
void DataSet::removeStaticFeatures()
{
	unordered_set<size_t> dynFs = dynamicFeatures();

	for (ContextVector::iterator ctxIter = d_contexts.begin();
		ctxIter != d_contexts.end(); ++ctxIter)
	{
		FeatureValues const &origFeatureVals = ctxIter->featureValues();
		FeatureValues featureVals(origFeatureVals.rows(),
			origFeatureVals.cols());

		for (int i = 0; i < origFeatureVals.outerSize(); ++i)
		{
			for (FeatureValues::InnerIterator fIter(origFeatureVals, i);
				fIter; ++fIter)
			{
				if (dynFs.find(fIter.index()) != dynFs.end())
					featureVals.coeffRef(i, fIter.index()) = fIter.value();
			}
		}
		
		ctxIter->featureValues(featureVals);
	}
}

void DataSet::sumContexts()
{
	for (ContextVector::iterator ctxIter = d_contexts.begin();
			ctxIter != d_contexts.end(); ++ctxIter)
		ctxIter->prob(ctxIter->eventProbs().sum());
}
