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
