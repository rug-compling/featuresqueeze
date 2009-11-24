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

#ifndef LOGGER_HH
#define LOGGER_HH

#include <iostream>
#include <string>

namespace fsqueeze
{

class Logger
{
public:
	Logger(std::ostream &outStream, std::ostream &errStream) : d_outStream(outStream), d_errStream(errStream) {}
	std::ostream &error();
	std::ostream &message();
private:
	std::ostream &d_outStream;
	std::ostream &d_errStream;
};

inline std::ostream &Logger::error()
{
	return d_errStream;
}

inline std::ostream &Logger::message()
{
	return d_outStream;
}

}

#endif // LOGGER_HH