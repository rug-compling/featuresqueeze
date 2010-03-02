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

#ifndef FEATURESQUEEZE_UTIL_HH
#define FEATURESQUEEZE_UTIL_HH

namespace fsqueeze {

template <typename T1, typename T2, typename T3>
struct Triple
{
	Triple(T1 const &newFirst, T2 const &newSecond, T3 const &newThird)
		: first(newFirst), second(newSecond), third(newThird) {}
	T1 first;
	T2 second;
	T3 third;
};

template <typename T1, typename T2, typename T3>
Triple<T1, T2, T3> makeTriple(T1 first, T2 second, T3 third)
{
	return Triple<T1, T2, T3>(first, second, third);
}

// less for pairs (ordering: second - first)
template <typename T>
struct PairReverseLess
{
	bool operator()(std::pair<size_t, T> const &f1, std::pair<size_t, T> const &f2)
	{
		// Treat NaN as no gain.
		T g1 = std::isnan(f1.second) ? 0.0 : f1.second;
		T g2 = std::isnan(f2.second) ? 0.0 : f2.second;
		
		if (g1 == g2)
			return f1.first < f2.first;
		
		return g1 > g2;
	}
};

// Handle NaNs for doubles.
template <>
struct PairReverseLess<double>
{
	bool operator()(std::pair<size_t, double> const &f1, std::pair<size_t, double> const &f2)
	{
		// Treat NaN as no gain.
		double g1 = std::isnan(f1.second) ? 0.0 : f1.second;
		double g2 = std::isnan(f2.second) ? 0.0 : f2.second;
		
		if (g1 == g2)
			return f1.first < f2.first;
		
		return g1 > g2;
	}
};

}

#endif // FEATURESQUEEZE_UTIL_HH
