/*
 ============================================================================
 Name        : exampleProgram.cpp
 Author      : Youssef Bagoulla
 Version     :
 Copyright   : 
 Description : Uses shared library to print greeting
               To run the resulting executable the LD_LIBRARY_PATH must be
               set to ${project_loc}/libdigitizersim/.libs
               Alternatively, libtool creates a wrapper shell script in the
               build directory of this program which can be used to run it.
               Here the script will be called exampleProgram.
 ============================================================================
 */

#include "libdigitizersim.h"

int main(void) {
  print_hello();
  return 0;
}
