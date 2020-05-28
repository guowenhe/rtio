/*
 * NotifierHandler.hpp
 *
 *  Created on: 6 Mar 2020
 *      Author: wenhe
 */

#include "APINotifier.h"
#include "RemoteCode.h"
#include "RtioExcept.h"
#include "HttpSession.h"
#include "RtioLog.h"
#include "Json.h"

namespace DMS
{


class NotifierHandler: public std::enable_shared_from_this<NotifierHandler>
{
public:
    NotifierHandler(std::shared_ptr<NotifyReq>& req,
            std::function<void(const ::std::shared_ptr<NotifyResp>& resp)>& response,
            const ::Ice::Current& current,
            asio::io_context& ioc) :
            _req(req), _response(response), _current(current), _ioc(ioc)
    {
        _resp = std::make_shared<NotifyResp>();
        _resp->sn = _req->sn;
    }
    ~NotifierHandler()
    {
    }
    void run()
    {
        logSet(_current.adapter->getCommunicator(), _req->sn);
        logI("deviceId=" << _req->deviceId);

        auto clientReq = std::make_shared<HttpClientReq>();
        clientReq->sn = _req->sn;
        clientReq->host = _current.adapter->getCommunicator()->getProperties()->getProperty("NotifyHost");
        clientReq->port = _current.adapter->getCommunicator()->getProperties()->getProperty("NotifyPort");
        clientReq->target = _current.adapter->getCommunicator()->getProperties()->getProperty("NotifyTarget");
        clientReq->deviceId = _req->deviceId;
        clientReq->message = _req->message;

        std::make_shared<HttpClientSession>(_ioc)->run(clientReq,
                std::bind(&NotifierHandler::doResponse, shared_from_this(), std::placeholders::_1));
    }
    void doResponse(std::shared_ptr<HttpClientResp> clientResp)
    {
        logSet(_current.adapter->getCommunicator(), clientResp->sn);
        logI("clientResp->code=" << clientResp->code);
        _resp->code = clientResp->code;
        _resp->message = clientResp->message;
        _response(_resp);
    }

private:
    std::shared_ptr<NotifyReq> _req;
    std::shared_ptr<NotifyResp> _resp;
    std::function<void(const ::std::shared_ptr<NotifyResp>& resp)> _response;
    Ice::Current _current;
    asio::io_context& _ioc;

};

} // DMS
