/*
 * MessageReporterHandler.h
 *
 *  Created on: 30 Mar 2020
 *      Author: wenhe
 */

#ifndef MESSAGEREPORTERHANDLER_H_
#define MESSAGEREPORTERHANDLER_H_

#include "MultiWorker.hpp"
#include "ReporterRedis.hpp"
#include "Common.h"
#include "MessageReporter.h"
#include "APINotifier.h"
#include "RemoteCode.h"

#define MESSAGEREPORTER_TRYING_AFTER_INIT    360
#define MESSAGEREPORTER_TRYING_AFTER_FAIL    120

struct MessageReporterConfig: public MultiWorker::Config
{
    std::string redisHost;
    int redisPort = 0;
};

class MessageReporterResource: public MultiWorker::Resource
{
private:
    const int _redisConnectCheckInterval = 5; // second
    const int _redisConnectTimeout       = 30; // second

public:
    void set(std::shared_ptr<MultiWorker::Config> config) override;
    void init() override;
    void check();

    std::shared_ptr<RedisClient::ReporterRedis> getRedis()
    {
        return _redis;
    }

private:
    std::shared_ptr<MessageReporterConfig> _config;
    std::shared_ptr<RedisClient::ReporterRedis> _redis;
    time_t _clockPoint = 0;
};

class ReportHandler: public MultiWorker::Handler
{
public:
    ReportHandler(::std::shared_ptr<DMS::ReportReq>& req,
            ::std::function<void(const ::std::shared_ptr<DMS::ReportResp>& resp)>& response,
            const ::Ice::Current& current) :
            _req(req), _response(response), _current(current)
    {
    }

    virtual void run(MultiWorker::Resource*) override;

    void notifyRespone(std::shared_ptr<DMS::NotifyResp> resp);
    void exception(::std::exception_ptr ex);
    void failed(RC::Code code, const std::string& what);

private:
    ::std::shared_ptr<DMS::ReportReq> _req;
    ::std::function<void(const ::std::shared_ptr<DMS::ReportResp>& resp)> _response;
    ::Ice::Current _current;
    std::shared_ptr<DMS::ReportResp>_resp;
    MessageReporterResource* _resource = nullptr;
};






#endif /* MESSAGEREPORTERHANDLER_H_ */
