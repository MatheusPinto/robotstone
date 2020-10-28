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
 * This module contains the implementation of the RobotSlave class,
 * which will be used by the benchmark Slave node.
 */

#ifndef ROBOTSTONE_SLAVE_HPP
#define ROBOTSTONE_SLAVE_HPP

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

class RobotSlave : public Robotstone
{
private:
	Task manTask; /**< The first task that manages the tests */
public:

	RobotSlave(void);

	void Start(void);

	void StartTasks(void);

	void StartExperiment(void);

	void InitExperiment(void);

	void ManTask(void);

	size_t ExperimentDeadlineMiss(void);

	void UpdateExperiment(size_t test);

	void UpdateExp4(size_t test);

	void UpdateExp5(size_t test);

	void UpdateExp6(size_t test);

	void UpdateExp7(size_t test);

	void CalculateDeadlineMiss(void);

	void StartReport(size_t test);

	void WaitTasksFinish(void);

	bool IsFinishedByMaster(void);

	void SetExperimentBaseline();

	void SignalDeadlineToMaster();

	void InitTest(uint8_t exp, uint8_t test);

	void PrintReport(size_t test);

	uint32_t TotalDeadlineMiss(void);

	friend void ManageSlaveTask(void *args);
}; 


/*ABSTRACT TYPES*/
/*=======================================================================================*/


#endif // ROBOTSTONE_SLAVE_HPP
/***************************************************************************************
 * END: Module - RobotSlave.hpp
 ***************************************************************************************/
