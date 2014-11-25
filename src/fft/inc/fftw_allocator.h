/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This file is part of REDHAWK Basic Components fftlib library.
 *
 * REDHAWK Basic Components fftlib library is free software: you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * REDHAWK Basic Components fftlib library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef FFTW_ALLOCATOR_H_
#define FFTW_ALLOCATOR_H_

#include <iostream>
#include <ostream>
#include <memory>
#include "fftw3.h"
#include <complex>
#include <vector>

   template <typename Tp>
   class fftwf_allocator : public std::allocator<Tp>

   {

   public:
	   //only allow allocator to be made for float and complex<float> types
	   fftwf_allocator()
	   {
		   throw "fftwf_allocator bad type - must be float or complex<float> only";
	   }

	   typedef typename std::allocator<Tp>::size_type size_type;
	   typedef typename std::allocator<Tp>::difference_type difference_type;
	   typedef typename std::allocator<Tp>::pointer pointer;
       typedef typename std::allocator<Tp>::const_pointer const_pointer;
       typedef typename std::allocator<Tp>::reference reference;
       typedef typename std::allocator<Tp>::const_reference const_reference;
       typedef Tp value_type;

      pointer allocate(size_type n, const void* = 0)
      {	return static_cast<Tp*>( fftwf_malloc(n * sizeof(Tp))); }

      void deallocate(pointer p, size_type) {  fftwf_free(p); }

   };

typedef std::complex<float> complex_float;

template <> inline
fftwf_allocator<float>::fftwf_allocator() : std::allocator<float>()
{};

template <> inline
fftwf_allocator<complex_float>::fftwf_allocator() : std::allocator<complex_float>()
{};

typedef std::vector<float, fftwf_allocator<float> > RealFFTWVector;
typedef std::vector<complex_float, fftwf_allocator<complex_float> > ComplexFFTWVector;

#endif /* FFTW_ALLOCATOR_H_ */
