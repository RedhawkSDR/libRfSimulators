/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This file is part of REDHAWK Basic Components fftlib library.
 *
 * REDHAWK Basic Components fftlib library is free software: you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * REDHAWK Basic Components fftlib library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program.  If not, see http://www.gnu.org/licenses/.
 */

#include "firfilter.h"

size_t getMaxTapsSize(size_t fftSize)
{
	// given our current fft size - what is the max taps we can support?
	//
	// Here are the two constraint equations we need to satisfy:
	//
	// frameSize_ = fftSize_-numTaps_+1; // convolution constraint equation - technically this is <= but we always choose = for efficiency
	// frameSize_ >= numTaps_-1 ;        // this is a constraint on the overlap -- we need the frameSize >= overlap (which happens to be numTaps_-1
	//                                   // techincally this is not strictly necessary for overlap-add
	//									 // but if this isn't met you have to add multiple ifft frames of data for each input frame
	//                                   // this would add significant complexity plus you lose efficiency for large tap sizes compared to fftSize anyway
	// (substitute the equations yields the following constraint)
	// fftSize_-numTaps_+1 >= numTaps_-1
	// 2*(numTaps-1)<=fftSize_
	// do the integer division trick to efficiently take the ceiling and you end up with the following equation

	return (fftSize+1)/2+1;
}

firfilter::firfilter(size_t fftSize, realVector& realOutput, complexVector& complexOutput) :
	realOutput_(realOutput),
	complexOutput_(complexOutput),
	realInputFft_(realInputFftIn_,freqDomainData_, fftSize,false),
	complexInputFft_(complexInputFftIn_,freqDomainData_, fftSize,false),
	realOutputFft_(realOutputFftOut_, freqDomainData_, fftSize, false),
	complexOutputFft_(complexOutputFftOut_, freqDomainData_, fftSize, false),
	realFramer_(1,0),
	complexFramer_(1,0),
	fftSize_(fftSize)
{
	RealFFTWVector tmp(1,1.0);
	setTaps(tmp);
}

template<typename T>
firfilter::firfilter(size_t fftSize, realVector& realOutput, complexVector& complexOutput, T& taps) :
	realOutput_(realOutput),
	complexOutput_(complexOutput),
	realInputFft_(realInputFftIn_,freqDomainData_, fftSize,false),
	complexInputFft_(complexInputFftIn_,freqDomainData_, fftSize,false),
	realOutputFft_(realOutputFftOut_, freqDomainData_, fftSize, false),
	complexOutputFft_(complexOutputFftOut_, freqDomainData_, fftSize, false),
	realFramer_(1,0),
	complexFramer_(1,0),
	fftSize_(fftSize)
{
	setTaps(taps);
}

firfilter::~firfilter() {
	// TODO Auto-generated destructor stub
}

void firfilter::mirror(ComplexFFTWVector& vector)
{
	//real data has a conjugate symetric fft -
	//expand the other half of it to get the "negative" frequencies
	//note - because we are not rotating the frequency data -
	//these are actually Postiive frequencies from fs/2 to fs and are appended to the frequency vector
	size_t vecSize = vector.size();
	vector.resize((vecSize-1)*2);
	ComplexFFTWVector::iterator i =vector.begin();
	ComplexFFTWVector::iterator o =vector.end();
	ComplexFFTWVector::iterator done = i+vecSize;
	for (;i!=done; i++, o--)
		*o=conj(*i);
}

void firfilter::setTaps(RealFFTWVector& taps)
{
	if (taps.empty())
		std::cout<<"firfilter::setTaps warning - ignore setTaps for empty taps"<<std::endl;
	else
	{
		boost::mutex::scoped_lock lock(boostLock_);
		realTaps_.assign(taps.begin(), taps.end());
		complexTaps_.clear();

		numTaps_ = taps.size();
		updateInternals();
	}
}
void firfilter::setTaps(ComplexFFTWVector& taps)
{
	if (taps.empty())
		std::cout<<"firfilter::setTaps warning - ignore setTaps for empty taps"<<std::endl;
	else
	{
		boost::mutex::scoped_lock lock(boostLock_);
		realTaps_.clear();
		complexTaps_.assign(taps.begin(), taps.end());

		numTaps_ = taps.size();
		updateInternals();
	}
}

void firfilter::setFftSize(size_t val)
{
	boost::mutex::scoped_lock lock(boostLock_);
	fftSize_ =val;
	realInputFft_.setLength(fftSize_);
	complexInputFft_.setLength(fftSize_);
	realOutputFft_.setLength(fftSize_);
	complexOutputFft_.setLength(fftSize_);
    updateInternals();
}

size_t firfilter::getMaxTaps()
{
	return getMaxTapsSize(fftSize_);
}

size_t firfilter::getNumTaps()
{
	return std::max(realTaps_.size(), complexTaps_.size());
}

void firfilter::updateInternals()
{
	size_t maxTapsAllowed = getMaxTaps();
	assert(maxTapsAllowed>=numTaps_);
	tapsFreq_.clear();
	if (!realTaps_.empty())
	{
		RealFwdFft fft(realTaps_,tapsFreq_,fftSize_,false);
		fft.run();
		//in case we get complex input with our real taps lets mirror
		//the full frequency response so we have the data available
		mirror(tapsFreq_);
	}
	else if(!complexTaps_.empty())
	{
		ComplexFwdFft fft(complexTaps_,tapsFreq_,fftSize_,false);
		fft.run();
	}
	else
	{
		std::cout<<"no taps!!!"<<std::endl;
		throw "no real or complex taps - this shouldn't happen!";
	}
	//scale the frequency response by 1 over the fft size since fftw introduces this extra factor
	// because it donesn't include the twiddle factor in either the forward or inverse fft
	// if we do it hear int the taps we don't have to do it every time on the input data
	float gain = 1.0/fftSize_;
	for (ComplexFFTWVector::iterator i = tapsFreq_.begin(); i!=tapsFreq_.end(); i++)
		(*i) *= gain;

	//technically the frame size can be any number
	//less than or equal to fftSize_-numTaps_+1 and the
	//overlap add algorthim works...but it is most efficient when it is as
	//large as possbible

	frameSize_ = fftSize_-numTaps_+1;

	if (frameSize_ < fftSize_/8.0)
		std::cout<<"WARNING firfilter::frameSize_ "<<frameSize_<<" is small -- you might consider increasing your filter size or increase your fftSize."<<std::endl;
	if (numTaps_ < fftSize_/8.0)
		std::cout<<"WARNING firfilter::numTaps_ "<<numTaps_<<" is small compared to your fftSize "<<fftSize_<< " -- You might consider decreasing your fft size"<<std::endl;
	realFramer_.setFrameSize(frameSize_);
	complexFramer_.setFrameSize(frameSize_);
}

void firfilter::newRealData(realVector& realInput)
{
	boost::mutex::scoped_lock lock(boostLock_);

	//frame up the input data and run on each frame
	realFramer_.newData(realInput,realFrames_);
	//if we have complex taps the output will be complex even though the input is real
	if (realTaps_.empty())
	{
		complexOutput_.resize(realFrames_.size()*frameSize_);
		realOutput_.clear();
	}
	else
	{
		//typical case - all real data in and out for real taps
		if (complexOverlap_.empty() )
		{
			realOutput_.resize(realFrames_.size()*frameSize_);
			complexOutput_.clear();
		}
		//if we have leftover data in the filter then we have a few complex elements at the start
		// add a bit of room for the complex output here
		else if (!realFrames_.empty())
		{
			//first frame has some complex elements dut to our previous overlap
			realOutput_.resize(realFrames_.size()*frameSize_ - complexOverlap_.size());
			complexOutput_.resize(complexOverlap_.size());
		}
	}

	complexVector::iterator complexOut = complexOutput_.begin();
	realVector::iterator realOut = realOutput_.begin();
	for (std::vector<framebuffer<realVector::iterator>::frame>::iterator frame = realFrames_.begin(); frame !=realFrames_.end(); frame++)
	{
		//take forward fft
		realInputFftIn_.assign(frame->begin, frame->end);
		freqDomainData_.clear();
		realInputFft_.run();
		if (realTaps_.empty())
			mirror(freqDomainData_);

		applyFilter();
		//take reverse fft and add previous data to get full output
		if (realTaps_.empty())
			doComplexIfft(complexOut);
		else
			doRealIfft(realOut, complexOut);
	}
	assert(realOut==realOutput_.end());
	assert(complexOut==complexOutput_.end());

}
void firfilter::newComplexData(complexVector& complexInput)
{
	boost::mutex::scoped_lock lock(boostLock_);
	complexFramer_.newData(complexInput,complexFrames_);
	complexOutput_.resize(complexFrames_.size()*frameSize_);
	realOutput_.clear();
	complexVector::iterator out = complexOutput_.begin();
	for (std::vector<framebuffer<complexVector::iterator>::frame>::iterator frame = complexFrames_.begin(); frame !=complexFrames_.end(); frame++)
	{
		complexInputFftIn_.assign(frame->begin, frame->end);
		freqDomainData_.clear();
		complexInputFft_.run();

		applyFilter();
		doComplexIfft(out);
	}
	assert(out==complexOutput_.end());

}

void firfilter::flush()
{
	realFramer_.flush();
	complexFramer_.flush();
	realOverlap_.clear();
	complexOverlap_.clear();
}

void firfilter::applyFilter()
{
	ComplexFFTWVector::iterator j = tapsFreq_.begin();
	//apply the filter via the frequency domain with multiplication
	for (ComplexFFTWVector::iterator i = freqDomainData_.begin(); i!=freqDomainData_.end(); i++, j++)
	{
		*i *= *j;
	}
}

void firfilter::doComplexIfft(complexVector::iterator& output)
{
	//take the inverse fft
	complexOutputFft_.run();
	size_t overlapSize = numTaps_-1;
	//we have limited support for the time domain -- all the last data will be 0
	complexOutputFftOut_.resize(frameSize_+overlapSize);
	ComplexFFTWVector::iterator newOutput = complexOutputFftOut_.begin();
	if (!complexOverlap_.empty())
	{
		for (complexVector::iterator oldOutput = complexOverlap_.begin(); oldOutput!=complexOverlap_.end(); oldOutput++, newOutput++, output++)
			*output = *oldOutput + *newOutput;
	}
	else if (!realOverlap_.empty())
	{
		for (realVector::iterator oldOutput = realOverlap_.begin(); oldOutput!=realOverlap_.end(); oldOutput++, newOutput++, output++)
			*output = *oldOutput + *newOutput;
		realOverlap_.clear();
	}

	//copy the remaining points which are legit
	ComplexFFTWVector::iterator overlapEnd = complexOutputFftOut_.begin()+frameSize_;
	output = copy(newOutput,overlapEnd,output);
	newOutput = overlapEnd;
	//finally assign the new overlap for next time
	complexOverlap_.assign(newOutput,complexOutputFftOut_.end());
}

void firfilter::doRealIfft(realVector::iterator& realOut, complexVector::iterator& complexOut)
{
	realOutputFft_.run();
	size_t overlapSize = numTaps_-1;
	//we have limited support for the time domain output -- all the last data will be 0
	realOutputFftOut_.resize(frameSize_+overlapSize);

	RealFFTWVector::iterator newOutput = realOutputFftOut_.begin();
	if (!realOverlap_.empty())
	{
		for (realVector::iterator oldOutput = realOverlap_.begin(); oldOutput!=realOverlap_.end(); oldOutput++, newOutput++, realOut++)
			*realOut = *oldOutput + *newOutput;
	}
	else if (!complexOverlap_.empty())
	{
		for (complexVector::iterator oldOutput = complexOverlap_.begin(); oldOutput!=complexOverlap_.end(); oldOutput++, newOutput++, complexOut++)
			*complexOut = *oldOutput + *newOutput;
		complexOverlap_.clear();
	}
	//copy the remaining points which are legit
	RealFFTWVector::iterator overlapEnd = realOutputFftOut_.begin()+frameSize_;
	realOut = copy(newOutput,overlapEnd,realOut);
	newOutput = overlapEnd;
	//finally assign the new overlap for next time
	realOverlap_.assign(newOutput,realOutputFftOut_.end());
}

template firfilter::firfilter(size_t, realVector&, complexVector&, RealFFTWVector&);
template firfilter::firfilter(size_t, realVector&, complexVector&, ComplexFFTWVector&);
