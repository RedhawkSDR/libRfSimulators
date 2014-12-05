#ifndef fmrdssimulatorimpl_H
#define fmrdssimulatorimpl_H

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <stdio.h>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <queue>
#include <complex>
#include "RfSimulator.h"

#include "CallbackInterface.h"

using namespace boost::filesystem;

namespace RfSimulators {

class FmRdsSimulatorImpl : public RfSimulator {
public:
	FmRdsSimulatorImpl();
	~FmRdsSimulatorImpl();

	/**
	 * Initializes the simulator.
	 * Input:
	 *  cfgFilePath - The path to a folder on the system that contains the XML configuration files as well as the wav files
	 *  userClass - A pointer to the class which implements the CallbackInterface used when data is available.
	 *  logLevel - The logging level of the library. Set to -1 to turn off, 0 for ERROR, 1 for WARN, 2 for DEBUG, 3 for TRACE.
	 * Returns 0 on success, -1 on failure.
	 */
	int init(std::string cfgFilePath, CallbackInterface * userClass, LogLevel logLevel);

	void setGain(float gain);
	float getGain();

	/**
	 * Set the size of the data queue that can build up if users do not
	 * service the callback fast enough.
	 */
	void setQueueSize(unsigned short queueSize);


	void setCenterFrequency(float freq);
	float getCenterFrequency();

	/**
	 * Sets the sample rate.
	 * Sample rate is limited to an even division of the max sample rate of 2,228,000 Samples per second
	 * Sample rate must be more than the minimum sample rate of 2,228.
	 */
	void setSampleRate(unsigned int sampleRate);
	unsigned int getSampleRate();

	void start();

	void stop();

private:
	int loadCfgFile(path filPath);
	CallbackInterface *userClass;
	unsigned int maxQueueSize;

	void dataGrab(const boost::system::error_code& error, boost::asio::deadline_timer* alarm);
	boost::asio::io_service io;
	boost::asio::deadline_timer * alarm;
	void _start();
	boost::thread *io_service_thread;
	bool stopped, initialized;
	float tunedFreq;
	float gain;
	unsigned int sampleRate;
	std::valarray<std::complex<float> > awgnNoise;
	std::valarray<std::complex<float> > postFiltArray, preFiltArray;

	boost::mutex filterMutex;

};
} // End of namespace
#endif
