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
 * SampleCallback.h
 *
 *  Created on: Nov 19, 2014
 */

#ifndef LIBFMRDSSIMULATOR_EXAMPLEPROGRAM_SAMPLECALLBACK_H_
#define LIBFMRDSSIMULATOR_EXAMPLEPROGRAM_SAMPLECALLBACK_H_

#include "CallbackInterface.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <sys/time.h>
#include <complex>

using namespace RfSimulators;

class SampleCallback: public CallbackInterface {
public:
	SampleCallback();
	virtual ~SampleCallback();
	void dataDelivery(std::valarray< std::complex<float> > &samples);

private:
	std::ofstream testFile;
	struct timeval tp;
};

#endif /* LIBFMRDSSIMULATOR_EXAMPLEPROGRAM_SAMPLECALLBACK_H_ */
