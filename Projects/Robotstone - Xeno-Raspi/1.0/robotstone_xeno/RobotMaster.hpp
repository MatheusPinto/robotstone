/**
 * @file	RobotMaster.hpp
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

#ifndef HARTROS_MASTER_HPP
#define HARTROS_MASTER_HPP

#include "Robotstone.hpp"
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

/*CLASSES*/
/*=======================================================================================*/

class RobotMaster : public Robotstone
{

private:

	size_t actualSlaveDeadlines;
	uint16_t receiverDeadlineMiss;
	bool manTaskSuspendStatus;
	Signal expSignal;
	Task manTask; /**< The first task that manages the tests */
	size_t uniBaselineWorkload[5];
	size_t distBaselineWorkload[3];
public:

	RobotMaster(void);

	void StartTasks(void);

	void FinishTasks(void);

	void RequestExperiment(uint8_t experiment);

	void InitExperiment(void);

	void StartDistributedExperiment(void);

	void StartUniExperiment(void);

	size_t ExperimentDeadlineMiss(void);

	void UpdateExperiment(size_t test);

	void UpdateExp1(size_t test);

	void UpdateExp2(size_t test);

	void UpdateExp3(size_t test);

	void UpdateExp4(size_t test);

	void UpdateExp5(size_t test);

	void UpdateExp6(size_t test);

	void UpdateExp7(size_t test);

	void CalculateDeadlineMiss(void);

	void StartReport(size_t test);

	void WaitTasksFinish(void);

	void SendExperimentStatus(size_t status);

	void SetExperimentUniBaseline();

	void SetExperimentDistBaseline();

	void ManTask(void);

	void WaitDeadlinesFromSlave(void);

	void PrintReport(size_t test);

	void PrintExpUniReport(size_t test);

	void PrintExpDistReport(size_t test);

	uint32_t TotalDeadlineMiss(void);

	friend void ManageMasterTask(void *args);
}; 


/*ABSTRACT TYPES*/
/*=======================================================================================*/


#endif // HARTROS_MASTER_HPP
/***************************************************************************************
 * END: Module - hartros_master.hpp
 ***************************************************************************************/
