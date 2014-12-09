/*
 ============================================================================
 Name        : exampleProgram.cpp
 Author      : Youssef Bagoulla
 Version     :
 Copyright   : 
 Description : Uses shared library to print greeting
               To run the resulting executable the LD_LIBRARY_PATH must be
               set to ${project_loc}/libdigitizersim/.libs
               Alternatively, libtool creates a wrapper shell script in the
               build directory of this program which can be used to run it.
               Here the script will be called exampleProgram.
 ============================================================================
 */

#include "SampleCallback.h"
#include "RfSimulatorFactory.h"
#include "RfSimulator.h"

using namespace RfSimulators;

int main(void) {

	SampleCallback callback;

	std::string p("/tmp/testDir");

	RfSimulator * digSim = RfSimulatorFactory::createFmRdsSimulator();

	digSim->init(p, &callback, TRACE);

	digSim->start();

	sleep(1);
	digSim->setSampleRate(228050*1);
	sleep(1);
	digSim->setSampleRate(228000*2);
	sleep(1);
	digSim->setSampleRate(228000*3);
	sleep(1);
	digSim->setSampleRate(228000*4);
	sleep(1);
	digSim->setSampleRate(228000*5);
	sleep(1);
	digSim->setSampleRate(228000*6);

	std::cout << "Stopping the simulator" << std::endl;
	digSim->stop();

	sleep(1);

	std::cout << "Restarting the simulator" << std::endl;
	digSim->start();
	sleep(5);

	delete(digSim);

	sleep(1);

  return 0;
}
