/*
 * Transmitter.cpp
 *
 *  Created on: Nov 17, 2014
 */

#include "Transmitter.h"
#include "DigitizerSimLogger.h"
#include <math.h>
#include "SimDefaults.h"

// From: http://gnuradio.org/redmine/projects/gnuradio/wiki/SignalProcessing
// sensitivity = (2 * pi * max_deviation) / samp_rate

Transmitter::Transmitter() :
		fm((2 * M_PI * MAX_FREQUENCY_DEVIATION) / BASE_SAMPLE_RATE),
		tuner(basebandCmplxUpSampled, basebandCmplxUpSampledTuned, 0)
		{

	centerFrequency = -1;
	rdsText = "REDHAWK Radio";
	numSamples = -1;
	initilized = false;

	fm_mpx_status_struct.phase_38 = 0;
	fm_mpx_status_struct.phase_19 = 0;
	fm_mpx_status_struct.audio_index = 0;
	fm_mpx_status_struct.audio_len = 0;
	fm_mpx_status_struct.fir_index = 0;

	unsigned int i;
	for (i = 0; i < FIR_SIZE; i++) {
		fm_mpx_status_struct.fir_buffer_mono[i] = 0;
		fm_mpx_status_struct.fir_buffer_stereo[i] = 0;
	}

	filePath = "";
	tunedFrequency = 0.0;


	/**
	 * This is a bit of a hack currently.
	 * We create a LPF with 30 taps.  Then create ten 3 tap filters from the designed filter taps for our polyphase filter.
	 */


	FIRFilter tmpFilter(basebandCmplx, basebandCmplxUpSampled, FIRFilter::lowpass, Real(FILTER_ATTENUATION), Real(FILTER_CUTOFF));

	// This will have 30 filter taps.
	RealArray filterTaps = tmpFilter.getFilterCoefficients();

	polyphaseFilterTaps.resize(10);
	polyphaseFilters.resize(10);

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
}

Transmitter::~Transmitter() {
	// Wait for the thread to join up
	for (int i = 0; i < polyphaseFilters.size(); ++i) {
		if (polyphaseFilters[i]) {
			delete (polyphaseFilters[i]);
		}
	}

	m_Thread.join();
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

void Transmitter::setRdsText(std::string rdsText) {
	TRACE("Entered Method");
	TRACE("Setting RDS Text to: " << rdsText);
	this->rdsText = rdsText;
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
	rds_status_struct.pi = 0x1234;
	set_rds_ps(const_cast<char *> (rdsText.c_str()), &rds_status_struct);
	set_rds_rt(const_cast<char *> (rdsText.c_str()), &rds_status_struct);

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
 * 3. Upsample to a higher sample rate so we can have higher bandwidth.
 * 4. Frequency shift up to he appropriate location.
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

		TRACE("Receiving samples from fm_mpx_get_samples() for file: " );
		if( fm_mpx_get_samples(&mpx_buffer[0], &rds_status_struct, &fm_mpx_status_struct) < 0 ) {
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


// From: http://stackoverflow.com/questions/1549930/c-equivalent-of-java-tostring
// Yes, that's right, I had to google that, sad face java developer. :-(
std::ostream& operator<<(std::ostream &strm, const Transmitter &tx) {
  return strm << std::endl
		  << "File Name: " << tx.filePath << std::endl
		  << "Center Frequency: " << tx.centerFrequency << std::endl
		  << "RDS String: " << tx.rdsText << std::endl;
}
