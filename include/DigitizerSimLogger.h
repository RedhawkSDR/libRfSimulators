/*
 * DigitizerSimLogger.h
 *
 *  Created on: Nov 19, 2014
 */

#ifndef DIGITIZERSIMLOGGER_H_
#define DIGITIZERSIMLOGGER_H_
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


#endif /* DIGITIZERSIMLOGGER_H_ */
