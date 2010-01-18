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

#ifndef EVENT_HH
#define EVENT_HH

#include <vector>

namespace fsqueeze {

typedef std::tr1::unordered_map<size_t, double> FeatureMap;

class Event {
public:
	Event(double prob, FeatureMap features) : d_prob(prob), d_features(features) {}
	
	/**
	 * Return the event probability.
	 */
	double prob() const;
	
	/**
	 * Set a new event probability.
	 */
	void prob(double newProb);
	
	/**
	 * Get the event feature map.
	 */
	FeatureMap const &features() const;
	
	/**
	 * Replace the event feature map.
	 */
	void features(FeatureMap const &newFeatures);
private:
	double d_prob;
	FeatureMap d_features;
};

inline double Event::prob() const
{
	return d_prob;
}

inline void Event::prob(double newProb)
{
	d_prob = newProb;
}

inline FeatureMap const &Event::features() const
{
	return d_features;
}

inline void Event::features(FeatureMap const &newFeatures)
{
	d_features = newFeatures;
}

}

#endif // EVENT_HH
