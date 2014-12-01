/*
 * UserDataQueue.cpp
 *
 *  Created on: Nov 26, 2014
 */

#include "UserDataQueue.h"
#include "DigitizerSimLogger.h"
#include "boost/bind.hpp"

UserDataQueue::UserDataQueue(unsigned short maxQueueDepth, CallbackInterface *userClass) {
	this->maxQueueDepth = maxQueueDepth;
	this->userClass = userClass;
	shuttingDown = false;
	waitForDataThread = NULL;
}

UserDataQueue::~UserDataQueue() {
	shutDown();
}

void UserDataQueue::waitForData()
{
	waitForDataThread = new boost::thread(boost::bind(&UserDataQueue::_waitForData, this));
}

// Taken from: http://www.boost.org/doc/libs/1_51_0/doc/html/thread/synchronization.html#thread.synchronization.condvar_ref
void UserDataQueue::_waitForData() {
	while(not shuttingDown) {

		std::valarray< std::complex<float> > dataCopy;

		{
			// This locks the mutex.
			boost::unique_lock<boost::mutex> lock(mut);
			while((internalDataBuffer.size() == 0)  && (not shuttingDown))
			{
				// This unlocks the mutex until the wait is over
				cond.wait(lock);

				// At this point the mutex is locked again.
			}

			if (shuttingDown) {
				break;
			}

			dataCopy.resize(internalDataBuffer.front().size());
			dataCopy = internalDataBuffer.front();

			internalDataBuffer.pop();
		}

		userClass->dataDelivery(dataCopy);
	}
}

void UserDataQueue::shutDown() {
	shuttingDown = true;
	cond.notify_all();
	if (waitForDataThread) {
		waitForDataThread->join();
	}

	if (waitForDataThread) {
		delete(waitForDataThread);
		waitForDataThread = NULL;
	}
}

void UserDataQueue::deliverData(std::valarray< std::complex<float> > &dataArray)
{
    {
        boost::lock_guard<boost::mutex> lock(mut);

        if (shuttingDown) {
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

		internalDataBuffer.push(dataArray);
    }

    cond.notify_one();
}

void UserDataQueue::setMaxQueueSize(unsigned short size) {
	maxQueueDepth = size;
}
