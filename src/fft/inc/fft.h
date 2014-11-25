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

#ifndef FFT_H_
#define FFT_H_


#include "fftw3.h"
#include "fftw_allocator.h"

template<typename TimeType>
class Fft
{
	public:
		// Constructor
		Fft(TimeType& time, ComplexFFTWVector& frequency, size_t length, bool wrapComplex);
		virtual ~Fft(void);
        virtual void createPlan();
	    virtual void run(void);
	    void setLength(size_t length);
	    size_t getLength();
	    size_t getNumBytesInput();
	    size_t getNumBytesOutput();
		virtual size_t getFreqPoints();

	protected:


        virtual void planCmd()=0;

		fftwf_plan* plan_;
	    size_t length_;
	    ComplexFFTWVector  &vFreq_;
	    TimeType &vTime_;
	    void* timePtr_;
	    void* freqPtr_;
	    bool wrapComplex_;

};

template<typename TimeType>
class FwdFft : public Fft<TimeType>
{
	public:

	// Constructor
	FwdFft(TimeType& time, ComplexFFTWVector& frequency, size_t length, bool wrapComplex);
		virtual ~FwdFft(void) {};
		virtual void run(void);

    protected:
		void run_(void);
		void planCmd();
};

template<typename TimeType>
class RevFft : public Fft<TimeType>
{
	public:

		// Constructor
		RevFft(TimeType& time, ComplexFFTWVector& frequency, size_t length, bool wrapComplex);
		virtual ~RevFft(void) {};
		virtual void run(void);

    protected:
		void run_(void);
		void planCmd();
};

template<typename TimeType>
class Psd : public FwdFft<TimeType>
{
	public:
		Psd(TimeType &time, RealFFTWVector &psd, ComplexFFTWVector& fft, size_t length, bool wrapComplex);
		virtual ~Psd(void) {};
		void run(void);
	protected:
	    RealFFTWVector &vpsd_;
};


typedef FwdFft<RealFFTWVector> RealFwdFft;
typedef RevFft<RealFFTWVector> RealRevFft;

typedef FwdFft<ComplexFFTWVector> ComplexFwdFft;
typedef RevFft<ComplexFFTWVector> ComplexRevFft;

typedef Psd<RealFFTWVector> RealPsd;
typedef Psd<ComplexFFTWVector> ComplexPsd;

#endif /* FFT_H_ */
