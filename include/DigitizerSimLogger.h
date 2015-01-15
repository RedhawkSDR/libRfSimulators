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
 * DigitizerSimLogger.h
 *
 *  Created on: Nov 19, 2014
 */

#ifndef LIBFMRDSSIMULATOR_INCLUDE_DIGITIZERSIMLOGGER_H_
#define LIBFMRDSSIMULATOR_INCLUDE_DIGITIZERSIMLOGGER_H_
// include log4cxx header files.
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include <string.h>
#include <iostream>

using namespace log4cxx;
using namespace log4cxx::helpers;

static LoggerPtr logger(Logger::getLogger("DigitizerSim"));

#define TRACE(msg) LOG4CXX_TRACE(logger, std::string(BOOST_CURRENT_FUNCTION) + ":\t" + msg);
#define INFO(msg) LOG4CXX_INFO(logger, std::string(BOOST_CURRENT_FUNCTION)  + ":\t" +  msg);
#define WARN(msg) LOG4CXX_WARN(logger, std::string(BOOST_CURRENT_FUNCTION)  + ":\t" +  msg);
#define ERROR(msg) LOG4CXX_ERROR(logger, std::string(BOOST_CURRENT_FUNCTION)  + ":\t" +  msg);


#endif /* LIBFMRDSSIMULATOR_INCLUDE_DIGITIZERSIMLOGGER_H_ */
