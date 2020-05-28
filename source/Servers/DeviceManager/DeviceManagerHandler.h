/*
 * DeviceManagerHandler.h
 *
 *  Created on: 4 Apr 2020
 *      Author: wenhe
 */

#ifndef DEVICEMANAGERHANDLER_H_
#define DEVICEMANAGERHANDLER_H_

#include "MultiWorker.hpp"
#include "RemoteCode.h"
#include "DeviceManager.h"

#include "DeviceManagerRedis.hpp"

struct DeviceManagerConfig: public MultiWorker::Config
{
    std::string redisHost;
    int redisPort = 0;
};

class DeviceManagerResource: public MultiWorker::Resource
{
private:
    const int _redisConnectCheckInterval = 5; // second
    const int _redisConnectTimeout       = 30; // second

public:
    void set(std::shared_ptr<MultiWorker::Config> config) override;
    void init() override;
    void check();

    std::shared_ptr<RedisClient::DeviceManagerRedis> getRedis()
    {
        return _redis;
    }

private:
    std::shared_ptr<DeviceManagerConfig> _config;
    std::shared_ptr<RedisClient::DeviceManagerRedis> _redis;
    time_t _clockPoint = 0;
};

class CreateDeviceHandler: public MultiWorker::Handler
{
public:
    CreateDeviceHandler(::std::shared_ptr<DMS::CreateDeviceReq>& req,
            ::std::function<void(const ::std::shared_ptr<DMS::CreateDeviceResp>& resp)>& response,
            const ::Ice::Current& current) :
            _req(req), _response(response), _current(current)
    {
    }

    virtual void run(MultiWorker::Resource*) override;
    void exception(::std::exception_ptr ex);
    void failed(RC::Code code, const std::string& what);

private:
    std::string genDeviceKey(const std::string_view sult, const std::string_view uuid);

    ::std::shared_ptr<DMS::CreateDeviceReq> _req;
    ::std::function<void(const ::std::shared_ptr<DMS::CreateDeviceResp>& resp)> _response;
    ::Ice::Current _current;
    std::shared_ptr<DMS::CreateDeviceResp> _resp;
    DeviceManagerResource* _resource = nullptr;
};


class AuthDeviceHandler: public MultiWorker::Handler
{
public:
    AuthDeviceHandler(::std::shared_ptr<DMS::AuthDeviceReq>& req,
            ::std::function<void(const ::std::shared_ptr<DMS::AuthDeviceResp>& resp)>& response,
            const ::Ice::Current& current) :
            _req(req), _response(response), _current(current)
    {
    }

    virtual void run(MultiWorker::Resource*) override;
    void exception(::std::exception_ptr ex);
    void failed(RC::Code code, const std::string& what);

private:
    ::std::shared_ptr<DMS::AuthDeviceReq> _req;
    ::std::function<void(const ::std::shared_ptr<DMS::AuthDeviceResp>& resp)> _response;
    ::Ice::Current _current;
    std::shared_ptr<DMS::AuthDeviceResp> _resp;
    DeviceManagerResource* _resource = nullptr;
};



#endif /* DEVICEMANAGERHANDLER_H_ */
