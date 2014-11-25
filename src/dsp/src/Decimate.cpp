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
#include "Decimate.h"

#ifdef DECIMATE_DEBUG
#include <fstream>
#include <iostream>
#endif

using namespace std;

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   Decimates's constructor.
//
// Parameters:
//   input - reference to an array of samples
//   output - reference to an output array of samples
//   d - decimation factor
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

Decimate::Decimate(ComplexVector& input, ComplexVector& output, size_t d) :
    _input(input),
    _output(output),
	_inputOffset(0),
	_d(d)
{
	_output.reserve(_input.size());
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   Decimate's destructor.
//
// Parameters:
//   None.
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

Decimate::~Decimate()
{
    //
    // Hook
    //
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   Decimate's main processing method.  It decimates the input buffer by the
//   decimation factor _d.
//
// Parameters:
//   input - the input samples
//   output - every _d input samples
//
// Return Value:
//   boolean - True if the buffer is full, otherwise false.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

bool Decimate::run(void)
{
	bool full = false;

	size_t i;
    for(i=(0+_inputOffset); i < _input.size(); i+= _d)
    	_output.push_back(_input[i]);

    // Calculate _inputOffset for next call
    _inputOffset = i - _input.size();

    if(_output.size() >= _input.size())
    	full = true;

    return full;
}
