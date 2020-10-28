/**
 * @file	task.hpp
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
 * This module contains the Task class, which provides the necessary services
 * to run a real-time task.
 */

#ifndef TASK_MACRO_HPP
#define TASK_MACRO_HPP


/*MODULE INCLUDES*/
/*=======================================================================================*/
#include <stdint.h>
#include "timer.hpp"
#include "task.hpp"
#include "list.hpp"

/*IMPLEMENTATION SPECIFIC INCLUDES*/
/*=======================================================================================*/
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alchemy/task.h>


/*MACROS*/
/*=======================================================================================*/
#define taskFUNCTION(taskFunc, arg) void taskFunc(void * arg) /**< A macro to facilitate the task functions definitions. */


namespace RealTime
{
/*MODULE TYPES*/
/*=======================================================================================*/
typedef void (*taskCode_t)(void*);
typedef RT_TASK taskHandle_t;


/*CLASSES*/
/*=======================================================================================*/

/**
 * @brief This class encapsulates all services related to the management of a real-time task.
 */
class Task{
public:
	// The maximum priority value of specific implementation.
	static const uint8_t  maxPriority 	 = 99;
	// The minimum priority value of specific implementation.
	static const uint8_t  minPriority 	 = 1;
private:
	void *args;
	uint32_t memory;
	size_t priority;
	taskCode_t func;
	taskHandle_t handle;
	size_t id;
	size_t validity;
	const char *name;
public:
	Task(taskCode_t code, uint8_t priority, uint32_t memory, void * args, size_t id, const char* name);
	~Task(void);
	bool operator == (const Task& task) const { return this->id == task.id; }
	bool operator == (const size_t id) const { return this->id == id; }
	void* GetArgs(void);
	uint8_t GetID(void);
	uint32_t GetMemoryLenght(void);
	uint8_t GetPriority(void);
	void Start(void);
	void IncreasePriority(size_t prioInc);
	taskHandle_t* GetHandle(void);
	static size_t GetPID(void);
	void Join(void);
	void Resume(void);
	void Unblock(void);

	static void Suspend(void);
	static void Delay(timeCount_t period);
	static void DelayUntil(timeCount_t date);


};
}
#endif //TASK_MACRO_HPP
