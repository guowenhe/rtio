/*
 * MessageTiggerI.cpp
 *
 *  Created on: 22 Jan 2020
 *      Author: wenhe
 */

#include "MessageTriggerI.h"
#include "VerijsLog.h"

using namespace DMS;

void MessageTiggerAI::reportAsync(::std::shared_ptr<ReportReq> req,
        ::std::function<void(const ::std::shared_ptr<ReportResp>& resp)> response,
        ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)
{
    logSet(current.adapter->getCommunicator(), req->sn);
    logI("req->message=" << req->message);
    auto resp = std::make_shared<ReportResp>();
    resp->code = 0;
    resp->sn = req->sn;
    response(resp);
}
void MessageTiggerBI::pushAsync(::std::shared_ptr<PushReq> req,
        ::std::function<void(const ::std::shared_ptr<PushResp>& resp)> response,
        ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)
{
    logSet(current.adapter->getCommunicator(), req->sn);
    logI("req->message=" << req->message);
    auto resp = std::make_shared<PushResp>();
    resp->code = 0;
    resp->sn = req->sn;
    response(resp);
}

