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

#include "FirFilterDesigner.h"
#include <iostream>

template<typename T, typename U>
void muxImaginaryZeros(T& fromVec,U& toVec)
{
	toVec.resize(fromVec.size());
	for (size_t i=0;  i!=toVec.size(); i++)
		toVec[i] = Complex(fromVec[i],0);
}

template<typename T>
size_t FirFilterDesigner::wdfir (
		T& filtCoeff,
		FIRFilter::filter_type type,
    Real  ripple,
    Real twNorm,
    Real  flNorm,
    Real  fhNorm,
    size_t minTaps,
    size_t maxTaps)
{
	if (type == FIRFilter::hilbert)
	{
		std::cerr<<"Hilbert filters are not supported"<<std::endl;
		filtCoeff.resize(0);
		return 0;
	}
	if (twNorm <0)
	{
		std::cout<<"FirFilterDesigner warning: twNorm < 0"<<std::endl;
		twNorm = -1*twNorm;
	}
	if (flNorm <0)
	{
		std::cout<<"FirFilterDesigner warning: flNorm < 0"<<std::endl;
		flNorm = -1*flNorm;
	}
	if (fhNorm <0)
	{
		std::cout<<"FirFilterDesigner warning: fhNorm < 0"<<std::endl;
		fhNorm = -1*fhNorm;
	}
	std::cout<<"desiging a real filter with properties type "<<type<<" ripple "<< ripple<< " twNorm = "<<twNorm<<" flNorm = "<<flNorm<<" fhNorm = "<<fhNorm<<std::endl;
    size_t ieo, nh;
    Real c, c1, c3, xn;

    size_t sz = kaiser.reset(ripple, twNorm);
    if ((maxTaps) != 0 and sz > maxTaps)
    {
        std::cout<<"your taps are too big; changing from "<<sz <<" to "<<maxTaps<<std::endl;
    	sz=maxTaps;
    	kaiser.overrideSize(sz);
    }
    else if (sz<minTaps)
    {
        std::cout<<"your taps are too small; changing from "<<sz <<" to "<<minTaps<<std::endl;
    	sz=minTaps;
    	kaiser.overrideSize(sz);
    }
    ieo = sz % 2;
    if (!(ieo == 1 || type == FIRFilter::lowpass || type == FIRFilter::bandpass))
    {
        //
        // Some filter types must have an odd number of taps
        //
    	std::cout<<"adjusting filter taps by one extra tap"<<std::endl;
    	if (sz<maxTaps)
    		sz++;
    	else
    		sz--;
    	ieo = 1;
        kaiser.overrideSize(sz);
    }
    filtCoeff.resize(sz);
	c1 = flNorm;
	if (type == FIRFilter::bandpass || type == FIRFilter::bandstop)
	{
		//swap frequency components if user specified them backwards
		if (fhNorm< flNorm)
		{
			std::cout<<"you specified your frequency components backwards - swapping them!"<<std::endl;
			Real tmp=fhNorm;
			fhNorm=flNorm;
			flNorm=tmp;
		}
		c1 = fhNorm - flNorm;
	}
	nh = (1 + sz) / 2;
	if (ieo == 1)
		filtCoeff[nh - 1] = 2.0 * c1;
	for (size_t ii = ieo; ii < nh; ii++)
	{
		xn = ii;
		if (ieo == 0)
			xn += 0.5;

		c = M_PI * xn;
		c3 = c * c1;
		if (type == FIRFilter::lowpass || type == FIRFilter::highpass)
			c3 *= 2.0;

		size_t jj = nh + ii - ieo;
		filtCoeff[jj] = sin (c3) / c;

		if (type == FIRFilter::bandpass || type == FIRFilter::bandstop)
			filtCoeff[jj] *= 2.0 * cos (c * (flNorm + fhNorm));

		filtCoeff[nh - ii - 1] = filtCoeff[jj];
	}

    RealArray window;
    kaiser.getWindow(window);
	for (size_t i = 0; i!= filtCoeff.size(); i++)
		filtCoeff[i] *= window[i];

    if (type == FIRFilter::lowpass || type == FIRFilter::bandpass)
    {
        return sz;
    }
    else if (type == FIRFilter::hilbert)
    {
        Real kk = 1;
        Real sum = Real(0);
        for (size_t ii = nh; ii < sz; ii += 2)
        {
            sum += kk * filtCoeff[ii];
            kk = -kk;
        }
    	for (size_t i = 0; i!= filtCoeff.size(); i++)
    		filtCoeff[i] *= (-0.5 / sum);

        return sz;
    }

	for (size_t i = 0; i!= filtCoeff.size(); i++)
		filtCoeff[i] *= -1;
    filtCoeff[nh - 1] += 1;
    return sz;
}  // end wdfir

template<typename T>
size_t FirFilterDesigner::wdfirHz (
		T& filtCoeff,
		FIRFilter::filter_type type,
    Real  ripple,
    Real twHz,
    Real  flHz,
    Real  fhHz,
    Real fsHz,
    size_t minTaps,
    size_t maxTaps)
{
	return wdfir(filtCoeff, type, ripple, twHz/fsHz, flHz/fsHz, fhHz/fsHz, minTaps, maxTaps);
}

template<>
size_t FirFilterDesigner::wdfir (
		ComplexArray& filtCoeff,
		FIRFilter::filter_type type,
    Real  ripple,
    Real twNorm,
    Real  flNorm,
    Real  fhNorm,
    size_t minTaps,
    size_t maxTaps)
{
	return wdfirCx(filtCoeff, type, ripple, twNorm, flNorm, fhNorm,minTaps,maxTaps);
}
template<>
size_t FirFilterDesigner::wdfir (
		ComplexVector& filtCoeff,
		FIRFilter::filter_type type,
    Real  ripple,
    Real twNorm,
    Real  flNorm,
    Real  fhNorm,
    size_t minTaps,
    size_t maxTaps)
{
	return wdfirCx(filtCoeff, type, ripple, twNorm, flNorm, fhNorm,minTaps,maxTaps);
}

template<typename T>
size_t FirFilterDesigner::wdfirCx (
		T& filtCoeff,
		FIRFilter::filter_type type,
    Real  ripple,
    Real twNorm,
    Real  flNorm,
    Real  fhNorm,
    size_t minTaps,
    size_t maxTaps)
{
	if ((type==FIRFilter::lowpass)||(type==FIRFilter::highpass))
	{
		//design a real filter and mux the imaginary zeros in there to make it complex
		RealArray r;
		size_t ret =  wdfir(r, type, ripple, twNorm, flNorm, 0, minTaps,maxTaps);
		muxImaginaryZeros(r, filtCoeff);
		return ret;
	}
	else
	{
		//create a lowpass or highpass filter and tune the filter with a complex tune to make the bandpass filter
		FIRFilter::filter_type inputType=FIRFilter::lowpass;
		if (type==FIRFilter::bandstop)
			inputType=FIRFilter::highpass;
		RealArray r;
		size_t ret =  wdfir(r, inputType, ripple, twNorm, fabs(fhNorm-flNorm)/2.0, 0, minTaps,maxTaps);

		filtCoeff.resize(ret);
		//tune the filter outputs to the specified center frequency
		//the tune phase is 2*pi*fc_norm = 2*pi*((fhNorm+flNorm)/2) = pi*(fhNorm+flNorm)
		Real tunePhaseUpdate=M_PI*(fhNorm+flNorm);
		Complex phasor(cos(tunePhaseUpdate),sin(tunePhaseUpdate));
		Complex tuner(phasor);
		for (size_t i=0; i!=ret; i++)
		{
			filtCoeff[i]=r[i]*tuner;
			tuner*=phasor;
		}
		return ret;
	}
}

//size_t FirFilterDesigner::wdfirHz (
//		ComplexArray& filtCoeff,
//		FIRFilter::filter_type type,
//    Real  ripple,
//    Real twHz,
//    Real  flHz,
//    Real  fhHz,
//    Real fsHz,
//    size_t minTaps,
//    size_t maxTaps)
//{
//	return wdfir(filtCoeff, type, ripple, twHz/fsHz, flHz/fsHz, fhHz/fsHz,minTaps,maxTaps);
//}

KaiserWindowDesigner::KaiserWindowDesigner()
{
	reset(.1, .1);
}

KaiserWindowDesigner::KaiserWindowDesigner(double ripple, double twHz, double fsHz)
{
	reset(ripple, twHz, fsHz);
}

KaiserWindowDesigner::KaiserWindowDesigner(double ripple, double twNorm)
{
	reset(ripple, twNorm);
}

size_t KaiserWindowDesigner::reset(double ripple, double twHz, double fsHz)
{
	return reset(ripple, twHz/fsHz);
}

size_t KaiserWindowDesigner::reset(double ripple, double twNorm)
{
	attenDB = -20.0*log10(ripple);
	double twRad = 2*M_PI*twNorm;
	int M; // filter order

	if(attenDB >= 20.96)
		M = ceil((attenDB-7.95)/(2.285*twRad));
	else // if(atten < 20.96)
		M = ceil(5.79/twRad);

	size = M+1; // M+1 = number of taps
	return size;
}

size_t KaiserWindowDesigner::getWindowSize()
{
	return size;
}

void KaiserWindowDesigner::overrideSize(size_t val)
{
	size=val;
}

void KaiserWindowDesigner::getWindow(RealArray &w)
{
	w.resize(size);
	Real beta = getBeta();
	calculateWindow(w, beta);
}

Real KaiserWindowDesigner::getBeta()
{
	double beta;
	if (attenDB > 50.0)
	{
		beta = 0.1102 * (attenDB - 8.7);
	}
	else if (attenDB >= 20.96 && attenDB <= 50.0)
	{
		Real tmp = attenDB - 20.96;
		beta = 0.58417 * pow (tmp, Real(0.4)) + 0.07886 * tmp;
	}
	else // if (atten < 20.96)
	{
		beta = Real(0);
	}
	return beta;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   This function implements a Kaiser (Kaiser-Bessel) window.
//
// Parameters:
//   w  - Window function (pointer to array of window coefficients)
//   beta - Control parameter
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void KaiserWindowDesigner::calculateWindow (RealArray &w, Real beta)
{
    size_t n = (1 + w.size()) / 2;
    size_t ieo = w.size() % 2;
    Real bes = in0 (beta);
    Real xind = pow ((w.size() - 1.0), 2);

    Real arg;

    for (size_t ii = 0; ii < n; ii++)
    {
        Real xi = ii;
        if (ieo == 0)
            xi += 0.5;
        size_t jj = n + ii - ieo;
        // there is a case do to floating point error when you end up with a very small negative number
        // therefore - ensure that the number is always non-negative so we can take the square root of it
        arg=std::max(1.0 - 4.0 * xi * xi / xind,0.0);
        w[n - 1 - ii] =
            w[jj] = in0 (beta * sqrt(arg)) / bes;
    }

}  // end calculateWindow

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   This function implements the Taylor series expansion of a 0th order Bessel
//   function (I0).
//
// Parameters:
//   x - ...
//
// Return Value:
//   e - I0(x)
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
Real KaiserWindowDesigner::in0 (Real x)
{
    const Real t = 1.0e-7;
    Real e = 1, de = 1;
    Real y = 0.5 * x;
    Real xi, sde;

    for (size_t ii = 1; ii < 25; ++ii)
    {
        xi = ii;
        de *= y / xi;
        sde = de * de;
        e += sde;
        if (e * t - sde > Real(0))
            return e;
    }

    return e;

}  // end in0


//need to put these bad boys in here for templates or you get undefined references when linking ...grr...
template size_t FirFilterDesigner::wdfirHz (
		ComplexVector& filtCoeff, FIRFilter::filter_type type, Real  ripple,
    Real twHz, Real  flHz, Real  fhHz, Real fsHz, size_t minTaps, size_t maxTaps);
template size_t FirFilterDesigner::wdfirHz (
		ComplexArray& filtCoeff, FIRFilter::filter_type type, Real  ripple,
    Real twHz, Real  flHz, Real  fhHz, Real fsHz, size_t minTaps, size_t maxTaps);
template size_t FirFilterDesigner::wdfirHz (
		RealVector& filtCoeff, FIRFilter::filter_type type, Real  ripple,
    Real twHz, Real  flHz, Real  fhHz, Real fsHz, size_t minTaps, size_t maxTaps);
template size_t FirFilterDesigner::wdfirHz (
		RealArray& filtCoeff, FIRFilter::filter_type type, Real  ripple,
    Real twHz, Real  flHz, Real  fhHz, Real fsHz, size_t minTaps, size_t maxTaps);
