/*
 * FrequencyModulator.cpp
 *
 *  Created on: Nov 21, 2014
 */

#include "FrequencyModulator.h"

FrequencyModulator::FrequencyModulator(float sensitivity) {
	d_sensitivity = sensitivity;
	d_phase = 0.0;
}

FrequencyModulator::~FrequencyModulator() {
}

// Algorithm taken from the gnuradio block frequency_modulator
void FrequencyModulator::modulate(std::vector<float> &input, std::vector< std::complex<float> > &output) {
     for(int i = 0; i < input.size(); i++) {
       d_phase = d_phase + d_sensitivity * input[i];

       while(d_phase > (float)(M_PI))
         d_phase -= (float)(2.0 * M_PI);
       while(d_phase < (float)(-M_PI))
         d_phase += (float)(2.0 * M_PI);

       float oi, oq;

       int32_t angle = float_to_fixed (d_phase);
       fixed_sincos(angle, &oq, &oi);
       output[i] = std::complex<float>(oi, oq);
     }
}

