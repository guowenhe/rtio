/*
 * DeviceManagerI.hpp
 *
 *  Created on: 4 Apr 2020
 *      Author: wenhe
 */

#ifndef DEVICEMANAGERI_HPP_
#define DEVICEMANAGERI_HPP_

#include "DeviceManager.h"
#include "DeviceManagerHandler.h"
namespace DMS
{

using DeviceManagerWorkers =  MultiWorker::Workers<DeviceManagerResource, DeviceManagerConfig>;


class DeviceManagerI: public DeviceManager
{
public:
    DeviceManagerI(std::shared_ptr<DeviceManagerWorkers> workers) :
            _workers(workers)
    {
    }
    virtual void createDeviceAsync(::std::shared_ptr<CreateDeviceReq> req,
            ::std::function<void(const ::std::shared_ptr<CreateDeviceResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override
    {
        logSet(current.adapter->getCommunicator(), req->sn);
        logI("req->nonce=" << req->nonce);

        auto handler = std::make_shared<CreateDeviceHandler>(req, response, current);
        _workers->push(handler);
    }
    virtual void authDeviceAsync(::std::shared_ptr<AuthDeviceReq> req,
            ::std::function<void(const ::std::shared_ptr<AuthDeviceResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override
    {
        logSet(current.adapter->getCommunicator(), req->sn);
        logI("req->deviceId=" << req->deviceId);

        auto handler = std::make_shared<AuthDeviceHandler>(req, response, current);
        _workers->push(handler);
    }



private:
    std::shared_ptr<DeviceManagerWorkers> _workers;
};

} // DMS

#endif /* DEVICEMANAGERI_HPP_ */
