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
	data_ready = false;
	this->userClass = userClass;
	shuttingDown = false;
}

UserDataQueue::~UserDataQueue() {
	shutDown();
}

// Taken from: http://www.boost.org/doc/libs/1_51_0/doc/html/thread/synchronization.html#thread.synchronization.condvar_ref
void UserDataQueue::waitForData()
{
	waitForDataThread = new boost::thread(boost::bind(&UserDataQueue::_waitForData, this));
}

void UserDataQueue::_waitForData() {
	while(not shuttingDown) {

		std::valarray< std::complex<float> > dataCopy;

		{
			boost::unique_lock<boost::mutex> lock(mut);
			while((dataBuffer.size() == 0)  && (not shuttingDown))
			{
				cond.wait(lock);
			}

			if (shuttingDown)
				break;

			//TODO: This is kind of ugly, fix it.
			dataCopy.resize(dataBuffer.front().size());
			dataCopy = dataBuffer.front();

			dataBuffer.pop();
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

		if (dataBuffer.size() > maxQueueDepth) {
			ERROR("Queue flushing!  Data was not serviced fast enough.");

			while (dataBuffer.size() > 0) {
				dataBuffer.pop();
			}

			return;
		}

		dataBuffer.push(dataArray);
    }

    cond.notify_one();
}

void UserDataQueue::setMaxQueueSize(unsigned short size) {
	maxQueueDepth = size;
}
