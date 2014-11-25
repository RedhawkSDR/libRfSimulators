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
#include "../inc/exp_agc.h"
#include <iostream>
#include <complex>

template<typename T>
T validateAlpha(T alpha) {
	if (alpha <= 1e-6) {
		std::cout << "AGC - error - alpha too small " << alpha << std::endl;
		alpha = 1e-6;
	} else if (alpha > .999999) {
		std::cout << "AGC - error - alpha too large " << alpha << std::endl;
		alpha = .999999;
	}

	return alpha;
}

//put this one first so it is chosen when it can be
template<typename T>
T abs2(std::complex<T> val)
{
	T re = val.real();
	T im = val.imag();

	return (re*re)+(im*im);
}

//here is the default one
template<typename T>
T abs2(T val)
{
	return val*val;
}

template<typename T, typename U>
ExpAgc<T,U>::ExpAgc(std::valarray<U>& input, std::valarray<U>& output, T avgPower, T minPower, T maxPower, T eps, T alpha):
 _input(input),
 _output(output),
 _currentPower(0.0),
 _avgPower(avgPower),
 _minPower(minPower),
 _maxPower(maxPower),
 _eps(eps),
 _init(false)
{
	setAlpha(alpha);
	_output.resize(input.size());
}

template<typename T, typename U>
ExpAgc<T,U>::~ExpAgc() {
}

template<typename T, typename U>
void ExpAgc<T,U>::process()
{
	if (_input.size()>0)
	{

		_output.resize(_input.size());
		if (!_init)
		{
			initialize();
		}
		else
		{
			U* Inptr = &_input[0];
			U* Outptr = &_output[0];
			for (unsigned int i=0; i!=_input.size(); i++)
			{
				_currentPower = _alpha*_currentPower+_omega*abs2(*Inptr);
				if (_minPower < _currentPower && _currentPower < _maxPower)
				{
					//our gain is already OK - no AGC applied
					*Outptr = *Inptr;
				}
				else
				{
					//apply agc if we are too big or too small
					float gain = sqrt(_avgPower/(_currentPower+_eps));
					*Outptr = *Inptr*gain;
				}
				++Inptr;
				++Outptr;
			}
		}
	}
}

template<typename T, typename U>
void ExpAgc<T,U>::initialize()
{
	U* Inptr = &_input[0];
	_currentPower=0;
	for (unsigned int i =0; i< _input.size(); i++)
	{
		_currentPower+=abs2(*Inptr);
		++Inptr;
	}
	//normalize power by number of elements
	_currentPower = _currentPower/_input.size();
	if (_currentPower > _maxPower || _currentPower < _minPower)
	{
		float gain = sqrt(_avgPower/(_currentPower+_eps));
		U* Outptr = &_output[0];
		Inptr = &_input[0];
		for (unsigned int i =0; i< _input.size(); i++)
		{
			*Outptr = (*Inptr)*gain;
			++Inptr;
			++Outptr;
		}
	}
	_init = true;
}

template<typename T, typename U>
T ExpAgc<T,U>::setAlpha(T alpha)
{
	alpha = validateAlpha(alpha);
	_alpha = alpha;
	_omega = 1-_alpha;
	return _alpha;
}

template<typename T, typename U>
T ExpAgc<T,U>::getAlpha()
{
	return _alpha;
}
template<typename T, typename U>
void ExpAgc<T,U>::setMaxPower(T maxPower)
{
	_maxPower = maxPower;
}
template<typename T, typename U>
T ExpAgc<T,U>::getMaxPower()
{
	return _maxPower;
}
template<typename T, typename U>
void ExpAgc<T,U>::setMinPower(T minPower)
{
	_minPower = minPower;
}
template<typename T, typename U>
T ExpAgc<T,U>::getMinPower()
{
	return _minPower;
}
template<typename T, typename U>
T ExpAgc<T,U>::getAvgPower()
{
	return _avgPower;
}
template<typename T, typename U>
void ExpAgc<T,U>::setAvgPower(T avgPower)
{
	_avgPower =avgPower;
}
template<typename T, typename U>
T ExpAgc<T,U>::getCurrentPower()
{
	return _currentPower;
}

// Instantiate myclass for the supported template type parameters
template class ExpAgc<float,float>;
template class ExpAgc<float, std::complex<float> >;
template float validateAlpha<float>(float);
