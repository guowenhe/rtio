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
#include "RtioExcept.h"

namespace RC // remote code
{

enum class Code: int
{
    //code is 4 digits，as position 1234, 12 - module number, 34 - error number
    //common
    SUCCESS             = 1000,
    FAIL              = 1999, // unknown error, failed
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

    // api sender
    API_SENDER_URI_INVALID          = 2300,
    API_SENDER_BODY_INVALID         = 2301,
    API_SENDER_METHOD_ERROR         = 2302,
    API_SENDER_JSON_PARSE_FAIL      = 2303,
    API_SENDER_JSON_FIELD_INVALID   = 2304,
    API_SENDER_SEND_FAILED          = 2305,

};

constexpr const char* describe(Code code)
{
    return  Code::SUCCESS == code ?             "SUCCESS":
            Code::FAIL == code ?                "FAIL":
            Code::TIMEOUT == code ?             "TIMEOUT":
            Code::SESSION_INVALID == code ?     "SESSION_INVALID":
            Code::NOTFOUND == code ?            "NOTFOUND":
            Code::PROXY_INVALID == code ?       "PROXY_INVALID":
            Code::API_SENDER_URI_INVALID == code ?          "URI_INVALID":
            Code::API_SENDER_BODY_INVALID == code ?         "BODY_INVALID":
            Code::API_SENDER_METHOD_ERROR == code ?         "METHOD_ERROR":
            Code::API_SENDER_JSON_PARSE_FAIL == code ?      "JSON_PARSE_FAIL":
            Code::API_SENDER_JSON_FIELD_INVALID == code ?   "JSON_FIELD_INVALID":
            Code::API_SENDER_SEND_FAILED == code ?          "SEND_FAILED ":
                                                            "undefined";
}

constexpr const char* describeExternal(Code code)
{
    return  Code::SUCCESS == code ?             "SUCCESS":
            Code::FAIL == code ?                "FAIL":
            Code::API_SENDER_URI_INVALID == code ?          "URI_INVALID":
            Code::API_SENDER_BODY_INVALID == code ?         "BODY_INVALID":
            Code::API_SENDER_METHOD_ERROR == code ?         "METHOD_ERROR":
            Code::API_SENDER_JSON_PARSE_FAIL == code ?      "JSON_PARSE_FAIL":
            Code::API_SENDER_JSON_FIELD_INVALID == code ?   "JSON_FIELD_INVALID":
            Code::API_SENDER_SEND_FAILED == code ?          "SEND_FAILED ":
                                                            "undefined";
}

constexpr int codeToInt(Code code)
{
    return static_cast<int>(code);
}
constexpr Code intToCode(int code)
{
    return static_cast<Code>(code);
}

inline Ice::LoggerOutputBase& operator<<(Ice::LoggerOutputBase& out, Code code)
{
    out << static_cast<int>(code) << "(" << describe(code) << ")";
    return out;
}
inline std::ostream& operator<<(std::ostream& out, Code code)
{
    out << static_cast<int>(code) << "(" << describe(code) << ")";
    return out;
}

class Except: public Rtio::Except<Code>
{
public:
    Except(Code code, const std::string& message) :
            Rtio::Except<Code>(code, message)
    {
    }
    Except(Code code, const std::string& message, const std::string& where) :
            Rtio::Except<Code>(code, message, where)
    {
    }
};
#define RC_ExceptWithDesc(code) RC::Except(code, describe(code), Rtio_where())


} // remote error code





#endif /* REMOTECODE_H_ */
