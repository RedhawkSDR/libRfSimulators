/*
 * UserDataQueue.h
 *
 *  Created on: Nov 26, 2014
 */

#ifndef USERDATAQUEUE_H_
#define USERDATAQUEUE_H_

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <valarray>
#include <complex>
#include <queue>
#include "CallbackInterface.h"


class UserDataQueue {
public:
	UserDataQueue(unsigned short maxQueueDepth, CallbackInterface *userClass);
	virtual ~UserDataQueue();

	void deliverData(std::valarray< std::complex<float> > &dataArray);
	void wait_for_data_to_process();
	void setMaxQueueSize(unsigned short size);

private:
	void process_data();

	boost::condition_variable cond;
	boost::mutex mut;
	bool data_ready;
	unsigned short maxQueueDepth;
	std::queue< std::valarray< std::complex<float> > > dataBuffer;
	CallbackInterface *userClass;
};

#endif /* USERDATAQUEUE_H_ */
