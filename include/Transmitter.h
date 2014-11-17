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


using namespace boost::filesystem;

class Transmitter {
public:
	Transmitter();
	void setCenterFrequency(float centerFreqeuncy);
	void setFilePath(path filePath);
	void setRdsText(std::string rdsText);
	virtual ~Transmitter();
	friend std::ostream& operator<<(std::ostream &strm, const Transmitter &tx);

private:
	float centerFrequency;
	path filePath;
	std::string rdsText;

};

#endif /* TRANSMITTER_H_ */
