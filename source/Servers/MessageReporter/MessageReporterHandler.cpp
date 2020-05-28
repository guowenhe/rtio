/*
 * MessageReporterHandler.cpp
 *
 *  Created on: 31 Mar 2020
 *      Author: wenhe
 */

#include "RtioLog.h"
#include "MessageReporterHandler.h"
#include "APINotifier.h"

using namespace DMS;
namespace Redis = RedisClient;


void MessageReporterResource::set(std::shared_ptr<MultiWorker::Config> config)
{
    _config = std::static_pointer_cast<MessageReporterConfig>(config);
}
void MessageReporterResource::init()
{
    _redis = std::make_shared<Redis::ReporterRedis>();

    Redis::RetCode ret = _redis->connect(_config->redisHost, _config->redisPort, _redisConnectTimeout);
    if(Redis::RetCode::SUCCEED != ret)
    {
        log2E(ServerGlobal::getInstance()->getCommunicator(), "redis connect error, ret=" << ret);
    }

}
void MessageReporterResource::check()
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

void ReportHandler::run(MultiWorker::Resource* resource)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);
    logI("deviceId=" << _req->deviceId);

    _resource = static_cast<MessageReporterResource*>(resource);
    _resp = std::make_shared<DMS::ReportResp>();


    try
    {
        if( _req->deviceId.empty())
        {
            throw RC_ExceptEx(RC::Code::MESSAGEREPORTER_DEVICEID_INVALID);
        }

        // save message
        Redis::RetCode ret = _resource->getRedis()->saveMessage(_req, time(NULL) + MESSAGEREPORTER_TRYING_AFTER_INIT);
        if(Redis::RetCode::SUCCEED != ret)
        {
            throw RC_ExceptEx2(RC::Code::MESSAGEREPORTER_REDIS_FAIL, "redis saveMessage error");
        }

        // call notify server
        auto proxy = _current.adapter->getCommunicator()->propertyToProxy("APINotifierIdentity");
        auto notifier = Ice::uncheckedCast<APINotifierPrx>(proxy);
        if(nullptr == notifier)
        {
            throw RC_ExceptEx(RC::Code::PROXY_INVALID);
        }

        auto notifyReq = std::make_shared<DMS::NotifyReq>(); // todo using same message
        notifyReq->sn = _req->sn;
        notifyReq->deviceId = _req->deviceId;
        notifyReq->message = _req->message;
        auto cbRespone = std::bind(&ReportHandler::notifyRespone,
                std::static_pointer_cast<ReportHandler>(shared_from_this()), std::placeholders::_1);
        auto cbException = std::bind(&ReportHandler::exception,
                std::static_pointer_cast<ReportHandler>(shared_from_this()), std::placeholders::_1);
        notifier->notifyAsync(notifyReq, cbRespone, cbException);

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

void ReportHandler::notifyRespone(std::shared_ptr<DMS::NotifyResp> resp)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);
    logI("deviceId=" << _req->deviceId);
    try
    {
        if(RC::intToCode(resp->code) == RC::Code::SUCCESS)
        {
            // set notify status = success
            Redis::RetCode ret = _resource->getRedis()->setMessageSuccess(_req);
            if(Redis::RetCode::SUCCEED != ret)
            {
                throw RC_ExceptEx2(RC::Code::MESSAGEREPORTER_REDIS_FAIL, "redis setMessageSuccess error");
            }
            _resp->code = RC::codeToInt(RC::Code::SUCCESS);
        }
        else
        {
            // set notify status = trying
            Redis::RetCode ret = _resource->getRedis()->setMessageTrying(_req, time(NULL) + MESSAGEREPORTER_TRYING_AFTER_FAIL);
            if(Redis::RetCode::SUCCEED != ret)
            {
                throw RC_ExceptEx2(RC::Code::MESSAGEREPORTER_REDIS_FAIL, "redis setMessageTrying error");
            }
            _resp->code = RC::codeToInt(RC::Code::MESSAGEREPORTER_TRYING);
        }

        _resp->sn = _req->sn;
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

void ReportHandler::exception(::std::exception_ptr ex)
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

void ReportHandler::failed(RC::Code code, const std::string& what)
{
    logSet(_current.adapter->getCommunicator(), _req->sn);
    logE("code=" << code << " what=" << what);
    _resp->sn = _req->sn;
    _resp->code = RC::codeToInt(code);
    _response(_resp);
}
