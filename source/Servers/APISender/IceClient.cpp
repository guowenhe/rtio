/*
 * IceClient.cpp
 *
 *  Created on: Dec 24, 2019
 *      Author: wenhe
 */

#include "IceClient.h"

#include "RtioLog.h"
#include "RemoteCode.h"
#include "Common.h"

using namespace GlobalResources;

RetCode IceClient::init(int argc, char* argv[], const std::string& config)
{
    try
    {
        Ice::CommunicatorHolder communicatorHolder(argc, argv, config);
        _communicatorHolder = std::move(communicatorHolder);
        _communicator = _communicatorHolder.communicator();

        if(!_communicator)
        {
            std::cerr << "communicator error" << std::endl;
            return RetCode::CommunicatorError;
        }

        log2D(getCommunicator(), "communicator initialed");

//        _messageTriggerBPrx = Ice::checkedCast<DMS::MessageTriggerBPrx>(
//                _communicator->propertyToProxy("MessageTriggerBIdentity"));
//        if(!_messageTriggerBPrx)
//        {
//            log2D(getCommunicator(), "MessageTrigger server proxy invalid.");
//            return RetCode::ObjectPrxError;
//        }
        _deviceHubBPrx = Ice::checkedCast<DMS::DeviceHubBPrx>(
                _communicator->propertyToProxy("DeviceHubBIdentity"));
        if(!_deviceHubBPrx)
        {
            log2D(getCommunicator(), "DeviceHub server proxy invalid.");
            return RetCode::ObjectPrxError;
        }
    }
    catch(const Ice::FileException& ex)
    {
        std::cerr << "FileException=" << ex.what() << std::endl;
        return RetCode::FileExeption;
    }
    catch(const Ice::ObjectNotExistException& ex)
    {
        log2E(getCommunicator(), "ObjectNotExistException=" << ex.what());
        std::cerr << "ObjectNotExistException=" << ex.what() << std::endl;
        return RetCode::ObjectPrxError;
    }
    catch(const std::exception& ex)
    {
        std::cerr << "exception=" << ex.what() << std::endl;
        log2E(getCommunicator(), "exception=" << ex.what());
        return RetCode::ObjectPrxError;
    }
    return RetCode::Success;

}

RetCode IceClient::dispatch(const DispatchPara& para, std::function<void(std::shared_ptr<DispatchReturn>)> cb)
{
    const int sn = logSnGen();
    log2I(getCommunicator(), "deviceId=" << para.deviceId << " nonce=" << para.nonce << " sn=" << sn);

    auto responseHandler = [this, cb](std::shared_ptr<::DMS::MessageBResp> resp)
    {
        log2I(getCommunicator(), "send responseHandler:"<< resp->sn << "," << resp->code);
        auto ret = std::make_shared<DispatchReturn>();

        RC::Code retCode = RC::intToCode(resp->code);

        if( retCode != RC::Code::SUCCESS
                && retCode != RC::Code::DEVICEHUB_DIVICE_NOT_ONLINE)
        {
            ret->code = (retCode == RC::Code::DEVICEHUB_DIVICE_NOTFUND)? RC::Code::API_SENDER_DEVICE_NOTFOUND: RC::Code::FAIL;
            cb(ret);
            return;
        }

        ret->code = RC::Code::SUCCESS;
        ret->deviceCode = resp->deviceCode;
        ret->deviceMessage = resp->deviceMessage;
        ret->deviceStatus = static_cast<DMS::ClientStatus>(resp->deviceStatus);
        cb(ret);
    };
    auto exceptionHandler = [this, cb](std::exception_ptr ex)
    {
        log2I(getCommunicator(), "send exceptionHandler");
        try
        {
            std::rethrow_exception(ex);
        }
        catch(const std::exception& ex)
        {
            log2I(getCommunicator(), "send exceptionHandler ex=" << ex.what());

            auto ret = std::make_shared<DispatchReturn>();
            ret->code = RC::Code::FAIL;
            cb(ret);
        }
    };

    auto req = std::make_shared<DMS::MessageBReq>();
    req->sn = sn;
    req->deviceId = para.deviceId;
    req->message = para.message;
    try
    {
        _deviceHubBPrx->dispatchAsync(req, responseHandler, exceptionHandler);
    }
    catch(const std::exception& ex)
    {
        log2I(getCommunicator(), "device ex=" << ex.what());
        return RetCode::UnknowError;
    }

    return RetCode::Success;
}












