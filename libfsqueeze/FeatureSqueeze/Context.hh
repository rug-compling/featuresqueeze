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

#ifndef CONTEXT_HH
#define CONTEXT_HH

#include <vector>

#include <Eigen/Core>
#include <Eigen/Sparse>

namespace fsqueeze {

typedef Eigen::VectorXd EventProbs;
typedef Eigen::DynamicSparseMatrix<double, Eigen::RowMajor> FeatureValues;

class Context {
public:
	/**
	 * Construct a context.
	 */
	Context(double prob, EventProbs const &eventProbs,
		FeatureValues const &featureValues)
	: d_prob(prob), d_eventProbs(eventProbs), d_featureValues(featureValues) {}
	
	/**
	 * Return the context probability.
	 */
	double prob() const;
	
	/**
	 * Set the context probability. Note that you'd normally want to update the
	 * probabilities of the events within this context as well.
	 */
	void prob(double newProb);
	
	/**
	 * Get the event vector.
	 */
	EventProbs const &eventProbs() const;
	
	/**
	 * Use a new event vector.
	 */
	void eventProbs(EventProbs const &eventProbs);
	
	/**
	 * Get the feature values.
	 */
	FeatureValues const &featureValues() const;
	
	/**
	 * Set feature values.
	 */
		void featureValues(FeatureValues const &featureValues);
private:
	double d_prob;
	EventProbs d_eventProbs;
	FeatureValues d_featureValues;
};

inline double Context::prob() const
{
	return d_prob;
}

inline void Context::prob(double newProb)
{
	d_prob = newProb;
}

inline EventProbs const &Context::eventProbs() const
{
	return d_eventProbs;
}

inline void Context::eventProbs(EventProbs const &eventProbs)
{
	d_eventProbs = eventProbs;
}

inline FeatureValues const &Context::featureValues() const
{
	return d_featureValues;
}

inline void Context::featureValues(FeatureValues const &featureValues)
{
	d_featureValues = featureValues;
}


}

#endif // CONTEXT_HH