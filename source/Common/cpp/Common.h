/*
 * Common.h
 *
 *  Created on: Dec 21, 2019
 *      Author: wenhe
 */

#ifndef COMMON_H_
#define COMMON_H_

namespace Common
{
    inline time_t getClock()
    {
        struct timespec t;
        //provides access to a raw hardware-based time
        //not subject to NTP adjustments or the incremental adjustments performed by adjtime.
        int ret = clock_gettime(CLOCK_MONOTONIC_RAW, &t);
        if(ret)
        {
            return -1;
        }
        return  t.tv_sec;
    }

    inline time_t getClockCoarse()
    {
        struct timespec t;
        int ret = clock_gettime(CLOCK_MONOTONIC_COARSE, &t);
        if(ret)
        {
            return -1;
        }
        return  t.tv_sec;
    }

} //Common






#endif /* COMMON_H_ */
