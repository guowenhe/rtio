/*
 * DeviceStatusHandler.cpp
 *
 *  Created on: Jan 6, 2020
 *      Author: wenhe
 */

#include "VerijsLog.h"
#include "ServerGlobal.h"
#include "StatusServerHandler.h"
#include "RemoteCode.h"

using namespace DMS;
namespace Redis = RedisClient;

void DeviceStatusResource::set(std::shared_ptr<MultiWorker::Config> config)
{
    _config = std::static_pointer_cast<DeviceStatusConfig>(config);
}
void DeviceStatusResource::init()
{
    _redis = std::make_shared<Redis::DeviceRedis>();

    Redis::RetCode ret = _redis->connect(_config->redisHost, _config->redisPort, _redisConnectTimeout);
    if(Redis::RetCode::SUCCEED != ret)
    {
        log2E(ServerGlobal::getInstance()->getCommunicator(), "redis connect error, ret=" << ret);
    }
}
void DeviceStatusResource::check()
{
    if(getRedis()->requireRedisCheck()
            || Util::getClockCoarse() - _clockPoint > _redisConnectCheckInterval)
    {
        _clockPoint = Util::getClockCoarse();

//        log2D(ServerGlobal::getInstance()->getCommunicator(), "clockPoint=" << _clockPoint);
        if(!getRedis()->ping())
        {
            getRedis()->reconnect();
        }
    }
}

void SetStatusHandler::processing(MultiWorker::Resource* resource)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);
    logI("deviceId=" << _req->deviceId);
    auto r = static_cast<DeviceStatusResource*>(resource);
    RC::Code code = RC::Code::FAIL;
    try
    {
        Redis::DiviceInfo info;
        info.status = static_cast<int>(_req->status);
        info.proxy = _req->accessProxy;

        Redis::RetCode ret = r->getRedis()->setDeviceInfo(_req->deviceId, info);
        if(Redis::RetCode::SUCCEED != ret)
        {
            logE("redis setDeviceInfo error, deviceId=" << _req->deviceId);
            throw std::logic_error("redis setDeviceInfo error");
        }
        code = RC::Code::SUCCESS;
    }
    catch(std::exception& ex)
    {
        logE("exception=" << ex.what());
    }

    auto resp = std::make_shared<DMS::SetStatusResp>();
    resp->sn = _req->sn;
    resp->code = static_cast<int>(code);
    _response(resp);
}

void QueryStatusHandler::processing(MultiWorker::Resource* resource)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);
    logI("deviceId=" << _req->deviceId);

    auto r = static_cast<DeviceStatusResource*>(resource);
    RC::Code code = RC::Code::FAIL;
    Redis::DiviceInfo info;
    try
    {
        Redis::RetCode ret = r->getRedis()->getDeviceInfo(_req->deviceId, info);
        if(Redis::RetCode::SUCCEED == ret)
        {
            code =  RC::Code::SUCCESS;
        }
        else if(Redis::RetCode::EMPTY == ret)
        {
            code =  RC::Code::STATUSSERVER_DIVICE_NOTFUND;
            logW("redis getDeviceInfo empty, deviceId=" << _req->deviceId);
        }
        else
        {
            logE("redis getDeviceInfo error, deviceId=" << _req->deviceId);
            throw std::logic_error("redis setDeviceInfo error");
        }
    }
    catch(std::exception& ex)
    {
        logE("exception=" << ex.what());
    }

    auto resp = std::make_shared<DMS::QueryStatusResp>();
    if(RC::Code::SUCCESS == code)
    {
        resp->status = static_cast<ClientStatus>(info.status);
        resp->accessProxy = info.proxy;
    }

    resp->sn = _req->sn;
    resp->code = static_cast<int>(code);
    _response(resp);
}



