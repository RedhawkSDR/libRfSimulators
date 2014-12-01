using namespace std;

#include "libdigitizersim.h"
#include "boost/bind.hpp"
#include "DigitizerSimLogger.h"
#include "boost/current_function.hpp"
#include "CallbackInterface.h"
#include "SimDefaults.h"
#include <complex>
#include "Transmitter.h"


// Call back interval is 1000ms / (samplerate / samples per block)
#define CALLBACK_INTERVAL (1000.0/(BASE_SAMPLE_RATE/FILE_INPUT_BLOCK_SIZE))// TODO: Make this configurable.

#define INITIAL_CENTER_FREQ 88500000
#define DEFAULT_QUEUE_SIZE 5

// This is put here rather than in the header file to prevent the user class from having to know about Transmitter.h
std::vector<Transmitter*> transmitters;

DigitizerSimulator::DigitizerSimulator() {
	maxQueueSize = DEFAULT_QUEUE_SIZE;
	stopped = true;
}

DigitizerSimulator::~DigitizerSimulator() {
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
}

int DigitizerSimulator::init(path cfgFilePath, CallbackInterface * userClass, int logLevel) {
	// Set up a simple configuration that logs on the console.
	BasicConfigurator::configure();

	if (logLevel < 0)
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff());

	switch ( logLevel )
	{
	 case 0:
		 log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getError());
		break;
	 case 1:
		 log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getWarn());
		break;
	 case 2:
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getDebug());
		break;
	 case 3:
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getTrace());
		break;
	}

	if (logLevel > 3)
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getAll());

	TRACE("Entered Method");
	alarm = new boost::asio::deadline_timer(io);

	this->userClass = userClass;
	transmitters.clear();

	directory_iterator end_itr;
	// cycle through the directory and save all the XML configurations.
	for (directory_iterator itr(cfgFilePath); itr != end_itr; ++itr)
	{
		// If it's not a directory, list it. If you want to list directories too, just remove this check.
		if (is_regular_file(itr->path())) {
			if(itr->path().extension() == ".xml") {
				loadCfgFile(itr->path());
			}
		}
	}

	TRACE("Leaving Method");
	return 0;
}


void DigitizerSimulator::stop() {
	TRACE("Entered Method");

	// Stop the Boost Asynchronous Service
	io.stop();

	// It was running in its own thread so make sure it completes
	io_service_thread->join();

	// Then destroy the memory for that thread
	if (io_service_thread) {
		delete(io_service_thread);
		io_service_thread = NULL;
	}

	// Now shutdown the data queue
	userDataQueue->shutDown();

	// And delete it.
	if (userDataQueue) {
		delete(userDataQueue);
		userDataQueue = NULL;
	}

	stopped = true;

	TRACE("Leaving Method");
}

void DigitizerSimulator::_start() {
	TRACE("Entered Method");
	alarm->expires_from_now(boost::posix_time::milliseconds(CALLBACK_INTERVAL));
	io.run();
	TRACE("Leaving Method");
}

void DigitizerSimulator::start() {
	TRACE("Entered Method");
	TRACE("Setting the boost asio deadline_timer");

	TRACE("Binding deadline_timer to dataGrab method");
	alarm->async_wait(boost::bind(&DigitizerSimulator::dataGrab, this, boost::asio::placeholders::error, alarm));

	TRACE("Running the asio io-service in new thread");

	userDataQueue = new UserDataQueue(maxQueueSize, this->userClass);

	// This runs in its own thread and waits for data to arrive.
	userDataQueue->waitForData();

	// TODO: The internal start method isn't needed.  We should be able to call this classes io.run from this line.
	io_service_thread = new boost::thread(boost::bind(&DigitizerSimulator::_start, this));

	stopped = false;
	TRACE("Leaving Method");

}

void DigitizerSimulator::dataGrab(const boost::system::error_code& error, boost::asio::deadline_timer* alarm) {
	TRACE("Entered Method");

	TRACE("Checking Timer isn't overdue");
	if ( (alarm->expires_from_now() + boost::posix_time::milliseconds(CALLBACK_INTERVAL)).is_negative() ) {
		//TODO: Should this be a warning or an error?  Or an exception?
		WARN("Data delivery is lagging from real-time.  Consider reducing the number of input files.");
	}

	TRACE("Reseting alarm");
	// Reset timer
	alarm->expires_at(alarm->expires_at() + boost::posix_time::milliseconds(CALLBACK_INTERVAL));
	alarm->async_wait(boost::bind(&DigitizerSimulator::dataGrab, this, boost::asio::placeholders::error, alarm));

	std::valarray< complex<float> > retVec;
	retVec.resize(OUTPUT_SAMPLES_BLOCK_SIZE, complex<float> (0.0, 0.0));

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

	// Collect the data and add it to the return vector
	TRACE("Collecting data");
	for (i = 0; i < transmitters.size(); ++i) {
		std::valarray< std::complex<float> > txData = transmitters[i]->getData();

		if (txData.size() != retVec.size()) {
			WARN("Vector size miss-match on transmitter: " << transmitters[i]->getFilePath().string())
			WARN("Vector size requested: " << retVec.size());
			WARN("Vector size provided: " << txData.size());
		} else {
			TRACE("Combining data with current collection");

			retVec = txData + retVec;
		}

	}

	TRACE("Delivering data to user class.");
	userDataQueue->deliverData(retVec);

	TRACE("Leaving Method");
}

int DigitizerSimulator::loadCfgFile(path filePath) {
	TRACE("Entered Method");

	TiXmlDocument doc(filePath.string());
	if(doc.LoadFile())
	{
		Transmitter * tx = new Transmitter();

	    TiXmlHandle hDoc(&doc);
	    TiXmlElement *pRoot, *pParm;
	    pRoot = doc.FirstChildElement("TxProps");


	    pParm = pRoot->FirstChildElement("FileName");
	    if (not pParm) {
	    	ERROR("FileName element is required within file: " << filePath.string());
	    	TRACE("Leaving Method");
	    	delete(tx);
	    	tx = NULL;
	    	return -1;
	    }

	    path filepath = path(filePath.parent_path().string() + "/" + pParm->GetText());

	    if (not exists(filepath)) {
	    	ERROR("Could not locate file: " << filepath.string());
			delete(tx);
			tx = NULL;
			return -1;
	    }

		tx->setFilePath(filepath);

	    pParm = pRoot->FirstChildElement("CenterFrequency");

	    if (not pParm) {
			ERROR("CenterFrequency element is required within file: " << filePath.string());
			TRACE("Leaving Method");
			delete(tx);
			tx = NULL;
			return -1;
	    }

	    float centerFreq = atof(pParm->GetText());

		pParm = pRoot->FirstChildElement("RDS");

		if (pParm)
			tx->setRdsText(pParm->GetText());
		else
			tx->setRdsText(DEFAULT_RDS_TEXT);

		if (tx->init(centerFreq, FILE_INPUT_BLOCK_SIZE) != 0) {
			ERROR("Initialization of transmitter failed!")
			TRACE("Leaving Method");
			return -1;
		}

		tx->setTunedFrequency(INITIAL_CENTER_FREQ);

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

void DigitizerSimulator::setQueueSize(unsigned short queueSize) {
	if (userDataQueue)
		userDataQueue->setMaxQueueSize(queueSize);

	maxQueueSize = queueSize;

	if(queueSize == 0) {
		WARN("Queue Size has been set to zero.  You will not receive any data");
	}
}
