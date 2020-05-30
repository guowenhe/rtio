/*
 * RemoteCode.h
 *
 *  Created on: Jan 12, 2020
 *      Author: wenhe
 */

#ifndef REMOTECODE_H_
#define REMOTECODE_H_

#include <string>

#include "RtioExcept.h"
#include "RtioLog.h"

namespace RC // remote code
{

enum class Code: int
{
    //code is 4 digits，as position 1234, 12 - module number, 34 - error number
    //common
    SUCCESS             = 1000,
    FAIL                = 1999, // unknown error, failed
    TIMEOUT             = 1002,
    SESSION_INVALID     = 1003,
    NOTFOUND            = 1004,
    PROXY_INVALID       = 1005,
    // api common
    API_URI_INVALID          = 1600,
    API_BODY_INVALID         = 1601,
    API_METHOD_ERROR         = 1602,
    API_JSON_PARSE_FAIL      = 1603,
    API_JSON_FIELD_INVALID   = 1604,
    API_DEVICE_NOTFOUND      = 1605,

    // access server
    ACCESSSERVER_SESSION_INVALID    = 2000,
    ACCESSSERVER_FAILED             = 2001,
    ACCESSSERVER_TIMEOUT            = 2002,

    // device message hub server
    DEVICEHUB_DIVICE_AS_FAILED    = 2100,
    DEVICEHUB_DIVICE_AS_TIMEOUT   = 2101,
    DEVICEHUB_DIVICE_NOTFUND      = 2102,
    DEVICEHUB_DIVICE_NOT_ONLINE   = 2103,

    // status server
    STATUSSERVER_DIVICE_NOTFUND  = 2200, // when query device status

    // api sender
    API_SENDER_URI_INVALID          = API_URI_INVALID,
    API_SENDER_BODY_INVALID         = API_BODY_INVALID,
    API_SENDER_METHOD_ERROR         = API_METHOD_ERROR,
    API_SENDER_JSON_PARSE_FAIL      = API_JSON_PARSE_FAIL,
    API_SENDER_JSON_FIELD_INVALID   = API_JSON_FIELD_INVALID,
    API_SENDER_DISPATCH_FAIL        = 2305,
    API_SENDER_DEVICE_NOTFOUND      = API_DEVICE_NOTFOUND,

    // api notifier
    API_NOTIFIER_TIMEOUT            = 2400,

    // message reporter
    MESSAGEREPORTER_REDIS_FAIL      = 2500,
    MESSAGEREPORTER_TRYING          = 2501,
    MESSAGEREPORTER_DEVICEID_INVALID= 2502,

    // message reporter
    DEVICEMANAGER_REDIS_FAIL        = 2600,
    DEVICEMANAGER_ID_ERROR          = 2601,
    DEVICEMANAGER_ID_NOTFUND        = 2602,
    DEVICEMANAGER_KEY_ERROR         = 2603,

    // api manage
    API_MANAGE_URI_INVALID          = API_URI_INVALID,
    API_MANAGE_BODY_INVALID         = API_BODY_INVALID,
    API_MANAGE_METHOD_ERROR         = API_METHOD_ERROR,
    API_MANAGE_JSON_PARSE_FAIL      = API_JSON_PARSE_FAIL,
    API_MANAGE_JSON_FIELD_INVALID   = API_JSON_FIELD_INVALID,
    API_MANAGE_CREATE_DEVICE_FAIL   = 2705,


};

constexpr const char* describe(Code code)
{
    return  Code::SUCCESS == code ?             "SUCCESS":
            Code::FAIL == code ?                "FAIL":
            Code::TIMEOUT == code ?             "TIMEOUT":
            Code::SESSION_INVALID == code ?     "SESSION_INVALID":
            Code::NOTFOUND == code ?            "NOTFOUND":
            Code::PROXY_INVALID == code ?       "PROXY_INVALID":

            Code::API_URI_INVALID == code ?          "URI_INVALID":
            Code::API_BODY_INVALID == code ?         "BODY_INVALID":
            Code::API_METHOD_ERROR == code ?         "METHOD_ERROR":
            Code::API_JSON_PARSE_FAIL == code ?      "JSON_PARSE_FAIL":
            Code::API_JSON_FIELD_INVALID == code ?   "JSON_FIELD_INVALID":
            Code::API_DEVICE_NOTFOUND == code ?       "DEVICE_NOTFOUND ":

            Code::MESSAGEREPORTER_REDIS_FAIL == code ?          "MESSAGEREPORTER_REDIS_FAIL":
            Code::MESSAGEREPORTER_TRYING == code ?              "MESSAGEREPORTER_TRYING":
            Code::MESSAGEREPORTER_DEVICEID_INVALID == code ?    "MESSAGEREPORTER_DEVICEID_INVALID":

            Code::API_SENDER_DISPATCH_FAIL == code ?        "DISPATCH_FAILED ":
            Code::API_MANAGE_CREATE_DEVICE_FAIL == code ?   "CREATE_DEVICE_FAIL":
                                                            "undefined";


}

//constexpr const char* describeExternal(Code code)
//{
//    return  Code::SUCCESS == code ?             "SUCCESS":
//            Code::FAIL == code ?                "FAIL":
//            Code::API_SENDER_URI_INVALID == code ?          "URI_INVALID":
//            Code::API_SENDER_BODY_INVALID == code ?         "BODY_INVALID":
//            Code::API_SENDER_METHOD_ERROR == code ?         "METHOD_ERROR":
//            Code::API_SENDER_JSON_PARSE_FAIL == code ?      "JSON_PARSE_FAIL":
//            Code::API_SENDER_JSON_FIELD_INVALID == code ?   "JSON_FIELD_INVALID":
//            Code::API_SENDER_DISPATCH_FAIL == code ?        "DISPATCH_FAIL ":
//                                                            "undefined";
//}

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
    Except(Code code) :
            Rtio::Except<Code>(code, describe(code))
    {
    }
    Except(Code code, const std::string& message) :
            Rtio::Except<Code>(code, message)
    {
    }
    // could use macro RC_ExceptEx instead
    Except(Code code, const std::string& message, const std::string& where) :
            Rtio::Except<Code>(code, message, where)
    {
    }
};
#define RC_ExceptEx(code) RC::Except(code, RC::describe(code), Rtio_where())
#define RC_ExceptEx2(code, message) RC::Except(code, message, Rtio_where())
} // RC





#endif /* REMOTECODE_H_ */
