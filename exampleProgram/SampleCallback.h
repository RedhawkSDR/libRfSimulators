/*
 * SampleCallback.h
 *
 *  Created on: Nov 19, 2014
 */

#ifndef LIBFMRDSSIMULATOR_EXAMPLEPROGRAM_SAMPLECALLBACK_H_
#define LIBFMRDSSIMULATOR_EXAMPLEPROGRAM_SAMPLECALLBACK_H_

#include "CallbackInterface.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <sys/time.h>
#include <complex>

using namespace RfSimulators;

class SampleCallback: public CallbackInterface {
public:
	SampleCallback();
	virtual ~SampleCallback();
	void dataDelivery(std::valarray< std::complex<float> > samples);

private:
	std::ofstream testFile;
	struct timeval tp;
};

#endif /* LIBFMRDSSIMULATOR_EXAMPLEPROGRAM_SAMPLECALLBACK_H_ */
