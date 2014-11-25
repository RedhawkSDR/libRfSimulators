/*
 * FrequencyModulator.h
 *
 *  Created on: Nov 21, 2014
 */

#ifndef FREQUENCYMODULATOR_H_
#define FREQUENCYMODULATOR_H_

#include <valarray>
#include <math.h>
#include <sys/types.h>
#include <complex>

static const int WORDBITS = 32;
static const int NBITS = 10;
const float s_sine_table[1 << NBITS][2] = {
	#include "sine_table.h"
};


static int32_t float_to_fixed(float x)
{
  // Fold x into -PI to PI.
  int d = (int)floor(x/2/M_PI+0.5);
  x -= d*2*M_PI;
  // And convert to an integer.
  return (int32_t) ((float) x * pow(2,31) / M_PI);
}

/*
 * \brief Given a fixedpoint angle x, return float cos(x) and sin (x)
 */
static void fixed_sincos(int32_t x, float *s, float *c)
{
  u_int32_t ux = x;
  int sin_index = ux >> (WORDBITS - NBITS);
  *s = s_sine_table[sin_index][0] * (ux >> 1) + s_sine_table[sin_index][1];

  ux = x + 0x40000000;
  int cos_index = ux >> (WORDBITS - NBITS);
  *c = s_sine_table[cos_index][0] * (ux >> 1) + s_sine_table[cos_index][1];

  return;
}



class FrequencyModulator {
public:
	FrequencyModulator(float sensitivity);
	virtual ~FrequencyModulator();
	void modulate(std::valarray<float> &input, std::valarray< std::complex<float> > &output);

private:
	float d_sensitivity;
	float d_phase;


};

#endif /* FREQUENCYMODULATOR_H_ */
