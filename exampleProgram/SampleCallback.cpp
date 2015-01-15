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
 * SampleCallback.cpp
 *
 *  Created on: Nov 19, 2014
 */

#include "SampleCallback.h"

SampleCallback::SampleCallback() {
	testFile.open("testFile.raw", std::ios::app | std::ios::out | std::ios::binary);
}

SampleCallback::~SampleCallback() {
	testFile.close();
}


void SampleCallback::dataDelivery(std::valarray< std::complex<float> > &samples) {
	gettimeofday(&tp, NULL);
	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000; //get current timestamp in milliseconds
	std::cout << ms;
	std::cout << ": CallbackInterface Received: " << samples.size() << " data points" << std::endl;

	testFile.write((char*)&samples[0], samples.size() * sizeof(samples[0]));
}

