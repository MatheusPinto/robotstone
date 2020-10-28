/**
 * @file	rtheap.hpp
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
 * This module contains services related to dynamic memory allocation.
 */


#ifndef RT_HEAP_MACRO_HPP
#define RT_HEAP_MACRO_HPP

#include "alchemy/heap.h"
#include "stdlib.h"
#include <stdio.h>
#include "stdout.hpp"

using namespace std;


namespace RealTime
{

/**
 * @brief Class used to share dynamic allocated memory between
 *        tasks using a common string identifier.
 */
class LocalHeap
{
private:
	RT_HEAP heap;
	size_t size;
public:
	LocalHeap(const char* name, size_t heapSize);
	void *Alloc(void);
	void Free(void * block);
};

/**
 * @brief Namespace related to dynamic allocation services.
 */
namespace Heap
{

void InitGlobal(void);

void *Alloc(size_t size);

void Free(void * block);

}
}


#endif // RT_HEAP_MACRO_HPP
