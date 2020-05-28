/*
 * DeviceManagerHandler.cpp
 *
 *  Created on: 4 Apr 2020
 *      Author: wenhe
 */
#include <string_view>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Common.h"
#include "RtioLog.h"
#include "DeviceManagerHandler.h"

#include "Common.h"

using namespace DMS;
namespace Redis = RedisClient;


void DeviceManagerResource::set(std::shared_ptr<MultiWorker::Config> config)
{
    _config = std::static_pointer_cast<DeviceManagerConfig>(config);
    _redis = std::make_shared<Redis::DeviceManagerRedis>();

    Redis::RetCode ret = _redis->connect(_config->redisHost, _config->redisPort, _redisConnectTimeout);
    if(Redis::RetCode::SUCCEED != ret)
    {
        log2E(ServerGlobal::getInstance()->getCommunicator(), "redis connect error, ret=" << ret);
    }

}
void DeviceManagerResource::init()
{
    _redis = std::make_shared<Redis::DeviceManagerRedis>();

    Redis::RetCode ret = _redis->connect(_config->redisHost, _config->redisPort, _redisConnectTimeout);
    if(Redis::RetCode::SUCCEED != ret)
    {
        log2E(ServerGlobal::getInstance()->getCommunicator(), "redis connect error, ret=" << ret);
    }

}
void DeviceManagerResource::check()
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

void CreateDeviceHandler::run(MultiWorker::Resource* resource)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);

    _resource = static_cast<DeviceManagerResource*>(resource);
    _resp = std::make_shared<DMS::CreateDeviceResp>();
    _resp->sn = _req->sn;
    try
    {
        // gen uuid and deviceId
        boost::uuids::uuid uId = boost::uuids::random_generator()();
        _resp->deviceId = Util::binToHex(std::string_view((char*)(uId.data), 16));
        logI("deviceId=" << _resp->deviceId);

        // gen uuid for deviceKey
        boost::uuids::uuid uKey = boost::uuids::random_generator()();
        logD("uKey=" << uKey);
        auto sult = _current.adapter->getCommunicator()->getProperties()->getProperty("DeviceKeySult");
        _resp->deviceKey  = genDeviceKey(sult, std::string_view((char*)uKey.data, uKey.size()));

        if(_resp->deviceKey.empty())
        {
            throw RC_ExceptEx2(RC::Code::DEVICEMANAGER_KEY_ERROR, "deviceKey empty");
        }

        logD("deviceKey=" << _resp->deviceKey );

        // save to redis
        auto ret = _resource->getRedis()->saveDevice(_resp->deviceId, _resp->deviceKey );

        if(Redis::RetCode::SUCCEED != ret)
        {
            throw RC_ExceptEx2(RC::Code::DEVICEMANAGER_REDIS_FAIL, "saveDevice error");
        }

        // response
        _resp->code = RC::codeToInt(RC::Code::SUCCESS);
        _response(_resp);

    }
    catch(RC::Except& ex)
    {
        failed(ex.code(), ex.what());
    }
    catch(std::exception& ex)
    {
        failed(RC::Code::FAIL, ex.what());
    }
}

void CreateDeviceHandler::exception(::std::exception_ptr ex)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);
    logI("-");
    try
    {
        std::rethrow_exception(ex);
    }
    catch(const std::exception& ex)
    {
        failed(RC::Code::FAIL, Rtio_where() + ex.what());
    }
}

void CreateDeviceHandler::failed(RC::Code code, const std::string& what)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);
    logE("code=" << code << " what=" << what);
    _resp->code = RC::codeToInt(code);
    if(RC::Code::SUCCESS != code)
    {
        _resp->deviceId.clear();
        _resp->deviceKey.clear();
    }
    _response(_resp);
}

std::string CreateDeviceHandler::genDeviceKey(const std::string_view sult, const std::string_view uuid)
{
    std::string preKey;
    preKey.resize(sult.size() + uuid.size());
    memcpy((void*)preKey.c_str(), sult.data(), sult.size());
    memcpy((void*)(preKey.c_str() + sult.length()), uuid.data(), uuid.size());

//        std::cout << "preKey=" << Util::binToHex(preKey) << std::endl;

    // get hash
    boost::uuids::detail::sha1 hash;
    hash.process_bytes(preKey.c_str(), preKey.size());
    boost::uuids::detail::sha1::digest_type result;
    hash.get_digest(result);

    std::array<uint8_t, 20> digest;
    for(int i = 0; i < 5; ++i)
    {
        digest[i*4] = (result[i] >> 24) & 0xFF;
        digest[i*4 + 1] = (result[i] >> 16) & 0xFF;
        digest[i*4 + 2] = (result[i] >> 8) & 0xFF;
        digest[i*4 + 3] = result[i] & 0xFF;
    }
//        for(int i = 0; i < 5; ++i)
//        {
//            cout << hex << std::setw(8) << std::setfill('0') << result[i];            // 以十六进制输出
//        }
//        cout << endl;
//        std::cout << "sha1=" << Util::binToHex(std::string_view((char*)digest.data(), digest.size())) << std::endl;
    // base64
    std::string s = Util::base64Encode(std::string_view((char*)digest.data(), digest.size()));
    // change to url safe string
//        std::cout << "base64Ret=" << s << std::endl;
    for(auto& c: s)
    {
        if('+' == c)
        {
           c = '-';
        }
        else if('/' == c)
        {
            c = '_';
        }
    }
    // delete "="
    auto pos = s.find_last_not_of('=');
    if(std::string::npos == pos)
    {
        return "";
    }
    return s.substr(0, pos+1);
}

void AuthDeviceHandler::run(MultiWorker::Resource* resource)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);

        _resource = static_cast<DeviceManagerResource*>(resource);
        _resp = std::make_shared<DMS::AuthDeviceResp>();
        _resp->sn = _req->sn;
        try
        {
            if(_req->deviceId.empty())
            {
                throw RC_ExceptEx(RC::Code::DEVICEMANAGER_ID_ERROR);
            }
            if(_req->deviceKey.empty())
            {
                throw RC_ExceptEx(RC::Code::DEVICEMANAGER_KEY_ERROR);
            }
            // get from redis
            std::string realKey;
            auto ret = _resource->getRedis()->getDeviceKey(_req->deviceId, realKey);
            if(Redis::RetCode::EMPTY == ret)
            {
                throw RC_ExceptEx(RC::Code::DEVICEMANAGER_ID_NOTFUND);
            }
            else if(Redis::RetCode::FAILED == ret)
            {
                throw RC_ExceptEx2(RC::Code::DEVICEMANAGER_REDIS_FAIL, "getDeviceKey error");
            }
            if(0 == realKey.compare(_req->deviceKey))
            {
                _resp->authCode = AuthCode::PASS;
            }
            else
            {
                _resp->authCode = AuthCode::FAIL;
            }
            // response
            _resp->code = RC::codeToInt(RC::Code::SUCCESS);
            _response(_resp);

        }
        catch(RC::Except& ex)
        {
            failed(ex.code(), ex.what());
        }
        catch(std::exception& ex)
        {
            failed(RC::Code::FAIL, ex.what());
        }
}
void AuthDeviceHandler::exception(::std::exception_ptr ex)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);
    logI("-");
    try
    {
        std::rethrow_exception(ex);
    }
    catch(const std::exception& ex)
    {
        failed(RC::Code::FAIL, Rtio_where() + ex.what());
    }
}
void AuthDeviceHandler::failed(RC::Code code, const std::string& what)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);
    logE("code=" << code << " what=" << what);
    _resp->code = RC::codeToInt(code);
    _response(_resp);
}





