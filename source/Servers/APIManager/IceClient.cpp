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

        _deviceManagerPrx = Ice::checkedCast<DMS::DeviceManagerPrx>(
                _communicator->propertyToProxy("DeviceManagerIdentity"));
        if(!_deviceManagerPrx)
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

RetCode IceClient::createDevice(const CreateDevicePara& para, std::function<void(std::shared_ptr<CreateDeviceReturn>)> cb)
{
    const int sn = logSnGen();
    log2I(getCommunicator(), " nonce=" << para.nonce << " sn=" << sn);

    auto responseHandler = [this, cb](std::shared_ptr<::DMS::CreateDeviceResp> resp)
    {
        log2I(getCommunicator(), "send responseHandler:"<< resp->sn << "," << resp->code);
        auto ret = std::make_shared<CreateDeviceReturn>();

        RC::Code retCode = RC::intToCode(resp->code);

        if( retCode != RC::Code::SUCCESS
                && retCode != RC::Code::DEVICEHUB_DIVICE_NOT_ONLINE)
        {
            ret->code = (retCode == RC::Code::DEVICEHUB_DIVICE_NOTFUND)? RC::Code::API_SENDER_DEVICE_NOTFOUND: RC::Code::FAIL;
            cb(ret);
            return;
        }

        ret->code = RC::Code::SUCCESS;
        ret->deviceId = resp->deviceId;
        ret->deviceKey = resp->deviceKey;
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

            auto ret = std::make_shared<CreateDeviceReturn>();
            ret->code = RC::Code::FAIL;
            cb(ret);
        }
    };

    auto req = std::make_shared<DMS::CreateDeviceReq>();
    req->sn = sn;
    req->nonce = para.nonce;
    try
    {
        _deviceManagerPrx->createDeviceAsync(req, responseHandler, exceptionHandler);
    }
    catch(const std::exception& ex)
    {
        log2I(getCommunicator(), "device ex=" << ex.what());
        return RetCode::UnknowError;
    }

    return RetCode::Success;
}












