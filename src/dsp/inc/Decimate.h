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
#ifndef __DECIMATE_H__
#define __DECIMATE_H__

#define DECIMATE_DEBUG // Comment out to disable

#include "DataTypes.h"

class Decimate
{
public:
    Decimate(ComplexVector& input, ComplexVector& output, size_t d);
    virtual ~Decimate();

    bool run(void);

private:
    ComplexVector& _input;    // Reference to input buffer
    ComplexVector& _output;   // Reference to output buffer

    size_t _inputOffset;
    size_t _d;                // Decimation factor
};

#endif  // __DECIMATE_H__
