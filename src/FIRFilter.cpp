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
//   This file contains the FIRFilter class implementation.  Filtering code was
//   copied and adapted from "C++ Algorithms for Digital Signal Processing",
//   2nd ed., by Embree & Danieli and modified for this application.  It
//   implements a FIR filter with coefficients referenced in the constructor.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include <stdexcept>
#include "FIRFilter.h"

//
// Parameter limits
//
const size_t MAX_TAPS = 99;


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   FIRFilter's constructor.  It implements a FIR filter with coefficients
//   referenced in the constructor.
//
// Parameters:
//   input - reference to input vector
//   output - reference to output vector
//   coef - pointer to filter coefficients
//   length - number of filter coefficients (taps)
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

FIRFilter::FIRFilter(
    const ComplexArray &input, ComplexArray &output,
    Real *coef, size_t length) :
    vIn(input),
    vOut(output),
    _filtCoeff(coef, length),
    _z(Complex(0,0), _filtCoeff.size())
{
    // Validate parameters
    if( coef == NULL )
        throw std::invalid_argument( "Null filter coefficients" );

    vOut.resize(vIn.size());

    reset();
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   FIRFilter's constructor.  It implements a FIR filter with coefficients
//   designed via the windowing method to meet the requirements specified by
//   the parameters.
//
// Parameters:
//   input -
//   type - lowpass, highpass, bandpass, bandstop, hilbert trans
//   atten - stopband attenuation in dB
//   Fl - low freq cutoff, normalized to nyquist frequency= 0.5
//   Fh - high freq cutoff, normalized to nyquist frequency= 0.5 (only used with
//       bandpass and bandstop filters).
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

FIRFilter::FIRFilter(const ComplexArray &input, ComplexArray &output,
    filter_type type, Real atten, Real Fl, Real Fh) :
    vIn(input),
    vOut(output)
{
    //
    // Obtain an appropriate low-pass filter.
    //
#if 1
    _filtCoeff.resize(25);          // 25 taps
    wdfir(type, atten, Fl, Fh);
#else
    Real normalizedCutoffFreq;
    Real num = _deciFactor*_interpFactor;
    Real denom;
    if (_interpFactor > _deciFactor)
    {
        normalizedCutoffFreq = 0.5/_interpFactor;
        denom = _interpFactor-_deciFactor;
    }
    else if (_interpFactor < _deciFactor)
    {
        normalizedCutoffFreq = 0.5/_deciFactor;
        denom = _deciFactor-_interpFactor;
    }
    size_t numFiltTaps = size_t(((atten-7.95) * num /denom));
    if (numFiltTaps > MAX_TAPS) numFiltTaps = MAX_TAPS;

    _filtCoeff.resize(numFiltTaps);
    wdfir(type, atten, Fl, Fh);
#endif

    vOut.resize(vIn.size());
    _z.resize(_filtCoeff.size());
    _z = Complex(0,0);
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   FIRFilter's destructor.
//
// Parameters:
//   None.
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

FIRFilter::~FIRFilter(void)
{
    //
    // Hook
    //
} // Destructor


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   This method implements the filtering function.  It assumes that the input
//   buffer contains all the samples to process, so it modifies the filtering at
//   the endpoints and it does not use the filtering memory buffer.
//
// Parameters:
//   None.
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void FIRFilter::run(void)
{
    // Set up coefficients
    const Real *startCoef = &_filtCoeff[0];
    size_t m_length(_filtCoeff.size());
    size_t lenCoef2 = (m_length + 1) / 2;

    // Set up input data pointers
    const Complex *endIn = &vIn[vIn.size() - 1];
    const Complex *ptrIn = &vIn[lenCoef2 - 1];

    // Initial value of accumulation length for startup
    size_t lenAcc = lenCoef2;

    for(size_t ii = 0; ii < vIn.size(); ++ii)
    {
        // Set up pointers for accumulation
        const Complex *ptrData = ptrIn;
        const Real *ptrCoef = startCoef;

        // Do accumulation and write result
        Complex acc = *ptrCoef++ * *ptrData--;
        for( size_t jj = 1; jj < lenAcc; jj++ )
            acc += *ptrCoef++ * *ptrData--;
        vOut[ii] = acc;

        // Check for end case
        if( ptrIn == endIn )
        {
            // One shorter each time
            lenAcc--;
            // Next coefficient each time
            startCoef++;
        }
        else
        {
            // Check for startup
            if( lenAcc < m_length )
            {
                // Add to input pointer
                lenAcc++;
            }
            ptrIn++;
        }
    }
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   This function implements a real-time filtering function to be used as the
//   samples are available.  It inputs the samples to the filter
//
// Parameters:
//   input - the new sample available to the filter
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

Complex FIRFilter::run(const Complex &input)
{
    // Start at beginning of history
    Complex *ptrHist, *ptrHist1;
    ptrHist = ptrHist1 = &_z[0];

    // point to last coefficient
    size_t m_length(_filtCoeff.size());
    const Real *ptrCoef = &_filtCoeff[m_length-1];

    // Form output accumulation
    Complex output = *ptrHist++ * *ptrCoef--;
    for(size_t ii = 2; ii < m_length; ++ii)
    {
        // Update history array
        *ptrHist1++ = *ptrHist;
        output += *ptrHist++ * *ptrCoef--;
    }

    // Input tap
    output += input * *ptrCoef;

    // Last history
    *ptrHist1 = input;

    return output;
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   This method zeroes out the filter's history buffer.
//
// Parameters:
//   None.
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void FIRFilter::reset(void)
{
    _z = Complex(0,0);
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   ...
//
// Parameters:
//   None.
//
// Return Value:
//   The number of taps of the filter
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

size_t FIRFilter::size(void)
{
    return _filtCoeff.size();
}


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
Real FIRFilter::in0 (Real x)
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

void FIRFilter::kaiser (RealArray &w, Real beta)
{
    size_t n = (1 + w.size()) / 2, ieo = w.size() % 2;
    Real bes = in0 (beta);
    Real xind = pow ((w.size() - 1.0), 2);

    for (size_t ii = 0; ii < n; ii++)
    {
        Real xi = ii;
        if (ieo == 0)
            xi += 0.5;
        size_t jj = n + ii - ieo;
        w[n - 1 - ii] =
            w[jj] = in0 (beta * sqrt(1.0 - 4.0 * xi * xi / xind)) / bes;
    }

}  // end kaiser


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   This function designs FIR filters via the window design method.
//
// Parameters:
//  type - lowpass, highpass, bandpass, bandstop, hilbert transform
//  atten - stopband attenuation in dB
//  f1 - low freq cutoff, normalized to nyquist frequency= 0.5
//  fh - high freq cutoff, normalized to nyquist frequency= 0.5 (only used with
//       bandpass and bandstop filters).
//
// Return Value:
//   None.
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void FIRFilter::wdfir (
    filter_type type,
    Real  atten,
    Real  fl,
    Real  fh)
{
    size_t ieo, nh;
    Real c, c1, c3, xn, beta, x;

    ieo = _filtCoeff.size() % 2;
    if (!(ieo == 1 || type == lowpass || type == bandpass))
    {
        //
        // Some filter types must have an odd number of taps
        //
        _filtCoeff.resize(_filtCoeff.size() - 1);
        ieo = 1;
    }

    //
    // Window coefficients
    //
    const size_t sz = _filtCoeff.size();
    RealArray winCoef(sz);

    //
    // Compute the ideal (unwindowed) impulse response
    //
    if (type == hilbert)
    {
        _filtCoeff = Real(0);

        nh = (1 + sz) / 2;
        for (size_t ii = nh; ii < sz; ii += 2)
        {
            x = M_PI * (ii + 1 - nh) * 0.5;
            _filtCoeff[ii] = -sin (x) * sin (x) / x;

            size_t jj = nh - (ii + 2 - nh);
            _filtCoeff[jj] = -_filtCoeff[ii];
        }
    }
    else
    {
        c1 = fl;
        if (type == bandpass || type == bandstop)
            c1 = fh - fl;

        nh = (1 + sz) / 2;
        if (ieo == 1)
            _filtCoeff[nh - 1] = 2.0 * c1;

        for (size_t ii = ieo; ii < nh; ii++)
        {
            xn = ii;
            if (ieo == 0)
                xn += 0.5;

            c = M_PI * xn;
            c3 = c * c1;
            if (type == lowpass || type == highpass)
                c3 *= 2.0;

            size_t jj = nh + ii - ieo;
            _filtCoeff[jj] = sin (c3) / c;

            if (type == bandpass || type == bandstop)
                _filtCoeff[jj] *= 2.0 * cos (c * (fl + fh));

            _filtCoeff[nh - ii - 1] = _filtCoeff[jj];
        }
    }

    //
    // Apply a Kaiser-Bessel window
    //
    if (atten > 50.0)
    {
        beta = 0.1102 * (atten - 8.7);
    }
    else if (atten >= 20.96 && atten <= 50.0)
    {
        Real tmp = atten - 20.96;
        beta = 0.58417 * pow (tmp, Real(0.4)) + 0.07886 * tmp;
    }
    else // if (atten < 20.96)
    {
        beta = Real(0);
    }
    kaiser (winCoef, beta);
    _filtCoeff *= winCoef;

    if (type == lowpass || type == bandpass)
    {
        return;
    }
    else if (type == hilbert)
    {
        Real kk = 1;
        Real sum = Real(0);
        for (size_t ii = nh; ii < sz; ii += 2)
        {
            sum += kk * _filtCoeff[ii];
            kk = -kk;
        }
        _filtCoeff *= (-0.5 / sum);

        return;
    }

    _filtCoeff *= -1;
    _filtCoeff[nh - 1] += 1;

}  // end wdfir


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Description:
//   ...
//
// Parameters:
//   None.
//
// Return Value:
//   _filtCoeff - the vector of filter coefficients
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

RealArray FIRFilter::getFilterCoefficients(void)
{
    return _filtCoeff;
}
