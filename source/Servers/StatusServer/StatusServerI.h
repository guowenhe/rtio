/*
 * StatusServerI.h
 *
 *  Created on: Jan 3, 2020
 *      Author: wenhe
 */

#ifndef STATUSSERVERI_H_
#define STATUSSERVERI_H_

#include "StatusServer.h"
#include "StatusServerHandler.h"
namespace DMS
{

using DeviceStatusWorker =  MultiWorker::Workers<DeviceStatusResource, DeviceStatusConfig>;

class StatusServerI:  public virtual StatusServer
{
public:
    StatusServerI(std::shared_ptr<DeviceStatusWorker> worker): _worker(worker)
    {
    }

    virtual void setStatusAsync(::std::shared_ptr<SetStatusReq> req,
            ::std::function<void(const ::std::shared_ptr<SetStatusResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override;

    virtual void queryStatusAsync(::std::shared_ptr<QueryStatusReq> req,
            ::std::function<void(const ::std::shared_ptr<QueryStatusResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override;

private:
    std::shared_ptr<DeviceStatusWorker> _worker;
};


} //DMS
#endif /* STATUSSERVERI_H_ */
