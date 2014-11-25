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

#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include <vector>
#include <boost/thread/mutex.hpp>

/**
 * \brief Class to buffer data for you and frame it up
 *
 * You get consistant frame sizes out no matter how much input is pushed in.
 * You can choose to overlap or skip data for the frames. If you set overlap > 0
 * then you overlap output data. If you set overlap < 0 then you throw data away.
 * This class works with iterators so that data copies are reduced to an absolute
 * minimium.
 */
template <typename iterT>
class framebuffer {
public:
    framebuffer(size_t frameSize, long overlap=0);
    virtual ~framebuffer();

    typedef typename iterT::value_type valueT;
    typedef typename std::vector<valueT> vectorT;

    struct frame
    {
        iterT begin;
        iterT end;
    };

    /**
     * Call this method in a loop to frame up your data
     *
     * \param output a vector of data frames for you to operate on
     */
    void newData(frame input, std::vector<frame>& output);

    /**
     * Convienience function for vectors - but the same as \ref #newData(frame, std::vector<frame>&)
     */
    void newData(vectorT& input, std::vector<frame>& output);

    //modify the frame parameters
    void setFrameSize(size_t frameSize);
    size_t getFrameSize();
    void setOverlap(long overlap);
    long getOverlap();
    void flush();

private:
    void updateInternals();
    size_t getNumFrames(size_t dataElements);
    vectorT vecA_;
    vectorT vecB_;
    vectorT* last_;
    vectorT* next_;

    size_t frameSize_;
    long overlap_;

    size_t stride_;
    // throwAwayIndex_ is used because we are throwing data away
    // because the overlap is negative and the user has requested this usage
    size_t throwAwayIndex_;

    boost::mutex boostLock_;

};

#endif /* FRAMEBUFFER_H_ */
