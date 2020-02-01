#include <Ice/Ice.h>
#include "DeviceHubI.h"
#include "VerijsLog.h"
#include "DispatchHandler.hpp"
using namespace DMS;



int MessageHubAI::addAccessServer(::std::shared_ptr<AccessServerPrx> server, const Ice::Current& current)
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

void MessageHubAI::reportAsync(::std::shared_ptr<MessageAReq> req,
        ::std::function<void(const ::std::shared_ptr<MessageAResp>& resp)> response,
        ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)
{
    logSet(current.adapter->getCommunicator(), req->sn);
    logI("req->message=" << req->message);

    auto resp = std::make_shared<MessageAResp>();
    resp->code = 0;
    resp->sn = req->sn;
    response(resp);

}

void MessageHubBI::dispatchAsync(::std::shared_ptr<MessageBReq> req,
        ::std::function<void(const ::std::shared_ptr<MessageBResp>& resp)> response,
        ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current)
{
    logSet(current.adapter->getCommunicator(), req->sn);
    logI("req->deviceId=" << req->deviceId);

    // dispatch message
    try
    {
        // check req valid
        // call handler
        std::make_shared<DispatchHandler>(req, response, current)->processing();

    }
    catch(std::logic_error& ex)
    {
        logE("ex=" << ex.what());
        ::std::shared_ptr<MessageBResp> resp = std::make_shared<MessageBResp>();
        resp->sn = req->sn;
        resp->code = -2;
        response(resp);
    }
    catch(std::exception& ex)
    {
        logE("ex=" << ex.what());
        ::std::shared_ptr<MessageBResp> resp = std::make_shared<MessageBResp>();
        resp->sn = req->sn;
        resp->code = -1;
        response(resp);
    }
}

