/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK librfsimulators.
 *
 * REDHAWK librfsimulators is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK librfsimulators is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
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
	INFO,
	WARN,
	ERROR,
	FATAL,
};


class RfSimulator
{

public:
	virtual int init(std::string cfgFilePath, CallbackInterface * userClass, LogLevel level) = 0;

	virtual void setCenterFrequencyRange(float freqMin, float freqMax) = 0;
	virtual void setGainRange(float freqMin, float freqMax) = 0;

	virtual void setGain(float gain) throw(OutOfRangeException) = 0;
	virtual float getGain() = 0;

	virtual void setQueueSize(unsigned short queueSize) = 0;

	virtual void setCenterFrequency(float freq) throw(OutOfRangeException) = 0;
	virtual float getCenterFrequency() = 0;

	virtual void setSampleRate(unsigned int sampleRate) throw(InvalidValue) = 0;
	virtual unsigned int getSampleRate() = 0;

	virtual void addNoise(bool addNoise) = 0;
	virtual void setNoiseSigma(float sigma) = 0;
	virtual float getNoiseSigma() = 0;

	virtual void start() = 0;
	virtual void stop() = 0;

private:
};

}



#endif /* RFSIMULATOR_H_ */
