/*
 * SimDefaults.h
 *
 *  Created on: Nov 18, 2014
 */

#ifndef SIMDEFAULTS_H_
#define SIMDEFAULTS_H_

#include <iostream>
#include <string>

using namespace std;

#define BASE_SAMPLE_RATE 228000.0

#define MAX_OUTPUT_SAMPLE_RATE (BASE_SAMPLE_RATE*10.0)

#define MAX_FREQUENCY_DEVIATION 75000.0

#define FILTER_ATTENUATION 70 // dB

static string DEFAULT_RDS_TEXT = "REDHAWK Radio, taking you to the cloud.";



#endif /* SIMDEFAULTS_H_ */
