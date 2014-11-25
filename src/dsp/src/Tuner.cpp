/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this 
 * source distribution.
 * 
 * This file is part of REDHAWK Basic Components dsp library.
 * 
 * REDHAWK Basic Components dsp library is free software: you can redistribute it and/or modify it under the terms of 
 * the GNU Lesser General Public License as published by the Free Software Foundation, either 
 * version 3 of the License, or (at your option) any later version.
 * 
 * REDHAWK Basic Components dsp library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with this 
 * program.  If not, see http://www.gnu.org/licenses/.
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   This file contains the Tuner class implementation.
//
//   TO DO:
//      1. *** WORK IN PROGRESS ***
//      2. Filter bandwidth needs to be added
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include <cmath>
#include <numeric>
#include "Tuner.h"
#include <iostream>

#ifdef TUNER_DEBUG
#include <fstream>
#include <iostream>
#endif

using namespace std;


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   A phasor class *** WORK IN PROGRESS ***
//
// Arguments:
//  None.
//
// Return Value:
//  None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/




//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   Tuner's constructor.  Use parameters to construct lowpass filter.
//
// Parameters:
//   input - reference to an array of samples
//   output - reference to output array of samples
//   normFc - normalized (ie, Fc/Fs) beat frequency
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

Tuner::Tuner(ComplexVector &input, ComplexVector &output, const Real normFc) :
    _input(input),
    _output(output)
{
    _output.resize(_input.size());
    reset();
    retune(normFc);
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   Tuner's destructor.
//
// Parameters:
//   None.
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

Tuner::~Tuner()
{
    //
    // Hook
    //
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   This method resets the phasor used for frequency-shifting the input
//   samples.
//
// Parameters:
//   normFc - normalized (ie, Fc/Fs) tune frequency w.r.t. 0
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void Tuner::retune(Real normFc)
{
	_dphasor     = Complex(cos(2*M_PI*normFc), sin(-2*M_PI*normFc));
    _dcycles = normFc;
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   This method resets the phasor back to its initial value.
//
// Parameters:
//   None.
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void Tuner::reset(void)
{
    _cycles = 0;
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   Tuner's main processing method.  It shifts the input samples by multiplying
//   them by a phasor.
//
// Parameters:
//   input - the input samples
//   output - the input samples shifted in frequency
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

bool Tuner::run(void)
{
    // bsg - I've made some modifications in here to try to compensate for drift and the magnitude
	// growing/shrinking due to floating point round of errors and abs(exp^(j*theta)) not being EXACTLY one
	// this caused a systemic problem which reduced/increased (depending upon the tune value used)
	// the magnitude of the tuner over time so that TFD, when ran for minutes/hours produced a notable gain offset

	// to cope with this - I'm storing all the phase values as double precision floating point values
	// this is now in fractions of a cycle (fs maps to 1)

	// in this loop - we create a complex floating point exponential _ph and use it with
	// compelex floating point differential exponental _dphasor

	// after the loop - we modify the double value "_cycles" to take into account the adjustment to the oscilator
	// thus - within the loop we get the fast floating point math (with small erorrs)
	// but outside the loop we keep acurate representation of the oscilator phase with double precision values to
	// avoid the systemic errors

	//current phase in radians
	double cyclesRad = 2*M_PI*_cycles;
	Complex phasor(cos(cyclesRad), -sin(cyclesRad));
    for (Complex *x= &_input[0],
                 *xend = &_input[_input.size()],
                 *y    = &_output[0];
                 x != xend; ++x, ++y)
    {
        *y = *x * phasor;
#ifdef TUNER_DEBUG
    	phasorVec.push_back(_ph);
#endif
    	//phasor is acumulating floating round off errors - but this is (hopefully) not for too many samples in this loop
    	phasor *= _dphasor;
    };

    // adjust the current phase for the number of samples processed
    _cycles +=(_input.size()*_dcycles);
    //now get rid of the integer part - we only care about the fractional part of the cycles
    //of _cycles
    double tmp;
    _cycles = modf(_cycles,&tmp);


#ifdef TUNER_DEBUG
    std::ofstream tunerFile("tunersinusoid.dat", ios_base::out|ios_base::binary);
	tunerFile.write((char *)&phasorVec[0], phasorVec.size()*sizeof(phasorVec[0]));
	tunerFile.close();
#endif

    return true;
}
