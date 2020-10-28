/**
 * @file	RobotTask.hpp
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

#ifndef ROBOT_TASK_MACRO_HPP
#define ROBOT_TASK_MACRO_HPP

#include "timer.hpp"
#include "list.hpp"
#include "communic.hpp"
#include "stdout.hpp"
#include "whetstone.h"
#include "rtsignal.hpp"

/**
 * @brief This class encapsulates the synthetic tasks functionalities.
 *
 */
class RobotTask
{
public:
	static const size_t maxMessageSize = 4194304; /**< The maximum messages size. Each synthetic task will always occupy this memory */
	static const size_t defaultStackSize = 500;
private:
	RealTime::timeCount_t period;
	RealTime::timeCount_t responseTime;
	RealTime::timeCount_t avgResponseJitter;
	RealTime::timeCount_t wcrt;
	RealTime::timeCount_t bcrt;
	uint32_t deadlineMiss; /**< Number of Missed Deadlines ordered by task index. */
	uint32_t deadlineMet; /**< Number of Met Deadlines ordered by task index. */
	uint32_t deadlineSkip; /**< Number of Skipped Deadlines ordered by task index. */
	uint32_t workload; /**< Tasks Loads ordered by task_index. */
	uint32_t messageSize; /**< Tasks messages size ordered by task_index. */
	size_t stackSize;
	uint8_t *stopCondition; /**< A shared variable between synthetic and management tasks to signal the end of a test step */
	RealTime::timeCount_t *initTime; /**< A shared variable between synthetic and management tasks with the initial time from test step. */
	uint16_t priority; /**< Tasks Priorities ordered by task_index */
	uint8_t message[maxMessageSize]; /**< The message buffer */
	uint8_t id; /**< The task identification. */
	double frequency; /**< Tasks Frequencies ordered by task_index. */
	double periodSec; /**< Tasks Periods ordered by task_index. */
	RealTime::Publisher* pub; /**< If task is publisher this will point to a valid instance. Otherwise is null. */
	RealTime::Subscriber* sub; /**< If task is subscriber this will point to a valid instance. Otherwise is null. */
	RealTime::Task* task;
	RealTime::Signal* signal; /**< A instance shared between synthetic and management tasks to signal the start of a test step */
	const char* name;

public:
	RobotTask(size_t id, size_t priority, RealTime::Signal* signal, uint8_t *stopCondition, RealTime::timeCount_t *initTime, const char *name);
	~RobotTask(void);
	void SetFrequency(double frequency);
	void SetPeriod(RealTime::timeCount_t period);
	void SetKWIPP(size_t kwipp);
	size_t PublishingRequest(size_t topic, size_t msgSize);
	size_t SubscribingRequest(size_t topic, size_t msgSize);
	size_t LeavePubTopic(void);
	size_t LeaveSubTopic(void);
	void Start(size_t cpuRun);
	void Join(void);
	void Unblock(void);
	size_t GetID(void);
	double GetFrequency(void);
	RealTime::timeCount_t GetPeriod(void);
	double GetPeriodSec(void);
	//List& GetPublishTopics(void);
	//List& GetSubscribeTopics(void);
	size_t GetKWIPP(void);
	double GetKWIPS(void);
	double GetUtilization(uint64_t rawSpeed);
	size_t GetDeadlineMiss(void);
	size_t GetDeadlineMet(void);
	size_t GetDeadlineSkip(void);
	double GetResponseTime(void);
	double GetResponseJitter(void);
	double GetWCRT(void);
	double GetWCRJ(void);
	void ClearWCRT(void);
	void ClearWCRJ(void);
	void WaitSignal(void);
	bool IsTaskStopped(void);
	uint32_t GetPubMessageSize(void);
	uint32_t GetSubMessageSize(void);
	size_t GetPubTopic(void);
	size_t GetSubTopic(void);
	void ClearMeasurements(void);
	bool operator == (const RobotTask& task) const { return this->id == task.id; }
	bool operator == (const size_t id) const { return this->id == id; }

private:
	friend taskFUNCTION(TaskCode, arg);
	void ReceiveMessage(void);
	void SendMessage(void);
	void ExecuteWorkload();
};


#endif // ROBOT_TASK_MACRO_HPP
/***************************************************************************************
 * END: Module - RobotTask.hpp
 ***************************************************************************************/
