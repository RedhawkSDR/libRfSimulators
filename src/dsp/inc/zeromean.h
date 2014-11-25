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

#ifndef ZEROMEAN_H_
#define ZEROMEAN_H_

/**
 * divide a number by a size_t
 *
 * \tparam T iterator
 * \tparam U iterator
 */
template<typename T>
inline T divide(const T& a, const size_t& b)
{
	return a/b;
}

//to do -- add more specializations here for other complex types if required
/**
 * divide a complex<float> by a size_t
 *
 * \tparam T iterator
 * \tparam U iterator
 */
template<>
inline std::complex<float> divide(const std::complex<float>& a, const size_t& b)
{
	return std::complex<float>(a.real()/b,a.imag()/b);
}

/**
 * Remove the mean from data in a container
 *
 * \tparam T iterator
 * \tparam U iterator
 */
template<typename T, typename U>
typename T::value_type zeroMean(T begin, T end, U out)
{
	typename T::value_type total=0;
    T i = begin;
    size_t count=0;
    for (; i!=end; i++, count++)
    {
        total+=*i;
    }
    typename T::value_type mean(divide(total, count));
    for (i=begin; i!=end; i++, out++)
    {
        *out=*i -mean;
    }
    return mean;
}

#endif /* ZEROMEAN_H_ */
