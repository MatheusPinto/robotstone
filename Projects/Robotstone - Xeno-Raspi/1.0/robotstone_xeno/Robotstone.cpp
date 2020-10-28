/**
 * @file
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

#include "Robotstone.hpp"

// Calculates the power of 2 from the value.
size_t pow2(size_t value)
{
	size_t retValue = 1;
	for(size_t i = 0; i < value; ++i)
	{
		retValue *= 2;
	}
	return retValue;
}


/**
 * @brief Robotstone high level class constructor.
 *
 */
Robotstone::Robotstone(void):
	experimentReachLimit(0),
	rstoneHeap("hart_heap", 2),
	initTimeHeap("initT_heap", sizeof(RealTime::timeCount_t)),
	actualDeadlineMiss(0),
	subMan(NULL), pubMan(NULL),
	rawSpeed(RAW_SPEED),
	rstoneTasks(),
	rawInterval(0),
	startTasksSig("start_sig"),
	actualExp()
{
	worstScenario.averageResp = 0;
	worstScenario.test = 0;
	worstScenario.wcrt = 0;
	worstScenario.taskID = 0;

	// The raw_interval is a time period used to calculate the raw speed of machine.
	// E.g.: if raw_single_load=10 and timerPeriod=1000, then raw_interval=10000.

	// In this case, if raw_speed=13007 then was executed 130070 KWI in 10 seconds.
	this->rawInterval = (Robotstone::rawSingleLoad) * (Timer::GetPeriod());

	this->stopCondTask = (uint8_t *)this->rstoneHeap.Alloc();
	*(this->stopCondTask) = 0;

	this->initialTime = (RealTime::timeCount_t *)this->initTimeHeap.Alloc();
	*(this->initialTime) = 0;
}

/**
 * @brief Send a message to the receiver, by means of a handshake.
 * 		  The sender will firstly send a handshake message to initiate
 * 		  the transaction.
 *
 * @param message - Pointer to the buffer message that will be sent.
 *
 */
void Robotstone::HandshakeSend(uint16_t *message)
{
	uint16_t resMsg = Robotstone::handshakeMsgStep1;

	// The topic 0 is always used by management tasks communication.
    // The topic transmit a 2 bytes message. That way, the same memory
    // buffer is used to transmit expMsg and resMsg.
	RealTime::Task::Delay(Timer::GetPeriod()/5);
	this->pubMan->Send(&resMsg);
	resMsg = 0;
	// The task will be suspended until message arrives.
	this->subMan->Receive(&resMsg);
	if(resMsg != Robotstone::handshakeMsgStep1)
	{
		Stdout::Print("Handshake Send Error: invalid response message in step 1.\n");
		exit(1);
	}
	RealTime::Task::Delay(Timer::GetPeriod()/5);
    this->pubMan->Send(message);

	// The task will be suspended until message arrives.
	this->subMan->Receive(&resMsg);
    if(resMsg != Robotstone::handshakeMsgStep2)
    {
		Stdout::Print("Handshake Send Error: invalid response message in step 2.\n");
		exit(1);
    }
    // Delay for the side on sender finish the transaction
   	// besides this is not a guarantee that other side will
   	// finish, that is a temporary solution for broadcast
   	// problem.
   	RealTime::Task::Delay(RealTime::Timer::GetPeriod());
}

/**
 * @brief Receive a message from the sender, by means of a handshake.
 * 		  The receiver will wait for a handshake message by the sender
 * 		  to initiate the transaction.
 *
 * @param message - Pointer to the buffer message that will be sent.
 *
 */
void Robotstone::HandshakeReceive(uint16_t *message)
{
	uint16_t resMsg = 0;

	// The task will be suspended until message arrives.
	this->subMan->Receive(&resMsg);
	if(resMsg != Robotstone::handshakeMsgStep1)
	{
		Stdout::Print("Handshake Receive Error: invalid response message in step 1.\n");
		exit(1);
	}

	RealTime::Task::Delay(Timer::GetPeriod()/5);
	this->pubMan->Send(&resMsg);

	// The task will be suspended until message arrives.
	this->subMan->Receive(message);

	RealTime::Task::Delay(Timer::GetPeriod()/5);
	resMsg = Robotstone::handshakeMsgStep2;
	this->pubMan->Send(&resMsg);

	// Delay for the side on sender finish the transaction
	// besides this is not a guarantee that other side will
	// finish, that is a temporary solution for broadcast
	// problem.
	//RealTime::Task::Delay(RealTime::Timer::GetPeriod());
}

/**
 * @brief Calculate the the processor raw speed in KWIPS and saves in
 *        internal Robotstone attributes.
 *
 */
void Robotstone::CalculeRawSpeed(void)
{
	timeCount_t startTime, periodReached = 0, executionTime;
	this->rawSpeed = 0;

	// If the loop reach interval time, return the KWIPS stored in raw_speed.
	while( periodReached < this->rawInterval ){

		startTime = RealTime::Timer::GetTime();

		WHETSTONE_EXECUTE(this->rawSingleLoad);

		executionTime = RealTime::Timer::GetTime() - startTime;

		this->rawSpeed++;
		periodReached += executionTime;

	}
}

/**
 * @brief Get the processor raw speed in KWIPS.
 *
 * @return The processor raw speed in KWIPS.
 *
 */
uint64_t Robotstone::GetRawSpeed(void)
{
	return this->rawSpeed;
}

/**
 * @brief Save the worst case scenario in the experiment.
 *
 * @param The test step in a experiment.
 *
 */
void Robotstone::GetWorstCase(size_t test)
{
	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		if((this->worstScenario.averageResp == 0) ||
           ((rstoneTasks.GetValue()->GetWCRT()/rstoneTasks.GetValue()->GetResponseTime()) >
								(this->worstScenario.wcrt/this->worstScenario.averageResp)))
		{
			this->worstScenario.wcrt = rstoneTasks.GetValue()->GetWCRT();
			this->worstScenario.averageResp = rstoneTasks.GetValue()->GetResponseTime();
			this->worstScenario.taskID = rstoneTasks.GetValue()->GetID();
			this->worstScenario.test = test;
		}
	}
}
