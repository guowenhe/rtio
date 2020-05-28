#ifndef __DeviceMessageHubI_h__
#define __DeviceMessageHubI_h__

#include "DeviceHub.h"
#include "RemoteCode.h"

namespace DMS
{

class DeviceHubAI: public virtual DeviceHubA
{
public:

    virtual int addAccessServer(::std::shared_ptr<AccessServerPrx>, const Ice::Current&) override;
    virtual void reportAsync(::std::shared_ptr<MessageAReq> req,
            ::std::function<void(const ::std::shared_ptr<MessageAResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override;
};

class DeviceHubBI: public virtual DeviceHubB
{
public:
    virtual void dispatchAsync(::std::shared_ptr<MessageBReq> req,
            ::std::function<void(const ::std::shared_ptr<MessageBResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override;

};

} // DMS

#endif
