/*
 * ReporterRedis.hpp
 *
 *  Created on: 31 Mar 2020
 *      Author: wenhe
 */

#ifndef REPORTERREDIS_HPP_
#define REPORTERREDIS_HPP_

#include "RedisClient.hpp"
#include "MessageReporter.h"

namespace RedisClient
{


enum class NotifyStatus
{
    INIT = 0,
    TRYING,
    SUCCESS,
    FAIL,
};


class ReporterRedis: public HiRedis
{
public:
    RetCode saveMessage(const std::shared_ptr<DMS::ReportReq> req, time_t retryPoint)
    {
        REDISLOG_D("deviceId=" << req->deviceId << " messageId=" << req->messageId);

        redisReply* reply = (redisReply *)redisCommand(_redisContext,
                "HSET msg_r:%s%s%d " // key
                "%s %s "             // message
                "%s %d "             // retry_times
                "%s %d "             // retry_point
                "%s %d",             // status
                req->deviceId.c_str(), ":", req->messageId,
                "message", req->message.c_str(),
                "retry_times", 1,
                "retry_point", retryPoint,
                "status", static_cast<int>(NotifyStatus::TRYING));

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
            REDISLOG_E("HSET failed deviceId=" << req->deviceId);
        }

        freeReplyObject(reply);
        return ret;
    }

    RetCode setMessageTrying(const std::shared_ptr<DMS::ReportReq> req, time_t retryPoint)
    {
        REDISLOG_D("deviceId=" << req->deviceId << " messageId=" << req->messageId);

         redisReply* reply = (redisReply *)redisCommand(_redisContext,
                 "HSET msg_r:%s%s%d "    // key
                 "%s %d "                // retry_times
                 "%s %d",                // status
                 req->deviceId.c_str(), ":", req->messageId,
                 "retry_point", retryPoint,
                 "status", static_cast<int>(NotifyStatus::TRYING));
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
             REDISLOG_E("HSET failed deviceId=" << req->deviceId);
         }

         freeReplyObject(reply);
         return ret;
    }

    RetCode setMessageSuccess(const std::shared_ptr<DMS::ReportReq> req)
    {
        REDISLOG_D("deviceId=" << req->deviceId << " messageId=" << req->messageId);

         redisReply* reply = (redisReply *)redisCommand(_redisContext,
                 "HSET msg_r:%s%s%d " // key
                 "%s %d",    // status
                 req->deviceId.c_str(), ":", req->messageId,
                 "status", static_cast<int>(NotifyStatus::SUCCESS));

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
             REDISLOG_E("HSET failed deviceId=" << req->deviceId);
         }

         freeReplyObject(reply);
         return ret;
     }
};

} //RedisClient






#endif /* REPORTERREDIS_HPP_ */
