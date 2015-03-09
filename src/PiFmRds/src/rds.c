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

#include "rds.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

uint16_t offset_words[] = {0x0FC, 0x198, 0x168, 0x1B4};
// We don't handle offset word C' here for the sake of simplicity

/* Classical CRC computation */
uint16_t crc(uint16_t block) {
    uint16_t crc = 0;
    int j; 
    for(j=0; j<BLOCK_SIZE; j++) {
        int bit = (block & MSB_BIT) != 0;
        block <<= 1;

        int msb = (crc >> (POLY_DEG-1)) & 1;
        crc <<= 1;
        if((msb ^ bit) != 0) {
            crc = crc ^ POLY;
        }
    }
    
    return crc;
}

/* Possibly generates a CT (clock time) group if the minute has just changed
   Returns 1 if the CT group was generated, 0 otherwise
*/
int get_rds_ct_group(uint16_t *blocks, struct rds_signal_info* rds_sig_info) {

	// Check time
    time_t now;
    struct tm *utc;
    
    now = time (NULL);
    utc = gmtime (&now);

    if(utc->tm_min != rds_sig_info->latest_minutes) {
        // Generate CT group
    	rds_sig_info->latest_minutes = utc->tm_min;
        
        int l = utc->tm_mon <= 1 ? 1 : 0;
        int mjd = 14956 + utc->tm_mday + 
                        (int)((utc->tm_year - l) * 365.25) +
                        (int)((utc->tm_mon + 2 + l*12) * 30.6001);
        
        blocks[1] = 0x4400 | (mjd>>15);
        blocks[2] = (mjd<<1) | (utc->tm_hour>>4);
        blocks[3] = (utc->tm_hour & 0xF)<<12 | utc->tm_min<<6;
        
        utc = localtime(&now);
        
        int offset = utc->tm_gmtoff / (30 * 60);
        blocks[3] |= abs(offset);
        if(offset < 0) blocks[3] |= 0x20;
        
        //printf("Generated CT: %04X %04X %04X\n", blocks[1], blocks[2], blocks[3]);
        return 1;
    } else return 0;
}

/* Creates an RDS group. This generates sequences of the form 0A, 0A, 0A, 0A, 2A, etc.
   The pattern is of length 5, the variable 'state' keeps track of where we are in the
   pattern. 'ps_state' and 'rt_state' keep track of where we are in the PS (0A) sequence
   or RT (2A) sequence, respectively.
*/
void get_rds_group(int *buffer, struct rds_content_struct* rds_params, struct rds_signal_info* rds_sig_info) {
    uint16_t blocks[GROUP_LENGTH] = {rds_params->pi, 0, 0, 0};
    
    // Generate block content
    if(! get_rds_ct_group(blocks, rds_sig_info)) { // CT (clock time) has priority on other group types
        if(rds_sig_info->state < 4) {
            blocks[1] = 0x0400 | rds_sig_info->ps_state;
            if(rds_params->ta) blocks[1] |= 0x0010;
            blocks[2] = 0xCDCD;     // no AF
            blocks[3] = rds_params->ps[rds_sig_info->ps_state*2]<<8 | rds_params->ps[rds_sig_info->ps_state*2+1];
            rds_sig_info->ps_state++;
            if(rds_sig_info->ps_state >= 4) rds_sig_info->ps_state = 0;
        } else { // state == 5
            blocks[1] = 0x2400 | rds_sig_info->rt_state;
            blocks[2] = rds_params->rt[rds_sig_info->rt_state*4+0]<<8 | rds_params->rt[rds_sig_info->rt_state*4+1];
            blocks[3] = rds_params->rt[rds_sig_info->rt_state*4+2]<<8 | rds_params->rt[rds_sig_info->rt_state*4+3];
            rds_sig_info->rt_state++;
            if(rds_sig_info->rt_state >= 16) rds_sig_info->rt_state = 0;
        }
    
        rds_sig_info->state++;
        if(rds_sig_info->state >= 5) rds_sig_info->state = 0;
    }
    
    // Calculate the checkword for each block and emit the bits
    int i;
    for(i=0; i<GROUP_LENGTH; i++) {
        uint16_t block = blocks[i];
        uint16_t check = crc(block) ^ offset_words[i];
        int j;
        for(j=0; j<BLOCK_SIZE; j++) {
            *buffer++ = ((block & (1<<(BLOCK_SIZE-1))) != 0);
            block <<= 1;
        }
        for(j=0; j<POLY_DEG; j++) {
            *buffer++= ((check & (1<<(POLY_DEG-1))) != 0);
            check <<= 1;
        }
    }
}

/* Get a number of RDS samples. This generates the envelope of the waveform using
   pre-generated elementary waveform samples, and then it amplitude-modulates the 
   envelope with a 57 kHz carrier, which is very efficient as 57 kHz is 4 times the
   sample frequency we are working at (228 kHz).
 */
void get_rds_samples(float *buffer, int count, struct rds_content_struct* rds_content, struct rds_signal_info * rds_signal) {

	int i;
    for(i=0; i<count; i++) {
        if(rds_signal->sample_count >= SAMPLES_PER_BIT) {
            if(rds_signal->bit_pos >= BITS_PER_GROUP) {
                get_rds_group(rds_signal->bit_buffer, rds_content, rds_signal);
                rds_signal->bit_pos = 0;
            }
            
            // do differential encoding
            rds_signal->cur_bit = rds_signal->bit_buffer[rds_signal->bit_pos];
            rds_signal->prev_output = rds_signal->cur_output;
            rds_signal->cur_output = rds_signal->prev_output ^ rds_signal->cur_bit;
            
            rds_signal->inverting = (rds_signal->cur_output == 1);

            float *src = waveform_biphase;
            int idx = rds_signal->in_sample_index;
            int j;
            for(j=0; j<FILTER_SIZE; j++) {
                float val = (*src++);
                if(rds_signal->inverting) val = -val;
                rds_signal->sample_buffer[idx++] += val;
                if(idx >= SAMPLE_BUFFER_SIZE) idx = 0;
            }

            rds_signal->in_sample_index += SAMPLES_PER_BIT;
            if(rds_signal->in_sample_index >= SAMPLE_BUFFER_SIZE) rds_signal->in_sample_index -= SAMPLE_BUFFER_SIZE;
            
            rds_signal->bit_pos++;
            rds_signal->sample_count = 0;
        }
        
        float sample = rds_signal->sample_buffer[rds_signal->out_sample_index];
        rds_signal->sample_buffer[rds_signal->out_sample_index] = 0;
        rds_signal->out_sample_index++;
        if(rds_signal->out_sample_index >= SAMPLE_BUFFER_SIZE) rds_signal->out_sample_index = 0;
        
        
        // modulate at 57 kHz
        // use phase for this
        switch(rds_signal->phase) {
            case 0:
            case 2: sample = 0; break;
            case 1: break;
            case 3: sample = -sample; break;
        }
        rds_signal->phase++;
        if(rds_signal->phase >= 4) rds_signal->phase = 0;
        
        *buffer++ = sample;
        rds_signal->sample_count++;
    }
}

void set_rds_rt(char *rt, struct rds_content_struct* rds_params) {
    strncpy(rds_params->rt, rt, 64);
    int i;
    for(i=0; i<64; i++) {
        if(rds_params->rt[i] == 0) rds_params->rt[i] = 32;
    }
}

void set_rds_ps(char *ps, struct rds_content_struct* rds_params) {
    strncpy(rds_params->ps, ps, 8);
    int i;
    for(i=0; i<8; i++) {
        if(rds_params->ps[i] == 0) rds_params->ps[i] = 32;
    }
}

