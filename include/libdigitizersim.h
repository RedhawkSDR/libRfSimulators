#ifndef libdigitizersim_H
#define libdigitizersim_H

#include "Transmitter.h"
#include "boost/filesystem.hpp"
#include <iostream>
#include <stdio.h>
#include "tinyxml.h"
#include "SimDefaults.h"
#include "CallbackInterface.h"
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::filesystem;

class DigitizerSimulator {
public:
	void print_hello ();

	/**
	 * Initializes the simulator.
	 * Input:
	 *  cfgFilePath - The path to a folder on the system that contains the XML configuration files as well as the wav files
	 *  userClass - A pointer to the class which implements the CallbackInterface used when data is available.
	 * Returns 0 on success, -1 on failure.
	 */
	int init(path cfgFilePath, CallbackInterface * userClass);

	/**
	 * Takes in a gain value, 0-11.  Units are in dB.
	 * Returns 0 on success.  -1 on failure.
	 */
	int setGain(unsigned short gain);

	/**
	 * Sets the center frequency between 80Mhz and 110Mhz.
	 * Units are in Mhz.
	 * Returns 0 on success, -1 on failure.
	 */
	int setCenterFrequency(float freq);

	/**
	 * Sets the sample rate.
	 * Sample rate range is between 10kHz and 2Mhz
	 * Returns 0 on success, -1 on failure.
	 */
	int setSampleRate(float rate);

	void connectCallback(CallbackInterface * userClass);

	void start();

private:
	int loadCfgFile(path filPath);
	std::vector<Transmitter*> transmitters;
	CallbackInterface *userClass;
	void dataGrab(const boost::system::error_code& error, boost::asio::deadline_timer* alarm);
};

#endif
