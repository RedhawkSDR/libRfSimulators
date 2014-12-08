/*
 * FmRdsSimulator.h
 *
 *  Created on: Dec 5, 2014
 */

#ifndef RFSIMULATOR_H_
#define RFSIMULATOR_H_

#include "CallbackInterface.h"
#include <string.h>
#include "Exceptions.h"

namespace RfSimulators {

enum LogLevel {
	TRACE,
	DEBUG,
	WARN,
	ERROR,
	FATAL,
};


class RfSimulator
{

public:
	virtual int init(std::string cfgFilePath, CallbackInterface * userClass, LogLevel level) = 0;

	virtual void setCenterFrequencyRange(float freqMin, float freqMax) = 0;

	virtual void setGain(float gain) throw(OutOfRangeException) = 0;
	virtual float getGain() = 0;

	virtual void setQueueSize(unsigned short queueSize) = 0;

	virtual void setCenterFrequency(float freq) throw(OutOfRangeException) = 0;
	virtual float getCenterFrequency() = 0;

	virtual void setSampleRate(unsigned int sampleRate) throw(InvalidValue) = 0;
	virtual unsigned int getSampleRate() = 0;

	virtual void start() = 0;
	virtual void stop() = 0;

private:
};

}



#endif /* RFSIMULATOR_H_ */
