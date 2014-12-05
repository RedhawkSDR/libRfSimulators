/*
 * CallbackInterface.h
 *
 *  Created on: Nov 18, 2014
 */

#ifndef LIBFMRDSSIMULATOR_INCLUDE_CALLBACKINTERFACE_H_
#define LIBFMRDSSIMULATOR_INCLUDE_CALLBACKINTERFACE_H_

#include <valarray>
#include <complex>

#define FILE_INPUT_BLOCK_SIZE 100000
#define OUTPUT_SAMPLES_BLOCK_SIZE FILE_INPUT_BLOCK_SIZE*10


namespace RfSimulators {

class CallbackInterface
{
public:
    virtual void dataDelivery(std::valarray< std::complex<float> > samples) = 0;
};

};



#endif /* LIBFMRDSSIMULATOR_INCLUDE_CALLBACKINTERFACE_H_ */
