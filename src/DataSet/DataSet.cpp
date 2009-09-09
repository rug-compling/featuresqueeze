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

DataSet::DataSet(ContextVector const &contexts) : d_contexts(contexts)
{
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
	buildFeatureMap();
}

void DataSet::buildFeatureMap()
{
	d_features.clear();

	for (auto ctxIter = d_contexts.begin(); ctxIter != d_contexts.end();
			++ctxIter)
		for (auto evtIter = ctxIter->events().begin(); evtIter != ctxIter->events().end();
				++evtIter)
			for (auto fIter = evtIter->features().begin(); fIter != evtIter->features().end();
					++fIter)
				d_features[fIter->first].push_back(make_pair(&(*evtIter), &(fIter->second)));
}

double DataSet::contextSum()
{
	double ctxSum = 0.0;

	for (auto ctxIter = d_contexts.begin(); ctxIter != d_contexts.end();
			++ctxIter)
		ctxSum += ctxIter->prob();
	
	return ctxSum;
}

unordered_set<size_t> DataSet::dynamicFeatures() const
{
	unordered_set<size_t> changing;
	
	for (auto ctxIter = d_contexts.begin(); ctxIter != d_contexts.end();
		++ctxIter)
	{
		// Find all (non-proven) features for the current context.
		unordered_set<size_t> ctxFs;
		for (auto evtIter = ctxIter->events().begin(); evtIter != ctxIter->events().end();
				++evtIter)
			for (auto fIter = evtIter->features().begin(); fIter != evtIter->features().end();
					++fIter)
				if (changing.find(fIter->first) == changing.end())
					ctxFs.insert(fIter->first);
		
		// Find all feature values
		unordered_map<size_t, unordered_set<double>> fVals;
		for (auto evtIter = ctxIter->events().begin(); evtIter != ctxIter->events().end();
				++evtIter)
			for (auto fIter = ctxFs.begin(); fIter != ctxFs.end(); ++fIter)
			{
				auto iter = evtIter->features().find(*fIter);
				if (iter == evtIter->features().end())
					fVals[*fIter].insert(0.0);
				else
					fVals[*fIter].insert(iter->second.value());
			}
		
		for (auto iter = fVals.begin(); iter != fVals.end(); ++iter)
			if (iter->second.size() > 1)
				changing.insert(iter->first);
	}
	
	return changing;
}

void DataSet::normalize()
{
	sumContexts();
	auto ctxSum = contextSum();
	normalizeContexts(ctxSum);
	normalizeEvents(ctxSum);
}

void DataSet::normalizeContexts(double ctxSum)
{
	for (auto ctxIter = d_contexts.begin(); ctxIter != d_contexts.end();
			++ctxIter)
		ctxIter->prob(ctxIter->prob() / ctxSum);
}

void DataSet::normalizeEvents(double ctxSum)
{
	for (auto ctxIter = d_contexts.begin(); ctxIter != d_contexts.end();
		++ctxIter)
	{
		vector<Event> normalizedEvents;
		transform(ctxIter->events().begin(), ctxIter->events().end(),
			back_inserter(normalizedEvents), NormalizeEvent(ctxSum));
		ctxIter->events(normalizedEvents);
	}
}

Event DataSet::readEvent(string const &eventLine)
{
	auto lineParts = stringSplit(eventLine);
	auto eventProb = parseString<double>(lineParts[0]);
	auto nFeatures = parseString<size_t>(lineParts[1]);
	
	FeatureMap features;

	for (size_t i = 0; i < nFeatures; i += 2)
	{
		auto fId = parseString<size_t>(lineParts[i + 2]);
		auto fVal = parseString<double>(lineParts[i + 3]);
		features[fId] = Feature(fId, fVal);
	}
	
	return Event(eventProb, features);
}

Context DataSet::readContext(istream &iss)
{
	string nEventStr;
	getline(iss, nEventStr);
	auto nEvents = parseString<size_t>(nEventStr);
	
	EventVector events;
	
	for (size_t i = 0; i < nEvents; ++i)
	{
		string line;
		getline(iss, line);
		events.push_back(readEvent(line));
	}
	
	return Context(0.0, events);	
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

void DataSet::removeStaticFeatures()
{
	auto dynFs = dynamicFeatures();

	for (auto ctxIter = d_contexts.begin(); ctxIter != d_contexts.end();
		++ctxIter)
	{
		EventVector events;

		for (auto evtIter = ctxIter->events().begin(); evtIter != ctxIter->events().end();
			++evtIter)
		{
			FeatureMap features;
			
			for (auto fIter = evtIter->features().begin(); fIter != evtIter->features().end();
					++fIter)
				if (dynFs.find(fIter->first) != dynFs.end())
					features.insert(*fIter);
			
			events.push_back(Event(evtIter->prob(), features));
		}
		
		ctxIter->events(events);
	}
}

void DataSet::sumContexts()
{
	for (auto ctxIter = d_contexts.begin(); ctxIter != d_contexts.end();
			++ctxIter)
	{
		auto sum = 0.0;
		for (auto evtIter = ctxIter->events().begin(); evtIter != ctxIter->events().end();
				++evtIter)
			sum += evtIter->prob();
		ctxIter->prob(sum);
	}
}
