/**
 * @file	rsignal.cpp
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

#include "rtsignal.hpp"
#include <stdio.h>

#include "test_led.hpp"

namespace RealTime
{

/**
 * @brief The Signal class constructor.
 *
 * @param name - A null terminated string to identify signal for sharing.
 */
Signal::Signal(const char* name):
		name(name)
{
	char mutexName[25];
	sprintf(mutexName, "mtx_%s", name);

	if(rt_cond_bind(&this->cond, name, 10000000) == 0)
	{
		rt_mutex_bind(&this->mutex, mutexName, TM_INFINITE);
	}
	else
	{
		if(rt_cond_create(&this->cond, name) || rt_mutex_create(&this->mutex, mutexName))
		{
			Stdout::Print("Impossible create %s Signal.\n", name);
			exit(1);
		}
	}
}

/**
 * @brief Sent the signal to one task.
 *
 * The task can be waiting in Signal::Wait method, or even not reach this method yet.
 * In the last case, as soon the task enters in Signal::SignalizeUni, it will go out if
 * signal was sent. Otherwise it will block until signal arrives.
 */
void Signal::SignalizeUni(void)
{
	rt_mutex_acquire(&this->mutex, TM_INFINITE);

	rt_cond_signal(&this->cond);

	rt_mutex_release(&this->mutex);
}

/**
 * @brief Broadcast the signal to many tasks.
 *
 * The task must be waiting in Signal::Wait method.
 * If not, the tasks that call Signal::Wait after a call to Signal::SignalizeBroad will
 * be blocked indefinitely.
 */
void Signal::SignalizeBroad(void)
{
	rt_mutex_acquire(&this->mutex, TM_INFINITE);

	rt_cond_broadcast(&this->cond);

	rt_mutex_release(&this->mutex);
}

/**
 * @brief Wait to the signal arrives.
 *
 * The task will be blocked until the signal arrives.
 * See Signal::SignalizeUni and Signal::SignalizeBroad
 * for sending a signal.
 */
void Signal::Wait(void)
{
	rt_mutex_acquire(&this->mutex, TM_INFINITE);
	rt_cond_wait(&this->cond, &this->mutex, TM_INFINITE);
	rt_mutex_release(&this->mutex);

}

/**
 * @brief Get the name which signal is referred.
 */
const char* Signal::GetName(void)
{
	return this->name;
}
}
