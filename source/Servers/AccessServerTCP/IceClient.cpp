/*
 * IceClient.cpp
 *
 *  Created on: Dec 24, 2019
 *      Author: wenhe
 */

#include "SessionManager.h"
#include "IceClient.h"

#include "RtioLog.h"
#include "RemoteCode.h"
#include "Common.h"

using namespace GlobalResources;


// deliver message to device
void AccessServerI::dispatchAsync(::std::shared_ptr<DMS::MessageAReq> req,
        ::std::function<void(const ::std::shared_ptr<DMS::MessageAResp>& resp)> response,
        ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)

{
    logSet(current.adapter->getCommunicator(), req->sn)
    logI("deviceId=" << req->deviceId);

    try
    {
        auto session = SessionManager::getInstance()->getSession(req->deviceId);
        if(nullptr == session)
        {
            throw RC_ExceptEx(RC::Code::ACCESSSERVER_SESSION_INVALID);
        }

        session->dispatch(req->message,
                            std::bind(  [response](int sn, std::shared_ptr<DispatchRespData> data)
                                        {
                                            auto resp = std::make_shared<DMS::MessageAResp>();
                                            resp->sn = sn;
                                            if(data->code == RC::Code::SUCCESS && !data->deviceMessage.empty())
                                            {
                                                resp->deviceMessage = data->deviceMessage;
                                            }
                                            resp->code = static_cast<int>(data->code);

                                            response(resp);
                                        }, req->sn, std::placeholders::_1));


    }
    catch(RC::Except& ex)
    {
        logE("RC::Except code=" << ex.code() << "what=" << ex.what());
        auto resp = std::make_shared<DMS::MessageAResp>();
        resp->sn = req->sn;
        resp->code = RC::codeToInt(ex.code());
        response(resp);
    }
    catch(std::exception& ex)
    {
        logE("exception what=" << ex.what());
        auto resp = std::make_shared<DMS::MessageAResp>();
        resp->sn = req->sn;
        resp->code = static_cast<int>(RC::Code::ACCESSSERVER_FAILED);
        response(resp);
    }
}


RetCode IceClient::init(int argc, char* argv[], const std::string& config)
{
    try
    {
        Ice::CommunicatorHolder communicatorHolder(argc, argv, config);
        _communicatorHolder = std::move(communicatorHolder);
        _communicator = _communicatorHolder.communicator();

        if(!_communicator)
        {
            std::cerr << "communicator error" << std::endl;
            return RetCode::CommunicatorError;
        }

        log2D(getCommunicator(), "communicator initialed");

        _messageHubAPrx = Ice::checkedCast<DMS::DeviceHubAPrx>(_communicator->propertyToProxy("DeviceHubIdentity"));
        if(!_messageHubAPrx)
        {
            log2D(getCommunicator(), "messageHub proxy invalid.");
            return RetCode::ObjectPrxError;
        }

        auto adapter = _communicator->createObjectAdapter("AccessServer");
        auto prx = adapter->addWithUUID(std::make_shared<AccessServerI>());
        auto accessServerPrx = Ice::uncheckedCast<DMS::AccessServerPrx>(prx);
        _accessServerPrxStr = getCommunicator()->proxyToString(accessServerPrx);
        adapter->activate();
//        int ret = _messageHubAPrx->addAccessServer(accessServerPrx);
//        if(ret)
//        {
//            log2E(getCommunicator(), "addClient failed ret=" << ret);
//            return RetCode::AddClientError;
//        }

        _statusServerPrx = Ice::checkedCast<DMS::StatusServerPrx>(_communicator->propertyToProxy("StatusServerIdentity"));
        if(!_statusServerPrx)
        {
            log2D(getCommunicator(), "statusServer proxy invalid.");
            return RetCode::ObjectPrxError;
        }

        _deviceManagerPrx = Ice::checkedCast<DMS::DeviceManagerPrx>(_communicator->propertyToProxy("DeviceManagerIdentity"));
        if(!_deviceManagerPrx)
        {
            log2D(getCommunicator(), "deviceManager proxy invalid.");
            return RetCode::ObjectPrxError;
        }
    }
    catch(const Ice::FileException& ex)
    {
        std::cerr << "FileException=" << ex.what() << std::endl;
        return RetCode::FileExeption;
    }
    catch(const Ice::ObjectNotExistException& ex)
    {
        log2E(getCommunicator(), "ObjectNotExistException=" << ex.what());
        std::cerr << "ObjectNotExistException=" << ex.what() << std::endl;
        return RetCode::ObjectPrxError;
    }
    catch(const std::exception& ex)
    {
        std::cerr << "exception=" << ex.what() << std::endl;
        log2E(getCommunicator(), "exception=" << ex.what());
        return RetCode::ObjectPrxError;
    }
    return RetCode::Success;

}
RetCode IceClient::reportMessage(const std::string& message, const std::string& deviceId)
{
    log2I(getCommunicator(), "message=" << message.substr(0, 100));
    std::function<void(::std::shared_ptr<::DMS::MessageAResp>)> responseHandler = [this](std::shared_ptr<::DMS::MessageAResp> resp)
    {
        log2I(getCommunicator(), "reportMessage responseHandler:"<< resp->sn << "," << resp->code);
    };
    std::function<void(::std::exception_ptr)> exceptionHandler = [this](std::exception_ptr ex)
    {
        log2I(getCommunicator(), "reportMessage exceptionHandler");
        try
        {
            std::rethrow_exception(ex);
        }
        catch(const std::exception& ex)
        {
            log2I(getCommunicator(), "reportMessage exceptionHandler ex=" << ex.what());
        }
    };

    auto req = std::make_shared<DMS::MessageAReq>();
    req->sn = time(NULL);
    req->deviceId = deviceId;
    req->message = message;

    try
    {
        _messageHubAPrx->reportAsync(req, responseHandler, exceptionHandler);
    }
    catch(const std::exception& ex)
    {
        log2I(getCommunicator(), "reportMessage ex=" << ex.what());
        return RetCode::UnknowError;
    }

    return RetCode::Success;

}

void IceClient::setStatus(const std::string& deviceId, DMS::ClientStatus status, std::function<void(GlobalResources::RetCode)> cb)
{
    log2I(getCommunicator(), "deviceId=" << deviceId << " status=" << status);
    auto responseHandler = [this, cb](std::shared_ptr<::DMS::SetStatusResp> resp)
    {
        log2I(getCommunicator(), "setStatus responseHandler:"<< resp->sn << "," << resp->code);
//        if(static_cast<RC::Code>(resp->code) == RC::Code::SUCCEED) // todo need modify status server
        if(resp->code == static_cast<int>(RC::Code::SUCCESS))
        {
            cb(GlobalResources::RetCode::Success);
        }
        else
        {
            cb(GlobalResources::RetCode::UnknowError);
        }
    };

    auto exceptionHandler = [this](std::exception_ptr ex)
    {
        log2I(getCommunicator(), "setStatus exceptionHandler");
        try
        {
            std::rethrow_exception(ex);
        }
        catch(const std::exception& ex)
        {
            log2I(getCommunicator(), "setStatus exceptionHandler ex=" << ex.what());
        }
    };

    try
    {
        auto req = std::make_shared<DMS::SetStatusReq>();
        req->sn = time(NULL);
        req->deviceId = deviceId;
        req->status = status;
        req->accessProxy = _accessServerPrxStr;
        _statusServerPrx->setStatusAsync(req, responseHandler, exceptionHandler);
    }
    catch(RC::Except& ex)
    {
        log2E(getCommunicator(), "code=" << ex.code() << " what=" << ex.what());
        cb(GlobalResources::RetCode::UnknowError);
    }
    catch(std::exception& ex)
    {
        log2E(getCommunicator(), ex.what());
        cb(GlobalResources::RetCode::UnknowError);
    }
}

void IceClient::authDevice(const std::string& deviceId, const std::string& deviceKey, std::function<void(RetCode)> cb)
{
    log2I(getCommunicator(), "deviceId=" << deviceId << " deviceKey=" << deviceKey);
    auto responseHandler = [this, cb](std::shared_ptr<::DMS::AuthDeviceResp> resp)
    {
        log2I(getCommunicator(), "authDevice responseHandler:"<< resp->sn << "," << resp->code);

        if(RC::intToCode(resp->code) != RC::Code::SUCCESS)
        {
            if(RC::intToCode(resp->code) == RC::Code::DEVICEMANAGER_ID_ERROR
                    || RC::intToCode(resp->code) == RC::Code::DEVICEMANAGER_ID_NOTFUND)
            {
                cb(RetCode::AuthIdInvalid);
            }
            else if(RC::intToCode(resp->code) == RC::Code::DEVICEMANAGER_KEY_ERROR)
            {
                cb(RetCode::AuthKeyInvalid);
            }
            else
            {
                cb(RetCode::UnknowError);
            }
        }
        else
        {
            auto ret = (resp->authCode == DMS::AuthCode::PASS) ? RetCode::AuthPass: RetCode::AuthFail;
            cb(ret);
        }
    };

    auto exceptionHandler = [this](std::exception_ptr ex)
    {
        log2I(getCommunicator(), "authDevice exceptionHandler");
        try
        {
            std::rethrow_exception(ex);
        }
        catch(const std::exception& ex)
        {
            log2I(getCommunicator(), "authDevice exceptionHandler ex=" << ex.what());
        }
    };

    try
    {
        auto req = std::make_shared<DMS::AuthDeviceReq>();
        req->sn = logSnGen();
        req->deviceId = deviceId;
        req->deviceKey = deviceKey;
        _deviceManagerPrx->authDeviceAsync(req, responseHandler, exceptionHandler);
    }
    catch(RC::Except& ex)
    {
        log2E(getCommunicator(), "code=" << ex.code() << " what=" << ex.what());
        cb(RetCode::UnknowError);
    }
    catch(std::exception& ex)
    {
        log2E(getCommunicator(), ex.what());
        cb(RetCode::UnknowError);
    }

}











