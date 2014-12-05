/*
 * RfSimulatorFactory.cpp
 *
 *  Created on: Dec 5, 2014
 */

#include "RfSimulatorFactory.h"
#include "FmRdsSimulatorImpl.h"

using namespace RfSimulators;


namespace RfSimulators {

RfSimulator* RfSimulatorFactory::createFmRdsSimulator() {
	return new FmRdsSimulatorImpl();
}


} /* namespace RfSimulators */
