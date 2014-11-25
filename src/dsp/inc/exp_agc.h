/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this 
 * source distribution.
 * 
 * This file is part of REDHAWK Basic Components dsp library.
 * 
 * REDHAWK Basic Components dsp library is free software: you can redistribute it and/or modify it under the terms of 
 * the GNU Lesser General Public License as published by the Free Software Foundation, either 
 * version 3 of the License, or (at your option) any later version.
 * 
 * REDHAWK Basic Components dsp library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with this 
 * program.  If not, see http://www.gnu.org/licenses/.
 */
#include "DataTypes.h"
#include <vector>

#ifndef EXP_AGC_H_
#define EXP_AGC_H_

template<typename T>
T validateAlpha(T alpha);

/**
 * \brief This class is used to implement an exponential moving average AGC
 * \see http://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average for an explanation
 */
template<typename T, typename U>
class ExpAgc {
public:
    ExpAgc(std::valarray<U>& input, std::valarray<U>& output, T avgPower, T minPower, T maxPower, T eps, T alpha);
    void process();
    T setAlpha(T alpha);
    T getAlpha();
    T getMaxPower();
    void setMaxPower(T maxPower);
    T getMinPower();
    void setMinPower(T minPower);
    T getAvgPower();
    void setAvgPower(T avgPower);
    T getCurrentPower();
    virtual ~ExpAgc();
private:

    void initialize();

    std::valarray<U> &_input;
    std::valarray<U> &_output;

    double _currentPower;
    T _avgPower;
    T _minPower;
    T _maxPower;
    T _eps;
    T _alpha;
    T _omega;
    bool  _init;
};

#endif /* EXP_AGC_H_ */
