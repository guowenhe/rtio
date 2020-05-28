/*
 * DeviceManagerRedis.hpp
 *
 *  Created on: 31 Mar 2020
 *      Author: wenhe
 */

#ifndef DEVICEMANAGERREDIS_HPP_
#define DEVICEMANAGERREDIS_HPP_

#include "RedisClient.hpp"
#include "DeviceManager.h"

namespace RedisClient
{

#define DEVICE_MANAGER_TABLE "devmgr"

class DeviceManagerRedis: public HiRedis
{
public:
    RetCode saveDevice(const std::string& deviceId, const std::string& deviceKey)
    {
        REDISLOG_D("deviceId=" << deviceId);

        redisReply* reply = (redisReply *)redisCommand(_redisContext,
                "HSET " DEVICE_MANAGER_TABLE ":%s " // deviceId
                "%s %s",          // deviceKey
                deviceId.c_str(),
                "key", deviceKey.c_str());

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

    RetCode getDeviceKey(const std::string& deviceId, std::string& deviceKey)
    {
        REDISLOG_D("deviceId=" << deviceId);

        redisReply* reply = (redisReply*) redisCommand(_redisContext,
                "HGET " DEVICE_MANAGER_TABLE ":%s " // deviceId
                "%s",// deviceKey
                deviceId.c_str(),
                "key");

        if(nullptr == reply)
        {
            REDISLOG_E("reply=" << "NULL");
            return RetCode::FAILED;
        }

        RetCode ret = RetCode::FAILED;
        if(reply->type == REDIS_REPLY_STRING && nullptr != reply->str)
        {
            deviceKey = reply->str;
            ret = RetCode::SUCCEED;
        }
        else if(reply->type == REDIS_REPLY_NIL)
        {
            ret = RetCode::EMPTY;
        }
        else
        {
            unexpectedReply(reply, __FUNCTION__);
        }

        if(RetCode::SUCCEED != ret && RetCode::EMPTY != ret)
        {
            REDISLOG_E("HGET failed deviceId=" << deviceId);
        }
        freeReplyObject(reply);
        return ret;
    }

};





} //RedisClient






#endif /* DEVICEMANAGERREDIS_HPP_ */
