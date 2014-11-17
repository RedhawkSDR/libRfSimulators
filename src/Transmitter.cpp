/*
 * Transmitter.cpp
 *
 *  Created on: Nov 17, 2014
 */

#include "Transmitter.h"

Transmitter::Transmitter() {
	this->centerFrequency = -1;
	this->rdsText = "";
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


// From: http://stackoverflow.com/questions/1549930/c-equivalent-of-java-tostring
std::ostream& operator<<(std::ostream &strm, const Transmitter &tx) {
  return strm << std::endl
		  << "File Name: " << tx.filePath << std::endl
		  << "Center Frequency" << tx.centerFrequency << std::endl
		  << "RDS String: " << tx.rdsText << std::endl;
}
