/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK librfsimulators.
 *
 * REDHAWK librfsimulators is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK librfsimulators is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
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
