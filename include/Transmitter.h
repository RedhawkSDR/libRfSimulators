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

extern "C" {
#include "rds.h"
#include "fm_mpx.h"
}

using namespace boost::filesystem;
using namespace boost;

class Transmitter {
public:
	Transmitter();
	void setCenterFrequency(float centerFreqeuncy);
	void setFilePath(path filePath);
	path getFilePath();
	void setRdsText(std::string rdsText);
	virtual ~Transmitter();
	std::vector< std::complex<float> >& getData();
	friend std::ostream& operator<<(std::ostream &strm, const Transmitter &tx);
	void start();
	void join();
	int init(int numSamples);

private:
	float centerFrequency;
	path filePath;
	std::string rdsText;
	thread m_Thread;
	int numSamples;
	int doWork();
	std::vector<float> mpx_buffer;
	std::vector< std::complex<float> > output_buffer;

	rds_struct rds_status_struct;
	fm_mpx_struct fm_mpx_status_struct;
	bool initilized;
	FrequencyModulator fm;
};

#endif /* TRANSMITTER_H_ */
