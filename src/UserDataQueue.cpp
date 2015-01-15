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
 * UserDataQueue.cpp
 *
 *  Created on: Nov 26, 2014
 */

#include "UserDataQueue.h"
#include "DigitizerSimLogger.h"
#include "boost/bind.hpp"

using namespace RfSimulators;

UserDataQueue::UserDataQueue(unsigned short maxQueueDepth, CallbackInterface *userClass) {
	TRACE("Entering Method");
	this->maxQueueDepth = maxQueueDepth;
	this->userClass = userClass;
	shuttingDown = false;
	waitForDataThread = NULL;
	TRACE("Leaving Method");
}

UserDataQueue::~UserDataQueue() {
	TRACE("Entering Method");
	shutDown();
	TRACE("Leaving Method");
}

void UserDataQueue::waitForData()
{
	TRACE("Entering Method");
	INFO("Launching thread Wait for Data Thread");
	waitForDataThread = new boost::thread(boost::bind(&UserDataQueue::_waitForData, this));
	TRACE("Leaving Method");
}

// Taken from: http://www.boost.org/doc/libs/1_51_0/doc/html/thread/synchronization.html#thread.synchronization.condvar_ref
void UserDataQueue::_waitForData() {
	TRACE("Entering Method");
	while(not shuttingDown) {

		std::valarray< std::complex<float> > dataCopy;

		{
			// This locks the mutex.
			boost::unique_lock<boost::mutex> lock(mut);
			while((internalDataBuffer.size() == 0)  && (not shuttingDown))
			{
				// This unlocks the mutex until the wait is over
				TRACE("Waiting until Data is available");
				cond.wait(lock);
				TRACE("Woken up, checking for data.");

				// At this point the mutex is locked again.
			}

			TRACE("Data is available!");

			if (shuttingDown) {
				TRACE("Shutting down, ignoring data");
				break;
			}

			TRACE("Copying data for user.  Size: " << internalDataBuffer.front().size());
			dataCopy.resize(internalDataBuffer.front().size());
			dataCopy = internalDataBuffer.front();

			TRACE("Removing data from queue");
			internalDataBuffer.pop();
		}

		TRACE("Passing " << dataCopy.size() << " data points to user");
		userClass->dataDelivery(dataCopy);
	}

	TRACE("Leaving Method");
}

void UserDataQueue::shutDown() {
	TRACE("Entering Method");
	shuttingDown = true;
	cond.notify_all();
	if (waitForDataThread) {
		waitForDataThread->join();
	}

	if (waitForDataThread) {
		delete(waitForDataThread);
		waitForDataThread = NULL;
	}
	TRACE("Leaving Method");
}

void UserDataQueue::deliverData(std::valarray< std::complex<float> > &dataArray)
{
	TRACE("Entering Method");

    {
        boost::lock_guard<boost::mutex> lock(mut);

        if (shuttingDown) {
        	INFO("Shutting down, refusing to pass data to user.");
        	return;
        }

        if (maxQueueDepth == 0) {
        		ERROR("Queue Size has been set to zero.  You will not receive any data");
        		// Fall through on purpose to flush queue.
        }

		if (internalDataBuffer.size() > maxQueueDepth) {
			ERROR("Queue flushing!  Data was not serviced fast enough.");

			while (internalDataBuffer.size() > 0) {
				internalDataBuffer.pop();
			}

			return;
		}
		TRACE("Adding array of size: " << dataArray.size() << " to UserDataQueue buffer");
		internalDataBuffer.push(dataArray);
    }

    cond.notify_one();
    TRACE("Leaving Method");
}

void UserDataQueue::setMaxQueueSize(unsigned short size) {
	TRACE("Entering Method");
	maxQueueDepth = size;
	TRACE("Leaving Method");
}
