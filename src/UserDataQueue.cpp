/*
 * UserDataQueue.cpp
 *
 *  Created on: Nov 26, 2014
 */

#include "UserDataQueue.h"
#include "DigitizerSimLogger.h"

UserDataQueue::UserDataQueue(unsigned short maxQueueDepth, CallbackInterface *userClass) {
	this->maxQueueDepth = maxQueueDepth;
	data_ready = false;
	this->userClass = userClass;
}

UserDataQueue::~UserDataQueue() {
}

// Taken from: http://www.boost.org/doc/libs/1_51_0/doc/html/thread/synchronization.html#thread.synchronization.condvar_ref
void UserDataQueue::wait_for_data_to_process()
{

	while(1) {

		std::valarray< std::complex<float> > dataCopy;

		{
			boost::unique_lock<boost::mutex> lock(mut);
			while(dataBuffer.size() == 0)
			{
				std::cout << "Setting conditional wait lock" << std::endl;
				cond.wait(lock);
			}

			//TODO: This is kind of ugly, fix it.
			dataCopy.resize(dataBuffer.front().size());
			dataCopy = dataBuffer.front();

			dataBuffer.pop();
		}

		userClass->dataDelivery(dataCopy);
	}
}


void UserDataQueue::deliverData(std::valarray< std::complex<float> > &dataArray)
{
    {
        boost::lock_guard<boost::mutex> lock(mut);

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

		std::cout << "Adding data to dataBuffer" << std::endl;
		dataBuffer.push(dataArray);
    }

    std::cout << "Notifying other thread of update" << std::endl;
    cond.notify_one();
}

void UserDataQueue::setMaxQueueSize(unsigned short size) {
	maxQueueDepth = size;
}
