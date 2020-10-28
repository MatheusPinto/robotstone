/**
 * @file	task.cpp
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


#include "task.hpp"

namespace RealTime
{

/**
 * @brief The Task class constructor.
 *
 * @param func - A pointer to the function where task will run. This function must be created with the macro #taskFUNCTION.
 * @param priority - The task priority.
 * @param memory - The stack memory size reserved for the task.
 * @param args - A pointer for task arguments.
 * @param id - A unique task identifier.
 * @param name - A optional task name or "".
 */
Task::Task(taskCode_t func, uint8_t priority, uint32_t memory, void* args, size_t id, const char* name):
id(id), validity(1)
{
	this->func = func;
	this->priority = priority;
	this->memory = memory;
	this->args = args;
	this->name = name;
}

/**
 * @brief The Task class destructor.
 */
Task::~Task(void)
{

}

/**
 * @brief Get the task arguments pointer.
 *
 * @return The task arguments pointer.
 */
void* Task::GetArgs(void){
	return this->args;	
}

/**
 * @brief Get the task identifier.
 *
 * @return The task identifier.
 */
uint8_t Task::GetID(void){
	return this->id;
}

/**
 * @brief Get the task memory length.
 *
 * @return The task memory length.
 */
uint32_t Task::GetMemoryLenght(void){
	return this->memory;	
}

/**
 * @brief Get the task priority.
 *
 * @return The task priority.
 */
uint8_t Task::GetPriority(void){
	return this->priority;	
}

/**
 * @brief Starts the task execution as soon it its turn to use CPU.
 */
void Task::Start(void)
{
	/*
	* Arguments: &task,
	*            name,
	*            stack size (0=default),
	*            priority,
	*            mode (FPU, start suspended, ...)
	*/
	rt_task_create( &this->handle,
				    this->name,
					this->memory,
					this->priority,
					T_JOINABLE);

	//rt_task_set_affinity(&this->handle, &cpus);

	/*
	* Arguments: &task,
	*            task function,
	*            function argument
	*/
	rt_task_start(&this->handle,
	              this->func,
	              (void*)this->args);
}

/**
 * @brief Increase the task priority.
 *
 * @param prioInc - The number of priority increments.
 */
void Task::IncreasePriority(size_t prioInc)
{
	uint8_t prio = this->priority;

	if(maxPriority > minPriority){

		if(prio < maxPriority){
			this->priority += prioInc;
		}

	}
	else{
		if(prio > maxPriority){
			this->priority -= prioInc;
		}

	}

}

/**
 * @brief Return the system implementation handler of the task.
 *
 * @return Pointer to the system implementation handler of the task.
 */
taskHandle_t* Task::GetHandle(void){
	return &(this->handle);
}

/**
 * @brief Return the system implementation PID reserved to the task.
 *
 * @return The system implementation PID reserved to the task.
 */
size_t Task::GetPID(void)
{
	RT_TASK *task = rt_task_self();
	RT_TASK_INFO info;

	rt_task_inquire(task, &info);

	return info.pid;
}

/**
 * @brief A call to this method will block the caller until
 * 		  the task finish its execution.
 */
void Task::Join(void)
{
	rt_task_join(&this->handle);
}

/**
 * @brief A call to this method will suspend the caller.
 */
void Task::Suspend(void)
{
	rt_task_suspend(NULL);
}

/**
 * @brief A call to this method will resume the task instance of the method.
 */
void Task::Resume(void)
{
	rt_task_resume(this->GetHandle());
}

/**
 * @brief A call to this method will unblock the task instance of the method.
 */
void Task::Unblock(void)
{
	rt_task_unblock(this->GetHandle());
}

/**
 * @brief Delay the caller for a time period, putting it in a suspended state.
 *
 * @param period - The delay period in implementation specific units.
 */
void Task::Delay(timeCount_t period)
{
	rt_task_sleep(period);
}

/**
 * @brief Delay the caller until it reaches a specified time instant, putting it in a suspended state.
 *
 * @param date - The time instant, in implementation specific units, which task must awake.
 */
void Task::DelayUntil(timeCount_t date)
{
	rt_task_sleep_until(date);
}


}
