/*
 * IceClient.cpp
 *
 *  Created on: Dec 24, 2019
 *      Author: wenhe
 */

#include "VerijsLog.h"
#include "SessionManager.h"
#include "IceClient.h"
#include "RemoteCode.h"
#include "GxError.hpp"
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
            throw GxError<RC::Code>(RC::Code::ACCESSSERVER_SESSION_INVALID, RC::where());
        }

        session->dispatch(req->message,
                            std::bind(  [response](int sn, int code)
                                        {
                                            auto resp = std::make_shared<DMS::MessageAResp>();
                                            resp->sn = sn;
                                            resp->code = code;
                                            response(resp);
                                        }, req->sn, std::placeholders::_1));



    }
    catch(GxError<RC::Code>& ex)
    {
        logE("GxError code=" << ex.code() << "what=" << ex.what());
        auto resp = std::make_shared<DMS::MessageAResp>();
        resp->sn = req->sn;
        resp->code = static_cast<int>(ex.code());
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

        _messageHubAPrx = Ice::checkedCast<DMS::MessageHubAPrx>(_communicator->propertyToProxy("DeviceHubIdentity"));
        if(!_messageHubAPrx)
        {
            log2D(getCommunicator(), "server proxy invalid.");
            return RetCode::ObjectPrxError;
        }

        auto adapter = _communicator->createObjectAdapter("AccessServer");
        auto prx = adapter->addWithUUID(std::make_shared<AccessServerI>());
        auto accessServerPrx = Ice::uncheckedCast<DMS::AccessServerPrx>(prx);
        _accessServerPrxStr = getCommunicator()->proxyToString(accessServerPrx);
        adapter->activate();

//        int ret = _messageHubAPrx->addAccessServer(accessServerPrx); // todo test
//        if(ret)
//        {
//            log2E(getCommunicator(), "addClient failed ret=" << ret);
//            return RetCode::AddClientError;
//        }
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
RetCode IceClient::reportMessage(const std::string& message)
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
    req->message = message; //todo message move

    try
    {
        _messageHubAPrx->reportAsync(req, responseHandler, exceptionHandler);
    }
    catch(const std::exception& ex)
    {
        log2I(getCommunicator(), "reportMessage ex=" << ex.what());
    }

    return RetCode::Success;

}
RetCode IceClient::reportMessage(std::shared_ptr<std::string> message)
{
    log2I(getCommunicator(), "message=" << message->substr(0, 100));
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
    req->message = *message; //todo message move
    _messageHubAPrx->reportAsync(req, responseHandler, exceptionHandler);

    return RetCode::Success;

}

RetCode IceClient::setStatus(const std::string& deviceId, DMS::ClientStatus status, std::function<void(int)> cb)
{
    log2I(getCommunicator(), "deviceId=" << deviceId << " status=" << status);
    auto responseHandler = [this, cb](std::shared_ptr<::DMS::SetStatusResp> resp)
    {
        log2I(getCommunicator(), "setStatus responseHandler:"<< resp->sn << "," << resp->code);
//        if(static_cast<RC::Code>(resp->code) == RC::Code::SUCCEED) // todo need modify status server
        if(resp->code == static_cast<int>(RC::Code::SUCCEED))
        {
            cb(0);
        }
        else
        {
            cb(-1);
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
            log2I(getCommunicator(), "reportMessage exceptionHandler ex=" << ex.what());
        }
    };

    try
    {
        auto statusServer = Ice::checkedCast<DMS::StatusServerPrx>(
                getCommunicator()->propertyToProxy("StatusServerIdentity"));
        if(nullptr == statusServer)
        {
            throw GxError<RC::Code>(RC::Code::PROXY_INVALID, "status proxy invalid");
        }

        auto req = std::make_shared<DMS::SetStatusReq>();
        req->sn = time(NULL);
        req->deviceId = deviceId;
        req->status = status;
        req->accessProxy = _accessServerPrxStr;
        statusServer->setStatusAsync(req, responseHandler, exceptionHandler);
    }
    catch(GxError<RC::Code>& ex)
    {
        log2E(getCommunicator(), "code=" << ex.code() << " what=" << ex.what());
        cb(-1);// todo  think error code trans
    }
    catch(std::exception& ex)
    {
        log2E(getCommunicator(), ex.what());
        cb(-1);
    }


    return RetCode::Success;
}

















