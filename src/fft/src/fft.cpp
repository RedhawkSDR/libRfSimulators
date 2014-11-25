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

#include "fft.h"

#include "fft.h"
#include <iostream>

//shift in frequency by multiplying in time by complex exponential
//to freqSift by fs/2 for display purposes
//it is the same as multipling by
//e^(j2*pi*(+/-fs/2)*i*1/T)= e^(+/-jpi*i) = {1,-1,1,-1 ...}
//so we can do a freq shift in either direction by muliplying every other guy by -1
void freqShift( ComplexFFTWVector& timeVec)
{
	for (unsigned int i=1; i<timeVec.size(); i+=2)
		timeVec[i]=-timeVec[i];
}

template<typename TimeType>
Fft<TimeType>::Fft(TimeType& time, ComplexFFTWVector& frequency, size_t length, bool wrapComplex) :
	plan_(NULL),
	length_(length),
	vFreq_(frequency),
	vTime_(time),
	wrapComplex_(wrapComplex)
{
}

template<typename TimeType>
Fft<TimeType>::~Fft(void)
{
	if (plan_)
		fftwf_destroy_plan(*plan_);
	plan_=NULL;
}

template<typename TimeType>
void Fft<TimeType>::createPlan()
{
	if (plan_)
		fftwf_destroy_plan(*plan_);
	//reserve data in case we are too small in time and frequency
	size_t timeSize=vTime_.size();
	if (timeSize < length_)
	{
		vTime_.reserve(length_);
	}

	size_t numFreqPts = this->getFreqPoints();
	if (vFreq_.size()<numFreqPts)
		vFreq_.reserve(numFreqPts);

	plan_ =new fftwf_plan;
	//save off these time pointers at plan creation time to be on the safe side
	timePtr_ = &vTime_[0];
	freqPtr_ = &vFreq_[0];
	//run the sub class method to create the plan depending upon what type of fft we are
	planCmd();
}

template<typename TimeType>
void Fft<TimeType>::run(void)
{
	// make sure you zero pad in both time and frquency
	// if the user hasn't provided us with enough data
	size_t timeSize=vTime_.size();
	if (timeSize < length_)
		vTime_.resize(length_,0);
	size_t numFreqPts = this->getFreqPoints();
	if (vFreq_.size()<numFreqPts)
		vFreq_.resize(numFreqPts,0);
	if (timePtr_!=&vTime_[0] || freqPtr_!=&vFreq_[0])
		createPlan();
	fftwf_execute(*plan_);
}

template<typename TimeType>
void Fft<TimeType>::setLength(size_t length)
{
	if (length_!=length)
	{
		length_=length;
		createPlan();
	}
}

template<typename TimeType>
size_t Fft<TimeType>::getLength()
{
	return length_;
}

template<typename TimeType>
size_t Fft<TimeType>::getNumBytesInput()
{
	return length_*sizeof(typename TimeType::value_type);
}

template<typename TimeType>
size_t Fft<TimeType>::getNumBytesOutput()
{
	return getFreqPoints()*sizeof(ComplexFFTWVector::value_type);
}

template<typename TimeType>
size_t Fft<TimeType>::getFreqPoints()
{
	throw "this code shouldn't be called";
}

//only the specialized guys should be called
// real ones have n/2+1 points because real and complex halves are conj. symetric
template<>
size_t Fft<RealFFTWVector>::getFreqPoints()
{
	return length_/2+1;
}
template<>
size_t Fft<ComplexFFTWVector>::getFreqPoints()
{
	return length_;
}

template<typename TimeType>
FwdFft<TimeType>::FwdFft(TimeType& time, ComplexFFTWVector& frequency, size_t length, bool wrapComplex) :
	Fft<TimeType>::Fft(time, frequency, length, wrapComplex)
{
	this->createPlan();
}

//this is the external interface to run
template<typename TimeType>
void FwdFft<TimeType>::run(void)
{
	run_();
}

//specialization if we are complex -- check to see if user requested we wrapped after we run
template<>
void FwdFft<ComplexFFTWVector>::run(void)
{
	if (wrapComplex_)
	{
		freqShift(vTime_);
	}
	run_();
}

//internal run method
template<typename TimeType>
void FwdFft<TimeType>::run_(void)
{
	//cash off the number of time points so we can resize it later
	TimeType& timeVec = this->vTime_;
	size_t timeSize=timeVec.size();
	if (timeSize > Fft<TimeType>::length_)
		std::cerr<<"warning time vector size "<<timeSize <<" > fftSize" << Fft<TimeType>::length_<<". Extra data thrown on the floor"<<std::endl;
	Fft<TimeType>::run();
	//git rid of the extra zeros if we zero padded
	if (timeSize < timeVec.size())
		timeVec.resize(timeSize);
}

template<typename TimeType>
void FwdFft<TimeType>::planCmd()
{
	throw "this shoudln't be called";
}

//do the two plan commands for forard ffts
template<>
void FwdFft<RealFFTWVector>::planCmd()
{
	//this should work but admitedly is a bit ghetto
	*plan_ =  fftwf_plan_dft_r2c_1d(length_, &vTime_[0], reinterpret_cast<fftwf_complex*>(&vFreq_[0]), FFTW_ESTIMATE);
}

template<>
void FwdFft<ComplexFFTWVector>::planCmd()
{
	*plan_ =  fftwf_plan_dft_1d(length_, reinterpret_cast<fftwf_complex*>(&vTime_[0]), reinterpret_cast<fftwf_complex*>(&vFreq_[0]), FFTW_FORWARD, FFTW_ESTIMATE);
}

template<typename TimeType>
RevFft<TimeType>::RevFft(TimeType& time, ComplexFFTWVector& frequency, size_t length, bool wrapComplex) :
	Fft<TimeType>::Fft(time, frequency, length, wrapComplex)
{
	this->createPlan();
}

//external run method
template<typename TimeType>
void RevFft<TimeType>::run(void)
{
	run_();
}

//specialization -- if complex then wrap if user requested it
template<>
void RevFft<ComplexFFTWVector>::run(void)
{
	run_();
	if (wrapComplex_)
		freqShift(vTime_);
}

// internal method
template<typename TimeType>
void RevFft<TimeType>::run_(void)
{
	ComplexFFTWVector& freVec = this->vFreq_;
	size_t freqSize=freVec.size();
	size_t freqPoints = this->getFreqPoints();
	if (freqSize > freqPoints)
		std::cerr<<"warning frequency vector "<<freqSize <<" > freqPoints" << freqPoints<<". Extra data thrown on the floor"<<std::endl;

	Fft<TimeType>::run();
	//git rid of extra zero padded frequency if necessary
	if (freqSize < freVec.size())
		freVec.resize(freqSize);
}

template<typename TimeType>
void RevFft<TimeType>::planCmd()
{
	throw "this shoudln't be called";
}

template<>
void RevFft<RealFFTWVector>::planCmd()
{
	*plan_ =  fftwf_plan_dft_c2r_1d(length_, reinterpret_cast<fftwf_complex*>(&vFreq_[0]), &vTime_[0], FFTW_ESTIMATE);
}

template<>
void RevFft<ComplexFFTWVector>::planCmd()
{
	*plan_ =  fftwf_plan_dft_1d(length_, reinterpret_cast<fftwf_complex*>(&vFreq_[0]), reinterpret_cast<fftwf_complex*>(&vTime_[0]), FFTW_BACKWARD, FFTW_ESTIMATE);
}


template<typename TimeType>
Psd<TimeType>::Psd(TimeType &time, RealFFTWVector &psd, ComplexFFTWVector& fft, size_t length, bool wrapComplex) :
	FwdFft<TimeType>(time, fft, length, wrapComplex),
	vpsd_(psd)
{}

template<typename TimeType>
void Psd<TimeType>::run(void)
{
	FwdFft<TimeType>::run();
	vpsd_.resize(FwdFft<TimeType>::vFreq_.size());
	for (unsigned int i=0; i!=FwdFft<TimeType>::vFreq_.size(); i++)
	{
		float& re = FwdFft<TimeType>::vFreq_[i].real();
		float& im = FwdFft<TimeType>::vFreq_[i].imag();
		vpsd_[i]=re*re+im*im;
	}
}

//explicit instantiation for the types we care about

template class Fft<RealFFTWVector>;
template class Fft<ComplexFFTWVector>;
template class FwdFft<RealFFTWVector>;
template class FwdFft<ComplexFFTWVector>;
template class RevFft<RealFFTWVector>;
template class RevFft<ComplexFFTWVector>;
template class Psd<RealFFTWVector>;
template class Psd<ComplexFFTWVector>;
