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

#ifndef VECTORMEAN_H
#define VECTORMEAN_H

#include <vector>

/**
 * Class to average and integer decimate framed vectors
 */
template<typename T, typename U=std::allocator<T>, typename V=std::allocator<T> >
class VectorMean {
public:
    VectorMean(size_t num, std::vector<T, U>& input, std::vector<T, V>& output);
    bool run();
    void setAvgNum(size_t val);
    size_t getAvgNum();
    void clear();
private:
    size_t count_;
    size_t numAvg_;
    float scale_;
    std::vector<T,U>& input_;
    std::vector<T,V>& output_;
};

template<typename T, typename U, typename V>
VectorMean<T,U,V>::VectorMean(size_t num, std::vector<T, U>& input, std::vector<T, V>& output) :
count_(0),
numAvg_(num-1),
scale_(1.0/num),
input_(input),
output_(output)
{}


template<typename T, typename U, typename V>
bool VectorMean<T,U,V>::run()
{
    if (input_.size() !=output_.size())
        count_=0;
    if(count_==0)
    {
        output_.assign(input_.begin(), input_.end());
    }else if (count_>=numAvg_)
    {
        for (size_t i=0; i!=input_.size(); i++)
            output_[i]=scale_*(output_[i]+input_[i]);
        count_=0;
        return true;
    }else
    {
        for (size_t i=0; i!=input_.size(); i++)
            output_[i]=output_[i]+input_[i];
    }
    count_++;
    return false;
}

template<typename T, typename U, typename V>
void VectorMean<T,U,V>::setAvgNum(size_t val)
{
    count_=0;
    numAvg_=val-1;
    scale_ = 1.0/val ;
}

template<typename T, typename U, typename V>
size_t VectorMean<T,U,V>::getAvgNum()
{
    return numAvg_+1;
}

template<typename T, typename U, typename V>
void VectorMean<T,U,V>::clear()
{
    count_=0;
}

template<typename T, typename U=std::allocator<T>, typename V=std::allocator<T> >
class VectorExponentialAvg {
public:
    VectorExponentialAvg(size_t num, std::vector<T, U>& input, std::vector<T, V>& output);
    bool run();
    void setAvgNum(size_t val);
    size_t getAvgNum();
    void clear();
private:
    void updateParams();
    size_t numAvg_;
    float alpha_;
    std::vector<T,U>& input_;
    std::vector<T,V>& output_;
};

template<typename T, typename U, typename V>
VectorExponentialAvg<T,U,V>::VectorExponentialAvg(size_t num, std::vector<T, U>& input, std::vector<T, V>& output) :
numAvg_(num),
input_(input),
output_(output)
{
    updateParams();
}

template<typename T, typename U, typename V>
bool VectorExponentialAvg<T,U,V>::run()
{
    if (input_.size() !=output_.size())
        output_.clear();
    if (output_.empty())
        output_.assign(input_.begin(), input_.end());
    else
    {
        typename std::vector<T,V>::iterator outIter = output_.begin();
        for(typename std::vector<T,U>::iterator inIter = input_.begin(); inIter!=input_.end(); inIter++, outIter++)
        {
            *outIter = *outIter + alpha_*(*inIter-*outIter);
        }
    }
    return true;
}

template<typename T, typename U, typename V>
void VectorExponentialAvg<T,U,V>::updateParams()
{
    float minAlpha = 1e-6;
    float maxAlpha = 1-minAlpha;
    //calculate the value of alpha so that after numAvg_ samples we are at 95% of our original value
    alpha_ = 1-std::pow(.05, 1.0/numAvg_);
    if (alpha_<minAlpha)
        alpha_ = minAlpha;
    else if (alpha_ > maxAlpha)
        alpha_=maxAlpha;
    std::cout<<"using alpha value of "<<alpha_<<std::endl;
}
template<typename T, typename U, typename V>
void VectorExponentialAvg<T,U,V>::setAvgNum(size_t val)
{
    numAvg_=val;
    updateParams();
}

template<typename T, typename U, typename V>
size_t VectorExponentialAvg<T,U,V>::getAvgNum()
{
    return numAvg_;
}
template<typename T, typename U, typename V>
void VectorExponentialAvg<T,U,V>::clear()
{
    output_.clear();
}


#endif /* VECTORMEAN_H */
