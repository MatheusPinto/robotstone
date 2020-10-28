/**
 * @file	RobotMaster.cpp
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
 * This module contains the implementation of the RobotMaster class,
 * which will be used by the benchmark Master node.
 */

#include "RobotMaster.hpp"
#include "RobotTask.hpp"
#include <iostream>

#include <alchemy/timer.h>
#include <alchemy/heap.h>
#include <sys/types.h>
#include <unistd.h>

#include <sched.h>

/*FUNCTIONS PROTOTYPES*/
/*=======================================================================================*/

/**
 * @brief The entry function of the management task in Master node.
 *
 * @param args - Arguments pointer passed to the management task.
 * 				 This will be the pointer of the RobotMaster object.
 *
 */
void ManageMasterTask(void *args);


/*CLASS METHODS*/
/*=======================================================================================*/

/**
 * @brief RobotMaster class constructor.
 *
 */
RobotMaster::RobotMaster(void):
				actualSlaveDeadlines(0),
				receiverDeadlineMiss(0),
				manTaskSuspendStatus(false),
				expSignal("exp_sig"),
				manTask(ManageMasterTask, RobotMaster::manTaskPriority, RobotMaster::taskManStackSize, (void*)this, 0, "")
{

	uint64_t baselineTaskKWIPS = (this->rawSpeed * 0.15) / 5;

	for(int i = 0; i < 5; ++i)
	{
		this->uniBaselineWorkload[i] = baselineTaskKWIPS / uniBaselineFrequency[i];
	}

	baselineTaskKWIPS = (this->rawSpeed * 0.15) / 3;
	for(int i = 0; i < 3; ++i)
	{
		this->distBaselineWorkload[i] = baselineTaskKWIPS / distBaselineFrequency[i];
	}


	// Create the first task that manages the tests
	// The management task will always have a id = 0, for scheduler.
	// The object pointer is passed to task for callback the appropriate
	// method that handle the management.
	this->manTask.Start();
}

/**
 * @brief This method will unblock the current task (the management task) and
 * 		  will set the experiment requested.
 *
 * @param experiment - A number from 1 to 7 referring to an experiment.
 * 					   Any other number will calculate the machine raw speed.
 *
 */
void RobotMaster::RequestExperiment(uint8_t experiment)
{
	this->actualExp = experiment;

	while(this->manTaskSuspendStatus == false)
	{

	}

	this->manTask.Resume();
	getchar();
}

/**
 * @brief Initialize the experiment parameters in Master node.
 */
void RobotMaster::InitExperiment(void)
{
	if(this->actualExp > Robotstone::exp3)
	{
		this->SetExperimentDistBaseline();
	}
	else
	{
		this->SetExperimentUniBaseline();
	}

	this->StartTasks();
}

/**
 * @brief This method is called by the management task function to operate
 * 		  inside the RobotMaster instance.
 */
void RobotMaster::ManTask(void){

	uint16_t hsMsg;


		// Wait for experiment sent in main function.
		this->manTaskSuspendStatus = true;
		this->manTask.Suspend();
		this->manTaskSuspendStatus = false;

		this->InitExperiment();

		if((this->actualExp < Robotstone::exp1) || (this->actualExp > Robotstone::exp7))
		{
			CalculeRawSpeed();
			Stdout::Print("Raw speed in Kilo-Whetstone Instructions Per Second (KWIPS): %lld\n\n", this->GetRawSpeed());
		}
		else
		{
			if(this->actualExp > Robotstone::exp3)
			{

				this->pubMan = Communic::PublishingRequest(0, sizeof(uint16_t));
				this->subMan = Communic::SubscribingRequest(1, sizeof(uint16_t));

				RealTime::Stdout::Print("Master ready to begin connection with Slave!\n");

				this->HandshakeReceive(&hsMsg);
				if(hsMsg != Robotstone::isStarted)
				{
					Stdout::Print("Problem in start Hartros experiment: handshake error.\n");
					exit(1);
				}
				this->HandshakeSend((uint16_t *)&this->actualExp);

				this->StartDistributedExperiment();
			}
			else
			{
				this->StartUniExperiment();
			}
			exit(0);
			led_TurnOn(8);

		}
}

/**
 * @brief Starts the experiment synthetic tasks.
 */
void RobotMaster::StartTasks(void)
{
	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->Start(RealTime::GetExecCPUS());
	}
}

/**
 * @brief Wait synthetic tasks to finish, i.e., go out its superloops.
 */
void RobotMaster::WaitTasksFinish(void)
{
	RealTime::Task::Delay(RealTime::Timer::GetPeriod());
	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->Join();
	}
}

/**
 * @brief Sends the experiment status to the Slave Node.
 *
 * @param status -
 * 				   - \a isFinshed indicates that experiment must be finished
 * 				   - \a isRun indicates that experiment is ready to run in Master side.
 */
void RobotMaster::SendExperimentStatus(size_t status)
{
	uint16_t handMsg = status;

	this->HandshakeSend(&handMsg);
}

/**
 * @brief Start the experiment related to the processing domain and
 * 		  run until experiment finishes.
 *
 * 		  After experiment initialization, this function will be the
 * 		  management task superloop, that will start the synthetic
 * 		  tasks, get their experiment results and re-run then case
 * 		  necessary or finish the experiment in Slave node.
 */
void RobotMaster::StartUniExperiment(void)
{
	size_t test = 1;
	while(1)
	{
		Task::Delay(Timer::GetPeriod()*3); // Wait 3 second for all tasks enters in signal wait condition.
		this->startTasksSig.SignalizeBroad(); // Signalize all tasks (sender/receivers) to start.

		*(this->initialTime) = RealTime::Timer::GetTime();
		// The management task will wait the task set execute until reach the test period.
		Task::Delay(RobotMaster::testPeriod);

		*(this->stopCondTask) = 1;// Signalize all tasks (sender/receivers) to finish.

		this->WaitTasksFinish();

		*(this->stopCondTask) = 0;

		this->CalculateDeadlineMiss();

		this->PrintReport(test);

		if(this->ExperimentDeadlineMiss() || this->experimentReachLimit)
		{
			Stdout::Print("Experiment finished!!!\n");
			break;
		}

		test++;

		this->UpdateExperiment(test);
	}
}

/**
 * @brief Start the experiment related to the processing and communication
 * 		  domain and run until experiment finishes.
 *
 * 		  After experiment initialization, this function will be the
 * 		  management task superloop, that will start the synthetic
 * 		  tasks, get their experiment results and re-run then case
 * 		  necessary or finish the experiment in Slave node.
 */
void RobotMaster::StartDistributedExperiment(void)
{
	size_t test = 1;
	uint16_t handMsg;
	while(1)
	{
		this->HandshakeReceive(&handMsg);

		Task::Delay(Timer::GetPeriod()*4); // Wait 4 seconds for all tasks enters in signal wait condition.

		*(this->initialTime) = RealTime::Timer::GetTime();
		this->startTasksSig.SignalizeBroad(); // Signalize all tasks (sender/receivers) to start.


		// The management task will wait the task set execute until reach the test period.
		Task::Delay(RobotMaster::testPeriod);

		this->HandshakeSend(&handMsg); // wakeup the slave executable for finish its tasks.

		*(this->stopCondTask) = 1;// Signalize all tasks (sender/receivers) to finish.

		this->WaitTasksFinish();

		this->WaitDeadlinesFromSlave();

		*(this->stopCondTask) = 0;

		this->GetWorstCase(test);

		if(this->ExperimentDeadlineMiss())
		{
			this->StartReport(test);
			this->SendExperimentStatus(RobotMaster::isFinished);
			Stdout::Print("Experiment finished!!!\n");
			Stdout::Print("Experiment deadlines: %d.\n!!!\n", this->ExperimentDeadlineMiss());
			break;
		}

		this->StartReport(test);
		this->SendExperimentStatus(RobotMaster::isRun);
		test++;

		this->UpdateExperiment(test);

		if(this->experimentReachLimit)
		{
			break;
		}

		led_TurnOn(8);
		for(int k = 0; k < 99999999; ++k);
		led_TurnOff(8);
		for(int k = 0; k < 99999999; ++k);
	}
}

/**
 * @brief Get the number of deadlines missed in the experiment.
 *
 * @return deadlines number.
 *
 */
size_t RobotMaster::ExperimentDeadlineMiss(void)
{
	return this->actualDeadlineMiss + this->actualSlaveDeadlines;
}

/**
 * @brief Set all the parameters and configurations to initialize the
 * 		  baseline tasks related to PD experiments.
 */
void RobotMaster::SetExperimentUniBaseline()
{

	for(int i = 0; i < Robotstone::baselineNumberExpUni; i++)
	{
		void *p = Heap::Alloc(sizeof(RobotTask));

		//Hartros::baselinePriorityExpUni+(Hartros::baselineNumberExpUni-(i+1)),
		// make the actual task with high priority than next task
		RobotTask *taskHandler = new(p) RobotTask(i+1,
												Robotstone::baselinePriorityExpUni+(Robotstone::baselineNumberExpUni-(i+1)),
												&this->startTasksSig,
												this->stopCondTask,
												this->initialTime,
												"");

		taskHandler->SetKWIPP(uniBaselineWorkload[i]);
		taskHandler->SetFrequency(uniBaselineFrequency[i]);

		this->rstoneTasks.InsertTail(taskHandler);
	}

}

/**
 * @brief Set all the parameters and configurations to initialize the
 * 		  baseline tasks related to PCD experiments.
 */
void RobotMaster::SetExperimentDistBaseline(){

	char *taskName;

	for(int i = 0; i < RobotMaster::baselineNumberExpDist; i++)
	{
		taskName = (char*)RealTime::Heap::Alloc(sizeof("Tpxx"));

		if(taskName == NULL)
		{
			exit(1);
		}
		sprintf(taskName, "Tp%d", i+1);

		void *p = Heap::Alloc(sizeof(RobotTask));

		RobotTask *taskHandler = new(p) RobotTask(i+1,
												Robotstone::baselinePriorityExpDist+(Robotstone::baselineNumberExpDist-(i+1)),
												&this->startTasksSig,
												this->stopCondTask,
												this->initialTime,
												taskName);

		taskHandler->SetKWIPP(distBaselineWorkload[i]);

		taskHandler->SetFrequency(distBaselineFrequency[i]);

		taskHandler->PublishingRequest(i+2, RobotMaster::baselineMessageSizeExpDist);

		this->rstoneTasks.InsertTail(taskHandler);
	}
}

/**
 * @brief Update the actual experiment step test.
 *
 * @param test - test step number.
 *
 */
void RobotMaster::UpdateExperiment(size_t test)
{
	switch(this->actualExp)
	{
	case Robotstone::exp1:
		this->UpdateExp1(test);
		break;
	case Robotstone::exp2:
		this->UpdateExp2(test);
		break;
	case Robotstone::exp3:
		this->UpdateExp3(test);
		break;
	case Robotstone::exp4:
		this->UpdateExp4(test);
		break;
	case Robotstone::exp5:
		this->UpdateExp5(test);
		break;
	case Robotstone::exp6:
		this->UpdateExp6(test);
		break;
	case Robotstone::exp7:
		this->UpdateExp7(test);
		break;

	}
}

/**
 * @brief Update the actual Experiment 1 step test.
 *
 * @param test - test step number.
 *
 */
void RobotMaster::UpdateExp1(size_t test)
{
	int i = 0;


	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->SetKWIPP(this->uniBaselineWorkload[i] * (1 + ((double)test)*0.1));
		rstoneTasks.GetValue()->ClearMeasurements();
		rstoneTasks.GetValue()->Start(RealTime::GetExecCPUS());
		++i;
	}
}

/**
 * @brief Update the actual Experiment 2 step test.
 *
 * @param test - test step number.
 *
 */
void RobotMaster::UpdateExp2(size_t test)
{
	int i = 0;

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->SetFrequency(uniBaselineFrequency[i]*(1 + ((double)test)*0.1));
		rstoneTasks.GetValue()->ClearMeasurements();
		rstoneTasks.GetValue()->Start(RealTime::GetExecCPUS());

		++i;
	}
}

/**
 * @brief Update the actual Experiment 3 step test.
 *
 * @param test - test step number.
 *
 */
void RobotMaster::UpdateExp3(size_t test)
{
	static int i = 1;

	void *p = Heap::Alloc(sizeof(RobotTask));

	// make the actual task with priority equal to task 3
	RobotTask *taskHandler = new(p) RobotTask(5+i,
											Robotstone::baselinePriorityExpUni+2,
											&this->startTasksSig,
											this->stopCondTask,
											this->initialTime,
											"");
	taskHandler->SetKWIPP(uniBaselineWorkload[2]);
	taskHandler->SetFrequency(uniBaselineFrequency[2]);
	this->rstoneTasks.InsertTail(taskHandler);
	++i;

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->ClearMeasurements();
		rstoneTasks.GetValue()->Start(RealTime::GetExecCPUS());
	}
}

/**
 * @brief Update the actual Experiment 4 step test.
 *
 * @param test - test step number.
 *
 */
void RobotMaster::UpdateExp4(size_t test)
{
	int i = 0;
	uint16_t resMsg;

	this->HandshakeReceive(&resMsg);
	this->HandshakeSend(&resMsg);

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
void RobotMaster::UpdateExp5(size_t test)
{
	size_t topic_i = 2;
	uint16_t resMsg;

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->LeavePubTopic();
		rstoneTasks.GetValue()->ClearMeasurements();
	}

	this->HandshakeReceive(&resMsg);
	this->HandshakeSend(&resMsg);

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->PublishingRequest(topic_i, rstoneEXP5_UPDATE_VALUE(test));
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
void RobotMaster::UpdateExp6(size_t test)
{
	uint16_t resMsg;
	int i = 0;

	this->HandshakeReceive(&resMsg);
	this->HandshakeSend(&resMsg);

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
void RobotMaster::UpdateExp7(size_t test)
{
	uint16_t resMsg;

	this->HandshakeReceive(&resMsg);
	if(resMsg == 1)
	{
		this->experimentReachLimit = 1;
		Stdout::Print("\n\nExperiment reach limit: Impossible create more receivers tasks!!!\n\n");
		this->HandshakeSend(&resMsg);
		Stdout::Print("Experiment finished!!!\n");
		Stdout::Print("Experiment deadlines: %d.\n!!!\n", this->ExperimentDeadlineMiss());
		return;
	}

	this->HandshakeSend(&resMsg);

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		rstoneTasks.GetValue()->ClearMeasurements();
		rstoneTasks.GetValue()->Start(RealTime::GetExecCPUS());
	}
}

/**
 * @brief Calculates the deadlines missed number and saves in
 * 		  internal RobotMaster attributes.
 */
void RobotMaster::CalculateDeadlineMiss(void)
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
void RobotMaster::StartReport(size_t test)
{
	this->CalculateDeadlineMiss();

	this->WaitDeadlinesFromSlave();

	this->PrintReport(test);
}

/**
 * @brief Wait from Slave a Signal with the number of deadlines missed.
 */
void RobotMaster::WaitDeadlinesFromSlave(void)
{
	uint16_t res;

	HandshakeReceive(&res);

	HandshakeReceive(&res);

	this->actualSlaveDeadlines =  res;
}

/**
 * @brief Prints the report of the test step results
 * 		  related to PD experiments.
 *
 * @param test - test step number.
 *
 */
void RobotMaster::PrintExpUniReport(size_t test)
{
	float totalKWIPS = 0, totalCPU = 0;

	led_TurnOn(9);

	Stdout::Print("========================================================================\n\n");
	Stdout::Print("Underlay Software Architecture:\n" robotRT_NAME "\n");
	Stdout::Print("Raw speed in Kilo-Whetstone Instructions Per Second (KWIPS): %lld\n\n", this->GetRawSpeed());
	Stdout::Print("Executable Control: ");

	Stdout::Print("Master\n\n");

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

	Stdout::Print("Initial condition: - all tasks workload set by \n\t\t\t{%d, %d, %d, %d, %d} KWIPP.\n", uniBaselineWorkload[0],
																								  uniBaselineWorkload[1],
																								  uniBaselineWorkload[2],
																								  uniBaselineWorkload[3],
																								  uniBaselineWorkload[4]);
	Stdout::Print("                   - all tasks frequency set by \n\t\t\t{"tCFormat", "tCFormat", "tCFormat", "tCFormat", "tCFormat"} Hertz.\n",
																																	uniBaselineFrequency[0],
																																	uniBaselineFrequency[1],
																																	uniBaselineFrequency[2],
																																	uniBaselineFrequency[3],
																																	uniBaselineFrequency[4]);
	Stdout::Print("                   - No message transferring.\n");

	switch(this->actualExp)
	{
	case Robotstone::exp1:
		Stdout::Print("Experiment step: Increase the KWIPP of all tasks by\n");
		Stdout::Print("\t\t\t1.1, 1.2, 1.3, ... of its baseline frequency.\n");
		break;
	case Robotstone::exp2:
		Stdout::Print("Experiment step: Increase the frequency of all tasks by\n");
		Stdout::Print("\t\t\t1.1, 1.2, 1.3, ... of its baseline frequency.\n");
		break;
	case Robotstone::exp3:
		Stdout::Print("Experiment step: Increase one task with");
		Stdout::Print("\t\t\tthe same parameters of task 3.\n");
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

	Stdout::Print("\n\n========================================================================\n\n\n\n");

#ifdef	hartEXTRA_VISUALIZATION
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
 * @brief Prints the report of the test step results
 * 		  related to PCD experiments.
 *
 * @param test - test step number.
 *
 */
void RobotMaster::PrintExpDistReport(size_t test)
{
	float totalKWIPS = 0, totalCPU = 0;

	Stdout::Print("========================================================================\n\n");
	Stdout::Print("Underlay Software Architecture:\n" robotRT_NAME "\n");
	Stdout::Print("Raw speed in Kilo-Whetstone Instructions Per Second (KWIPS): %lld\n\n", this->GetRawSpeed());
	Stdout::Print("Executable Control: ");

	Stdout::Print("Master\n\n");

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

	Stdout::Print("Task\tMessage Length (B)\tTopic\n");

	for(rstoneTasks.InitIteration(); !rstoneTasks.IsEnd(); ++rstoneTasks)
	{
		Stdout::Print("%d\t%d\t\t\t%d\n", rstoneTasks.GetValue()->GetID(),
														 rstoneTasks.GetValue()->GetPubMessageSize(),
														 rstoneTasks.GetValue()->GetPubTopic());
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

	Stdout::Print("\nTotal of Deadlines Misses by Receivers: %d", this->actualSlaveDeadlines);
	Stdout::Print("\n\n========================================================================\n\n");

#ifdef	hartEXTRA_VISUALIZATION
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
 * @brief Prints the report of the test step results.
 *
 * @param test - test step number.
 *
 */
void RobotMaster::PrintReport(size_t test){

	if(this->actualExp > Robotstone::exp3)
	{
		this->PrintExpDistReport(test);
	}
	else
	{
		this->PrintExpUniReport(test);
	}
}


/**
 * @brief The entry function of the management task in Master node.
 *
 * @param args - Arguments pointer passed to the management task.
 * 				 This will be the pointer of the RobotMaster object.
 *
 */
taskFUNCTION(ManageMasterTask, args)
{
	RobotMaster* bench = (RobotMaster*) args;

	bench->ManTask();
}

/***************************************************************************************
 * END: Module - hartstone.cpp
 ***************************************************************************************/
