/*
 * MessageTriggerI.h
 *
 *  Created on: 22 Jan 2020
 *      Author: wenhe
 */

#ifndef MESSAGEREPORTERI_H_
#define MESSAGEREPORTERI_H_

#include "MessageReporter.h"
#include "MessageReporterHandler.h"
namespace DMS
{

using MessageReporterWorkers =  MultiWorker::Workers<MessageReporterResource, MessageReporterConfig>;


class MessageReporterI: public MessageReporter
{
public:
    MessageReporterI(std::shared_ptr<MessageReporterWorkers> workers) :
            _workers(workers)
    {
    }
    virtual void reportAsync(::std::shared_ptr<ReportReq> req,
            ::std::function<void(const ::std::shared_ptr<ReportResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override;

private:
    std::shared_ptr<MessageReporterWorkers> _workers;
};



} // DMS




#endif /* MESSAGEREPORTERI_H_ */
