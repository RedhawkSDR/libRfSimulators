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
 * UserDataQueue.h
 *
 *  Created on: Nov 26, 2014
 */

#ifndef LIBFMRDSSIMULATOR_INCLUDE_USERDATAQUEUE_H_
#define LIBFMRDSSIMULATOR_INCLUDE_USERDATAQUEUE_H_

#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <valarray>
#include <complex>
#include <queue>
#include "CallbackInterface.h"

using namespace RfSimulators;

class UserDataQueue {
public:
	UserDataQueue(unsigned short maxQueueDepth, CallbackInterface *userClass);
	virtual ~UserDataQueue();

	void deliverData(std::valarray< std::complex<float> > &dataArray);
	void waitForData();
	void shutDown();
	void setMaxQueueSize(unsigned short size);

private:
	void process_data();

	bool shuttingDown;

	boost::condition_variable cond;
	boost::mutex mut;
	unsigned short maxQueueDepth;
	std::queue< std::valarray< std::complex<float> > > internalDataBuffer;
	CallbackInterface *userClass;
	void _waitForData();
	boost::thread *waitForDataThread;
};

#endif /* LIBFMRDSSIMULATOR_INCLUDE_USERDATAQUEUE_H_ */
