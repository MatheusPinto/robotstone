/**
 * @file	rtheap.cpp
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

#include "rtheap.hpp"

namespace RealTime
{

/**
 * @brief The LocalHeap class constructor.
 *
 * @param name - A null terminated string to identify allocated memory for sharing.
 * @param heapSize - The memory size inquire for allocation in bytes.
 */
LocalHeap::LocalHeap(const char* name, size_t heapSize)
{
	if(rt_heap_bind(&heap, name, 1000000) != 0)
	{
		if(rt_heap_create(&heap, name, heapSize, H_PRIO|H_SINGLE) != 0)
		{
			Stdout::Print("Impossible create %s heap.\n", name);
			exit(1);
		}
	}
	this->size = heapSize;
}

/**
 * @brief Allocates the memory region inquired.
 *
 * @return
 * 			- A pointer to the allocated block memory if success;
 * 			- NULL otherwise.
 *
 */
void* LocalHeap::Alloc(void)
{
	void *blockPointer;

	if(rt_heap_alloc(&heap,
					 this->size,
					 TM_INFINITE,
					 (void**)&blockPointer)
		!= 0)
	{
		return NULL;
	}
	return blockPointer;
}

/**
 * @brief Free the memory region previously allocated.
 *
 * @param block - The pointer of the previously allocated block memory.
 */
void LocalHeap::Free(void * block)
{
	rt_heap_free(&heap, block);
}

namespace Heap
{
//static uint64_t globalPoolSize = 251658240; // size for experiments in uniprocessor
//static uint64_t globalPoolSize = 335544320; // size for experiments in distribuited
static uint64_t globalPoolSize = 251658240;//167772160;

static RT_HEAP globalHeap;

/**
 * @brief Initialized the system global parameters for system allocation.
 */
void InitGlobal(void)
{
	if(rt_heap_bind(&globalHeap, "globalHeap", 1000000) != 0)
	{
		if(rt_heap_create(&globalHeap, "globalHeap", globalPoolSize, H_PRIO) != 0)
		{
			rt_printf("Impossible create global heap.\n");
			exit(1);
		}
	}
}

/**
 * @brief Allocates a memory region dynamically.
 *
 * @param size - The memory block size.
 *
 * @return
 * 			- A pointer to the allocated block memory if success;
 * 			- NULL otherwise.
 */
void *Alloc(size_t size)
{
	void *blockPointer;

	if(rt_heap_alloc(&globalHeap,
					 size,
					 1000000,
					 (void**)&blockPointer)
		!= 0)
	{
		return NULL;
	}
	return blockPointer;
}

/**
 * @brief Free the memory region previously allocated.
 *
 * @param block - The pointer of the previously allocated block memory.
 */
void Free(void * block)
{
	rt_heap_free(&globalHeap, block);
}

}
}
