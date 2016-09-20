#ifndef _gpuCacheTimeInterval_h_
#define _gpuCacheTimeInterval_h_

//-
//**************************************************************************/
// Copyright 2012 Autodesk, Inc. All rights reserved. 
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+


namespace GPUCache {


//==============================================================================
// CLASS TimeInterval
//==============================================================================

// Start index is inclusive, end index is exclusive.
class TimeInterval
{
public:
    enum InfiniteTag {
        kInfinite
    };

    enum InvalidTag{
        kInvalid
    };

    // Infinite time range!
    TimeInterval(InfiniteTag)
        : fStartTime(-std::numeric_limits<double>::max()),
        fEndTime(+std::numeric_limits<double>::max())
    {}

    // Invalid time range!
    TimeInterval(InvalidTag)
        : fStartTime(+std::numeric_limits<double>::max()),
        fEndTime(-std::numeric_limits<double>::max())
    {}

    // Assumes that time is stored in seconds.
    TimeInterval(double startTime, double endTime)
        : fStartTime(startTime),
        fEndTime(endTime)
    {}

    bool contains(double time) const
    {
        return fStartTime <= time && time < fEndTime;
    }

    bool contains(const TimeInterval& other) const
    {
        return fStartTime <= other.fStartTime && other.fEndTime <= fEndTime;
    }

    // Intersection of time ranges
    void operator&=(const TimeInterval& other) 
    {
        fStartTime = std::max(fStartTime, other.fStartTime);
        fEndTime   = std::min(fEndTime,   other.fEndTime);
    }

    TimeInterval operator&(const TimeInterval& other) 
    {
        TimeInterval res = *this;
        res &= other;
        return res;
    }

    // Union of time ranges
    void operator|=(const TimeInterval& other)
    {
        fStartTime = std::min(fStartTime, other.fStartTime);
        fEndTime   = std::max(fEndTime,   other.fEndTime);
    }

    TimeInterval operator|(const TimeInterval& other)
    {
        TimeInterval res = *this;
        res |= other;
        return res;
    }

    double startTime() const
    {
        return fStartTime;
    }

    double endTime() const
    {
        return fEndTime;
    }

    bool valid() const
    {
        return fStartTime < fEndTime;
    }

    bool operator==(const TimeInterval& other) 
    {
        return fStartTime == other.fStartTime && fEndTime == other.fEndTime;
    }

    bool operator!=(const TimeInterval& other) 
    {
        return fStartTime != other.fStartTime || fEndTime != other.fEndTime;
    }

private:
    double fStartTime;
    double fEndTime;
};


} // namespace GPUCache

#endif

