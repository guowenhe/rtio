/*
 * IceClient.cpp
 *
 *  Created on: Dec 24, 2019
 *      Author: wenhe
 */

#include "IceClient.h"

#include "VerijsLog.h"
#include "RemoteCode.h"
#include "GxError.hpp"
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

        _messageTriggerBPrx = Ice::checkedCast<DMS::MessageTriggerBPrx>(_communicator->propertyToProxy("MessageTriggerBIdentity"));
        if(!_messageTriggerBPrx)
        {
            log2D(getCommunicator(), "server proxy invalid.");
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
RetCode IceClient::send(const SendPara& para, std::function<void(std::shared_ptr<SendReturn>)> cb)
{
    const int sn = Util::getSerianNumber();
    log2I(getCommunicator(), "deviceId=" << para.deviceId << " nonce=" << para.nonce << " sn=" << sn);

    auto responseHandler = [this, cb](std::shared_ptr<::DMS::SendResp> resp)
    {
        log2I(getCommunicator(), "send responseHandler:"<< resp->sn << "," << resp->code);
        auto ret = std::make_shared<SendReturn>();
        ret->code = RC::Code::SUCCESS;// static_cast<RC::Code>(resp->code);
        ret->deviceCode = resp->deviceCode;
        ret->deviceMessage = "reponse a device message";
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

            auto ret = std::make_shared<SendReturn>();
            ret->code = RC::Code::FAIL;
            cb(ret);
        }
    };

    auto req = std::make_shared<DMS::SendReq>();
    req->sn = sn;
    req->message = para.message;
    try
    {
        _messageTriggerBPrx->sendAsync(req, responseHandler, exceptionHandler);
    }
    catch(const std::exception& ex)
    {
        log2I(getCommunicator(), "send ex=" << ex.what());
        return RetCode::UnknowError;
    }

    return RetCode::Success;
}














