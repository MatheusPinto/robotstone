/**
 * @file	stdout.cpp
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
 * This module contains the services related to standard output for real-time tasks.
 */

#include "stdout.hpp"


namespace RealTime
{

/**
 * @brief Namespace with services related to standard output for real-time tasks..
 */
namespace Stdout
{

/**
 * @brief Initialize the system related to the standard output.
 */
void Init(void)
{
	// Perform auto-init of rt_print buffers if the task doesn't do so
	rt_print_auto_init(1);

}

/**
 * @brief Print a formatted string in the stdout.
 *
 * @param message - A formatted string.
 * @param a list of values to be formatted.
 */
void Print(const char *message, ...)
{
	va_list args;
	va_start(args, message);
	rt_vprintf(message, args);
	va_end(args);
}

}

}
