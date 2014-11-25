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

#ifndef _FIRFILTER_H
#define _FIRFILTER_H

#include "DataTypes.h"

/**
 * \brief Filter class
 *
 * Code was copied from "C++ Algorithms for Digital Signal Processing",
 * 2nd ed. by Embree & Danieli and modified.
 */
class FIRFilter
{
public:
    typedef enum
    {
        lowpass  = 1,
        highpass = 2,
        bandpass = 3,
        bandstop = 4,
        hilbert  = 5
    } filter_type;

    // Constructor
    FIRFilter(const ComplexArray &input, ComplexArray &output,
        Real *coef, size_t length);
    FIRFilter(const ComplexArray &input, ComplexArray &output,
        filter_type type = lowpass, Real atten = 70, Real Fl = Real(0.5), Real Fh = 0);
    virtual ~FIRFilter(void);
    
    virtual void run(void);
    virtual Complex run(const Complex &input);
    virtual void reset(void);
    size_t size(void);
    RealArray getFilterCoefficients(void);

protected:
    const ComplexArray &vIn;
    ComplexArray &vOut;
    RealArray _filtCoeff;       ///< Filter coefficients
    ComplexArray _z;            ///< Filter history
    Complex *m_hist;            ///< Constant ptr to filter history

private:
    FIRFilter();                // No default constructor

    void wdfir (
        filter_type type,
        Real  atten,
        Real  fl,
        Real  fh);
    Real in0 (Real x);
    void kaiser (RealArray &w, Real beta);
};

#endif // _FIRFILTER_H
