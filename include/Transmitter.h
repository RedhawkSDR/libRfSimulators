/*
 * Transmitter.h
 *
 *  Created on: Nov 17, 2014
 */

#ifndef LIBFMRDSSIMULATOR_INCLUDE_TRANSMITTER_H_
#define LIBFMRDSSIMULATOR_INCLUDE_TRANSMITTER_H_

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
#include "SimDefaults.h"

extern "C" {
#include "rds.h"
#include "fm_mpx.h"
}

using namespace boost::filesystem;
using namespace boost;

#define FILTER_CUTOFF (0.5*0.5*(BASE_SAMPLE_RATE / MAX_OUTPUT_SAMPLE_RATE)) // normalized frequency

class Transmitter {
public:
	Transmitter();
	void setTunedFrequency(float centerFreqeuncy);
	void setFilePath(path filePath);
	path getFilePath();
	void setRdsFullText(std::string fullText);
	void setRdsShortText(std::string shortText);
	void setRdsCallSign(std::string callSign);
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

	std::string rdsFullText;
	std::string rdsShortText;
	std::string rdsCallSign;

	thread m_Thread;
	int numSamples;
	int doWork();
	unsigned int callSignToInt(std::string callSign);

	std::valarray<float> mpx_buffer;
	std::valarray< std::complex<float> > basebandCmplx;
	std::valarray< std::complex<float> > basebandCmplx_polyPhaseout;
	std::valarray< std::complex<float> > basebandCmplxUpSampled;
	std::valarray< std::complex<float> > basebandCmplxUpSampledTuned;
	std::vector<FIRFilter *> polyphaseFilters;
	std::vector< std::vector<float> > polyphaseFilterTaps;


	rds_struct rds_status_struct;
	fm_mpx_struct fm_mpx_status_struct;
	bool initilized;
	FrequencyModulator fm;


	Tuner tuner;
	boost::mutex tunerMutex;

};

#endif /* LIBFMRDSSIMULATOR_INCLUDE_TRANSMITTER_H_ */
