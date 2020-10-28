/**
 * @file	timer.cpp
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
 * This module contains the timer services and definitions.
 */

#include "timer.hpp"

/*CLASS METHODS*/
/*=======================================================================================*/

namespace RealTime
{

namespace Timer
{

static timeCount_t timerPeriod;

/**
 * @brief Initialize the services related to timing.
 *
 */
void Init(void)
{
	RT_TIMER_INFO timerInfo;

	rt_timer_inquire(&timerInfo);
	timerPeriod = 1000000000/(timeCount_t)(timerInfo.period);
}

/**
 * @brief Get the actual time of the system.
 *
 * @return The system current time, given in its specified units.
 *
 */
timeCount_t GetTime(void){
	return (timeCount_t)rt_timer_read();
}

/**
 * @brief Get corresponding value in system units related to one second.
 *
 * @return The value in system units related to one second.
 *
 */
timeCount_t GetPeriod(void){
	return timerPeriod;
}

/**
 * @brief Get a semi-random value in positive integer value.
 *
 * @return The semi-random value in positive integer value.
 *
 */
size_t GetRandomValue(void)
{
	size_t value;
	value = GetTime()-getpid();
	value = ((uint8_t)value);
	return value;
}
}
}
