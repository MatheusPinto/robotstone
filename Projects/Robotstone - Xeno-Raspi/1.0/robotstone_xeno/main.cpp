// Comment IS_MASTER if you want compile code for Slave node
// Uncomment IS_MASTER if you want compile code for Master node

#define IS_MASTER
#ifdef IS_MASTER

#include "RobotMaster.hpp"
#include <iostream>
#include "realtime.hpp"

using namespace std;

int main(int argc, char* argv[]){

	int expID;

	RealTime::Init();
	RobotMaster bench;

	Stdout::Print("Insert Experiment ID + ENTER: 1 to 7\n");
	Stdout::Print("Or press any other key to get processor raw speed.\n");

		expID = getchar();getchar();
		switch(expID)
		{
		case '1':
			bench.RequestExperiment(Robotstone::exp1);
			break;
		case '2':
			bench.RequestExperiment(Robotstone::exp2);
			break;
		case '3':
			bench.RequestExperiment(Robotstone::exp3);
			break;
		case '4':
			bench.RequestExperiment(Robotstone::exp4);
			break;
		case '5':
			bench.RequestExperiment(Robotstone::exp5);
			break;
		case '6':
			bench.RequestExperiment(Robotstone::exp6);
			break;
		case '7':
			bench.RequestExperiment(Robotstone::exp7);
			break;
		default:
			bench.RequestExperiment(Robotstone::calculateRawSpeed);
		}
} 

#else

#include "RobotSlave.hpp"
#include <iostream>
#include "realtime.hpp"

using namespace std;

int main(int argc, char* argv[]){

	RealTime::Init();

	RobotSlave bench;

	bench.Start();
}

#endif
