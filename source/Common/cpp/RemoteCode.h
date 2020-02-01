/*
 * RemoteCode.h
 *
 *  Created on: Jan 12, 2020
 *      Author: wenhe
 */

#ifndef REMOTECODE_H_
#define REMOTECODE_H_

#include <string>
#include "VerijsLog.h"

namespace RC // remote code
{

enum class Code: int
{
    //code is 4 digits，as position 1234, 12 - module number, 34 - error number
    //common
    SUCCEED             = 1000,
    FAILED              = 1999, // unknown error, failed
    TIMEOUT             = 1002,
    SESSION_INVALID     = 1003,
    NOTFOUND            = 1004,
    PROXY_INVALID       = 1005,

    // access server special
    ACCESSSERVER_SESSION_INVALID    = 2000,
    ACCESSSERVER_FAILED             = 2001,
    ACCESSSERVER_TIMEOUT            = 2002,

    // device message hub server special
    DEVICEHUB_DIVICE_AS_FAILED   = 2100,
    DEVICEHUB_DIVICE_AS_TIMEOUT   = 2101,
    DEVICEHUB_DIVICE_NOTFUND   = 2102,
    DEVICEHUB_DIVICE_NOT_ONLINE   = 2103,

    // status server special
    STATUSSERVER_DIVICE_NOTFUND  = 2200, // when query device status

};

constexpr const char* Describe(Code code)
{
    return  Code::SUCCEED == code ?             "SUCCEED":
            Code::FAILED == code ?              "FAILED":
            Code::TIMEOUT == code ?             "TIMEOUT":
            Code::SESSION_INVALID == code ?     "SESSION_INVALID":
            Code::NOTFOUND == code ?            "NOTFOUND":
            Code::PROXY_INVALID == code ?       "PROXY_INVALID":
                                                "undefined";
}

inline Ice::LoggerOutputBase& operator<<(Ice::LoggerOutputBase& out, Code code)
{
    out << static_cast<int>(code) << "(" << Describe(code) << ")";
    return out;
}
inline std::ostream& operator<<(std::ostream& out, Code code)
{
    out << static_cast<int>(code) << "(" << Describe(code) << ")";
    return out;
}

inline const std::string _where(const char* file, const int line, const char* fun) // todo using constexpr
{
    std::stringstream ss;
    ss << "(" << file << ":" << line << ":" << fun << ")";
    return ss.str();
}

#define where() _where(__FILE__, __LINE__, __FUNCTION__)


} // remote error code





#endif /* REMOTECODE_H_ */
