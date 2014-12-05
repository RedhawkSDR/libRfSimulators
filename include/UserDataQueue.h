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
