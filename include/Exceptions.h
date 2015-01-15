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
