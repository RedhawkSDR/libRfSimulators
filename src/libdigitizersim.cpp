using namespace std;

#include "libdigitizersim.h"
#include "boost/bind.hpp"

#define CALLBACK_INTERVAL 250 // TODO: Make this configurable.
#define DATA_SIZE 1024 // TODO: Make this configurable.

int DigitizerSimulator::init(path cfgFilePath, CallbackInterface * userClass) {
	this->userClass = userClass;
//	transmitters.clear();

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
	return 0;
}


void DigitizerSimulator::print_hello(){
  printf("Hi Youssef!\n");
}


void DigitizerSimulator::start() {
	std::cout << "Entered start method" << std::endl;
	boost::asio::io_service io;
	std::cout << "Setting alarm" << std::endl;
	boost::asio::deadline_timer alarm(io, boost::posix_time::milliseconds(CALLBACK_INTERVAL));

	std::cout << "Binding alarm" << std::endl;
	alarm.async_wait(boost::bind(&DigitizerSimulator::dataGrab, this, boost::asio::placeholders::error, &alarm));

	std::cout << "Running the io_service" << std::endl;
	io.run();
}

void DigitizerSimulator::dataGrab(const boost::system::error_code& error, boost::asio::deadline_timer* alarm) {
	std::cout << "Entered the dataGrab method" << std::endl;

	std::cout << "Reseting alarm" << std::endl;
	// Reset timer
	alarm->expires_at(alarm->expires_at() + boost::posix_time::milliseconds(CALLBACK_INTERVAL));

	std::vector<float> retVec;
	retVec.resize(DATA_SIZE, 0.0);

	int i;
	// Kick off all the worker threads
	std::cout << "Starting all of the worker threads" << std::endl;
	for (i = 0; i < transmitters.size(); ++i) {
		transmitters[i]->start(DATA_SIZE);
	}

	// Join them back up.
	std::cout << "Joining all the worker threads back to the main process" << std::endl;
	for (i = 0; i < transmitters.size(); ++i) {
		transmitters[i]->join();
	}

	// Collect the data and add it to the return vector
	std::cout << "Collecting data" << std::endl;
	for (i = 0; i < transmitters.size(); ++i) {
		std::cout << "Retrieving datapoints: " << transmitters[i]->getData().size() << std::endl;
		std::vector<float> txData = transmitters[i]->getData();

		if (txData.size() != retVec.size()) {
			std::cerr << "Vector size missmatch on transmitter: " << transmitters[i]->getFilePath().string() << std::endl;
		} else {
			std::transform(retVec.begin(), retVec.end(), txData.begin(), retVec.begin(), std::plus<float>());
		}

	}

	std::cout << "Calling the users data delivery method" << std::endl;
	userClass->dataDelivery(retVec);
}

int DigitizerSimulator::loadCfgFile(path filePath) {

	TiXmlDocument doc(filePath.string());
	if(doc.LoadFile())
	{
		Transmitter * tx = new Transmitter();

	    TiXmlHandle hDoc(&doc);
	    TiXmlElement *pRoot, *pParm;
	    pRoot = doc.FirstChildElement("TxProps");


	    pParm = pRoot->FirstChildElement("FileName");
	    if (not pParm) {
	    	std::cerr << "FileName element is required within file: " << filePath.string() << std::endl;
	    	delete(tx);
	    	return -1;
	    }

	    path filepath = path(filePath.parent_path().string() + "/" + pParm->GetText());

	    if (not exists(filepath)) {
	    	std::cerr << "Could not locate file: " << filepath.string() << std::endl;
			delete(tx);
			return -1;
	    }

		tx->setFilePath(filepath);

	    pParm = pRoot->FirstChildElement("CenterFrequency");

	    if (not pParm) {
			std::cerr << "CenterFrequency element is required within file: " << filePath.string() << std::endl;
			delete(tx);
			return -1;
	    }

	    tx->setCenterFrequency(atof(pParm->GetText()));

		pParm = pRoot->FirstChildElement("RDS");

		if (pParm)
			tx->setRdsText(pParm->GetText());
		else
			tx->setRdsText(DEFAULT_RDS_TEXT);

		transmitters.push_back(tx);

		std::cout << "Read XML File" << std::endl;
		std::cout << *tx << std::endl;


	} else {
		std::cerr << "Malformed xml file: " << filePath.string() << std::endl;
		return -1;
	}


	return 0;
}
