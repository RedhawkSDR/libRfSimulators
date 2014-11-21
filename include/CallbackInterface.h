/*
 * CallbackInterface.h
 *
 *  Created on: Nov 18, 2014
 */

#ifndef CALLBACKINTERFACE_H_
#define CALLBACKINTERFACE_H_

#include <vector>
#include <complex>

#define DATA_BLOCK_SIZE 10000

class CallbackInterface
{
public:
    virtual void dataDelivery(std::vector< std::complex<float> > samples) = 0;
};




#endif /* CALLBACKINTERFACE_H_ */
