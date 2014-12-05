/*
 * RfSimulatorFactory.h
 *
 *  Created on: Dec 5, 2014
 */

#ifndef RFSIMULATORFACTORY_H_
#define RFSIMULATORFACTORY_H_

#include "RfSimulator.h"

namespace RfSimulators {

class RfSimulatorFactory {
public:
	static RfSimulators::RfSimulator* createFmRdsSimulator();
};

} /* namespace RfSimulators */

#endif /* RFSIMULATORFACTORY_H_ */
