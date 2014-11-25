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

#ifndef AUTOCORRELATE_H_
#define AUTOCORRELATE_H_

#include "fft.h"
#include "framebuffer.h"
#include "vectormean.h"
#include "DataTypes.h"

namespace autocorrelator_output
{
	enum type {STANDARD, ROTATED, SUPERIMPOSED};
};

template<typename T>
class Autocorrelator
{
public:

	typedef typename std::vector<T> VectorT;
	typedef typename std::vector<T, fftwf_allocator<T> > FFTWVectorT;
	typedef typename VectorT::iterator IterT;
	typedef typename VectorT::reverse_iterator ReverseIterT;
	typedef typename FFTWVectorT::iterator FFTWIterT;
	typedef typename framebuffer<IterT>::frame FrameIterT;

	Autocorrelator(VectorT& realOutput, size_t correlationSize, long overlap, size_t numAverages, autocorrelator_output::type outType, bool zeroMean=true, bool zeroCenter=false) :
		correlationSize_(correlationSize),
		fftSize_(getFftSize()),
		outputType_(outType),
		zeroMean_(zeroMean),
		zeroCenter_(zeroCenter),
		realOutput_(realOutput),
		realInputFft_(framedTimeDomain_, fftData_, fftSize_, false),
		realOutputFft_(autocorrelationOutput_, fftData_, fftSize_, false),
		realFramer_(correlationSize, overlap),
		vecMean_(numAverages, autocorrelationRotated_, autocorrelationAverage_)
	{
		std::cout << "using correlationSize of " << correlationSize << " fftSize of " << fftSize_ << std::endl;
		//framedTimeDomain_.resize(fftSize_,0.0);
	}

	void run(VectorT& realInput);
	void flush();
	void setCorrelationSize(size_t size);
	void setOverlap(long overlap);
	void setNumAverages(size_t numAverages);
	void setOutputType(autocorrelator_output::type outType);
	void setZeroMean(bool zeroMean);
	void setZeroCenter(bool zeroCenter);

private:
	template<typename U>
	void appendOutput(U& inVec);
	size_t getFftSize();

	size_t correlationSize_;
	size_t fftSize_;
	autocorrelator_output::type outputType_;
	bool zeroMean_;
	bool zeroCenter_;

	VectorT& realOutput_;
	FFTWVectorT framedTimeDomain_;
	ComplexFFTWVector fftData_;
	FFTWVectorT autocorrelationOutput_;
	VectorT autocorrelationRotated_;
	VectorT autocorrelationAverage_;
	std::vector<FrameIterT> realFrames_;

	//foward fft
	FwdFft<FFTWVectorT> realInputFft_;
	RevFft<FFTWVectorT> realOutputFft_;
	framebuffer<IterT> realFramer_;
	VectorExponentialAvg<T> vecMean_;

};

#endif /* AUTOCORRELATE_H_ */
