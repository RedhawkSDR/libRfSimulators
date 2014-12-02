/*
 * Transmitter.h
 *
 *  Created on: Nov 17, 2014
 */

#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#include "boost/filesystem.hpp"
#include <string>
#include <iostream>
#include "boost/thread.hpp"
#include "FrequencyModulator.h"
#include <complex>
#include "Tuner.h"
#include "FIRFilter.h"
#include "FirFilterDesigner.h"
#include "fftw3.h"
#include "fftw_allocator.h"

extern "C" {
#include "rds.h"
#include "fm_mpx.h"
}

using namespace boost::filesystem;
using namespace boost;

#define FILTER_ATTENUATION 70 // dB
#define FILTER_CUTOFF (0.5*0.5*(BASE_SAMPLE_RATE / MAX_OUTPUT_SAMPLE_RATE)) // normalized frequency

class Transmitter {
public:
	Transmitter();
	void setTunedFrequency(float centerFreqeuncy);
	void setFilePath(path filePath);
	path getFilePath();
	void setRdsText(std::string rdsText);
	virtual ~Transmitter();
	std::valarray< std::complex<float> >& getData();
	friend std::ostream& operator<<(std::ostream &strm, const Transmitter &tx);
	void start();
	void join();
	int init(float centerFreq, int numSamples);

private:

	// This is the station frequency from the XML file
	float centerFrequency;

	// This is the frequency we are currently tuned to so we can calculate the relative frequency to shift to.
	float tunedFrequency;

	path filePath;
	std::string rdsText;
	thread m_Thread;
	int numSamples;
	int doWork();
	std::valarray<float> mpx_buffer;
	std::valarray< std::complex<float> > basebandCmplx;
	std::valarray< std::complex<float> > basebandCmplxUpSampled;
	std::valarray< std::complex<float> > basebandCmplxUpSampled_tmp;
	std::valarray< std::complex<float> > basebandCmplxUpSampledTuned;

	rds_struct rds_status_struct;
	fm_mpx_struct fm_mpx_status_struct;
	bool initilized;
	FrequencyModulator fm;
	FIRFilter filter;
	Tuner tuner;

};

#endif /* TRANSMITTER_H_ */