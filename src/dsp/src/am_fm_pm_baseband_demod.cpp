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
#include"am_fm_pm_baseband_demod.h"

#include <cmath>
#include <numeric>
#include <algorithm>

using namespace std;

AmFmPmBasebandDemod::AmFmPmBasebandDemod(ComplexArray &input,
							 RealArray *am,
							 RealArray* pm,
							 RealArray* fm,
							 double freqMag,
							 double phaseMag,
							 float initalPhase) :
		_input(input),
		_am(am),
		_fm(fm),
		_pm(pm),
		freqGain(freqMag),
		phaseGain(phaseMag),
		phaseA(initalPhase),
		phaseB(0.0),
		oldPhase(&phaseA)
{
}
AmFmPmBasebandDemod::~AmFmPmBasebandDemod()
{
}

bool AmFmPmBasebandDemod::process()
{
	Real *phase=NULL;
	Real *freq=NULL;
	Real *tmp;
	Real *mag = NULL;
	//set-up for this loop
	if (_pm || _fm)
	{
		if (_pm)
		{
			//set up the phase buffer & Pointer to start of buffer
			_pm->resize(_input.size());
			phase = &(*_pm)[0];
		}
		else if (_fm)
		{
			//write our phase to the phaseB value since we have no phaseBuffer to store the values
			phase = &phaseB;
		}
		if (_fm)
		{
			//set up the fm buffer & Pointer
			_fm->resize(_input.size());
			freq = &(*_fm)[0];
		}
	}
	if (_am)
	{
		//set up the am buffer & Pointer
		_am->resize(_input.size());
		mag = &(*_am)[0];
	}
	if (_am || _fm || _pm)
	{
		//do the processing on the input
		for(Complex *x=&_input[0], *xend=&_input[_input.size()]; x!=xend; x++)
		{
			if (freq || _pm)
			{
				//we need to calculate the phase
				*phase=oneOver2Pi*arg(*x); //-.5<phase<.5
				if(freq)
				{
					//do the frequency calculation as a difference of the phase values
					*freq = *phase- *oldPhase; //-1<freq<1 because we are subtracting 2 numbers in range [-.5,.5]
					// wrap freq between -.5 and .5 and multiply by the freqGain
					if(*freq > .5)
						*freq =(*freq-1)*freqGain;
					else if(*freq < -.5)
						*freq = (*freq+1)*freqGain;
					else
						*freq = *freq*freqGain;
					freq++;
				}
				if (_pm)
				{
					//scale the oldPhase by its gain
					//we are choosing to do this here as we are done with him
					//for any fm calculations at this point
					(*oldPhase) *= phaseGain;
					//advance both old and new pointers
					oldPhase = phase;
					phase++;
				}
				else
				{
					//swap oldPhase and phase pointers
					tmp = phase;
					phase = oldPhase;
					oldPhase = tmp;
				}
			}
			if (mag)
			{
				//do the am calculation
				*mag = abs(*x);
				mag++;
			}
		}
		//outside of for loop - update things for next loop
		if (_fm || _pm)
		{
			//copy oldPhase value to phaseA
			phaseA = *oldPhase;
			if (_pm)
				//scale oldPhase as we have not yet scaled the last element in the array
				*(oldPhase) *= phaseGain;
			//update the pointer so that oldPhase points at phaseA for next loop
			oldPhase = &phaseA;
		}
	}
	return true;
}
