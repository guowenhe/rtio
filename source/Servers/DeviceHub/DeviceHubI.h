#ifndef __DeviceMessageHubI_h__
#define __DeviceMessageHubI_h__

#include "DeviceHub.h"

namespace DMS
{

class MessageHubAI: public virtual MessageHubA
{
public:

    virtual int addAccessServer(::std::shared_ptr<AccessServerPrx>, const Ice::Current&) override;
    virtual void reportAsync(::std::shared_ptr<MessageAReq> req,
            ::std::function<void(const ::std::shared_ptr<MessageAResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override;
};

class MessageHubBI: public virtual MessageHubB
{
public:
    virtual void dispatchAsync(::std::shared_ptr<MessageBReq> req,
            ::std::function<void(const ::std::shared_ptr<MessageBResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override;

};

} // DMS

#endif
