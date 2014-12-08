/*
 * Exceptions.h
 *
 *  Created on: Dec 8, 2014
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <iostream>
#include <exception>
using namespace std;

namespace RfSimulators {


class OutOfRangeException: public exception
{
  virtual const char* what() const throw()
  {
    return "Value specified is outside of the allowed range";
  }
};

class InvalidValue: public exception
{
  virtual const char* what() const throw()
  {
    return "Value specified is not valid";
  }
};

};



#endif /* EXCEPTIONS_H_ */
