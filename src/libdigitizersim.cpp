using namespace std;

#include "libdigitizersim.h"
#include "boost/bind.hpp"
#include "DigitizerSimLogger.h"
#include "boost/current_function.hpp"

#define CALLBACK_INTERVAL 250 // TODO: Make this configurable.
#define DATA_SIZE 1024 // TODO: Make this configurable.

DigitizerSimulator::DigitizerSimulator() {

}

DigitizerSimulator::~DigitizerSimulator() {
	if (alarm)
		delete(alarm);
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
	alarm = new boost::asio::deadline_timer(io, boost::posix_time::milliseconds(CALLBACK_INTERVAL));

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


void DigitizerSimulator::print_hello(){
	TRACE("Entered Method");
	printf("Hi Youssef!\n");
	TRACE("Leaving Method");
}


void DigitizerSimulator::stop() {
	TRACE("Entered Method");
	io.stop();
	TRACE("Leaving Method");
}

void DigitizerSimulator::_start() {
	TRACE("Entered Method");
	io.run();
	TRACE("Leaving Method");
}

void DigitizerSimulator::start() {
	TRACE("Entered Method");
	TRACE("Setting the boost asio deadline_timer");

	TRACE("Binding deadline_timer to dataGrab method");
	alarm->async_wait(boost::bind(&DigitizerSimulator::dataGrab, this, boost::asio::placeholders::error, alarm));

	TRACE("Running the asio io-service in new thread");

	// TODO: The internal start method isn't needed.  We should be able to call this classes io.run from this line.
	boost::thread iot(boost::bind(&DigitizerSimulator::_start, this));
	TRACE("Leaving Method");
}

void DigitizerSimulator::dataGrab(const boost::system::error_code& error, boost::asio::deadline_timer* alarm) {
	TRACE("Entered Method");

	TRACE("Reseting alarm");
	// Reset timer
	alarm->expires_at(alarm->expires_at() + boost::posix_time::milliseconds(CALLBACK_INTERVAL));
	alarm->async_wait(boost::bind(&DigitizerSimulator::dataGrab, this, boost::asio::placeholders::error, alarm));

	std::vector<float> retVec;
	retVec.resize(DATA_SIZE, 0.0);

	int i;
	// Kick off all the worker threads
	TRACE("Starting all of the worker threads");
	for (i = 0; i < transmitters.size(); ++i) {
		transmitters[i]->start(DATA_SIZE);
	}

	// Join them back up.
	TRACE("Joining all the worker threads back to the main process");
	for (i = 0; i < transmitters.size(); ++i) {
		transmitters[i]->join();
	}

	// Collect the data and add it to the return vector
	TRACE("Collecting data");
	for (i = 0; i < transmitters.size(); ++i) {
		std::vector<float> txData = transmitters[i]->getData();

		if (txData.size() != retVec.size()) {
			WARN("Vector size miss-match on transmitter: " << transmitters[i]->getFilePath().string())
			WARN("Vector size requested: " << retVec.size());
			WARN("Vector size provided: " << txData.size());
		} else {
			TRACE("Combining data with current collection");
			std::transform(retVec.begin(), retVec.end(), txData.begin(), retVec.begin(), std::plus<float>());
		}

	}

	// While it would appear this should be in its own thread, the dataGrab method is in its own thread that gets respawned
	// so, worst case scenario the users class causes a build up of threads and the system runs out of resources...
	TRACE("Delivering data to user class.");
	userClass->dataDelivery(retVec);

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
	    	return -1;
	    }

	    path filepath = path(filePath.parent_path().string() + "/" + pParm->GetText());

	    if (not exists(filepath)) {
	    	ERROR("Could not locate file: " << filepath.string());
			delete(tx);
			return -1;
	    }

		tx->setFilePath(filepath);

	    pParm = pRoot->FirstChildElement("CenterFrequency");

	    if (not pParm) {
			ERROR("CenterFrequency element is required within file: " << filePath.string());
			TRACE("Leaving Method");
			delete(tx);
			return -1;
	    }

	    tx->setCenterFrequency(atof(pParm->GetText()));

		pParm = pRoot->FirstChildElement("RDS");

		if (pParm)
			tx->setRdsText(pParm->GetText());
		else
			tx->setRdsText(DEFAULT_RDS_TEXT);

		if (tx->init(DATA_SIZE) != 0) {
			ERROR("Initialization of transmitter failed!")
			TRACE("Leaving Method");
			return -1;
		}

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
