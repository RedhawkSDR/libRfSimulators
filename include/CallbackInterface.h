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
 * CallbackInterface.h
 *
 *  Created on: Nov 18, 2014
 */

#ifndef LIBFMRDSSIMULATOR_INCLUDE_CALLBACKINTERFACE_H_
#define LIBFMRDSSIMULATOR_INCLUDE_CALLBACKINTERFACE_H_

#include <valarray>
#include <complex>

#define FILE_INPUT_BLOCK_SIZE 100000
#define OUTPUT_SAMPLES_BLOCK_SIZE FILE_INPUT_BLOCK_SIZE*10


namespace RfSimulators {

class CallbackInterface
{
public:
    virtual void dataDelivery(std::valarray< std::complex<float> > &samples) = 0;
};

};



#endif /* LIBFMRDSSIMULATOR_INCLUDE_CALLBACKINTERFACE_H_ */
