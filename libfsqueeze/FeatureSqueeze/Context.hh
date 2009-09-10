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

#include "Event.hh"

namespace fsqueeze {

typedef std::vector<Event> EventVector;

class Context {
public:
	/**
	 * Construct a context.
	 */
	Context(double prob, EventVector const &events) : d_prob(prob), d_events(events) {}
	
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
	EventVector const &events() const;
	
	/**
	 * Use a new event vector.
	 */
	void events(EventVector const &events);
private:
	double d_prob;
	EventVector d_events;
};

inline double Context::prob() const
{
	return d_prob;
}

inline void Context::prob(double newProb)
{
	d_prob = newProb;
}

inline EventVector const &Context::events() const
{
	return d_events;
}

inline void Context::events(EventVector const &events)
{
	d_events = events;
}

}

#endif // CONTEXT_HH