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

#ifndef QUANTIZED_CACHE_H_
#define QUANTIZED_CACHE_H_

#include <boost/function.hpp>

/**
 * \brief Reduce a complex function calculation to a look-up to increase performance
 */
template<typename OutType, typename InType>
class QuantizedCash
{
public:
    typedef boost::function<OutType(InType)> function;

    /**
     * Pass in a function with the number of points to use in Quantization
     * and a min & max value for the inputs to your function
     */
    QuantizedCash(function f, size_t numPts, InType minValue, InType maxValue);

    /**
     * Call to use the cache and retrieve your output value
     */
    OutType getValue(InType& input);
private:
    std::vector<OutType> cachVec;
    size_t getIndex(InType& input);
    InType minVal;
    InType maxVal;
    InType offset;
    double quantization;
    function func;
};

//keeping implementation here so that the templates work properly

template<typename OutType, typename InType>
QuantizedCash<OutType, InType>::QuantizedCash(function f, size_t numPts, InType minValue, InType maxValue) :
    minVal(minValue),
    maxVal(maxValue),
    offset(-minValue),
    func(f)
{
    if (numPts>1)
    {
        quantization = ((maxValue-minValue)/(numPts-1)),
        //number of cached points is maxIndex+1 since maxIndex is zero based
        cachVec.resize(numPts);
        InType val;
        for (size_t i=0; i!=cachVec.size(); i++)
        {
            //compute the quantized input value to the function
            val=i*quantization+minVal;
            //store off the output value in the cachVec
            cachVec[i]=func(val);
        }
    }
    else
        quantization=0;
}

template<typename OutType, typename InType>
OutType QuantizedCash<OutType, InType>::getValue(InType& input)
{
    //if the value is in the cached range - look up the input index and return the output value
    if (cachVec.size()>0 && input <=maxVal && input>=minVal)
    {
        size_t i = getIndex(input);
        return cachVec[i];
    }
    else
        //if we are not in the cached range call the function to directly calculate the value
        return func(input);
}

template<typename OutType, typename InType>
size_t QuantizedCash<OutType, InType>::getIndex(InType& input)
{
    //get the closest index corresponding to the cached value
    return static_cast<size_t>(round((input+offset)/quantization));
}

#endif /* QUANTIZED_CACHE_H_ */
