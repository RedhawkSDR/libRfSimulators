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
#ifndef RESAMPLER_H_
#define RESAMPLER_H_

#include <stdlib.h>
#include <math.h>
#include <vector>
#include <deque>
#include <complex>
#include "quantized_cache.h"

/**
 * Convert data from real to complex by zeroing the imaginary part
 */
void toCx(std::deque<float>& oldData, std::deque<std::complex<float> >& newData);

/**
 * Copy up to n elments from oldData to newData
 */
template<typename T>
void copy_n_elements(std::deque<T>& oldData, std::deque<T>& newData, size_t n)
{
    size_t cpyNum = std::min(n,oldData.size());
    newData.resize(cpyNum);
    typename std::deque<T>::iterator o =oldData.begin();
    for (typename std::deque<T>::iterator i =newData.begin(); i!=newData.end(); i++)
    {
        *i=*o;
        o++;
    }
}

/**
 * \brief This is a class to compute the Lanczos Kernel for resampling fitler coefficients
 */
template<typename OutType, typename InType>
class LanczosKernel
{
public:
    /**
     * Create the kernel. There are typically 2*a input points used to compute
     * one resampled output. So the bigger a, the more history, delay,
     * computation, etc.
     */
    LanczosKernel(size_t a);
    OutType getValue(InType& x);
    size_t _a;
};

template<typename OutType, typename InType>
LanczosKernel<OutType, InType>::LanczosKernel(size_t a) :
 _a(a)
 {}

template<typename OutType, typename InType>
OutType LanczosKernel<OutType, InType>::getValue(InType& x)
 {
    if (x==0)
        return 1.0;
    float piX = M_PI*x;
    float denominator=std::pow(piX,2);
    float numerator = _a*sin(piX)*sin(piX/_a);
    return numerator/denominator;
}

/**
 * \brief This is an arbitrary rate resampler for non-integer value decimation/interpolation
 *
 * All computations are done in the time domain with a time varient filter whose
 * coefficients are calculated by the LanczosKernel.  This is slower than a frequency
 * domain approach but can be useful for cases where exact rate resampling is required
 */
class ArbitraryRateResamplerClass
{
public:
    /**
     * \param inputRate Required param
     * \param outputRate Required param
     * \param a Required param
     * \param quantizationPts Required param
     * \param real_out Required param
     * \param complex_out Required param
     * \param startTime Do not pass and the filter will default appropriately
     * \param realHistory Do not pass in if there is none
     * \param cmplxHistory Do not pass in if there is none
     */
    ArbitraryRateResamplerClass(float inputRate,
                                float outputRate,
                                size_t a,
                                size_t quantizationPts,
                                std::vector<float>& real_out,
                                std::vector<std::complex<float> >& complex_out,
                                float* startTime=NULL,
                                std::deque<float>* realHistory=NULL,
                                std::deque<std::complex<float> >* cmplxHistory=NULL);

    /**
     * Pass in new real input data.  Return the delay associated with the first output sample.
     */
    float newData(std::vector<float>& real_in);

    /**
     * Pass in new real output data.  Return the delay associated with the first output sample.
     */
    float newData(std::vector<std::complex<float> >& complex_in);

    /**
     * Get the nextOutput time.  Includes delay offset for the next output sample.
     */
    float getNextOutputDelay();

    float getInRate();
    std::deque<float>* getRealHistory();
    std::deque<std::complex<float> >* getComplexHistory();

private:
    template<typename T>
    void next(T& val,  std::vector<T>& output, std::deque<T>& oldData);

    LanczosKernel<float,float> kernel;
    QuantizedCash<float, float> cache;
    boost::function<float(float)> func;

    float inRate;
    float outRate;
    double outputTime;
    float rateFactor;
    float filterDelay;
    std::vector<float>& realOut;
    std::vector<std::complex<float> >& cmplxOut;

    std::deque<float> realOldData;
    std::deque<std::complex<float> > cmplxOldData;
};


#endif /* RESAMPLER_H_ */
