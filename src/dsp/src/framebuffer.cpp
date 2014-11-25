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

#include "framebuffer.h"
#include <assert.h>
#include <iostream>
#include <complex>

template <typename iterT>
framebuffer<iterT>::framebuffer(size_t frameSize, long overlap) :
last_(&vecA_),
next_(&vecB_),
frameSize_(frameSize),
overlap_(overlap),
throwAwayIndex_(0)

{
	updateInternals();
}

template <typename iterT>
framebuffer<iterT>::~framebuffer() {}

//compute the number of valid fames per startIndex
template <typename iterT>
size_t framebuffer<iterT>::getNumFrames(size_t dataElements)
{
	if (dataElements < frameSize_)
		return 0;
	//this is the largest startIndex at which you can have a full frame of data
	long lastValidStartIndex =  dataElements-frameSize_;
	//Here is the equation to solve to compute the number of frames
	// you need the minus one because numFrames is ones based counting
	//(numFrames-1)*stride_<= lastValidStartIndex
	//so the solution is:
	return static_cast<size_t>(lastValidStartIndex/stride_+1);
}

template <typename iterT>
void framebuffer<iterT>::flush()
{
	boost::mutex::scoped_lock lock(boostLock_);
	last_->clear();
	next_->clear();
	throwAwayIndex_=0;
}

template <typename iterT>
void framebuffer<iterT>::newData(frame input, std::vector<frame>& output)
{
	size_t inputSize = input.end-input.begin;
	size_t numLeftover = last_->size();
	//you can't have data leftover from last loop if you are throwing data away!
	assert(numLeftover==0 || throwAwayIndex_==0);

	iterT dataIter = input.begin;
	//do case where we are throwing data away because we are not at a frame boundary
	//if we didn't get enough data -- decrement the index and return
	if (throwAwayIndex_ > inputSize)
	{
		output.clear();
		throwAwayIndex_-=inputSize;
		return;
	}
	else if(throwAwayIndex_>0)
	{
		//if we are throwing away data but we have enough
		//update our internal data members and process
		dataIter+=throwAwayIndex_;
		inputSize-=throwAwayIndex_;
		throwAwayIndex_=0;
	}
	//the total data we have to work with this loop
	size_t dataSize = inputSize+numLeftover;

	{
		boost::mutex::scoped_lock lock(boostLock_);

		size_t numOutputFrames = getNumFrames(dataSize);
		output.resize(numOutputFrames);
		//handle case where we got data but we don't have enough to do a full
		//frame.  Just append the data to last and call it for this loop
		if (numOutputFrames==0)
		{
			copy(dataIter,input.end, back_inserter(*last_));
			return;
		}
		//if we make it here we will new output data
		next_->clear();
		size_t frameIndex=0;
		//if we have leftover data output frames based upon the internal last vector we have stored
		if (numLeftover!=0)
		{
			//this is the number of frames necessary to make use of all the leftover data
			size_t numLeftoverFramesDesired = (numLeftover+stride_-1)/stride_;
			size_t numLeftoverFrames = numLeftoverFramesDesired;
			//check for the case where we don't have enough new data to output all the frames
			if (numLeftoverFramesDesired>numOutputFrames)
				numLeftoverFrames= numOutputFrames;
	//		std::cout<<"numLeftoverFrames = "<<numLeftoverFrames<<std::endl;
	//		std::cout<<"numLeftoverFramesDesired = "<<numLeftoverFramesDesired<<std::endl;
			size_t numLefoverElementsNeeded = (numLeftoverFrames-1)*stride_+frameSize_;
			//we should never have more data needed than we have data available
			assert((numLefoverElementsNeeded<=dataSize));
			//check for the case where we need to copy some of the input to get an exact integer number of frames out
			//this will typically be the case
			if (numLefoverElementsNeeded>numLeftover)
			{
				size_t numCopy = numLefoverElementsNeeded-numLeftover;
				copy(dataIter,dataIter+numCopy, back_inserter(*last_));
			}
			//now output data from the last vector
			iterT lastIter = last_->begin();
			for (;frameIndex!=numLeftoverFrames; frameIndex++)
			{
				output[frameIndex].begin =lastIter;
				output[frameIndex].end =lastIter+frameSize_;
				lastIter+=stride_;
			}
			//not enough data to output everything that is desired - copy data to next that we will need later
			if (numLeftoverFramesDesired!=numLeftoverFrames)
			{
				next_->assign(lastIter, last_->begin()+numLeftover);
			}
			else
			{
				//we've copied all the data we need from last -
				//update the dataIter so that it reflects the start of the next frame of data
				//using only new data
				size_t newInputIndex = numLeftoverFrames*stride_-numLeftover;
	//			std::cout<<"newInputIndex "<< newInputIndex<<std::endl;
	//			std::cout<<"inputSize "<< inputSize<<std::endl;
				dataIter += newInputIndex;
			}
		}
		//output any frames using only new data
		for (;frameIndex!=numOutputFrames; frameIndex++)
		{
			output[frameIndex].begin =dataIter;
			output[frameIndex].end =dataIter+frameSize_;
			dataIter+=stride_;
		}
	}

	//prepair for next loop
	//check to see if we have too much extra data
	//or too little (if we are throwing data away) and update accordingly
	if (dataIter > input.end)
		throwAwayIndex_=dataIter-input.end;
	else if (input.end > dataIter)
		//copy any unused new data to the next iterator to use on future outputs
		copy(dataIter,input.end, back_inserter(*next_));
	//swap the next and last iterators for next go
	vectorT* tmp=next_;
	next_=last_;
	last_=tmp;
}


template <typename iterT>
void framebuffer<iterT>::newData(vectorT& input, std::vector<frame>& output)
{
	frame inFrame;
	inFrame.begin = input.begin();
	inFrame.end = input.end();
	newData(inFrame,output);
}


template <typename iterT>
void framebuffer<iterT>::setFrameSize(size_t frameSize)
{
	frameSize_=frameSize;
	updateInternals();
}

template <typename iterT>
size_t framebuffer<iterT>::getFrameSize()
{
	return frameSize_;
}

template <typename iterT>
void framebuffer<iterT>::setOverlap(long overlap)
{
	overlap_ = overlap;
	updateInternals();
}

template <typename iterT>
long framebuffer<iterT>::getOverlap()
{
	return overlap_;
}

template <typename iterT>
void framebuffer<iterT>::updateInternals()
{
	boost::mutex::scoped_lock lock(boostLock_);
	assert(static_cast<long>(frameSize_) > overlap_);
	stride_ = frameSize_-overlap_;
}

template class framebuffer<std::vector<float>::iterator>;
template class framebuffer<std::vector<std::complex<float> >::iterator>;
