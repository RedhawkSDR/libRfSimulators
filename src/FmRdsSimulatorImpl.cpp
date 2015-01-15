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


using namespace std;

#include "FmRdsSimulatorImpl.h"
#include "boost/bind.hpp"
#include "DigitizerSimLogger.h"
#include "boost/current_function.hpp"
#include "CallbackInterface.h"
#include "SimDefaults.h"
#include "tinyxml.h"

#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include <float.h>


namespace RfSimulators {
// Call back interval is 1000ms / (samplerate / samples per block)
#define CALLBACK_INTERVAL (1000.0/(BASE_SAMPLE_RATE/FILE_INPUT_BLOCK_SIZE))// TODO: Make this configurable.

#define INITIAL_CENTER_FREQ 88500000
#define DEFAULT_QUEUE_SIZE 5


FmRdsSimulatorImpl::FmRdsSimulatorImpl() {
	maxQueueSize = DEFAULT_QUEUE_SIZE;
	stopped = true;
	initialized = false;
	shouldAddNoise = true;

	// Initialize to 0 -> float max, no harm in this.
	minFreq = 0.0;
	maxFreq = FLT_MAX;

	io_service_thread = NULL;
	alarm = NULL;
	userClass = NULL;
	userDataQueue = NULL;
	filter = NULL;

	tunedFreq = INITIAL_CENTER_FREQ;
	gain = 0.0;
	minGain = -100;
	maxGain = 100;
	sampleRate = MAX_OUTPUT_SAMPLE_RATE;
	noiseSigma = 0.1;


	// Initialize our noise vector.  We always use the same noise vector to keep the processing down.
	awgnNoise.resize(MAX_OUTPUT_SAMPLE_RATE);

	fillNoiseArray();

	preFiltArray.resize(OUTPUT_SAMPLES_BLOCK_SIZE, complex<float> (0.0, 0.0));


	// Filter is used for the sample rate conversions
	float cutOff = (0.5*(sampleRate / MAX_OUTPUT_SAMPLE_RATE)); // normalized frequency
	filter = new FIRFilter(preFiltArray, postFiltArray, FIRFilter::lowpass, Real(FILTER_ATTENUATION), Real(cutOff));

	// 0.5 because of cast truncation.
	unsigned int maxSampleRateInt = (unsigned int) (MAX_OUTPUT_SAMPLE_RATE + 0.5);
	int iterator = 2;
	unsigned int tmpSampleRate = maxSampleRateInt;
	availableSampleRates.push_back(tmpSampleRate);

	while (tmpSampleRate >= MIN_OUTPUT_SAMPLE_RATE) {
		if (maxSampleRateInt % iterator == 0) {
			tmpSampleRate = maxSampleRateInt / iterator;

			if (tmpSampleRate >= MIN_OUTPUT_SAMPLE_RATE) {
				availableSampleRates.push_back(tmpSampleRate);
			}
		}
		++iterator;
	}

	std::sort (availableSampleRates.begin(), availableSampleRates.end());

}

FmRdsSimulatorImpl::~FmRdsSimulatorImpl() {
	if (not stopped) {
		stop();
	}
	if (alarm) {
		delete(alarm);
		alarm = NULL;
	}

	for (int i = 0; i < transmitters.size(); ++i) {
		if (transmitters[i]) {
			delete(transmitters[i]);
			transmitters[i] = NULL;
		}
	}

	transmitters.clear();

	if (filter) {
		delete(filter);
		filter = NULL;
	}
}

int FmRdsSimulatorImpl::init(std::string cfgFileDir, CallbackInterface * userClass, LogLevel logLevel) {

	boost::filesystem::path cfgFilePath(cfgFileDir);

	if (not initialized) {

		// TODO: This sets the root logger which is also what redhawk uses so it's a bit of a pain.
		// TODO: Maybe we have a second init method that takes in a logging config so we can just pass the current?
		// Set up a simple configuration that logs on the console.
		BasicConfigurator::configure();

		if (logLevel < 0)
			log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff());

		switch ( logLevel )
		{
		 case FATAL:
			 log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getFatal());
			break;
		 case ERROR:
			 log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getError());
			break;
		 case WARN:
			log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getWarn());
			break;
		 case INFO:
			log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getInfo());
			break;
		 case DEBUG:
			log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getDebug());
			break;
		 case TRACE:
			 log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getTrace());
			 break;
		}

	}
	TRACE("Entered Method");

	if (initialized) {
		WARN("Already initialized.  This call to init will be ignored");
		return -1;
	}

	// Somewhat of a useless check
	if (userClass == NULL) {
		ERROR("User class is NULL!");
		return -1;
	}

	alarm = new boost::asio::deadline_timer(io);

	this->userClass = userClass;
	transmitters.clear();

	directory_iterator end_itr;
	// cycle through the directory and save all the XML configurations.
	for (directory_iterator itr(cfgFilePath); itr != end_itr; ++itr) {
		// If it's not a directory, list it. If you want to list directories too, just remove this check.
		if (is_regular_file(itr->path())) {
			if(itr->path().extension() == ".xml") {
				loadCfgFile(itr->path());
			}
		}
	}

	initialized = true;

	TRACE("Leaving Method");
	return 0;
}


void FmRdsSimulatorImpl::stop() {
	TRACE("Entered Method");

	if (stopped) {
		WARN("FmRdsSimulator already stopped.  Call start before stopping.");
		return;
	}

	TRACE("Stopping the Boost Async service");
	// Stop the Boost Asynchronous Service
	io.stop();


	if (io_service_thread) {
		TRACE("Joining all io service threads");

		// Then destroy the memory for that thread
		// It was running in its own thread so make sure it completes
		io_service_thread->join();

		TRACE("Deleting the io service thread object");
		delete(io_service_thread);
		io_service_thread = NULL;
	}


	if (userDataQueue) {
		TRACE("Shutting down the user data queue");
		// Now shutdown the data queue
		userDataQueue->shutDown();


		TRACE("Deleting the user data queue object");
		delete(userDataQueue);
		userDataQueue = NULL;
	}

	TRACE("Reseting the Boost Async service in preperation for next run");
	io.reset();

	TRACE("Stop completed");
	stopped = true;

	TRACE("Leaving Method");
}

void FmRdsSimulatorImpl::_start() {
	TRACE("Entered Method");
	alarm->expires_from_now(boost::posix_time::milliseconds(CALLBACK_INTERVAL));
	io.run();
	TRACE("Leaving Method");
}

void FmRdsSimulatorImpl::start() {
	TRACE("Entered Method");

	if (not stopped) {
		WARN("FmRdsSimulator already started.  Stop before calling start");
		return;
	}

	TRACE("Binding deadline_timer to dataGrab method");
	alarm->async_wait(boost::bind(&FmRdsSimulatorImpl::dataGrab, this, boost::asio::placeholders::error, alarm));

	TRACE("Creating a data queue object for user data");
	userDataQueue = new UserDataQueue(maxQueueSize, this->userClass);

	TRACE("Running the data handler in new thread");
	// This runs in its own thread and waits for data to arrive.
	userDataQueue->waitForData();

	TRACE("Running the asio io-service in new thread");
	io_service_thread = new boost::thread(boost::bind(&FmRdsSimulatorImpl::_start, this));

	stopped = false;

	TRACE("Leaving Method");
}

void FmRdsSimulatorImpl::dataGrab(const boost::system::error_code& error, boost::asio::deadline_timer* alarm) {
	TRACE("Entered Method");

	TRACE("Checking Timer isn't overdue by a full cycle");
	if ( (alarm->expires_from_now() + boost::posix_time::milliseconds(CALLBACK_INTERVAL)).is_negative() ) {
		//TODO: Should this be a warning or an error?  Or an exception?
		WARN("Data delivery is lagging from real-time.  Consider reducing the number of input files.");
	}

	TRACE("Reseting alarm");
	// Reset timer
	alarm->expires_at(alarm->expires_at() + boost::posix_time::milliseconds(CALLBACK_INTERVAL));
	alarm->async_wait(boost::bind(&FmRdsSimulatorImpl::dataGrab, this, boost::asio::placeholders::error, alarm));

	int i;
	// Kick off all the worker threads
	TRACE("Starting all of the worker threads");
	for (i = 0; i < transmitters.size(); ++i) {
		transmitters[i]->start();
	}

	// Join them back up.
	TRACE("Joining all the worker threads back to the main process");
	for (i = 0; i < transmitters.size(); ++i) {
		transmitters[i]->join();
	}

	// Clear out the old data
	preFiltArray *= 0;

	// Collect the data and add it to the return vector
	TRACE("Collecting data");
	for (i = 0; i < transmitters.size(); ++i) {
		std::valarray< std::complex<float> > &txData = transmitters[i]->getData();
		TRACE("Collected: " << txData.size() << " samples from: " << transmitters[i]->getFilePath());

		if (txData.size() != preFiltArray.size()) {
			WARN("Vector size miss-match on transmitter: " << transmitters[i]->getFilePath().string())
			WARN("Vector size provided: " << txData.size());
		} else {
			TRACE("Combining data with current collection");
			preFiltArray += txData;
		}
	}

	if (shouldAddNoise) {
		{
			boost::mutex::scoped_lock lock(noiseArrayMutex);
			preFiltArray += awgnNoise;
		}
	}

	// So if the max rate was 1,000 and we want a sample rate of 250
	// the puncture rate would be 4, we would keep 1 out of every 4 samples.
	unsigned int punctureRate = MAX_OUTPUT_SAMPLE_RATE/sampleRate;

	{
		boost::mutex::scoped_lock lock(filterMutex);
		filter->run();
	}

	int size = postFiltArray.size()/punctureRate;
	std::valarray<std::complex<float> > retVec = postFiltArray[std::slice(0, size, punctureRate)];

	// Apply gain factor
	float linearGain = powf(10.0, gain/10.0);
	retVec *= linearGain;


	TRACE("Delivering " << retVec.size() << " data points to data queue.");
	userDataQueue->deliverData(retVec);

	TRACE("Leaving Method");
}

int FmRdsSimulatorImpl::loadCfgFile(path filePath) {
	TRACE("Entered Method");

	TiXmlDocument doc(filePath.string());

	TRACE("Loading XML File");
	if(doc.LoadFile()) {
		TRACE("Creating new transmitter object");
		Transmitter * tx = new Transmitter();

	    TiXmlHandle hDoc(&doc);
	    TiXmlElement *pRoot, *pParm;
	    pRoot = NULL;
	    pParm = NULL;

	    TRACE("Getting the TxProps root element of the XML");
	    pRoot = doc.FirstChildElement("TxProps");

	    if (not pRoot) {
			ERROR("Malformed xml file: " << filePath.string());
			TRACE("Leaving Method");
			return -1;
	    }

	    TRACE("Getting the FileName element");
	    pParm = pRoot->FirstChildElement("FileName");
	    if (not pParm) {
	    	ERROR("FileName element is required within file: " << filePath.string());
	    	TRACE("Leaving Method");
	    	delete(tx);
	    	tx = NULL;
	    	return -1;
	    }

	    path filepath = path(filePath.parent_path().string() + "/" + pParm->GetText());

	    TRACE("Deleting the reference to the filepath XML object");
	    // Done with file param;
	    pParm = NULL;

	    TRACE("Checking file exists");
	    if (not exists(filepath)) {
	    	ERROR("Could not locate file: " << filepath.string());
			delete(tx);
			tx = NULL;
			return -1;
	    }

	    TRACE("Setting filepath into transmitter");
		tx->setFilePath(filepath);

		TRACE("Getting Center Frequency element.");
	    pParm = pRoot->FirstChildElement("CenterFrequency");

	    if (not pParm) {
			ERROR("CenterFrequency element is required within file: " << filePath.string());
			TRACE("Leaving Method");
			delete(tx);
			tx = NULL;
			return -1;
	    }

	    float centerFreq = atof(pParm->GetText());

		TiXmlElement *rdsRoot;
		rdsRoot = NULL;

		TRACE("Finding RDS Root element.");
		rdsRoot = pRoot->FirstChildElement("RDS");

		if (rdsRoot) {

			TRACE("Finding CallSign XML element");
			pParm = rdsRoot->FirstChildElement("CallSign");
			if (pParm) {
				TRACE("Setting CallSign on Transmitter object");
				tx->setRdsCallSign(pParm->GetText());
				pParm = NULL;
			} else {
				TRACE("Setting default CallSign element");
				tx->setRdsCallSign(DEFAULT_RDS_CALL_SIGN);
			}

			TRACE("Finding ShortText XML element");
			pParm = rdsRoot->FirstChildElement("ShortText");
			if (pParm) {
				TRACE("Setting ShortText on Transmitter object");
				tx->setRdsShortText(pParm->GetText());
				pParm = NULL;
			} else {
				TRACE("Setting default Short Text element");
				tx->setRdsShortText(DEFAULT_RDS_SHORT_TEXT);
			}

			TRACE("Finding Full Text XML element");
			pParm = rdsRoot->FirstChildElement("FullText");
			if (pParm) {
				TRACE("Setting Full Text on Transmitter object");
				tx->setRdsFullText(pParm->GetText());
				pParm = NULL;
			} else {
				TRACE("Setting default Full Text element");
				tx->setRdsFullText(DEFAULT_RDS_FULL_TEXT);
			}

		} else {
			TRACE("RDS XML root not set, using defaults.");
			tx->setRdsCallSign(DEFAULT_RDS_CALL_SIGN);
			tx->setRdsShortText(DEFAULT_RDS_SHORT_TEXT);
			tx->setRdsFullText(DEFAULT_RDS_FULL_TEXT);
		}

		TRACE("Initializing the Transmitter object");
		if (tx->init(centerFreq, FILE_INPUT_BLOCK_SIZE) != 0) {
			TRACE("Something went wrong.  Deleting the transmitter object");
			delete(tx);
			tx = NULL;
			ERROR("Initialization of transmitter failed!")
			TRACE("Leaving Method");
			return -1;
		}

		TRACE("Setting the tuned frequency of the Transmitter object");
		tx->setTunedFrequency(tunedFreq);

		TRACE("Pushing the transmitter object onto the vector of transmitters");
		transmitters.push_back(tx);
		TRACE("Stored following: " << *tx);

	} else {
		ERROR("Malformed xml file: " << filePath.string());
		TRACE("Leaving Method");
		return -1;
	}

	TRACE("Leaving Method");
	return 0;
}

void FmRdsSimulatorImpl::setQueueSize(unsigned short queueSize) {
	TRACE("Entered Method");
	if (userDataQueue) {
		userDataQueue->setMaxQueueSize(queueSize);
	}

	maxQueueSize = queueSize;

	if(queueSize == 0) {
		WARN("Queue Size has been set to zero.  You will not receive any data");
	}

	TRACE("Leaving Method");
}


void FmRdsSimulatorImpl::setCenterFrequency(float freq) throw(OutOfRangeException){
	TRACE("Entered Method");

	if (freq + (1/2 * sampleRate) > maxFreq || freq - (1/2 * sampleRate) < minFreq) {
		WARN("Frequency out of range");
		throw OutOfRangeException();
	}

	tunedFreq = freq;

	for (int i = 0; i < transmitters.size(); ++i) {
		transmitters[i]->setTunedFrequency(tunedFreq);
	}

	TRACE("Leaving Method");
}

float FmRdsSimulatorImpl::getCenterFrequency() {
	TRACE("Entered Method");
	TRACE("Leaving Method");
	return tunedFreq;
}

void FmRdsSimulatorImpl::setGain(float gain) throw(OutOfRangeException) {
	TRACE("Entered Method");
	if (gain > maxGain || gain < minGain) {
		ERROR("Gain value of: " << gain << " is out of the acceptable range of [" << minGain << ", " << maxGain << "]");
		throw OutOfRangeException();
	}

	INFO("Setting gain to " << gain);
	this->gain = gain;
	TRACE("Leaving Method");
}

float FmRdsSimulatorImpl::getGain() {
	TRACE("Entered Method");
	TRACE("Leaving Method");
	return gain;
}

void FmRdsSimulatorImpl::setSampleRate(unsigned int sampleRate) throw(InvalidValue) {
	TRACE("Entered Method");
	if (sampleRate > MAX_OUTPUT_SAMPLE_RATE) {
		WARN("User requested sample rate of " << sampleRate << " is higher than max: " << MAX_OUTPUT_SAMPLE_RATE);
		INFO("Sample Rate request: " << sampleRate);
		throw InvalidValue();
		return;
	} else if (sampleRate < MIN_OUTPUT_SAMPLE_RATE) {
		WARN("User requested sample rate of " << sampleRate << " is lower than min: " << MIN_OUTPUT_SAMPLE_RATE);
		INFO("Sample Rate request: " << sampleRate);
		throw InvalidValue();
		return;
	}

	std::vector<unsigned int>::iterator closestIterator;
	closestIterator = std::lower_bound(availableSampleRates.begin(), availableSampleRates.end(), sampleRate);

	if (closestIterator == availableSampleRates.end()) {
		ERROR("Did not find sample rate in available list...this should not happen");
		throw InvalidValue();
	}

	unsigned int closestSampleRate = *closestIterator;

	INFO("Setting sample rate to closest available: " << closestSampleRate);

	{
		TRACE("Locking filterMutex")
		boost::mutex::scoped_lock lock(filterMutex);
		TRACE("filterMutex Locked")
		this->sampleRate = closestSampleRate;

		TRACE("Deleting current filter")
		if (filter) {
			delete(filter);
			filter = NULL;
		}

		float cutOff = (0.5*(closestSampleRate / MAX_OUTPUT_SAMPLE_RATE)); // normalized frequency

		TRACE("Creating new filter with cut off of " << cutOff);
		filter = new FIRFilter(preFiltArray, postFiltArray, FIRFilter::lowpass, Real(FILTER_ATTENUATION), cutOff);
	}

	TRACE("Leaving Method");
}

unsigned int FmRdsSimulatorImpl::getSampleRate() {
	TRACE("Entered Method");
	TRACE("Leaving Method");
	return sampleRate;
}

void FmRdsSimulatorImpl::setCenterFrequencyRange(float minFreq, float maxFreq) {
	TRACE("Entered Method");
	this->minFreq= minFreq;
	this->maxFreq = maxFreq;
	TRACE("Leaving Method");
}

void FmRdsSimulatorImpl::setGainRange(float minGain, float maxGain) {
	TRACE("Entered Method");
	this->minGain= minGain;
	this->maxGain = maxGain;
	TRACE("Leaving Method");
}

void FmRdsSimulatorImpl::addNoise(bool shouldAddNoise) {
	TRACE("Entered Method");
	this->shouldAddNoise = shouldAddNoise;
	TRACE("Leaving Method");
}

void FmRdsSimulatorImpl::setNoiseSigma(float noiseSigma) {
	TRACE("Entered Method");
	if (noiseSigma < 0) {
		WARN("Negative standard deviation does not make sense.  Using absolute value.")
	}

	this->noiseSigma = fabs(noiseSigma);
	fillNoiseArray();
	TRACE("Leaving Method");
}

float FmRdsSimulatorImpl::getNoiseSigma() {
	TRACE("Entered Method");
	TRACE("Leaving Method");
	return noiseSigma;
}

void FmRdsSimulatorImpl::fillNoiseArray() {
	TRACE("Entered Method");
	boost::variate_generator<boost::mt19937, boost::normal_distribution<float> > generator(boost::mt19937(time(0)), boost::normal_distribution<float>(0.0, noiseSigma));

	{
		TRACE("Filling awgnNoise array");
		boost::mutex::scoped_lock lock(noiseArrayMutex);
		for (int i = 0; i < awgnNoise.size(); ++i) {
			awgnNoise[i] = std::complex<float>(generator(), generator());
		}
	}
	TRACE("Leaving Method");
}

}
