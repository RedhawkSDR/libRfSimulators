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


using namespace boost::filesystem;
using namespace boost;

class Transmitter {
public:
	Transmitter();
	void setCenterFrequency(float centerFreqeuncy);
	void setFilePath(path filePath);
	void setRdsText(std::string rdsText);
	virtual ~Transmitter();
	friend std::ostream& operator<<(std::ostream &strm, const Transmitter &tx);
	void start(int numSamples);
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

};

#endif /* TRANSMITTER_H_ */
