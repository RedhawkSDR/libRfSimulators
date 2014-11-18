/*
 * CallbackInterface.h
 *
 *  Created on: Nov 18, 2014
 */

#ifndef CALLBACKINTERFACE_H_
#define CALLBACKINTERFACE_H_

#include <vector>

class CallbackInterface
{
public:
    virtual void dataDelivery(std::vector<float> samples) = 0;
};




#endif /* CALLBACKINTERFACE_H_ */
