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

#ifndef FEATURE_HH
#define FEATURE_HH

#include <tr1/unordered_map>

namespace fsqueeze {

class Feature {
public:
	Feature() : d_id(0), d_value(0) {}
	Feature(size_t id, double value) : d_id(id), d_value(value) {}
	
	/**
	 * Get the feature identifier
	 */
	size_t id() const;
	
	/**
	 * Get the feature value.
	 */
	double value() const;
private:
	size_t d_id;
	double d_value;
};

inline size_t Feature::id() const
{
	return d_id;
}

inline double Feature::value() const
{
	return d_value;
}

typedef std::tr1::unordered_map<size_t, Feature> FeatureMap;

}

#endif // FEATURE_HH