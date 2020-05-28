/*
 * ReportHandler.hpp
 *
 *  Created on: 25 May 2020
 *      Author: wenhe
 */

#ifndef REPORTHANDLER_HPP_
#define REPORTHANDLER_HPP_

#include "RtioLog.h"
#include "DeviceHub.h"
#include "MessageReporter.h"

namespace DMS
{

class ReportHandler: public std::enable_shared_from_this<ReportHandler>
{
public:
    ReportHandler(std::shared_ptr<MessageAReq>& req,
            std::shared_ptr<MessageAResp>& resp,
            ::std::function<void(const ::std::shared_ptr<MessageAResp>& resp)>& response,
             const ::Ice::Current& current): _req(req),_resp(resp), _response(response), _current(current)
    {
    }
    ~ReportHandler()
    {
    }

    void report()
    {
        logSet(_current.adapter->getCommunicator(), _req->sn);
        logI("-");

        try
        {
            auto proxy = _current.adapter->getCommunicator()->propertyToProxy("MessageReporterIdentity");
            auto reporter = Ice::uncheckedCast<MessageReporterPrx>(proxy);
            if(nullptr == reporter)
            {
                throw RC_ExceptEx(RC::Code::PROXY_INVALID);
            }

            auto reportReq = std::make_shared<DMS::ReportReq>();
            reportReq->sn = _req->sn;
            reportReq->deviceId = _req->deviceId;
            reportReq->message = _req->message;

            reporter->reportAsync(reportReq,
                            std::bind(&ReportHandler::reportResponse, shared_from_this(), std::placeholders::_1),
                            std::bind(&ReportHandler::exception, shared_from_this(), std::placeholders::_1));

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
    void reportResponse(std::shared_ptr<ReportResp> resp)
    {
        logSet(_current.adapter->getCommunicator(), _req->sn);
        logI("QueryStatusResp code=" << RC::intToCode(resp->code));
        _resp->sn = resp->sn;
        _resp->code = resp->code;
        _response(_resp);
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
        _resp->code = RC::codeToInt(code);
        _response(_resp);
    }

private:

    std::shared_ptr<MessageAReq> _req;
    std::shared_ptr<MessageAResp> _resp;
    std::function<void(const ::std::shared_ptr<MessageAResp>& resp)> _response;
    Ice::Current _current;
};

} // DMS

#endif /* REPORTHANDLER_HPP_ */
