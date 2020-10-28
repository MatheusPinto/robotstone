/**
 * @file	RobotSlave.cpp
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
 * This module contains the implementation of the RobotSlave class,
 * which will be used by the benchmark Slave node.
 */


#include "RobotSlave.hpp"
#include "RobotTask.hpp"
#include <iostream>

#include <alchemy/timer.h>
#include <alchemy/heap.h>
#include <sys/types.h>
#include <unistd.h>

#include <sched.h>


/*FUNCTIONS*/
/*=======================================================================================*/

/**
 * @brief The entry function of the management task in Slave node.
 *
 * @param args - Arguments pointer passed to the management task.
 * 				 This will be the pointer of the RobotSlave object.
 *
 */

void ManageSlaveTask(void *args);


/*CLASS METHODS*/
/*=======================================================================================*/

/**
 * @brief RobotSlave class constructor.
 *
 */
RobotSlave::RobotSlave(void):
		manTask(ManageSlaveTask, RobotSlave::manTaskPriority, RobotSlave::taskManStackSize, (void*)this, 0, "")

{
	uint64_t baselineTaskKWIPS = (this->rawSpeed * 0.15) / 3;

	for(int i = 0; i < 3; ++i)
	{
		this->distBaselineWorkload[i] = baselineTaskKWIPS / distBaselineFrequency[i];
	}
}


/**
 * @brief Starts the operation of the Robotstone benchmark in the Slave.
 *
 */
void RobotSlave::Start(void)
{
	// Create the first task that manages the tests
	// The management task will always have a id = 0, for scheduler.
	// The object pointer is passed to task for callback the appropriate
	// method that handle the management.
	this->manTask.Start();

	getchar();
}

/**
 * @brief Initialize the experiment parameters in Slave node.
 *        The Master node must be initialized beforehand.
 */
void RobotSlave::InitExperiment(void)
{
	this->SetExperimentBaseline();
	this->StartTasks();
}

/**
 * @brief This method is called by the management task function to operate
 * 		  inside the RobotSlave instance.
 */
void RobotSlave::ManTask(void){

	uint16_t hsMsg = Robotstone::isStarted;

	cpu_set_t cpus;

	CPU_ZERO(&cpus);
	CPU_SET(1, &cpus);

	//sched_setaffinity(0, sizeof(cpu_set_t), &cpus);

#ifndef RAW_SPEED
	this->CalculeRawSpeed();
#endif
	this->HandshakeSend(&hsMsg);
	this->HandshakeReceive((uint16_t *)&this->actualExp);
	RealTime::Stdout::Print("Connection established with Master!\n");
	RealTime::Stdout::Print("Experiment %d will be performed...\n", this->actualExp);
	this->InitExperiment();
	this->StartExperiment();

}

/**
 * @brief Starts the experiment synthetic tasks.
 */
void RobotSlave::StartTasks(void)
{
	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->Start(RealTime::GetExecCPUS());
	}
}

/**
 * @brief Wait synthetic tasks to finish, i.e., go out its superloops.
 */
void RobotSlave::WaitTasksFinish(void)
{
	Task::Delay(Timer::GetPeriod());

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->Unblock();
		rstoneTasks.GetValue()->Join();
	}
}

/**
 * @brief Verify if experiment was finished my the Master node.
 *
 * @return true - if experiment was finished;
 * 		   false - otherwise.
 */
bool RobotSlave::IsFinishedByMaster(void)
{
	uint16_t res;

	HandshakeReceive(&res);

	if(res == RobotSlave::isFinished)
	{
		return true;
	}
	return false;
}

/**
 * @brief After experiment initialization, this function will be the
 * 		  management task superloop, that will start the synthetic
 * 		  tasks, get their experiment results and re-run then case
 * 		  necessary or finish the experiment in Slave node.
 */
void RobotSlave::StartExperiment(void)
{
	size_t test = 1;
	uint16_t handMsg = RobotSlave::isStarted;

	while(1)
	{
		this->HandshakeSend(&handMsg);

		this->HandshakeReceive(&handMsg);

		this->WaitTasksFinish();

		this->SignalDeadlineToMaster();

		this->GetWorstCase(test);

		this->StartReport(test);

		if(this->IsFinishedByMaster())
		{
			Stdout::Print("Experiment finished!!!\n");
			break;
		}

		test++;
		this->UpdateExperiment(test);

		if(this->experimentReachLimit)
		{
			break;
		}
	}
}

/**
 * @brief Get the number of deadlines missed in the experiment.
 *
 * @return deadlines number.
 *
 */
size_t RobotSlave::ExperimentDeadlineMiss(void)
{
	return this->actualDeadlineMiss;
}


/**
 * @brief Set all the parameters and configurations to initialize the
 * 		  baseline tasks.
 */
void RobotSlave::SetExperimentBaseline()
{

	for(int i = 0; i < RobotSlave::baselineNumberExpDist; i++)
	{
		void *p = Heap::Alloc(sizeof(RobotTask));

		if(p == NULL)
		{
			Stdout::Print("Impossible create Baseline Tasks.\nExiting experiment...");
			exit(1);
		}

		// make the actual task with high priority than next task
		RobotTask *taskHandler = new(p) RobotTask(i+1,
												Robotstone::baselinePriorityExpDist+(Robotstone::baselineNumberExpDist-(i+1)),
												&this->startTasksSig,
												this->stopCondTask,
												this->initialTime,
												"");

		taskHandler->SetKWIPP(this->distBaselineWorkload[i]);
		taskHandler->SetFrequency(distBaselineFrequency[i]);

		taskHandler->SubscribingRequest(i+2, RobotSlave::baselineMessageSizeExpDist);

		this->rstoneTasks.InsertTail(taskHandler);
	}
}

/**
 * @brief Update the actual experiment step test.
 *
 * @param test - test step number.
 *
 */
void RobotSlave::UpdateExperiment(size_t test)
{
	switch(this->actualExp)
	{
	case RobotSlave::exp4:
		this->UpdateExp4(test);
		break;
	case RobotSlave::exp5:
		this->UpdateExp5(test);
		break;
	case RobotSlave::exp6:
		this->UpdateExp6(test);
		break;
	case RobotSlave::exp7:
		this->UpdateExp7(test);
		break;
	}
}

/**
 * @brief Update the actual Experiment 4 step test.
 *
 * @param test - test step number.
 *
 */
void RobotSlave::UpdateExp4(size_t test)
{
	int i = 0;
	uint16_t resMsg;

	this->HandshakeSend(&resMsg);
	this->HandshakeReceive(&resMsg);

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->SetKWIPP(this->distBaselineWorkload[i] * (1 + ((double)test)*0.1));
		rstoneTasks.GetValue()->ClearMeasurements();
		rstoneTasks.GetValue()->Start(RealTime::GetExecCPUS());
		++i;
	}
}

/**
 * @brief Update the actual Experiment 5 step test.
 *
 * @param test - test step number.
 *
 */
void RobotSlave::UpdateExp5(size_t test)
{
	size_t topic_i = 2;
	uint16_t resMsg = 0;

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->LeaveSubTopic();

		rstoneTasks.GetValue()->ClearMeasurements();
	}

	this->HandshakeSend(&resMsg);
	this->HandshakeReceive(&resMsg);


	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->SubscribingRequest(topic_i, rstoneEXP5_UPDATE_VALUE(test));
		rstoneTasks.GetValue()->Start(RealTime::GetExecCPUS());

		topic_i++;
	}
}

/**
 * @brief Update the actual Experiment 6 step test.
 *
 * @param test - test step number.
 *
 */
void RobotSlave::UpdateExp6(size_t test)
{
	uint16_t resMsg;
	int i = 0;

	this->HandshakeSend(&resMsg);
	this->HandshakeReceive(&resMsg);

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->SetFrequency(rstoneEXP6_UPDATE_VALUE(test, i));
		rstoneTasks.GetValue()->ClearMeasurements();
		rstoneTasks.GetValue()->Start(RealTime::GetExecCPUS());

		++i;
	}
}

/**
 * @brief Update the actual Experiment 7 step test.
 *
 * @param test - test step number.
 *
 */
void RobotSlave::UpdateExp7(size_t test)
{
	static size_t taskId = 4;
	uint16_t resMsg = 0;

	void *p = Heap::Alloc(sizeof(RobotTask));

	if(p == NULL)
	{
		resMsg = 1;
		this->experimentReachLimit = 1;
		Stdout::Print("\n\nExperiment reach limit: Impossible create more tasks!!!\n\n");
		this->HandshakeSend(&resMsg);
		this->HandshakeReceive(&resMsg);
		Stdout::Print("Experiment finished!!!\n");
		return;
	}

	// make the actual task with priority equal to Task 2
	RobotTask *taskHandler = new(p) RobotTask(taskId,
											Robotstone::baselinePriorityExpDist+1,
											&this->startTasksSig,
											this->stopCondTask,
											this->initialTime,
											"");

	taskHandler->SetKWIPP(this->distBaselineWorkload[1]);
	taskHandler->SetFrequency(distBaselineFrequency[1]);

	// Subscribing in Task 2 topic (Topic 3)
	taskHandler->SubscribingRequest(3, Robotstone::baselineMessageSizeExpDist);

	this->rstoneTasks.InsertTail(taskHandler);

	++taskId;

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->ClearMeasurements();
		rstoneTasks.GetValue()->Start(RealTime::GetExecCPUS());
	}


	this->HandshakeSend(&resMsg);
	this->HandshakeReceive(&resMsg);
}

/**
 * @brief Calculates the deadlines missed number and saves in
 * 		  internal RobotSlave attributes.
 */
void RobotSlave::CalculateDeadlineMiss(void)
{
	this->actualDeadlineMiss = 0;

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		this->actualDeadlineMiss += rstoneTasks.GetValue()->GetDeadlineMiss();
	}

}

/**
 * @brief Starts the report of the test step results.
 *
 * @param test - test step number.
 *
 */
void RobotSlave::StartReport(size_t test)
{
	this->CalculateDeadlineMiss();

	this->SignalDeadlineToMaster();

	this->PrintReport(test);
}

/**
 * @brief Signal to Master the number of deadlines missed in Slave.
 */
void RobotSlave::SignalDeadlineToMaster()
{
	uint16_t deadlineSig = RobotSlave::deadlineHandshake;

	HandshakeSend(&deadlineSig);

	this->CalculateDeadlineMiss();
	deadlineSig = (uint16_t)this->actualDeadlineMiss;

	HandshakeSend(&deadlineSig);
}

/**
 * @brief Prints the report of the test step results.
 *
 * @param test - test step number.
 *
 */
void RobotSlave::PrintReport(size_t test)
{
	float totalKWIPS = 0, totalCPU = 0;

	Stdout::Print("========================================================================\n\n");
	Stdout::Print("Underlay Software Architecture:\n" robotRT_NAME "\n");
	Stdout::Print("Raw speed in Kilo-Whetstone Instructions Per Second (KWIPS): %lld\n\n", this->GetRawSpeed());
	Stdout::Print("Executable Control: ");

	Stdout::Print("Slave\n\n");

	Stdout::Print("Experiment: %d\n\n", this->actualExp);
	Stdout::Print("Test %d characteristics:\n\n", test);

	Stdout::Print("Task\tFrequency(Hz)\tKWIPP\tKWIPS\t\tCPU Utilization\n");

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		Stdout::Print("%d\t%f\t%d\t%f\t%f%\n", rstoneTasks.GetValue()->GetID(),
														 rstoneTasks.GetValue()->GetFrequency(),
														 rstoneTasks.GetValue()->GetKWIPP(),
														 rstoneTasks.GetValue()->GetKWIPS(),
														 rstoneTasks.GetValue()->GetUtilization(this->GetRawSpeed()));
		totalKWIPS += rstoneTasks.GetValue()->GetKWIPS();
		totalCPU += rstoneTasks.GetValue()->GetUtilization(this->GetRawSpeed());
	}

	Stdout::Print("\t\t\t\t-------\t\t-------\n");
	Stdout::Print("\t\t\t\t%f\t%f%\n\n", totalKWIPS, totalCPU);


	Stdout::Print("Task\tMessage Length(B)\tFrequency(Hz)\tTopic\n");

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		Stdout::Print("%d\t%d\t\t\t%f\t%d\n", rstoneTasks.GetValue()->GetID(),
														 rstoneTasks.GetValue()->GetSubMessageSize(),
														 rstoneTasks.GetValue()->GetFrequency(),
														 rstoneTasks.GetValue()->GetSubTopic());
	}

	Stdout::Print("\n");
	Stdout::Print("Initial condition: - all tasks workload set by \n\t\t\t{%d, %d, %d} KWIPP.\n", distBaselineWorkload[0],
																									  distBaselineWorkload[1],
																									  distBaselineWorkload[2]);

	Stdout::Print("                   - all tasks frequency set by {"tCFormat", "tCFormat", "tCFormat"} Hertz.\n",
																												distBaselineFrequency[0],
																												distBaselineFrequency[1],
																												distBaselineFrequency[2]);
	Stdout::Print("                   - Message length set by %d bytes.\n", Robotstone::baselineMessageSizeExpDist);


	switch(this->actualExp)
	{
	case Robotstone::exp4:
		Stdout::Print("Experiment step: Increase all tasks workloads by\n");
		Stdout::Print("\t\t\t10%% of its baseline value.\n");
		break;
	case Robotstone::exp5:
		Stdout::Print("Experiment step: scale message length by \n");
		Stdout::Print("\t\t\t the baseline length power to test step number.\n");
		break;
	case Robotstone::exp6:
		Stdout::Print("Experiment step: Increase the frequency of all tasks by \n");
		Stdout::Print("\t\t\t10%% of its baseline frequency.\n");
		break;
	case Robotstone::exp7:
		Stdout::Print("Experiment step: Increase one task with the same\n");
		Stdout::Print("\t\t\tparameters of medium priority subscriber task on Slave.\n");
		break;
	}

	Stdout::Print("------------------------------------------------------------------------\n\n");

	Stdout::Print("Test %d results:\n\n", test);
	Stdout::Print("Test duration (seconds): %lld\n\n", this->testPeriod/Timer::GetPeriod());

	Stdout::Print("Task\tPeriod(s)\tMet\t\tMissed\t\tSkipped\t\n");
		Stdout::Print("    \t         \tdeadlines\tdeadlines\tdeadlines\n");

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		Stdout::Print("%d\t%f\t%d\t\t%d\t\t%d\n", rstoneTasks.GetValue()->GetID(),
															  rstoneTasks.GetValue()->GetPeriodSec(),
															  rstoneTasks.GetValue()->GetDeadlineMet(),
															  rstoneTasks.GetValue()->GetDeadlineMiss(),
															  rstoneTasks.GetValue()->GetDeadlineSkip());
	}

	Stdout::Print("\nTask\tAverage (s)\tWCRT (s)\tAverage (s)\tWCRJ (s)\n");
	Stdout::Print("    \tResp. Time\t    \t\tResp. Jitter\n");

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		Stdout::Print("%d\t%f\t%f\t%f\t%f\n", rstoneTasks.GetValue()->GetID(),
															rstoneTasks.GetValue()->GetResponseTime(),
															rstoneTasks.GetValue()->GetWCRT(),
															rstoneTasks.GetValue()->GetResponseJitter(),
															rstoneTasks.GetValue()->GetWCRJ());
	}

	Stdout::Print("\nScenario with worst WCRT in relation with average:\n");
	Stdout::Print("- Test: %d\n", worstScenario.test);
	Stdout::Print("- Task: %d\n", worstScenario.taskID);
	Stdout::Print("- WCRT: %f seconds\n", worstScenario.wcrt);
	Stdout::Print("- Aver. Response: %f seconds\n", worstScenario.averageResp);
	Stdout::Print("\n\n========================================================================\n\n");

#ifdef	robotEXTRA_VISUALIZATION
	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		Stdout::Print("\t\t\t\t\t\t\t\t\t\t%d %f %d %d %d %f %f %f %f\n", rstoneTasks.GetValue()->GetID(),
															  	   rstoneTasks.GetValue()->GetPeriodSec(),
																   rstoneTasks.GetValue()->GetDeadlineMet(),
																   rstoneTasks.GetValue()->GetDeadlineMiss(),
																   rstoneTasks.GetValue()->GetDeadlineSkip(),
																   rstoneTasks.GetValue()->GetResponseTime(),
																   rstoneTasks.GetValue()->GetWCRT(),
																   rstoneTasks.GetValue()->GetResponseJitter(),
																   rstoneTasks.GetValue()->GetWCRJ());
	}
#endif


}

/**
 * @brief The entry function of the management task in Slave node.
 *
 * @param args - Arguments pointer passed to the management task.
 * 				 This will be the pointer of the RobotSlave object.
 *
 */
void ManageSlaveTask(void *args){
	RobotSlave* bench = (RobotSlave*) args;

	bench->pubMan = Communic::PublishingRequest(1, sizeof(uint16_t));
	bench->subMan = Communic::SubscribingRequest(0, sizeof(uint16_t));
	bench->ManTask();
}

/***************************************************************************************
 * END: Module - RobotSlave.cpp
 ***************************************************************************************/
