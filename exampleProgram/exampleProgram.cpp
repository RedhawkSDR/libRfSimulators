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

#include "libdigitizersim.h"
#include "boost/filesystem.hpp"
#include "SampleCallback.h"

using namespace boost::filesystem;

int main(void) {

	SampleCallback callback;

	path p("/tmp/testDir");

	DigitizerSimulator digSim;

	digSim.print_hello();

	digSim.init(p, &callback, 10);

	digSim.start();

	sleep(5);

	digSim.stop();

  return 0;
}
