/*
 * Transmitter.cpp
 *
 *  Created on: Nov 17, 2014
 */

#include "Transmitter.h"

extern "C" {
#include "rds.h"
#include "fm_mpx.h"
}


Transmitter::Transmitter() {
	this->centerFrequency = -1;
	this->rdsText = "REDHAWK Radio";
}

Transmitter::~Transmitter() {
}


void Transmitter::setCenterFrequency(float centerFreqeuncy) {
	this->centerFrequency = centerFreqeuncy;
}

void Transmitter::setFilePath(path filePath) {
	this->filePath = filePath;
}

void Transmitter::setRdsText(std::string rdsText) {
	this->rdsText = rdsText;
}

void Transmitter::start(int numSamples) {
	this->m_Thread = boost::thread(&Transmitter::doWork, this);
}


void Transmitter::join() {
	this->m_Thread.join();
}

int Transmitter::init(int numSamples) {
	this->numSamples = numSamples;

	set_rds_pi(0x1234);
	set_rds_ps(const_cast<char *> (rdsText.c_str()));
	set_rds_rt(const_cast<char *> (rdsText.c_str()));

    if(fm_mpx_open(const_cast<char *> (filePath.string().c_str()), this->numSamples) != 0) {
        std::cerr << "Could not setup FM mulitplex generator." << std::endl;
        return -1;
    }

    this->mpx_buffer.clear();
    this->mpx_buffer.resize(numSamples);

    return 0;
}


/**
 * Here is what we need to do each pass
 * 1. Bring in the audio
 * 2. Add the RDS to the audio
 * 3. Add the pilot tone
 */
int Transmitter::doWork() {
	if( fm_mpx_get_samples(&mpx_buffer[0]) < 0 ) {
		std::cerr << "Error occurred adding RDS data to sound file." << std::endl;
		return -1;
	}

	// scale samples
	for(int i = 0; i < numSamples; i++) {
		mpx_buffer[i] /= 10.;
	}

    return 0;
}





// From: http://stackoverflow.com/questions/1549930/c-equivalent-of-java-tostring
// Yes, that's right, I had to google that, sad face java developer. :-(
std::ostream& operator<<(std::ostream &strm, const Transmitter &tx) {
  return strm << std::endl
		  << "File Name: " << tx.filePath << std::endl
		  << "Center Frequency" << tx.centerFrequency << std::endl
		  << "RDS String: " << tx.rdsText << std::endl;
}
