/*
 * MessageTiggerI.cpp
 *
 *  Created on: 22 Jan 2020
 *      Author: wenhe
 */

#include "MessageReporterI.h"

#include "RtioLog.h"

using namespace DMS;

void MessageReporterI::reportAsync(::std::shared_ptr<ReportReq> req,
        ::std::function<void(const ::std::shared_ptr<ReportResp>& resp)> response,
        ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)
{
    logSet(current.adapter->getCommunicator(), req->sn);
    logI("req->deviceId=" << req->deviceId);

    auto handler = std::make_shared<ReportHandler>(req, response, current);
    _workers->push(handler);
}

