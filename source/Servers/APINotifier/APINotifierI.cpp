/*
 * APINotifier.cpp
 *
 *  Created on: 29 Feb 2020
 *      Author: wenhe
 */


#include "APINotifierI.h"
#include "RtioLog.h"
#include "NotifierHandler.hpp"


using namespace DMS;
void APINotifierI::notifyAsync(::std::shared_ptr<NotifyReq> req,
    ::std::function<void(const ::std::shared_ptr<NotifyResp>& resp)> response,
    ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)
{
    logSet(current.adapter->getCommunicator(), req->sn);
    logI("deviceId=" << req->deviceId);

    try
    {
        std::make_shared<NotifierHandler>(req, response, current, _ioc)->run();
    }
    catch(std::exception& ex)
    {
        logE("ex=" << ex.what());
        ::std::shared_ptr<NotifyResp> resp = std::make_shared<NotifyResp>();
        resp->sn = req->sn;
        resp->code = RC::codeToInt(RC::Code::FAIL);
        response(resp);
    }

}
