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
#ifndef BASEBAND_DEMOD_H_
#define BASEBAND_DEMOD_H_

#include "DataTypes.h"

class AmFmPmBasebandDemod
{
public:
    AmFmPmBasebandDemod(ComplexArray &input, RealArray *am, RealArray* pm, RealArray* fm, double freqMag, double phaseMag,float initalPhase = 0);
    virtual ~AmFmPmBasebandDemod(void);
    bool process(void);
    Real getPhase()
    {
        return phaseA;
    }
private:
    ComplexArray &_input;
    RealArray *_am;
    RealArray *_fm;
    RealArray *_pm;
    static const Real oneOver2Pi=1.0/(2*M_PI);
    Real freqGain;
    Real phaseGain;
    Real phaseA;
    Real phaseB;
    Real* oldPhase;

};

#endif /* BASEBAND_DEMOD_H_ */
