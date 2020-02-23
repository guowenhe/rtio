/*
 * DispatchHandler.hpp
 *
 *  Created on: Dec 21, 2019
 *      Author: wenhe
 */

#ifndef DISPATCHHANDLER_HPP_
#define DISPATCHHANDLER_HPP_

#include "DeviceHub.h"
#include "Common.h"
#include "RemoteCode.h"
#include "StatusServer.h"
#include "GxError.hpp"

namespace DMS
{

class DispatchHandler: public std::enable_shared_from_this<DispatchHandler>
{
public:
    DispatchHandler(std::shared_ptr<MessageBReq>& req,
            ::std::function<void(const ::std::shared_ptr<MessageBResp>& resp)>& response,
             const ::Ice::Current& current): _req(req), _response(response), _current(current)
    {
    }
    ~DispatchHandler()
    {
    }

    void processing()
    {
        status();
    }

    void status()
    {
        logSet(_current.adapter->getCommunicator(), _req->sn);
        logI("-");

        try
        {
            auto proxy = _current.adapter->getCommunicator()->propertyToProxy("StatusServerIdentity");
            auto status = Ice::uncheckedCast<StatusServerPrx>(proxy);
            if(nullptr == status)
            {
                throw GxError<RC::Code>(RC::Code::PROXY_INVALID, "status proxy invalid");
            }

            std::shared_ptr<QueryStatusReq> queryStatusReq = std::make_shared<QueryStatusReq>();
            queryStatusReq->sn = _req->sn;
            queryStatusReq->deviceId = _req->deviceId;
            status->queryStatusAsync(queryStatusReq,
                            std::bind(&DispatchHandler::statusResponse, shared_from_this(), std::placeholders::_1),
                            std::bind(&DispatchHandler::exception, shared_from_this(), std::placeholders::_1));

        }
        catch(GxError<RC::Code>& ex)
        {
            failed(ex.code(), Rtio_where() + ex.what());
        }
        catch(std::exception& ex)
        {
            failed(RC::Code::FAIL, Rtio_where() + ex.what());
        }
    }
    void statusResponse(std::shared_ptr<QueryStatusResp> resp)
    {
        logSet(_current.adapter->getCommunicator(), _req->sn);
        logI("QueryStatusResp code=" << static_cast<RC::Code>(resp->code));
        try
        {
            if(static_cast<RC::Code>(resp->code) != RC::Code::SUCCESS)
            {
                throw GxError<RC::Code>(static_cast<RC::Code>(resp->code), "failed");
            }
            if(resp->accessProxy.empty())
            {
                throw GxError<RC::Code>(static_cast<RC::Code>(resp->code), "accessProxy is empty string");
            }

            if(resp->status == DMS::ClientStatus::ONLINE)
            {
                dispatch(resp->accessProxy);
            }
            else
            {
                auto respB = std::make_shared<MessageBResp>();
                respB->sn = _req->sn;
                respB->code = static_cast<int>(RC::Code::DEVICEHUB_DIVICE_NOT_ONLINE);
                _response(respB);
            }

        }
        catch(GxError<RC::Code>& ex)
        {
            failed(ex.code(), Rtio_where() + ex.what());
        }
        catch(std::exception& ex)
        {
            failed(RC::Code::FAIL, Rtio_where() + ex.what());
        }
    }

    void dispatch(std::string& accessProxy)
    {
        logSet(_current.adapter->getCommunicator(), _req->sn);
        logI("accessProxy=" << accessProxy);

        try
        {
            auto accessServer = Ice::uncheckedCast<AccessServerPrx>(
                    _current.adapter->getCommunicator()->stringToProxy(accessProxy));

            if(nullptr == accessServer)
            {
                throw GxError<RC::Code>(RC::Code::PROXY_INVALID, "accessServer is nullptr");
            }

            auto reqA = std::make_shared<MessageAReq>();
            reqA->sn = _req->sn;
            reqA->deviceId =_req->deviceId;
            reqA->message = _req->message; // todo message memory copy

            accessServer->dispatchAsync(reqA,
                    std::bind(&DispatchHandler::dispatchResponse, shared_from_this(), std::placeholders::_1),
                    std::bind(&DispatchHandler::exception, shared_from_this(), std::placeholders::_1));
        }
        catch(GxError<RC::Code>& ex)
        {
            failed(ex.code(), Rtio_where() + ex.what());
        }
        catch(std::exception& ex)
        {
            failed(RC::Code::FAIL, Rtio_where() + ex.what());
        }
    }
    void dispatchResponse(const ::std::shared_ptr<::DMS::MessageAResp> respA)
    {
        try
        {
            logSet(_current.adapter->getCommunicator(),  respA->sn);
            logI("respA->code="<< static_cast<RC::Code>(respA->code));

            if(static_cast<RC::Code>(respA->code) !=  RC::Code::SUCCESS)
            {
                if(static_cast<RC::Code>(respA->code) == RC::Code::ACCESSSERVER_TIMEOUT)
                {
                    throw GxError<RC::Code>(RC::Code::DEVICEHUB_DIVICE_AS_TIMEOUT, "accessServer dispatch timeout");
                }
                throw GxError<RC::Code>(RC::Code::DEVICEHUB_DIVICE_AS_FAILED, "accessServer dispatch resp code error");
            }

            auto respB = std::make_shared<MessageBResp>();
            respB->sn = respA->sn;
            respB->asCode = respA->code;
            respB->code = static_cast<int>(RC::Code::SUCCESS);
            _response(respB);
        }
        catch(GxError<RC::Code>& ex)
        {
            failed(ex.code(), Rtio_where() + ex.what());
        }
        catch(std::exception& ex)
        {
            failed(RC::Code::FAIL, Rtio_where() + ex.what());
        }
    }
    void exception(::std::exception_ptr ex)
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
    void failed(RC::Code code, const std::string& what)
    {
        logSet(_current.adapter->getCommunicator(), _req->sn);
        logE("code=" << code << " what=" << what);
        auto resp = std::make_shared<MessageBResp>();
        resp->sn = _req->sn;
        resp->asCode = (code == RC::Code::DEVICEHUB_DIVICE_AS_TIMEOUT) ?
                        static_cast<int>(RC::Code::ACCESSSERVER_TIMEOUT) : static_cast<int>(RC::Code::FAIL);
        resp->code = static_cast<int>(code);
        _response(resp);
    }

private:

    std::shared_ptr<MessageBReq> _req;
    std::function<void(const ::std::shared_ptr<MessageBResp>& resp)> _response;
    Ice::Current _current;
};

} // DMS


#endif /* DISPATCHHANDLER_HPP_ */
