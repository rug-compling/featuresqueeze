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

/*
 * Datasets for maximum entropy modelling.
 */

#ifndef DATASET_HH
#define DATASET_HH

#include <istream>
#include <utility>
#include <vector>

#include <tr1/unordered_map>
#include <tr1/unordered_set>

#include "Context.hh"
#include "Event.hh"
#include "Feature.hh"

namespace fsqueeze {

typedef std::vector<Context> ContextVector;
typedef std::tr1::unordered_map<size_t,
	std::vector<std::pair<Event const *, Feature const *> > > DsFeatureMap;

/**
 * This class represents datasets to be used for feature selection. Datasets
 * can be read from a stream using one of the static members (currently only
 * readTADMDataSet).
 */
class DataSet
{
public:
	/**
	 * Construct a dataset from a context vector. The DataSet constructor
	 * will remove static (non-changing) features, and normalize the event
	 * and context probabilities.
	 */
	DataSet(ContextVector const &contexts);
	
	DataSet(DataSet const &other);
	
	DataSet &operator=(DataSet const &other);

	/**
	 * Get contexts.
	 */
	ContextVector const &contexts() const;
	
	/**
	 * Get dataset features.
	 */
	DsFeatureMap const &features() const;

	/**
	 * Read a TADM-style dataset from an input stream.
	 */
	static DataSet readTADMDataSet(std::istream &iss);
private:
	void copy(DataSet const &other);
	void buildFeatureMap();
	double contextSum() const;
	std::tr1::unordered_set<size_t> dynamicFeatures() const;
	void normalize();
	void normalizeContexts(double ctxSum);
	void normalizeEvents(double ctxSum);
	static Event readEvent(std::string const &eventLine);
	static Context readContext(std::istream &iss);
	void removeStaticFeatures();
	void sumContexts();
	
	ContextVector d_contexts;
	DsFeatureMap d_features;
};

class NormalizeEvent
{
public:
	NormalizeEvent(double ctxSum) : d_ctxSum(ctxSum) {}
	Event operator()(Event const &event) const;
private:
	double d_ctxSum;
};

template <typename T>
struct SumProb
{
	SumProb(double initial = 0.0) : sum(initial) {}
	void operator()(T const &v);
	double sum;
};

// Summed context probabilities.
inline double DataSet::contextSum() const
{
	return for_each(d_contexts.begin(), d_contexts.end(), SumProb<Context>()).sum;
}


inline ContextVector const &DataSet::contexts() const
{
	return d_contexts;
}

inline DsFeatureMap const &DataSet::features() const
{
	return d_features;
}

inline Event NormalizeEvent::operator()(Event const &event) const
{
	return Event(event.prob() / d_ctxSum, event.features());
}

template <typename T>
void SumProb<T>::operator()(T const &v)
{
	sum += v.prob();
}

}

#endif // DATASET_HH