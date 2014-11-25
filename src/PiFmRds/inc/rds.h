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


#define RT_LENGTH 64
#define PS_LENGTH 8
#define GROUP_LENGTH 4

struct rds_struct {
    uint16_t pi;
    int ta;
    char ps[PS_LENGTH];
    char rt[RT_LENGTH];
};


extern void get_rds_samples(float *buffer, int count, struct rds_struct * rds_params);
extern void set_rds_rt(char *rt, struct rds_struct* rds_params);
extern void set_rds_ps(char *ps, struct rds_struct* rds_params);
extern void set_rds_ta(int ta);

#endif /* RDS_H */
