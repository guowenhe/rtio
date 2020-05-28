/*
 * Common.h
 *
 *  Created on: Dec 21, 2019
 *      Author: wenhe
 */

#ifndef COMMON_H_
#define COMMON_H_
#include <string>
#include <map>


#define RTIO_SERVER_STRING "rtio/0.0.1"


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

    class IdGenerator
    {
    private:
        IdGenerator() : _index(0)
        {
        }
    public:
        static IdGenerator* getInstance();
        void setIndex(uint8_t index);
        uint64_t get();     // 10W = 35419us(with clock)
    private:
        uint8_t _index;
    };

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

    constexpr bool isLittleEndian()
    {
        return (std::endian::native == std::endian::little);
    }


    // base64
    std::string base64Encode(std::string_view s);
    std::string base64Decode(std::string_view s);

    // url
    void urlQueryStringParse(const std::string_view& queryString, std::map<std::string, std::string>& queryMap);
    void urlQueryStringSerial(const std::map<std::string, std::string>& queryMap, std::string& queryString);

    // bin to hex
    std::string binToHex(std::string_view a);



} //Util






#endif /* COMMON_H_ */
