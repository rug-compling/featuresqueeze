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

}

#endif // FEATURESQUEEZE_UTIL_HH
