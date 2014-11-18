#ifndef libdigitizersim_H
#define libdigitizersim_H

#include "boost/filesystem.hpp"
#include "Transmitter.h"
#include <iostream>
#include <stdio.h>
#include "tinyxml.h"
#include "SimDefaults.h"

using namespace boost::filesystem;

class DigitizerSimulator {
public:
	void print_hello ();

	/**
	 * Initialize the simulator by passing it a boost filesystem path containing
	 * the XML and wav files to be used in the simulation.
	 * Returns 0 on success, -1 on failure.
	 */
	int init(path path);

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

	/**
	 * Gets a buffer of samples from the device.
	 * The size of the buffer maxs out at 2 Million samples.  The minimum
	 */
	int getSamples(std::vector<float> &samples);

private:
	int loadCfgFile(path filPath);
	std::vector<Transmitter> transmitters;
};

#endif
