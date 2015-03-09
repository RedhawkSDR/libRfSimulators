/*
    PiFmRds - FM/RDS transmitter for the Raspberry Pi
    Copyright (C) 2014 Christophe Jacquet, F8FTK
    
    See https://github.com/ChristopheJacquet/PiFmRds

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

#ifndef RDS_H
#define RDS_H


#include <stdint.h>
#include "waveforms.h"

#define RT_LENGTH 64
#define PS_LENGTH 8
#define GROUP_LENGTH 4

/* Here, the first member of the struct must be a scalar to avoid a
   warning on -Wmissing-braces with GCC < 4.8.3
   (bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119)
*/

/* The RDS error-detection code generator polynomial is
   x^10 + x^8 + x^7 + x^5 + x^4 + x^3 + x^0
*/
#define POLY 0x1B9
#define POLY_DEG 10
#define MSB_BIT 0x8000
#define BLOCK_SIZE 16

#define BITS_PER_GROUP (GROUP_LENGTH * (BLOCK_SIZE+POLY_DEG))
#define SAMPLES_PER_BIT 192
#define FILTER_SIZE (sizeof(waveform_biphase)/sizeof(float))
#define SAMPLE_BUFFER_SIZE (SAMPLES_PER_BIT + FILTER_SIZE)


struct rds_content_struct {
    uint16_t pi;
    int ta;
    char ps[PS_LENGTH];
    char rt[RT_LENGTH];
};


// YLB: These were static function variables in the original rds.c file but to allow for multi-threading they have
// been moved into a struct that is passed in.
struct rds_signal_info {
	int bit_buffer[BITS_PER_GROUP];
	int bit_pos;
	float sample_buffer[SAMPLE_BUFFER_SIZE];
	int prev_output;
	int cur_output;
	int cur_bit;
	int sample_count;
	int inverting;
	int phase;
	int in_sample_index;
	int out_sample_index;
	int latest_minutes;
	int state;
	int ps_state;
	int rt_state;
};

extern void get_rds_samples(float *buffer, int count, struct rds_content_struct* rds_content, struct rds_signal_info * rds_signal);
extern void set_rds_rt(char *rt, struct rds_content_struct* rds_params);
extern void set_rds_ps(char *ps, struct rds_content_struct* rds_params);
extern void set_rds_ta(int ta);

#endif /* RDS_H */
