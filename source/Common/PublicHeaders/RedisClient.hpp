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
#include "RtioLog.h"

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





} //RedisClient

#endif /* REDIS_CLIENT_REDISCLIENT_HPP_ */
