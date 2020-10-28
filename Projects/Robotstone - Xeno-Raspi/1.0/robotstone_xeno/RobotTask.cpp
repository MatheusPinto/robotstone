/**
 * @file	RobotTask.cpp
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
 * This module contains the implementation of the RobotTask class,
 * which encapsulate synthetic tasks functionalities.
 */

#include "RobotTask.hpp"
#include "rtheap.hpp"
#include <iostream> //for use of new(pointer)
#include <sched.h>

#define Abs( x ) (((x) > 0) ? x : -(x))

taskFUNCTION(TaskCode, arg);

/*FUNCTIONS*/
/*=======================================================================================*/

/**
 * @brief Evaluate if a value is inside a ceiling.
 *
 * @param x - the value for compare.
 * @param y - the ceiling value.
 *
 * @return 1 - if "x" = 0 or if "x" value is less or equal then "y" ceiling;
 *		   !1 - if "x" value is bigger then "y" ceiling.
 *
 */
#define CEILING(x,y) ((x == 0)? 1 : (1 + ((x - 1) / y)))

/**
 * @brief RobotTask class constructor.
 *
 */
RobotTask::RobotTask(size_t id, size_t priority, RealTime::Signal* signal, uint8_t *stopCondition, RealTime::timeCount_t *initTime, const char *name):
		period(0),
		responseTime(0),
		avgResponseJitter(0),
		wcrt(0),
		bcrt(RealTime::Timer::GetPeriod()*100),
		deadlineMiss(0), deadlineMet(0), deadlineSkip(0),
		workload(0),
		messageSize(0),
		stackSize(RobotTask::defaultStackSize),
		stopCondition(stopCondition),
		initTime(initTime),
		priority(priority),
		id(id),
		frequency(0),
		periodSec(0),
		pub(NULL), sub(NULL),
		signal(signal),
		name(name)
{
	void *p = RealTime::Heap::Alloc(sizeof(RealTime::Task));

	this->task = new(p) RealTime::Task(TaskCode, priority, this->stackSize, this, id, name);
}

/**
 * @brief RobotTask class destructor.
 *
 */
RobotTask::~RobotTask(void)
{
}

/**
 * @brief Set the task activation period.
 *
 * @param period - The activation period in implementation specific unit.
 *
 */
void RobotTask::SetPeriod(RealTime::timeCount_t period)
{
	this->period = period;
	this->periodSec = ((double)period)/((double)RealTime::Timer::GetPeriod());
	this->frequency = 1.0000/this->periodSec;
}

/**
 * @brief Set the task activation frequency.
 *
 * @param frequency - The activation frequency in seconds.
 *
 */
void RobotTask::SetFrequency(double frequency)
{
	this->periodSec = (1.0000/frequency);
	this->period = (this->periodSec*((double)RealTime::Timer::GetPeriod()));
	this->frequency = frequency;
}

/**
 * @brief Set the task workload.
 *
 * @param kwipp - The task workload in KWIPP.
 *
 */
void RobotTask::SetKWIPP(size_t kwipp)
{
	this->workload = kwipp;
}

/**
 * @brief Request to be a publisher in a specific topic using the
 * 		  publisher/subscriber mechanism.
 *
 * @param topic - topic identifier.
 * @param msgSize - the message size in bytes.
 *
 * @return 1 if was successfully requested and 0 otherwise.
 *
 */
size_t RobotTask::PublishingRequest(size_t topic, size_t msgSize)
{
	this->pub = RealTime::Communic::PublishingRequest(topic, msgSize);

	if(pub == NULL)
	{
		RealTime::Stdout::Print("Problem in publish request for topic %d by hart task %d.\n", topic, this->id);
		return 1;
	}

	return 0;
}

/**
 * @brief Request to be a subscriber in a specific topic using the
 * 		  publisher/subscriber mechanism.
 *
 * @param topic - topic identifier.
 * @param msgSize - the message size in bytes.
 *
 * @return 1 if was successfully requested and 0 otherwise.
 *
 */
size_t RobotTask::SubscribingRequest(size_t topic, size_t msgSize)
{
	this->sub = RealTime::Communic::SubscribingRequest(topic, msgSize);

	if(sub == NULL)
	{
		RealTime::Stdout::Print("Problem in subscribe request for topic %d by hart task %d.\n", topic, this->id);
		return 1;
	}

	return 0;
}

/**
 * @brief Leave a previous topic as publisher.
 *
 * @return 1 if was successfully requested and 0 otherwise.
 *
 */
size_t RobotTask::LeavePubTopic(void)
{
	RealTime::Communic::LeaveTopic(this->pub);
	return 0;
}

/**
 * @brief Leave a previous topic as subscriber.
 *
 * @return 1 if was successfully requested and 0 otherwise.
 *
 */
size_t RobotTask::LeaveSubTopic(void)
{
	RealTime::Communic::LeaveTopic(this->sub);
	return 0;
}

/**
 * @brief Start to run the task as soon as is its turn.
 *
 * @param cpuRun - cpu number to run.
 *
 */
void RobotTask::Start(size_t cpuRun)
{
	this->task->Start();
}

/**
 * @brief A call to this method will block the caller until
 * 		  the synthetic task leaves its superlooop.
 */
void RobotTask::Join(void)
{
	this->task->Join();
}

/**
 * @brief Unblock the synthetic task.
 */
void RobotTask::Unblock(void)
{
	this->task->Unblock();
}

/**
 * @brief Get the task identifier.
 *
 * @return The task identifier.
 */
size_t RobotTask::GetID(void)
{
	return this->id;
}

/**
 * @brief Get the task activation frequency.
 *
 * @return The task activation frequency in seconds.
 */
double RobotTask::GetFrequency(void)
{
	return this->frequency;
}

/**
 * @brief Get the task activation period in seconds.
 *
 * @return The task activation period in seconds.
 */
double RobotTask::GetPeriodSec(void)
{
	return this->periodSec;
}

/**
 * @brief Get the task activation period in implementation specific units.
 *
 * @return The task activation period in implementation specific units.
 */
RealTime::timeCount_t RobotTask::GetPeriod(void)
{
	return this->period;
}

/**
 * @brief Get the task workload in KWIPP.
 *
 * @return The task workload in KWIPP.
 */
size_t RobotTask::GetKWIPP(void)
{
	return this->workload;
}

/**
 * @brief Get the task utilization in KWIPS.
 *
 * @return The task utilization in KWIPS.
 */
double RobotTask::GetKWIPS(void)
{
	return this->frequency * (double)this->workload;
}

/**
 * @brief Get the task utilization in percentage value.
 *
 * @param rawSpeed - The processor speed in KWIPS.
 *
 * @return The task utilization in percentage value.
 */
double RobotTask::GetUtilization(uint64_t rawSpeed)
{
	return (((double)(this->frequency * (double)this->workload))/ ((double)rawSpeed))*100;
}

/**
 * @brief Get the task deadlines missed in a test step.
 *
 * @return The number of deadlines missed.
 */
size_t RobotTask::GetDeadlineMiss(void)
{
	return this->deadlineMiss;
}


/**
 * @brief Get the task deadlines met in a test step.
 *
 * @return The number of deadlines met.
 */
size_t RobotTask::GetDeadlineMet(void)
{
	return this->deadlineMet;
}

/**
 * @brief Get the task deadlines skipped in a test step.
 *
 * @return The number of deadlines skipped.
 */
size_t RobotTask::GetDeadlineSkip(void)
{
	return this->deadlineSkip;
}

/**
 * @brief Get the task worst-case response time in seconds.
 *
 * @return The task worst-case response time in seconds.
 */
double RobotTask::GetWCRT(void)
{
	return (double)((this->wcrt)/(RealTime::Timer::GetPeriod()));
}

/**
 * @brief Get the task worst-case response jitter in seconds.
 *
 * @return The task worst-case response jitter in seconds.
 */
double RobotTask::GetWCRJ(void)
{
	return (double)((this->wcrt - this->bcrt)/(RealTime::Timer::GetPeriod()));
}

/**
 * @brief Get the task average-case response time in seconds.
 *
 * @return The task average-case response time in seconds.
 */
double RobotTask::GetResponseTime(void)
{
	return ((double)this->responseTime)/((double)RealTime::Timer::GetPeriod());
}

/**
 * @brief Get the task average-case response jitter in seconds.
 *
 * @return The task average-case response jitter in seconds.
 */
double RobotTask::GetResponseJitter(void)
{
	return ((double)this->avgResponseJitter)/((double)RealTime::Timer::GetPeriod());
}

/**
 * @brief Blocks the synthetic task until a signal is sent from management task.
 */
void RobotTask::WaitSignal(void)
{
	this->signal->Wait();
}

/**
 * @brief Blocks the synthetic task until a complete message is read from its topic.
 *
 * The message has no useful information and is filled with dummy bytes by publisher tasks.
 */
void RobotTask::ReceiveMessage(void)
{
	if(this->sub == NULL)
	{
		return;
	}

	this->sub->Receive(this->message);
}

/**
 * @brief Blocks the synthetic task until a complete message is sent to its topic.
 *
 * The message has no useful information and is filled with dummy bytes by publisher tasks.
 */
void RobotTask::SendMessage(void)
{
	if(this->pub == NULL)
	{
		return;
	}

	this->pub->Send(this->message);
}

/**
 * @brief Execute the synthetic workload.
 */
void RobotTask::ExecuteWorkload(void)
{
	WHETSTONE_EXECUTE(this->workload);
}

/**
 * @brief Verify if task must stop its execution to finish the test step.
 */
bool RobotTask::IsTaskStopped(void)
{
	return *(this->stopCondition);
}

/**
 * @brief Get the size of the messages published by the synthetic task.
 *
 * @return The message size in bytes.
 */
uint32_t RobotTask::GetPubMessageSize(void)
{
	return this->pub->GetMessageSize();
}

/**
 * @brief Get the size of the messages received by the synthetic task.
 *
 * @return The message size in bytes.
 */
uint32_t RobotTask::GetSubMessageSize(void)
{
	return this->sub->GetMessageSize();
}

/**
 * @brief Get the topic identification where task is publisher.
 *
 * @return The topic id.
 */
size_t RobotTask::GetPubTopic(void)
{
	return this->pub->GetTopic();
}

/**
 * @brief Get the topic identification where task is subscriber.
 *
 * @return The topic id.
 */
size_t RobotTask::GetSubTopic(void)
{
	return this->sub->GetTopic();
}

/**
 * @brief Clear all the measurements related to the task in the test step.
 */
void RobotTask::ClearMeasurements(void)
{
	this->deadlineMet = 0;
	this->deadlineMiss = 0;
	this->deadlineSkip = 0;
	this->responseTime = 0;
	this->avgResponseJitter = 0;
	this->wcrt = 0;
	this->bcrt = RealTime::Timer::GetPeriod()*100;

}

/**
 * @brief The code executed by all the synthetic tasks.
 *
 * 		  The behavior of the code will vary accordingly to the
 * 		  task workload, frequency and messages transactions.
 *
 * @arg - A RobotTask instance with all necessary information to run the task.
 *
 */
taskFUNCTION(TaskCode, arg)
{
	RobotTask *taskHandler = (RobotTask *)arg;

	RealTime::timeCount_t activationTime, completeTime, actualResponseTime, beforeResponseTime = 0;
	float ceilingPeriod;

	uint32_t numberOfExecs = 0;

	RealTime::timeCount_t taskPeriod = taskHandler->period;

	taskHandler->WaitSignal();

	// Get the first activation time of task from baseline initial time.
	activationTime = *(taskHandler->initTime);


    while(true)
    {
    	numberOfExecs++;

    	//RealTime::Stdout::Print("%s\n", taskHandler->GetName());
    	taskHandler->ReceiveMessage();

    	// Execute the task workload if it has.
    	if(taskHandler->workload)
    	{
    		WHETSTONE_EXECUTE(taskHandler->workload);
    	}

    	taskHandler->SendMessage();


		// If in this point, the hartros management execute, so it interfere on scheduling to stop
		// the experiment. So the before calculus no make sense anymore and task will out
		// of its superloop.
		if(taskHandler->IsTaskStopped())
		{
			taskHandler->responseTime /= numberOfExecs-1;
			taskHandler->avgResponseJitter /= numberOfExecs-1;
			break;
		}

    	completeTime = RealTime::Timer::GetTime();

    	//actualResponseTime = completeTime - activationTime;

		/* Test to does not overflow*/

		if(completeTime >= activationTime)
		{
			actualResponseTime = completeTime - activationTime;
		}
		else
		{
			actualResponseTime = timerMAX_DELAY - completeTime + activationTime;
		}

		taskHandler->responseTime += actualResponseTime;

		if(numberOfExecs != 1)
		{
			if(actualResponseTime > beforeResponseTime) /* Test to not overflow*/
			{
				taskHandler->avgResponseJitter = taskHandler->avgResponseJitter + actualResponseTime - beforeResponseTime;
			}
			else
			{
				taskHandler->avgResponseJitter = taskHandler->avgResponseJitter + beforeResponseTime - actualResponseTime;
			}
		}

		beforeResponseTime = actualResponseTime;

		if(actualResponseTime > taskHandler->wcrt)
		{
			taskHandler->wcrt = actualResponseTime;
		}
		if(actualResponseTime < taskHandler->bcrt)
		{
			taskHandler->bcrt = actualResponseTime;
		}

		// If the task complete its execution before the deadline (inside its period), so
		// the ceilingPeriod = 1. If miss the deadline, the value will be bigger than 1,
		// resulting in a activation time corresponding to the number of periods lost.
		ceilingPeriod = CEILING(actualResponseTime, taskPeriod);
		activationTime = activationTime + ceilingPeriod * taskPeriod;


		if(ceilingPeriod == 1)
		{
			taskHandler->deadlineMet++;
		}
		else
		{
			taskHandler->deadlineMiss++;
			taskHandler->deadlineSkip += ceilingPeriod;
		}

		RealTime::Task::DelayUntil(activationTime);
    }
}

/***************************************************************************************
 * END: Module - RobotTask.cpp
 ***************************************************************************************/
