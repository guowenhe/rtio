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
#include "RtioExcept.h"

namespace DMS
{

class DispatchHandler: public std::enable_shared_from_this<DispatchHandler>
{
public:
    DispatchHandler(std::shared_ptr<MessageBReq>& req,
            std::shared_ptr<MessageBResp>& resp,
            ::std::function<void(const ::std::shared_ptr<MessageBResp>& resp)>& response,
             const ::Ice::Current& current): _req(req), _resp(resp), _response(response), _current(current)
    {
    }
    ~DispatchHandler()
    {
    }

    void run()
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
                throw RC_ExceptEx(RC::Code::PROXY_INVALID);
            }

            std::shared_ptr<QueryStatusReq> queryStatusReq = std::make_shared<QueryStatusReq>();
            queryStatusReq->sn = _req->sn;
            queryStatusReq->deviceId = _req->deviceId;
            status->queryStatusAsync(queryStatusReq,
                            std::bind(&DispatchHandler::statusResponse, shared_from_this(), std::placeholders::_1),
                            std::bind(&DispatchHandler::exception, shared_from_this(), std::placeholders::_1));

        }
        catch(RC::Except& ex)
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
        logI("QueryStatusResp code=" << RC::intToCode(resp->code));
        _resp->sn = _req->sn;
        try
        {
            if(RC::intToCode(resp->code) != RC::Code::SUCCESS)
            {
                if(RC::intToCode(resp->code) == RC::Code::STATUSSERVER_DIVICE_NOTFUND)
                {
                    _resp->code = RC::codeToInt(RC::Code::DEVICEHUB_DIVICE_NOTFUND);
                    _response(_resp);
                    return;
                }
                throw RC_ExceptEx(RC::intToCode(resp->code));
            }
            if(resp->accessProxy.empty())
            {
                throw RC_ExceptEx(RC::intToCode(resp->code));
            }

            _resp->deviceStatus = static_cast<int>(resp->status);
            if(resp->status == DMS::ClientStatus::ONLINE)
            {
                dispatch(resp->accessProxy);
            }
            else
            {
                _resp->code = RC::codeToInt(RC::Code::DEVICEHUB_DIVICE_NOT_ONLINE);
                _response(_resp);
            }

        }
        catch(RC::Except& ex)
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
                throw RC_ExceptEx(RC::Code::PROXY_INVALID);
            }

            auto reqA = std::make_shared<MessageAReq>();
            reqA->sn = _req->sn;
            reqA->deviceId =_req->deviceId;
            reqA->message = _req->message; // todo message memory copy

            accessServer->dispatchAsync(reqA,
                    std::bind(&DispatchHandler::dispatchResponse, shared_from_this(), std::placeholders::_1),
                    std::bind(&DispatchHandler::exception, shared_from_this(), std::placeholders::_1));
        }
        catch(RC::Except& ex)
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
            logI("respA->code="<< RC::intToCode(respA->code));

            if(RC::intToCode(respA->code) !=  RC::Code::SUCCESS)
            {
                if(RC::intToCode(respA->code) == RC::Code::ACCESSSERVER_TIMEOUT)
                {
                    throw RC_ExceptEx(RC::Code::DEVICEHUB_DIVICE_AS_TIMEOUT);
                }
                throw RC_ExceptEx(RC::Code::DEVICEHUB_DIVICE_AS_FAILED);
            }
            _resp->sn = respA->sn;
            _resp->deviceCode = respA->code;
            _resp->deviceMessage = respA->deviceMessage;
            _resp->code = RC::codeToInt(RC::Code::SUCCESS);
            _response(_resp);
        }
        catch(RC::Except& ex)
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
        _resp->sn = _req->sn;
        _resp->deviceCode = RC::codeToInt((code == RC::Code::DEVICEHUB_DIVICE_AS_TIMEOUT) ?
                        RC::Code::ACCESSSERVER_TIMEOUT : RC::Code::FAIL);
        _resp->code = RC::codeToInt(code);
        _response(_resp);
    }

private:

    std::shared_ptr<MessageBReq> _req;
    std::shared_ptr<MessageBResp> _resp;
    std::function<void(const ::std::shared_ptr<MessageBResp>& resp)> _response;
    Ice::Current _current;
};

} // DMS


#endif /* DISPATCHHANDLER_HPP_ */
