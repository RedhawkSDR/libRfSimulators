/*
 * SampleCallback.cpp
 *
 *  Created on: Nov 19, 2014
 */

#include "SampleCallback.h"

SampleCallback::SampleCallback() {
	testFile.open("testFile.raw", std::ios::app | std::ios::out | std::ios::binary);
}

SampleCallback::~SampleCallback() {
	testFile.close();
}


void SampleCallback::dataDelivery(std::valarray< std::complex<float> > samples) {
	gettimeofday(&tp, NULL);
	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000; //get current timestamp in milliseconds
	std::cout << ms;
	std::cout << ": CallbackInterface Received: " << samples.size() << " data points" << std::endl;

	testFile.write((char*)&samples[0], samples.size() * sizeof(samples[0]));
}

