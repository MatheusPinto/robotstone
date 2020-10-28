/**
 * @file Robotstone.hpp
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
 * This module contains the implementation of the Robotstone high-level class,
 * which will be inherited from more specific classes for the Master and Slave node.
 */


#ifndef ROBOTSTONE_MACRO_HPP
#define ROBOTSTONE_MACRO_HPP

#include <stdint.h>
#include "task.hpp"
#include "realtime.hpp"
#include "communic.hpp"
#include "RobotTask.hpp"
#include "whetstone.h"
#include "stdout.hpp"
#include "signal.h"
#include "rtheap.hpp"
#include "test_led.hpp"

using namespace RealTime;

typedef struct
{
	double wcrt;
	double averageResp;
	uint8_t taskID;
	uint32_t test;
}worstCaseScenario_t;


/*MACROS*/
/*=======================================================================================*/

#define robotRT_NAME "Xenomai 3.0.8 on Raspbian Linux 4.9.80 Debian 9 stretch"

#define robotEXTRA_VISUALIZATION

#define RAW_SPEED 413500

/*MODULE TYPES*/
/*=======================================================================================*/

size_t pow2(size_t value);

static const RealTime::timeCount_t uniBaselineFrequency[5] = {63, 30, 14, 10, 6};


static const RealTime::timeCount_t distBaselineFrequency[3] = {7, 5, 3};


/*CLASSES*/
/*=======================================================================================*/

class Robotstone
{
public:
	static const uint16_t handshakeMsgStep1 = 666;
	static const uint16_t handshakeMsgStep2 = 667;
	static const size_t deadlineHandshake = 777;
	static const size_t calculateRawSpeed = 0;
	static const size_t exp1 = 1;
	static const size_t exp2 = 2;
	static const size_t exp3 = 3;
	static const size_t exp4 = 4;
	static const size_t exp5 = 5;
	static const size_t exp6 = 6;
	static const size_t exp7 = 7;
	static const size_t senders = 1;
	static const size_t isMaster = 1;
	static const size_t isSlave = 0;
	static const size_t isFinished = 1;
	static const size_t isStarted = 0;
	static const size_t isRun = 2;
	static const uint32_t rawSingleLoad = 30; /**< The number of KWI executed in each raw_speed calculation iteration. */
	static const uint16_t baselineNumberExpUni = 5;
	static const uint16_t baselineNumberExpDist = 3;
	static const uint32_t baselineMessageSizeExpDist = sizeof(size_t); /**< Minimal message length. */
	static const uint16_t baselinePriorityExpUni = 2;
	static const uint16_t baselinePriorityExpDist = 2; /**< Priority of the lowest priority task in the Baseline Task-Set. */
	static const uint8_t manTaskPriority = 50; /**< The priority of management task */
	static const uint64_t testPeriod	= 10000000000; /**< Duration of each test. The expressed unity is implementation dependent. (in ns. 10 seconds.) */
	static const uint32_t taskManStackSize = 1200; /**< Management Task Stack Depth. */
	worstCaseScenario_t worstScenario;

#define rstoneEXP5_UPDATE_VALUE(test) pow(2,test+1)
#define rstoneEXP6_UPDATE_VALUE(test, i) distBaselineFrequency[i]*(1 + ((double)test)*0.1)

protected:
	LocalHeap rstoneHeap;
	LocalHeap initTimeHeap;
	size_t actualDeadlineMiss;
	Subscriber *subMan; /**< Subscriber handler for management tasks inter-communication */
	Publisher *pubMan;  /**< Publisher handler for management tasks inter-communication */
	List<RobotTask, size_t> rstoneTasks;
	uint64_t rawSpeed;	/**< The actual processor raw speed in KWIPS */
	timeCount_t rawInterval;	/**< The measurement interval of KWI executed by processor */
	Signal startTasksSig;
	size_t actualExp;
	size_t experimentReachLimit;
	uint8_t *stopCondTask;
	RealTime::timeCount_t *initialTime;
	size_t distBaselineWorkload[3];


public:
	Robotstone(void);

	void HandshakeSend(uint16_t *message);

	void HandshakeReceive(uint16_t *message);

	void CalculeRawSpeed(void);

	uint64_t GetRawSpeed(void);

	void GetWorstCase(size_t test);
};


#endif // ROBOTSTONE_MACRO_HPP
/***************************************************************************************
 * END: Module - Robotstone.hpp
 ***************************************************************************************/
