/**
 * @file	rsignal.hpp
 * @author  Matheus Leitzke Pinto <matheus.pinto@ifsc.edu.br>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This module contains the Signal class implementation, which provides
 * services to signal tasks that a condition was satisfied.
 */


#ifndef SIGNAL_HPP_
#define SIGNAL_HPP_
#include <alchemy/cond.h>
#include <alchemy/mutex.h>
#include "realtime.hpp"

namespace RealTime
{

/**
 * @brief Class that provides services to signal tasks that a condition was satisfied.
 */
class Signal
{
private:
	RT_COND cond;
	RT_MUTEX mutex;
	const char* name;
public:
	Signal(const char* name);
	void SignalizeUni(void);
	void SignalizeBroad(void);
	void Wait(void);
	const char* GetName(void);
};
}

#endif /* SIGNAL_HPP_ */
