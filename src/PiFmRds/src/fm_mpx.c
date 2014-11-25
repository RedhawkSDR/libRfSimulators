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
    
    fm_mpx.c: generates an FM multiplex signal containing RDS plus possibly
    monaural or stereo audio.
*/

#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "fm_mpx.h"

float *alloc_empty_buffer(size_t length) {
    float *p = malloc(length * sizeof(float));
    if(p == NULL) return NULL;
    
    bzero(p, length * sizeof(float));
    
    return p;
}


int fm_mpx_open(char *filename, size_t len, struct fm_mpx_struct* fm_mpx_status) {
	fm_mpx_status->length = len;

    if(filename != NULL) {
        // Open the input file
        SF_INFO sfinfo;
 
        // stdin or file on the filesystem?
        if(filename[0] == '-') {
            if(! (fm_mpx_status->inf = sf_open_fd(fileno(stdin), SFM_READ, &sfinfo, 0))) {
                fprintf(stderr, "Error: could not open stdin for audio input.\n") ;
                return -1;
            } else {
                printf("Using stdin for audio input.\n");
            }
        } else {
            if(! (fm_mpx_status->inf = sf_open(filename, SFM_READ, &sfinfo))) {
                fprintf(stderr, "Error: could not open input file %s.\n", filename) ;
                return -1;
            } else {
                printf("Using audio file: %s\n", filename);
            }
        }
            
        int in_samplerate = sfinfo.samplerate;
        fm_mpx_status->downsample_factor = 228000. / in_samplerate;
    
        printf("Input: %d Hz, upsampling factor: %.2f\n", in_samplerate, fm_mpx_status->downsample_factor);

        fm_mpx_status->channels = sfinfo.channels;
        if(fm_mpx_status->channels > 1) {
            printf("%d channels, generating stereo multiplex.\n", fm_mpx_status->channels);
        } else {
            printf("1 channel, monophonic operation.\n");
        }
    
    
        // Create the low-pass FIR filter
        float cutoff_freq = 15000 * .8;
        if(in_samplerate/2 < cutoff_freq) cutoff_freq = in_samplerate/2 * .8;
    
    
    
        fm_mpx_status->low_pass_fir[FIR_HALF_SIZE-1] = 2 * cutoff_freq / 228000 /2;
        // Here we divide this coefficient by two because it will be counted twice
        // when applying the filter

        // Only store half of the filter since it is symmetric
        int i;
        for(i=1; i<FIR_HALF_SIZE; i++) {
        	fm_mpx_status->low_pass_fir[FIR_HALF_SIZE-1-i] =
                sin(2 * PI * cutoff_freq * i / 228000) / (PI * i)      // sinc
                * (.54 - .46 * cos(2*PI * (i+FIR_HALF_SIZE) / (2*FIR_HALF_SIZE)));
                                                              // Hamming window
        }
        printf("Created low-pass FIR filter for audio channels, with cutoff at %.1f Hz\n", cutoff_freq);
    
        /*
        for(int i=0; i<FIR_HALF_SIZE; i++) {
            printf("%.5f ", low_pass_fir[i]);
        }
        printf("\n");
        */
        
        fm_mpx_status->audio_pos = fm_mpx_status->downsample_factor;
        fm_mpx_status->audio_buffer = alloc_empty_buffer(fm_mpx_status->length * fm_mpx_status->channels);
        if(fm_mpx_status->audio_buffer == NULL) return -1;

    } // end if(filename != NULL)
    else {
    	fm_mpx_status->inf = NULL;
        // inf == NULL indicates that there is no audio
    }
    
    return 0;
}


// samples provided by this function are in 0..10: they need to be divided by
// 10 after.
int fm_mpx_get_samples(float *mpx_buffer, struct rds_struct* rds_params, struct fm_mpx_struct * fm_mpx_status) {
    get_rds_samples(mpx_buffer, fm_mpx_status->length, rds_params);

    if(fm_mpx_status->inf  == NULL) return 0; // if there is no audio, stop here
    int i; 
    for(i=0; i<fm_mpx_status->length; i++) {
        if(fm_mpx_status->audio_pos >= fm_mpx_status->downsample_factor) {
        	fm_mpx_status->audio_pos -= fm_mpx_status->downsample_factor;
            
            if(fm_mpx_status->audio_len == 0) {
                int j;
                for(j=0; j<2; j++) { // one retry
                	fm_mpx_status->audio_len = sf_read_float(fm_mpx_status->inf, fm_mpx_status->audio_buffer, fm_mpx_status->length);
                    if (fm_mpx_status->audio_len < 0) {
                        fprintf(stderr, "Error reading audio\n");
                        return -1;
                    }
                    if(fm_mpx_status->audio_len == 0) {
                        if( sf_seek(fm_mpx_status->inf, 0, SEEK_SET) < 0 ) {
                            fprintf(stderr, "Could not rewind in audio file, terminating\n");
                            return -1;
                        }
                    } else {
                        break;
                    }
                }
                fm_mpx_status->audio_index = 0;
            } else {
            	fm_mpx_status->audio_index += fm_mpx_status->channels;
            	fm_mpx_status->audio_len -= fm_mpx_status->channels;
            }
        }

        
        // First store the current sample(s) into the FIR filter's ring buffer
        if(fm_mpx_status->channels == 0) {
        	fm_mpx_status->fir_buffer_mono[fm_mpx_status->fir_index] = fm_mpx_status->audio_buffer[fm_mpx_status->audio_index];
        } else {
            // In stereo operation, generate sum and difference signals
        	fm_mpx_status->fir_buffer_mono[fm_mpx_status->fir_index] =
        			fm_mpx_status->audio_buffer[fm_mpx_status->audio_index] + fm_mpx_status->audio_buffer[fm_mpx_status->audio_index + 1];
        	fm_mpx_status->fir_buffer_stereo[fm_mpx_status->fir_index] =
        			fm_mpx_status->audio_buffer[fm_mpx_status->audio_index] - fm_mpx_status->audio_buffer[fm_mpx_status->audio_index + 1];
        }
        fm_mpx_status->fir_index++;
        if(fm_mpx_status->fir_index >= FIR_SIZE) fm_mpx_status->fir_index = 0;
        
        // Now apply the FIR low-pass filter
        
        /* As the FIR filter is symmetric, we do not multiply all 
           the coefficients independently, but two-by-two, thus reducing
           the total number of multiplications by a factor of two
        */
        float out_mono = 0;
        float out_stereo = 0;
        int ifbi = fm_mpx_status->fir_index;  // ifbi = increasing FIR Buffer Index
        int dfbi = fm_mpx_status->fir_index;  // dfbi = decreasing FIR Buffer Index
        int fi;
        for(fi=0; fi<FIR_HALF_SIZE; fi++) {  // fi = Filter Index
            dfbi--;
            if(dfbi < 0) dfbi = FIR_SIZE-1;
            out_mono += 
            		fm_mpx_status->low_pass_fir[fi] *
                    (fm_mpx_status->fir_buffer_mono[ifbi] + fm_mpx_status->fir_buffer_mono[dfbi]);
            if(fm_mpx_status->channels > 1) {
                out_stereo += 
                		fm_mpx_status->low_pass_fir[fi] *
                        (fm_mpx_status->fir_buffer_stereo[ifbi] + fm_mpx_status->fir_buffer_stereo[dfbi]);
            }
            ifbi++;
            if(ifbi >= FIR_SIZE) ifbi = 0;
        }
        // End of FIR filter
        

        mpx_buffer[i] = 
            mpx_buffer[i] +    // RDS data samples are currently in mpx_buffer
            4.05*out_mono;     // Unmodulated monophonic (or stereo-sum) signal
            
        if(fm_mpx_status->channels>1) {
            mpx_buffer[i] +=
                4.05 * carrier_38[fm_mpx_status->phase_38] * out_stereo + // Stereo difference signal
                .9*carrier_19[fm_mpx_status->phase_19];                  // Stereo pilot tone

            fm_mpx_status->phase_19++;
            fm_mpx_status->phase_38++;
            if(fm_mpx_status->phase_19 >= 12) fm_mpx_status->phase_19 = 0;
            if(fm_mpx_status->phase_38 >= 6) fm_mpx_status->phase_38 = 0;
        }
            
        fm_mpx_status->audio_pos++;
        
    }
    
    return 0;
}


int fm_mpx_close(struct fm_mpx_struct* fm_mpx_status) {
    if(sf_close(fm_mpx_status->inf) ) {
        fprintf(stderr, "Error closing audio file");
    }
    
    if(fm_mpx_status->audio_buffer != NULL) free(fm_mpx_status->audio_buffer);
    
    return 0;
}
