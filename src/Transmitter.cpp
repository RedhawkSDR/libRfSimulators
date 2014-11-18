/*
 * Transmitter.cpp
 *
 *  Created on: Nov 17, 2014
 */

#include "Transmitter.h"


Transmitter::Transmitter() {
	this->centerFrequency = -1;
	this->rdsText = "REDHAWK Radio";
	this->numSamples = -1;

	this->fm_mpx_status_struct.phase_38 = 0;
	this->fm_mpx_status_struct.phase_19 = 0;
	this->fm_mpx_status_struct.audio_index = 0;
	this->fm_mpx_status_struct.audio_len = 0;
	this->fm_mpx_status_struct.fir_index = 0;

	unsigned int i;
	for (i = 0; i < FIR_SIZE; i++) {
		this->fm_mpx_status_struct.fir_buffer_mono[i] = 0;
		this->fm_mpx_status_struct.fir_buffer_stereo[i] = 0;
	}

}

Transmitter::~Transmitter() {
}


void Transmitter::setCenterFrequency(float centerFreqeuncy) {
	this->centerFrequency = centerFreqeuncy;
}

void Transmitter::setFilePath(path filePath) {
	this->filePath = filePath;
}

path Transmitter::getFilePath() {
	return this->filePath;
}

void Transmitter::setRdsText(std::string rdsText) {
	this->rdsText = rdsText;
}

/**
 * TODO: This does not seem ideal.  The creation of a thread is time consuming and resource heavy.  We should have the thread in a wait state
 * and then on start it takes off.
 */
void Transmitter::start(int numSamples) {
	m_Thread = boost::thread(&Transmitter::doWork, this);
}


void Transmitter::join() {
	this->m_Thread.join();
}

int Transmitter::init(int numSamples) {
	this->numSamples = numSamples;

	rds_status_struct.pi = 0x1234;
	set_rds_ps(const_cast<char *> (rdsText.c_str()), &rds_status_struct);
	set_rds_rt(const_cast<char *> (rdsText.c_str()), &rds_status_struct);


    if(fm_mpx_open(const_cast<char *> (filePath.string().c_str()), numSamples, &fm_mpx_status_struct) != 0) {
        std::cerr << "Could not setup FM mulitplex generator." << std::endl;
        return -1;
    }

    mpx_buffer.clear();
    mpx_buffer.resize(numSamples);

    return 0;
}


/**
 * Here is what we need to do each pass
 * 1. Bring in the audio
 * 2. Add the RDS to the audio
 * 3. Add the pilot tone
 */
int Transmitter::doWork() {
	if( fm_mpx_get_samples(&mpx_buffer[0], &rds_status_struct, &fm_mpx_status_struct) < 0 ) {
		std::cerr << "Error occurred adding RDS data to sound file." << std::endl;
		return -1;
	}

	// scale samples
	for(int i = 0; i < numSamples; i++) {
		mpx_buffer[i] /= 10.;
	}

    return 0;
}


std::vector<float>& Transmitter::getData() {
	return mpx_buffer;
}


// From: http://stackoverflow.com/questions/1549930/c-equivalent-of-java-tostring
// Yes, that's right, I had to google that, sad face java developer. :-(
std::ostream& operator<<(std::ostream &strm, const Transmitter &tx) {
  return strm << std::endl
		  << "File Name: " << tx.filePath << std::endl
		  << "Center Frequency: " << tx.centerFrequency << std::endl
		  << "RDS String: " << tx.rdsText << std::endl;
}
