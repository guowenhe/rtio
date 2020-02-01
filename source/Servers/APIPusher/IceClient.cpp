/*
 * IceClient.cpp
 *
 *  Created on: Dec 24, 2019
 *      Author: wenhe
 */

#include "VerijsLog.h"
#include "IceClient.h"
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
RetCode push(const std::string& message)
{


    return RetCode::Success;
}















