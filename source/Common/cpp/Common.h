/*
 * Common.h
 *
 *  Created on: Dec 21, 2019
 *      Author: wenhe
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <atomic>
#include <random>

#include <string>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>

namespace Util
{
    constexpr const char* getFileName(const char* const name)
    {
        const char* p = name;
        for(const auto* q = name; *q != '\0'; ++q)
        {
            if(*q == '\\' || *q == '/')
            {
                p = q;
            }
        }
        return p != name ? ++p : name;
    }

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

    inline unsigned int getSerianNumber()
    {
        std::default_random_engine e;
        e.seed(clock());
        static std::atomic_uint serial(e());
        return serial++;
    }

    inline void urlQueryStringParse(const std::string& queryString, std::map<std::string, std::string>& queryMap)
    {
        std::vector<std::string> v1;
        boost::split(v1, queryString, boost::is_any_of("&"), boost::token_compress_on);

        for(auto& c: v1)
        {
            std::vector<std::string> v2;
            boost::split(v2, c, boost::is_any_of("="), boost::token_compress_on);
            if(v2.size() == 2)
            {
                queryMap[v2[0]] = v2[1];

            }
            else
            {
                queryMap[v2[0]] = "";
            }
        }
    }

    inline void urlQueryStringSerial(const std::map<std::string, std::string>& queryMap, std::string& queryString)
    {
        for(auto& c: queryMap)
        {
            queryString += c.first + "=" + c.second + "&";
        }
        queryString.resize(queryString.size() - 1);
    }

} //Util






#endif /* COMMON_H_ */
