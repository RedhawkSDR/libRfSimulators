/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this 
 * source distribution.
 * 
 * This file is part of REDHAWK Basic Components dsp library.
 * 
 * REDHAWK Basic Components dsp library is free software: you can redistribute it and/or modify it under the terms of 
 * the GNU Lesser General Public License as published by the Free Software Foundation, either 
 * version 3 of the License, or (at your option) any later version.
 * 
 * REDHAWK Basic Components dsp library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with this 
 * program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef DATATYPES_H_
#define DATATYPES_H_

typedef short Sample;
typedef float Real;

#include<complex>
typedef std::complex<Real> Complex;
#include <queue>
typedef std::queue<Real> RealQueue;
typedef std::queue<Complex> ComplexQueue;

#include <valarray>
typedef std::valarray<Sample> SampleArray;
typedef std::valarray<Real> RealArray;
typedef std::valarray<Complex> ComplexArray;

#include <vector>
typedef std::vector<Sample> SampleVector;
typedef std::vector<Real> RealVector;
typedef std::vector<Complex> ComplexVector;

typedef std::queue<RealVector> RealVectorQueue;
typedef std::queue<ComplexVector> ComplexVectorQueue;

#include <bitset>
typedef float Symbol;

typedef std::queue<Symbol> SymbolQueue;
typedef std::valarray<Symbol> SymbolArray;
typedef std::bitset<64> Frame;
typedef std::valarray<Frame> FrameArray;
typedef std::queue<Frame> FrameQueue;

#endif /* DATATYPES_H_ */
