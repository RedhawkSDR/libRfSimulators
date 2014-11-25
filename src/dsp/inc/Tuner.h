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

#ifndef __TUNER_H__
#define __TUNER_H__

//#define TUNER_DEBUG // Comment out to disable

#include "DataTypes.h"

/**
 * \brief Tuner class
 * \todo Filter bandwidth needs to be added
 */
class Tuner
{
public:
    Tuner(ComplexVector &input, ComplexVector &output, const Real normFc);
    virtual ~Tuner();

    bool run(void);
    void retune(Real normFc);
    void reset(void);

private:
    ComplexVector    &_input;               // Reference to input buffer
    ComplexVector    &_output;              // Reference to output buffer

    double          _cycles;              // Current phase in cycles (fs maps to 1)
    double 		 	_dcycles;             // Phase increment in cycles
    Complex 		_dphasor;			  // Complex phasor representing Phase increment

#ifdef TUNER_DEBUG
    ComplexVector phasorVec;
#endif
};

#endif  // __TUNER_H__
