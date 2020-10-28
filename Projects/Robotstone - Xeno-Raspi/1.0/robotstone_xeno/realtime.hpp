/**
 * @file	realtime.hpp
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

#ifndef REALTIME_HPP_
#define REALTIME_HPP_

/*IMPLEMENTATION SPECIFIC INCLUDES*/
/*=======================================================================================*/
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include "timer.hpp"
#include "rtheap.hpp"
#include "stdout.hpp"
#include "communic.hpp"

/**
 * @brief Namespace related to real-time services.
 */
namespace RealTime
{

const int ok = 0;
const int fail = 1;

volatile static size_t execCPUS;

void Init(void);

int SetExecCPUS(size_t cpus);

size_t GetExecCPUS();

}

#endif /* REALTIME_HPP_ */
