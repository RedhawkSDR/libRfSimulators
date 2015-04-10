/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK librfsimulators.
 *
 * REDHAWK librfsimulators is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK librfsimulators is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
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

#include <unistd.h>
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

	sleep(120);



	delete(digSim);

	sleep(1);

  return 0;
}
