using namespace std;

#include "libdigitizersim.h"

int DigitizerSimulator::init(boost::filesystem::path path) {

	directory_iterator end_itr;
	// cycle through the directory and save all the XML configurations.
	for (directory_iterator itr(path); itr != end_itr; ++itr)
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



int DigitizerSimulator::loadCfgFile(path filePath) {
	Transmitter tx;
	TiXmlDocument doc(filePath.string());
	if(doc.LoadFile())
	{
	    TiXmlHandle hDoc(&doc);
	    TiXmlElement *pRoot, *pParm;
	    pRoot = doc.FirstChildElement("TxProps");


	    pParm = pRoot->FirstChildElement("FileName");
	    if (not pParm) {
	    	std::cerr << "FileName element is required within file: " << filePath.string() << std::endl;
	    	return -1;
	    }

		tx.setFilePath(path(pParm->GetText()));

	    pParm = pRoot->FirstChildElement("CenterFrequency");

	    if (not pParm) {
			std::cerr << "CenterFrequency element is required within file: " << filePath.string() << std::endl;
			return -1;
	    }

	    tx.setCenterFrequency(atof(pParm->GetText()));

		pParm = pRoot->FirstChildElement("RDS");

		if (pParm)
			tx.setRdsText(pParm->GetText());
		else
			tx.setRdsText(DEFAULT_RDS_TEXT);

		std::cout << "Read XML File" << std::endl;
		std::cout << tx << std::endl;


	} else {
		std::cerr << "Malformed xml file: " << filePath.string() << std::endl;
		return -1;
	}


	return 0;
}
