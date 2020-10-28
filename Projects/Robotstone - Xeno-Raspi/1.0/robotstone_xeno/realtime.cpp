/**
 * @file	realtime.cpp
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
 * This module contains the system initialization API related to the real-time services.
 */

/*IMPLEMENTATION SPECIFIC INCLUDES*/
/*=======================================================================================*/
#include "realtime.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

namespace RealTime
{

/**
 * @brief Initialize the system real-time parameters.
 *
 */
void Init(void)
{
	// Lock memory : avoid memory swapping for this program
	mlockall(MCL_CURRENT|MCL_FUTURE);

	//char linuxCommand[sizeof("sudo echo -17 > /proc/999999/oom_score_adj")];

	led_Init(8);
	led_Init(9);
	led_TurnOff(8);
	led_TurnOff(9);

	// prevent process to be killed while is running
	//sprintf(linuxCommand, "sudo echo -17 > /proc/%d/oom_score_adj", getpid());
	//system(linuxCommand);


	Stdout::Init();
	Timer::Init();
	Heap::InitGlobal();
	Communic::Init();
}


int SetExecCPUS(size_t cpus)
{
	RealTime::execCPUS = cpus;
	return 1;
}


size_t GetExecCPUS()
{
	return RealTime::execCPUS;
}


}


