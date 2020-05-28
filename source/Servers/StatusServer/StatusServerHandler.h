/*
 * StatusServerHandler.h
 *
 *  Created on: Jan 6, 2020
 *      Author: wenhe
 */

#ifndef STATUSSERVERHANDLER_H_
#define STATUSSERVERHANDLER_H_

#include "MultiWorker.hpp"
#include "StatusServer.h"
#include "DeviceRedis.hpp"
#include "Common.h"

inline Ice::LoggerOutputBase& operator<<(Ice::LoggerOutputBase& out, DMS::ClientStatus status)
{
    switch(status)
    {
    case DMS::ClientStatus::UNKNOWN:
        out << "UNKNOWN";
        break;
    case DMS::ClientStatus::ONLINE:
        out << "ONLINE";
        break;
    case DMS::ClientStatus::OFFLINE:
        out << "OFFLINE";
        break;
    default:
        out << "no-desc(" << static_cast<int>(status) << ")";
    }

    return out;
}


struct DeviceStatusConfig: public MultiWorker::Config
{
    std::string redisHost;
    int redisPort = 0;

};

class DeviceStatusResource: public MultiWorker::Resource
{
private:
    const int _redisConnectCheckInterval = 5; // second
    const int _redisConnectTimeout       = 30; // second

public:
    void set(std::shared_ptr<MultiWorker::Config> config) override;
    void init() override;
    void check();

    std::shared_ptr<RedisClient::DeviceRedis> getRedis()
    {
        return _redis;
    }

private:
    std::shared_ptr<DeviceStatusConfig> _config;
    std::shared_ptr<RedisClient::DeviceRedis> _redis;
    time_t _clockPoint = 0;
};

class SetStatusHandler: public MultiWorker::Handler
{
public:
    SetStatusHandler(::std::shared_ptr<DMS::SetStatusReq>& req,
            ::std::function<void(const ::std::shared_ptr<DMS::SetStatusResp>& resp)>& response,
            const ::Ice::Current& current) :
            _req(req), _response(response), _current(current)
    {
    }

    virtual void run(MultiWorker::Resource*) override;

private:
    ::std::shared_ptr<DMS::SetStatusReq> _req;
    ::std::function<void(const ::std::shared_ptr<DMS::SetStatusResp>& resp)> _response;
    ::Ice::Current _current;
};

class QueryStatusHandler: public MultiWorker::Handler
{
public:
    QueryStatusHandler(::std::shared_ptr<DMS::QueryStatusReq>& req,
            ::std::function<void(const ::std::shared_ptr<DMS::QueryStatusResp>& resp)>& response,
            const ::Ice::Current& current) :
            _req(req), _response(response), _current(current)
    {
    }

    virtual void run(MultiWorker::Resource*) override;

private:
    ::std::shared_ptr<DMS::QueryStatusReq> _req;
    ::std::function<void(const ::std::shared_ptr<DMS::QueryStatusResp>& resp)> _response;
    ::Ice::Current _current;
};




#endif /* STATUSSERVERHANDLER_H_ */
