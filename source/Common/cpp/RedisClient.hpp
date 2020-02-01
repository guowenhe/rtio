/*
 * HiRedis.hpp
 *
 *  Created on: Jan 5, 2020
 *      Author: wenhe
 */

#ifndef REDIS_CLIENT_REDISCLIENT_HPP_
#define REDIS_CLIENT_REDISCLIENT_HPP_

#include "hiredis.h"
#include <tuple>


namespace RedisClient
{

enum class RetCode
{
    FAILED  = -1,
    SUCCEED = 0,
    EMPTY   = 1,
};

} // RedisClient




#ifdef REDIS_LOG_USING_STDIO
#include <iostream>

#define LOG_PREFIX __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << "| "
#define REDISLOG_E(content)\
    do\
    {\
        std::cout << "ERROR " << LOG_PREFIX << content << std::endl;\
    }\
    while(0)
#define REDISLOG_W(content)\
    do\
    {\
        std::cout << "WARN  " << LOG_PREFIX << content << std::endl;\
    }\
    while(0)
#define REDISLOG_I(content)\
    do\
    {\
        std::cout << "INFO  " << LOG_PREFIX << content << std::endl;\
    }\
    while(0)
#define REDISLOG_D(content)\
    do\
    {\
        std::cout << "DEBUG " << LOG_PREFIX << content << std::endl;\
    }\
    while(0)

#else //!REDIS_LOG_USING_STDIO
#include "ServerGlobal.h"
#include "VerijsLog.h"

#define REDISLOG_E(content) log2E(ServerGlobal::getInstance()->getCommunicator(), content)
#define REDISLOG_W(content) log2W(ServerGlobal::getInstance()->getCommunicator(), content)
#define REDISLOG_I(content) log2I(ServerGlobal::getInstance()->getCommunicator(), content)
#define REDISLOG_D(content) log2D(ServerGlobal::getInstance()->getCommunicator(), content)

inline Ice::LoggerOutputBase& operator<<(Ice::LoggerOutputBase& out, RedisClient::RetCode code)
{
    switch(code)
    {
    case RedisClient::RetCode::FAILED:
        out << "FAILED";
        break;
    case RedisClient::RetCode::SUCCEED:
        out << "SUCCEED";
        break;
    case RedisClient::RetCode::EMPTY:
        out << "EMPTY";
        break;
    default:
        out << "no-desc(" << static_cast<int>(code) << ")";
    }
    return out;
}
#endif //REDIS_LOG_USING_STDIO



namespace RedisClient
{

class HiRedis
{
public:
    ~HiRedis()
    {
        disConnect();
    }
    RetCode connect(const std::string& host, int port, int timeout) // timeout unit is second
    {
        REDISLOG_I("host=" << host << " port=" << port << " timeout=" << timeout);
        struct timeval t = { timeout, 0 };
        _redisContext = redisConnectWithTimeout(host.c_str(), port, t);
        if(_redisContext == NULL || _redisContext->err)
        {
            if(_redisContext)
            {
                REDISLOG_E("Connection error: " << _redisContext->errstr);
//                redisFree(_redisContext);
            }
            else
            {
                REDISLOG_E("Connection error: can't allocate redis context");
            }
            return RetCode::FAILED;
        }
        return RetCode::SUCCEED;
    }
    void disConnect()
    {
        REDISLOG_I("-");
        if(nullptr != _redisContext)
        {
            redisFree(_redisContext);
        }
    }

    void unexpectedReply(const redisReply* reply, const std::string& mark)
    {
        if(reply->type == REDIS_REPLY_ERROR)
        {
            REDISLOG_E(mark << ": str=" << reply->str);
        }
        else
        {
            REDISLOG_D(mark << ": type=" << reply->type << "(STRING 1, ARRAY 2, INTEGER 3, NIL 4, STATUS 5, ERROR 6, DOUBLE 7, BOOL 8, VERB 9, MAP 9, SET 10, ATTR 11, PUSH 12, BIGNUM 13)");
        }
    }
    bool requireRedisCheck()
    {
//        REDISLOG_D("_requireCheck=" << (int)_requireCheck);
        return _requireCheck;
    }

    bool ping()
    {
        _requireCheck = false;
        redisReply* reply = (redisReply *)redisCommand(_redisContext, "PING");
        if(nullptr == reply)
        {
            REDISLOG_E("reply=" << "NULL");
            return false;
        }
        if(reply->type != REDIS_REPLY_STATUS || strcmp(reply->str,"PONG") != 0)
        {
            REDISLOG_E("reply=[" << reply->str << "]");
            unexpectedReply(reply, __FUNCTION__);
            return false;
        }
        REDISLOG_I("succeed");
        return true;
    }

    bool reconnect()
    {
        if(REDIS_OK == redisReconnect(_redisContext))
        {
            REDISLOG_I("succeed");
            return true;
        }
        REDISLOG_E("failed");
        return false;
    }

protected:

    void checkConnection()
    {
        _requireCheck = true;
    }
    redisContext* _redisContext;
    bool _requireCheck = false;
};




struct DiviceInfo
{
    int status = 0;
    std::string proxy;

    using T = const char*;
    const std::tuple<T, T> _fields =
            std::make_tuple("status", "proxy");
};

class DeviceRedis: public HiRedis
{
public:
    RetCode setDeviceInfo(const std::string& deviceId, const DiviceInfo& info)
    {
        REDISLOG_D("deviceId=" << deviceId << " status=" << info.status << " proxy=" << info.proxy);
        redisReply* reply = (redisReply *)redisCommand(_redisContext,"HSET device:%s %s %d %s %s", + deviceId.c_str(),
                std::get<0>(info._fields), info.status, std::get<1>(info._fields), info.proxy.c_str());

        if(nullptr == reply)
        {
            REDISLOG_E("reply=" << "NULL");
            checkConnection();
            return RetCode::FAILED;
        }

        RetCode ret = RetCode::FAILED;
        if(reply->type == REDIS_REPLY_INTEGER)
        {
            REDISLOG_D("reply integer=" << reply->integer);
            ret = RetCode::SUCCEED;
        }
        else
        {
            unexpectedReply(reply, __FUNCTION__);
        }

        if(RetCode::SUCCEED != ret)
        {
            REDISLOG_E("HSET failed deviceId=" << deviceId);
        }

        freeReplyObject(reply);
        return ret;
    }
    RetCode getDeviceInfo(const std::string& deviceId, DiviceInfo& info)
    {
        REDISLOG_D("deviceId=" << deviceId);

        redisReply* reply = (redisReply *)redisCommand(_redisContext, "HMGET device:%s %s %s", deviceId.c_str(),
               std::get<0>(info._fields), std::get<1>(info._fields));

        if(nullptr == reply)
        {
            REDISLOG_E("reply=" << "NULL");
            checkConnection();
            return RetCode::FAILED;
        }

        RetCode ret = RetCode::FAILED;
        if(reply->type == REDIS_REPLY_ARRAY)
        {
            const int fieldSize =  std::tuple_size<decltype(info._fields)>::value;
            if(reply->elements < fieldSize)
            {
                REDISLOG_E("field is missing, reply elements=" << reply->elements << " fieldSize=" << fieldSize);
            }
            else
            {
                if(nullptr == reply->element[0] || nullptr == reply->element[1])
                {
                    REDISLOG_W("reply element=" << "NULL");
                }
                else
                {
                    if(nullptr == reply->element[0]->str || nullptr == reply->element[1]->str )
                    {
                        REDISLOG_W("reply element empty");
                        ret = RetCode::EMPTY;
                    }
                    else
                    {
                        info.status = std::stoi(reply->element[0]->str);
                        info.proxy.assign(reply->element[1]->str, reply->element[1]->len);
                        ret = RetCode::SUCCEED;
                    }
                }
            }
        }
        else
        {
            unexpectedReply(reply, __FUNCTION__);
        }

        if(RetCode::SUCCEED != ret && RetCode::EMPTY != ret)
        {
            REDISLOG_E("HMGET failed deviceId=" << deviceId);
        }
        freeReplyObject(reply);
        return ret;
    }

};












} //HiRedis





//    int set(const std::string& key, const std::string& value, int timeout=0, bool isBinary=false) // timeout unit is second
//    {
//        redisReply* reply = (redisReply *)redisCommand(_redisContext,"SET %s %s", key.c_str(), value.c_str());
//        printf("SET: %s\n", reply->str);
//
//        int ret = -1;
//        if(reply->type == REDIS_REPLY_STRING && reply->str == "OK")
//        {
//            ret = 0;
//        }
//        freeReplyObject(reply);
//        return ret;
//    }
//    int get(const std::string& key, std::string& value)
//    {
//        redisReply* reply = (redisReply *)redisCommand(_redisContext,"GET %s", key.c_str());
//        printf("SET: %s\n", reply->str);
//
//        int ret = -1;
//        if(reply->type == REDIS_REPLY_STRING)
//        {
//            value = reply->str;
//            ret = 0;
//        }
//        freeReplyObject(reply);
//        return ret;
//    }

#endif /* REDIS_CLIENT_REDISCLIENT_HPP_ */
