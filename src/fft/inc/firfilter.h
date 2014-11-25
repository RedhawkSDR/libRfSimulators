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

#ifndef FIRFILTER_H_
#define FIRFILTER_H_

#include "fft.h"
#include "framebuffer.h"
#include <complex>

template<typename T>
void printVec(T vec)
{
	for (unsigned int i=0; i!=vec.size(); i++)
		std::cout<<vec[i]<<",";
	std::cout<<std::endl;
};

size_t getMaxTapsSize(size_t fftSize);

class firfilter {
	//This fir filter implementation uses the
public:

	typedef std::vector<float> realVector;
	typedef std::vector<std::complex<float> > complexVector;

	firfilter(size_t fftSize, realVector& realOutput, complexVector& complexOutput);
	template<typename T>
	firfilter(size_t fftSize, realVector& realOutput, complexVector& complexOutput, T& taps);
	virtual ~firfilter();

	void setTaps(RealFFTWVector& taps);
	void setTaps(ComplexFFTWVector& taps);

	void setFftSize(size_t val);
	size_t getMaxTaps();
	size_t getNumTaps();

	void newRealData(realVector& realInput);
	void newComplexData(complexVector& complexInput);
	void flush();

private:

	void mirror(ComplexFFTWVector& vector);
	void applyFilter();
	void doComplexIfft(complexVector::iterator& output);
	void doRealIfft(realVector::iterator& realOut, complexVector::iterator& complexOut);
	void updateInternals();

	//inputs and outputs for the class
	realVector& realOutput_;
	complexVector& complexOutput_;


	//inputs for the fft
	RealFFTWVector realInputFftIn_;
	ComplexFFTWVector complexInputFftIn_;

	//frequency domain  working vector
	ComplexFFTWVector freqDomainData_;

	//outputs for the output fft
	RealFFTWVector realOutputFftOut_;
	ComplexFFTWVector complexOutputFftOut_;

	//last data vectors
	realVector realOverlap_;
	complexVector complexOverlap_;

	//frequency domain filter
	ComplexFFTWVector tapsFreq_;
	//time domain cash of the taps
	//need to store them off in case the fftsize changes on us
	RealFFTWVector realTaps_;
	ComplexFFTWVector complexTaps_;

	//foward fft
	RealFwdFft realInputFft_;
	ComplexFwdFft complexInputFft_;

	//reverse fft
	RealRevFft realOutputFft_;
	ComplexRevFft complexOutputFft_;

	//frame buffers
	framebuffer<realVector::iterator> realFramer_;
	framebuffer<complexVector::iterator> complexFramer_;

	std::vector<framebuffer<realVector::iterator>::frame> realFrames_;
	std::vector<framebuffer<complexVector::iterator>::frame> complexFrames_;

	size_t fftSize_;
	size_t numTaps_;
	size_t frameSize_;

	boost::mutex boostLock_;

};

#endif /* FIRFILTER_H_ */
