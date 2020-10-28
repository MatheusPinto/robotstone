/**
 * @file	stdout.hpp
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


#ifndef STDOUT_HPP_
#define STDOUT_HPP_

#include <stdio.h>
#include <stdint.h>
#include <trank/rtdk.h>

namespace RealTime
{
namespace Stdout
{

void Init(void);
void Print(const char *message, ...);

}

}

#endif /* STDOUT_HPP_ */
