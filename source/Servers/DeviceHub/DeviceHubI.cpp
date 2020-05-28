#include <Ice/Ice.h>
#include "RtioLog.h"
#include "DeviceHubI.h"
#include "DispatchHandler.hpp"
#include "ReportHandler.hpp"

using namespace DMS;



int DeviceHubAI::addAccessServer(::std::shared_ptr<AccessServerPrx> server, const Ice::Current& current)
{
    log2I(current.adapter->getCommunicator(),
            "identity[" << Ice::identityToString(server->ice_getIdentity()) << "]");

    try
    {
//        g_accessServerprx = current.adapter->getCommunicator()->proxyToString(server);
//
//        auto accessServer = Ice::uncheckedCast<AccessServerPrx>(
//                current.adapter->getCommunicator()->stringToProxy(g_accessServerprx));
//
//        auto reqA = std::make_shared<MessageAReq>();
//        auto respA = std::make_shared<MessageAResp>();
//        reqA->message = "test111111111111";
//
//        accessServer->dispatch(reqA, respA);
//
//        log2I(current.adapter->getCommunicator(), "respA->code" << respA->code);

    }
    catch(std::exception& ex)
    {
        log2E(current.adapter->getCommunicator(), "ex=" << ex);
    }
    return 0;

}

void DeviceHubAI::reportAsync(::std::shared_ptr<MessageAReq> req,
        ::std::function<void(const ::std::shared_ptr<MessageAResp>& resp)> response,
        ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)
{
    logSet(current.adapter->getCommunicator(), req->sn);
    logI("deviceId=" << req->deviceId);
    auto resp = std::make_shared<MessageAResp>();
    try
    {
       std::make_shared<ReportHandler>(req, resp, response, current)->report();
    }
    catch(std::exception& ex)
    {
        logSet(current.adapter->getCommunicator(), req->sn);
        logE("what=" << ex.what());
        resp->sn = req->sn;
        resp->code = RC::codeToInt(RC::Code::FAIL);
        response(resp);
    }
}

void DeviceHubBI::dispatchAsync(::std::shared_ptr<MessageBReq> req,
        ::std::function<void(const ::std::shared_ptr<MessageBResp>& resp)> response,
        ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)
{
    logSet(current.adapter->getCommunicator(), req->sn);
    logI("req->deviceId=" << req->deviceId);

    auto resp = std::make_shared<MessageBResp>();

    // dispatch message
    try
    {
        // check req valid
        // call handler
        std::make_shared<DispatchHandler>(req, resp, response, current)->run();

    }
    catch(std::logic_error& ex)
    {
        logE("ex=" << ex.what());
        resp->sn = req->sn;
        resp->code = -2;
        response(resp);
    }
    catch(std::exception& ex)
    {
        logE("ex=" << ex.what());
        resp->sn = req->sn;
        resp->code = -1;
        response(resp);
    }
}
