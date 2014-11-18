/*
    PiFmRds - FM/RDS transmitter for the Raspberry Pi
    Copyright (C) 2014 Christophe Jacquet, F8FTK
    
    See https://github.com/ChristopheJacquet/PiFmRds
    
    rds_wav.c is a test program that writes a RDS baseband signal to a WAV
    file. It requires libsndfile.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FM_MPX_H_
#define FM_MPX_H_

#include <sndfile.h>
#include "rds.h"

#define PI 3.141592654


#define FIR_HALF_SIZE 30
#define FIR_SIZE (2*FIR_HALF_SIZE-1)

static float carrier_38[] = {0.0, 0.8660254037844386, 0.8660254037844388, 1.2246467991473532e-16, -0.8660254037844384, -0.8660254037844386};
static float carrier_19[] = {0.0, 0.5, 0.8660254037844386, 1.0, 0.8660254037844388, 0.5, 1.2246467991473532e-16, -0.5, -0.8660254037844384, -1.0, -0.8660254037844386, -0.5};




struct fm_mpx_struct {
	size_t length;
	// coefficients of the low-pass FIR filter
	float low_pass_fir[FIR_HALF_SIZE];
	int phase_38;
	int phase_19;
	float downsample_factor;
	float *audio_buffer;
	int audio_index;
	int audio_len;
	float audio_pos;
	float fir_buffer_mono[FIR_SIZE];
	float fir_buffer_stereo[FIR_SIZE];
	int fir_index;
	int channels;
	SNDFILE *inf;
};

extern int fm_mpx_open(char *filename, size_t len, struct fm_mpx_struct * fm_mpx_status);
extern int fm_mpx_get_samples(float *mpx_buffer, struct rds_struct* rds_params, struct fm_mpx_struct * fm_mpx_status);
extern int fm_mpx_close(struct fm_mpx_struct * fm_mpx_status);


/**
 * TODO: Initialize me to these values
struct fm_mpx_struct {
	size_t length;
	// coefficients of the low-pass FIR filter
	float low_pass_fir[FIR_HALF_SIZE];
	int phase_38 = 0;
	int phase_19 = 0;
	float downsample_factor;
	float *audio_buffer;
	int audio_index = 0;
	int audio_len = 0;
	float audio_pos;
	float fir_buffer_mono[FIR_SIZE] = {0};
	float fir_buffer_stereo[FIR_SIZE] = {0};
	int fir_index = 0;
	int channels;
	SNDFILE *inf;

};
*/
#endif /* FM_MPX_H_ */
