/*
 * StatusServerI.cpp
 *
 *  Created on: Jan 3, 2020
 *      Author: wenhe
 */

#include "StatusServerI.h"

#include "StatusServerHandler.h"
#include "RtioLog.h"
using namespace DMS;

void StatusServerI::setStatusAsync(::std::shared_ptr<SetStatusReq> req,
            ::std::function<void(const ::std::shared_ptr<SetStatusResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)
{
    logSet(current.adapter->getCommunicator(), req->sn);
    logI("deviceId=" << req->deviceId);
    logD("status="<< req->status << " accessProxy=" << req->accessProxy);

    auto handler = std::make_shared<SetStatusHandler>(req, response, current);
    _worker->push(handler);
}

void StatusServerI::queryStatusAsync(::std::shared_ptr<QueryStatusReq> req,
            ::std::function<void(const ::std::shared_ptr<QueryStatusResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)
{
    logSet(current.adapter->getCommunicator(), req->sn);
    logD("deviceId=" << req->deviceId);
    auto handler = std::make_shared<QueryStatusHandler>(req, response, current);
    _worker->push(handler);

}
