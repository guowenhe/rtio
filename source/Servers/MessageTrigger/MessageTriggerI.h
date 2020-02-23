/*
 * MessageTriggerI.h
 *
 *  Created on: 22 Jan 2020
 *      Author: wenhe
 */

#ifndef MESSAGETRIGGERI_H_
#define MESSAGETRIGGERI_H_

#include "MessageTrigger.h"

namespace DMS
{

class MessageTiggerAI: public MessageTriggerA
{
public:
    virtual void reportAsync(::std::shared_ptr<ReportReq> req,
            ::std::function<void(const ::std::shared_ptr<ReportResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override;

};
class MessageTiggerBI: public MessageTriggerB
{
public:
    virtual void sendAsync(::std::shared_ptr<SendReq> req,
            ::std::function<void(const ::std::shared_ptr<SendResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override;

};



} // DMS




#endif /* MESSAGETRIGGERI_H_ */
