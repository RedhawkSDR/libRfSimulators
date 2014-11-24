/*
 * CallbackInterface.h
 *
 *  Created on: Nov 18, 2014
 */

#ifndef CALLBACKINTERFACE_H_
#define CALLBACKINTERFACE_H_

#include <vector>
#include <complex>

#define FILE_INPUT_BLOCK_SIZE 100000
#define OUTPUT_SAMPLES_BLOCK_SIZE FILE_INPUT_BLOCK_SIZE*10

class CallbackInterface
{
public:
    virtual void dataDelivery(std::vector< std::complex<float> > samples) = 0;
};




#endif /* CALLBACKINTERFACE_H_ */
