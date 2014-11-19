/*
 * SampleCallback.h
 *
 *  Created on: Nov 19, 2014
 */

#ifndef SAMPLECALLBACK_H_
#define SAMPLECALLBACK_H_

#include "CallbackInterface.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <sys/time.h>

class SampleCallback: public CallbackInterface {
public:
	SampleCallback();
	virtual ~SampleCallback();
	void dataDelivery(std::vector<float> samples);

private:
	std::ofstream testFile;
	struct timeval tp;
};

#endif /* SAMPLECALLBACK_H_ */
