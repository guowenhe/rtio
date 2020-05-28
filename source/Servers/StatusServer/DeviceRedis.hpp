/*
 * DeviceRedis.hpp
 *
 *  Created on: 31 Mar 2020
 *      Author: wenhe
 */

#ifndef DEVICEREDIS_HPP_
#define DEVICEREDIS_HPP_
#include "RedisClient.hpp"


namespace RedisClient
{


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



} //RedisClient



#endif /* DEVICEREDIS_HPP_ */
