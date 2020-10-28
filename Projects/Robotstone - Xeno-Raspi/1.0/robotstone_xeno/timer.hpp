/**
 * @file	timer.hpp
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

#ifndef RT_TIMER_MACRO_HPP
#define RT_TIMER_MACRO_HPP

/*MODULE INCLUDES*/
/*=======================================================================================*/
#include <stdint.h>


/*IMPLEMENTATION SPECIFIC INCLUDES*/
/*=======================================================================================*/
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alchemy/timer.h>

#define timerMAX_DELAY 0x7FFFFFFFFFFFFFFF

namespace RealTime
{
/*MACROS*/
/*=======================================================================================*/


/*MODULE TYPES*/
/*=======================================================================================*/

typedef RTIME timeCount_t; /**< This type corresponds to time values related to the system specific units*/
#define tCFormat "%lld" /**< String formatter for timeCount_t values. */

/*CLASSES*/
/*=======================================================================================*/

/**
 * @brief Namespace related to timing services.
 */
namespace Timer
{

void Init(void);

timeCount_t GetTime(void);

timeCount_t GetPeriod(void);

size_t GetRandomValue(void);
}
}

#endif //RT_TIMER_MACRO_HPP
