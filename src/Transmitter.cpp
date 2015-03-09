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
 * Transmitter.cpp
 *
 *  Created on: Nov 17, 2014
 */

#include "Transmitter.h"
#include "DigitizerSimLogger.h"
#include <math.h>
#include "SimDefaults.h"
#include <algorithm>

// From: http://gnuradio.org/redmine/projects/gnuradio/wiki/SignalProcessing
// sensitivity = (2 * pi * max_deviation) / samp_rate

Transmitter::Transmitter() :
		fm((2 * M_PI * MAX_FREQUENCY_DEVIATION) / BASE_SAMPLE_RATE),
		tuner(basebandCmplxUpSampled, basebandCmplxUpSampledTuned, 0)
		{

	TRACE("Entered Method");
	centerFrequency = -1;

	rdsFullText = "REDHAWK Radio, Rock the Hawk!";
	rdsShortText = "REDHAWK!";
	rdsCallSign = "WSDR";

	numSamples = -1;
	initilized = false;

	fm_mpx_status_struct.phase_38 = 0;
	fm_mpx_status_struct.phase_19 = 0;
	fm_mpx_status_struct.audio_index = 0;
	fm_mpx_status_struct.audio_len = 0;
	fm_mpx_status_struct.fir_index = 0;

	TRACE("Clearing out the fm_mpx structs");
	unsigned int i;
	for (i = 0; i < FIR_SIZE; i++) {
		fm_mpx_status_struct.fir_buffer_mono[i] = 0;
		fm_mpx_status_struct.fir_buffer_stereo[i] = 0;
	}

	filePath = "";
	tunedFrequency = 0.0;

	/**
	 * This is a bit of a messy approach currently and should be rolled into the Redhawk DSP library.
	 * Initially, we inserted zeros to upsample then filtered the upsampled data.  This was a big strain on CPU.
	 * The approach outlined in section 3.4 of http://www.dspguru.com/dsp/faqs/multirate/interpolation provides
	 * a polyphase filter approach.  This allows us to create an LPF with 30 taps, then based on that, create ten 3 tap filters.
	 * The data is then filtered by each of the 3 tap filters and combined to form the upsampled version.
	 *
	 * This is more efficient since we are filtering the data at the original sample rate 10 times with our 3 tap filters
	 * instead of the upsampled rate (10x) once with our 30 tap filter.
	 */
	TRACE("Generating the temporary filter for the sole purpose of stealing the tap values");
	FIRFilter tmpFilter(basebandCmplx, basebandCmplxUpSampled, FIRFilter::lowpass, Real(FILTER_ATTENUATION), Real(FILTER_CUTOFF));

	// This will have 30 filter taps.
	RealArray filterTaps = tmpFilter.getFilterCoefficients();

	polyphaseFilterTaps.resize(10);
	polyphaseFilters.resize(10, NULL);

	TRACE("Parsing tmpFilter taps and creating the 10 polyPhase filters");
	for (int i = 0; i < 10; ++i) {

		std::vector<float> tmpTaps;
		tmpTaps.resize(3);
		for (int ii = 0; ii < 3; ++ii) {
			tmpTaps[ii] = filterTaps[i+ii*10];
		}

		polyphaseFilterTaps[i].resize(tmpTaps.size());
		polyphaseFilterTaps[i] = tmpTaps;
		polyphaseFilters[i] = new FIRFilter(basebandCmplx, basebandCmplx_polyPhaseout, &polyphaseFilterTaps[i][0], tmpTaps.size());

	}

	TRACE("Initialzing the RTL signal struct");
	// Initialize the rds_signal info
	rds_sig_info.bit_pos = BITS_PER_GROUP;

	for (i = 0; i < SAMPLE_BUFFER_SIZE; ++i) {
		rds_sig_info.sample_buffer[i] = 0;
	}

	rds_sig_info.prev_output = 0;
	rds_sig_info.cur_output = 0;
	rds_sig_info.cur_bit = 0;
	rds_sig_info.sample_count = SAMPLES_PER_BIT;
	rds_sig_info.inverting = 0;
	rds_sig_info.phase = 0;
	rds_sig_info.in_sample_index = 0;
	rds_sig_info.out_sample_index = SAMPLE_BUFFER_SIZE-1;
	rds_sig_info.latest_minutes = -1;
	rds_sig_info.state = 0;
	rds_sig_info.ps_state = 0;
	rds_sig_info.rt_state = 0;


	TRACE("Exiting Method");
}

Transmitter::~Transmitter() {
	TRACE("Entered Method");

	TRACE("Deleting polyphase filters");
	// Wait for the thread to join up
	for (int i = 0; i < polyphaseFilters.size(); ++i) {
		if (polyphaseFilters[i]) {
			delete (polyphaseFilters[i]);
		}
	}
	TRACE("Joining up the Transmitter thread");
	m_Thread.join();
	TRACE("Exiting Method");
}


void Transmitter::setTunedFrequency(float tunedFrequency) {
	TRACE("Entered Method");
	TRACE("Setting Tuned Frequency to : " << tunedFrequency);
	TRACE("Song is setup with a center frequency of: " << centerFrequency << " and sample rate of " << MAX_OUTPUT_SAMPLE_RATE);
	this->tunedFrequency = tunedFrequency;

	float normFc = (this->tunedFrequency - centerFrequency) / MAX_OUTPUT_SAMPLE_RATE;



	{
		boost::mutex::scoped_lock lock(tunerMutex);
		TRACE("Therefore this is a normFc of : " << normFc);
		tuner.retune(normFc);
	}

	TRACE("Exited Method");
}

void Transmitter::setFilePath(path filePath) {
	TRACE("Entered Method");
	TRACE("Setting File Path to : " << filePath.string());
	this->filePath = filePath;
	TRACE("Exited Method");
}

path Transmitter::getFilePath() {
	TRACE("Entered Method");
	return this->filePath;
	TRACE("Exited Method");
}

void Transmitter::setRdsFullText(std::string rdsFullText) {
	TRACE("Entered Method");
	TRACE("Setting RDS Full Text to: " << rdsFullText);
	this->rdsFullText = rdsFullText;
	TRACE("Exited Method");
}

void Transmitter::setRdsShortText(std::string rdsShortText) {
	TRACE("Entered Method");
	TRACE("Setting RDS Short Text to: " << rdsShortText);
	this->rdsShortText = rdsShortText;
	TRACE("Exited Method");
}

void Transmitter::setRdsCallSign(std::string rdsCallSign) {
	TRACE("Entered Method");

	TRACE("Converting call-sign to upper-case");
	std::transform(rdsCallSign.begin(), rdsCallSign.end(),rdsCallSign.begin(), ::toupper);

	TRACE("Setting RDS Call Sign Text to: " << rdsCallSign);
	this->rdsCallSign = rdsCallSign;

	TRACE("Exited Method");
}

void Transmitter::start() {
	TRACE("Entered Method");

	if (not initilized) {
		ERROR("Transmitter asked to start but has not been initialized!  Request ignored.");
	} else {
		TRACE("Starting boost thread");
		m_Thread = boost::thread(&Transmitter::doWork, this);
	}

	TRACE("Exited Method");
}


void Transmitter::join() {
	TRACE("Entered Method");
	TRACE("joining thread");
	this->m_Thread.join();
	TRACE("Exited Method");
}

int Transmitter::init(float centerFrequency, int numSamples) {
	TRACE("Entered Method");
	this->numSamples = numSamples;
	this->centerFrequency = centerFrequency;


	TRACE("Initializing RDS struct");

	if (rdsCallSign.length() > 4) {
		WARN("Call Sign must be 4 characters.  Truncating " << rdsCallSign);
		rdsCallSign.resize(4);
	} else if (rdsCallSign.length() < 4) {
		WARN("Call Sign must be 4 characters.  Prepending with Ws to: " << rdsCallSign);
		while (rdsCallSign.length() < 4) {
			rdsCallSign.insert(0, "W");
		}
	}

	if (rdsShortText.length() > 8) {
		WARN("RDS Short Text cannot be longer than 8 characters.  Truncating");
		rdsShortText.resize(8);
	}

	rds_content.pi = callSignToInt(rdsCallSign);
	set_rds_ps(const_cast<char *> (rdsShortText.c_str()), &rds_content);
	set_rds_rt(const_cast<char *> (rdsFullText.c_str()), &rds_content);

	if (filePath == "") {
		ERROR("File Path to wav file has not been set!");
		return -1;
	}

	TRACE("Opening fm mpx file for " << filePath.string());
    if(fm_mpx_open(const_cast<char *> (filePath.string().c_str()), numSamples, &fm_mpx_status_struct) != 0) {
        ERROR("Could not setup FM mulitplex generator.");
        return -1;
    }

    TRACE("Clearing MPX and output vector buffer and resizing for " << numSamples << " samples");
    mpx_buffer.resize(numSamples, 0);

    basebandCmplx.resize(numSamples, std::complex<float>(0.0,0.0));
    basebandCmplx_polyPhaseout.resize(numSamples, std::complex<float>(0.0,0.0));

    basebandCmplxUpSampled.resize(numSamples*10, std::complex<float>(0.0,0.0));
    basebandCmplxUpSampledTuned.resize(numSamples*10, std::complex<float>(0.0,0.0));
    initilized = true;
	TRACE("Exited Method");
    return 0;
}


/**
 * The data returned is coming in at 228000 from the fm / rds library.  If we return 1000 samples per call
 * then we need to be called 228 times a second.
 * Here is what we need to do each pass
 * 1. Use the PiRds library to take in the file and prep for FM modulation (Add RDS & pilot tone & separate the stereo etc)
 * 2. FM Modulate using the GNURadio implementation of FM modulation.
 * 3. Upsample to a higher sample rate so we can have higher bandwidth using the method from http://www.dspguru.com/dsp/faqs/multirate/interpolation
 * 4. Frequency shift up to he appropriate location based on our current tuned frequency and the location of our station.
 */
int Transmitter::doWork() {
	TRACE("Entered Method");

	// Only do work if our frequency is within the bandwidth of the tuner.
	TRACE("Checking if there is any reason to do work.");
	if (abs(centerFrequency - tunedFrequency) > 0.5 * MAX_OUTPUT_SAMPLE_RATE) {
		TRACE("Transmitter is not in tuned range.  Returning all zeros.");
		basebandCmplxUpSampledTuned.resize(numSamples*10, std::complex<float>(0.0,0.0));
		return 0;
	} else {

		TRACE("Receiving samples from fm_mpx_get_samples() for file: ");
		if( fm_mpx_get_samples(&mpx_buffer[0], &rds_content, &rds_sig_info, &fm_mpx_status_struct) < 0 ) {
			ERROR("Error occurred adding RDS data to sound file.");
			return -1;
		}


		TRACE("Scaling samples");
		mpx_buffer /= 10.;

		TRACE("FM Modulating the real data");
		fm.modulate(mpx_buffer, basebandCmplx);

		TRACE("Polyphase filtering for upsampling");
		for (int i = 0; i < 10; ++i) {
			polyphaseFilters[i]->run();
			for (int ii = 0; ii < basebandCmplx_polyPhaseout.size(); ++ii) {
				basebandCmplxUpSampled[i + 10*ii] = basebandCmplx_polyPhaseout[ii];
			}
		}


		{
			boost::mutex::scoped_lock lock(tunerMutex);
			TRACE("Tuning to the relative frequency");
			tuner.run();
		}

		TRACE("Exited Method");
		return 0;
	}
}


std::valarray< std::complex<float> >& Transmitter::getData() {
	TRACE("Entered Method");
	TRACE("Returning complex float vector of size: " << basebandCmplxUpSampledTuned.size());
	TRACE("Exited Method");
	return basebandCmplxUpSampledTuned;
}

// Algorithm from: www.w9wi.com/articles/rdsreverse.htm
unsigned int Transmitter::callSignToInt(std::string callSign) {
	TRACE("Entered Method");
	unsigned int retVal = 0x1234;

	if (callSign.length() != 4) {
		ERROR("Expected callsign to be 4 characters.");
	} else {
		unsigned int letter1 = callSign[0] - 65;
		unsigned int letter2 = callSign[1] - 65;
		unsigned int letter3 = callSign[2] - 65;
		unsigned int letter4 = callSign[3] - 65;

		retVal = (26*26*letter2) + (26*letter3) + letter4;

		if (letter1 == 10) {
			retVal = retVal + 4096;
		} else {
			retVal += 21672;
		}
	}

	TRACE("Exited Method");
	return retVal;
}

std::ostream& operator<<(std::ostream &strm, const Transmitter &tx) {
  return strm << std::endl
		  << "File Name: " << tx.filePath << std::endl
		  << "Center Frequency: " << tx.centerFrequency << std::endl
		  << "RDS Call Sign: " << tx.rdsCallSign << std::endl
		  << "RDS Short text: " << tx.rdsShortText << std::endl
		  << "RDS Full text: " << tx.rdsFullText << std::endl;
}
