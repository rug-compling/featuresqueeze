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

#ifndef FEATURESQUEEZE_FUNCTIONAL_HH
#define FEATURESQUEEZE_FUNCTIONAL_HH

#include <functional>

namespace fsqueeze {

// Inspired by Josuttis, 1999
template <typename Fun1, typename Fun2>
class compose_f_gx_t : public std::unary_function<typename Fun1::argument_type,
	typename Fun2::argument_type>
{
public:
	compose_f_gx_t(Fun1 const &fun1, Fun2 const &fun2) : d_fun1(fun1), d_fun2(fun2) {}
	typename Fun1::result_type operator()(typename Fun2::argument_type const &x) const;
private:
	Fun1 d_fun1;
	Fun2 d_fun2;
};

template <typename Fun1, typename Fun2>
inline typename Fun1::result_type compose_f_gx_t<Fun1, Fun2>::operator()(typename Fun2::argument_type const &x) const
{
	return d_fun1(d_fun2(x));
}

template <typename Fun1, typename Fun2>
inline compose_f_gx_t<Fun1, Fun2> compose_f_gx(Fun1 const &fun1, Fun2 const &fun2)
{
	return compose_f_gx_t<Fun1, Fun2>(fun1, fun2);
}

}

#endif // FEATURESQUEEZE_FUNCTIONAL_HH