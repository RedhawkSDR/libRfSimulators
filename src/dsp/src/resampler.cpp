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
#include "resampler.h"
#include <boost/bind.hpp>

void toCx(std::deque<float>& oldData, std::deque<std::complex<float> >& newData)
{
	newData.clear();
	for (std::deque<float>::iterator i = oldData.begin(); i!=oldData.end(); i++)
	{
		newData.push_back(*i);
	}
}

ArbitraryRateResamplerClass::ArbitraryRateResamplerClass(float inputRate,
														 float outputRate,
														 size_t a,
														 size_t quantizationPts,
														 std::vector<float>& real_out,
														 std::vector<std::complex<float> >& complex_out,
														 float* startTime,
														 std::deque<float>* realHistory,
										                 std::deque<std::complex<float> >* cmplxHistory):
	kernel(a),
	cache(boost::bind(&LanczosKernel<float,float>::getValue,&kernel, _1), quantizationPts, -a, a),
	inRate(inputRate),
	outRate(outputRate),
	rateFactor(inputRate/outputRate),
	filterDelay(-float(a)/inputRate),
	realOut(real_out),
	cmplxOut(complex_out)
{
    //if we don't have a startTime given to us - set the outputTime to be 0
	if (startTime==NULL)
		outputTime=0;
	else
		outputTime=(*startTime-filterDelay)*inRate;
	//if we have real history then copy it
	if (realHistory && !realHistory->empty())
		copy_n_elements(*realHistory,realOldData,2*a);
	//if we haev complex history copy it
	if (cmplxHistory && !cmplxHistory->empty())
		copy_n_elements(*cmplxHistory,cmplxOldData,2*a);
	//if quantization is requested use the cache
	if (quantizationPts>100)
		func= boost::bind(&QuantizedCash<float,float>::getValue, cache, _1);
	else
		func= boost::bind(&LanczosKernel<float,float>::getValue, kernel, _1);
}

float ArbitraryRateResamplerClass::getInRate()
{
	return inRate;
}

float ArbitraryRateResamplerClass::getNextOutputDelay()
{
	return filterDelay+outputTime/inRate;
}

std::deque<float>* ArbitraryRateResamplerClass::getRealHistory()
{
	if (!realOldData.empty())
		return &realOldData;
	return NULL;
}
std::deque<std::complex<float> >* ArbitraryRateResamplerClass::getComplexHistory()
{
	if (!cmplxOldData.empty())
		return &cmplxOldData;
	return NULL;
}

float ArbitraryRateResamplerClass::newData(std::vector<float>& real_in)
{
	realOut.clear();
	cmplxOut.clear();
	//calculate our time offset (in seconds) for the start of the output
	float offset = getNextOutputDelay();
	std::vector<float>::iterator i = real_in.begin();
	if (! cmplxOldData.empty())
	{
		//push through up to 2*a complex samples to clear the filter taps from all the complex data
		size_t twoA = 2*kernel._a;
		size_t numSamples = std::min(twoA, real_in.size());
		bool cmplxFlushed = numSamples==twoA;
		std::complex<float>val(0,0);
		//do 2*a real samples as complex samples
		for (size_t j=0; j!=numSamples; j++)
		{
			//note I would like to use
			// val.real(*i);
			// but it is not available in C++98
			// as such you can use the copy operator and it will
			// more or less do the same thing since the imaginary part
			// is identically 0
			val = *i;
			next(val, cmplxOut, cmplxOldData);
			if (cmplxFlushed)
				realOldData.push_front(*i);
			i++;
		}
		if (cmplxFlushed)
			cmplxOldData.clear();
	}
	//Process real data one sample at a time
	for(;i!=real_in.end(); i++)
	{
		next(*i, realOut, realOldData);
	}
	return offset;
}
float ArbitraryRateResamplerClass::newData(std::vector<std::complex<float> >& complex_in)
{
	//calculate our time offset (in seconds) for the start of the output
	realOut.clear();
	cmplxOut.clear();
	float offset = getNextOutputDelay();

	if (! realOldData.empty())
	{
		//convert the real history to complex if required so we can use it with the new complex data
		toCx(realOldData, cmplxOldData);
		realOldData.clear();
	}
	//Process real data one sample at a time
	for(std::vector<std::complex<float> >::iterator i=complex_in.begin(); i!=complex_in.end(); i++)
	{
		next(*i, cmplxOut, cmplxOldData);
	}
	return offset;
}

template<typename T>
void ArbitraryRateResamplerClass::next(T& val,  std::vector<T>& output, std::deque<T>& oldData)
{
	//process a single input to produce zero or more outputs depending upon our state

	//If our history is full - get rid of the last element
	if (oldData.size()==2*kernel._a)
		oldData.pop_back();
	//add our current sample to the history
	oldData.push_front(val);
	T out;
	//x is our fractional number in input samples.  We use x to cacluate our filter weight for each value in our history buffer
	float x;
	//i is used to access our history buffer to compute each output sample
	typename std::deque<T>::iterator i;
	int minXVal=-kernel._a;
	// valid outputTime is between 0 and 1
    //if ouptutTime is  > 1 we are downsampling -- we don't produce any output for this input sample
	//while outputTime remains <1 we are upsampling -- we produce multiple outputs for this input sample
	while (outputTime <=1.0)
	{
		out = 0;
		x = outputTime-kernel._a;
		i=oldData.begin();
		//this first while loop is typically not entered into.
		//It is only for a startTime was specified in the constructor where you get in this situation
		//for this case we really need prior history (before what is stored in our buffer to compute the times appropriately
		//we don't have it - just advance furtther in time until we get to the end of our buffer or until we get to a point we have valid history
		while (x<minXVal)
		{
			x += 1;
			i++;
			if (i==oldData.end())
				//if we get here we are at the end of our data buffer - we can't compute a valid output sample
				//we will output a 0 to zero pad the output
				break;
		}
		//typical case - compute filter coefficients for each "x" value apply to our history buffer to compute this output
		for (; i !=oldData.end(); i++)
		{
			out+=*i*func(x);
			x += 1;
		}
		output.push_back(out);
		//update the outputTime for this outputSample by the time difference
		//this is the decimal value representing the distance between ouptuts measured in terms of input samples
		outputTime+=rateFactor;
	}
	//decrease outputTime by one sample to account for this input sample
	outputTime-=1.0;
}
