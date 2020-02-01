/*
 * ResponseManager.hpp
 *
 *  Created on: Dec 27, 2019
 *      Author: wenhe
 */

#ifndef RESPONSEMANAGER_HPP_
#define RESPONSEMANAGER_HPP_

#include <chrono>
#include <map>
#include <functional>
#include "IceClient.h"

namespace Util
{

#define RESPONE_TIMEOUT_TICKS 20 // seconds
#define RESPONE_TOTAL_NUM 5

template<typename FuncType, typename RespType>
class ResponseManager
{
    struct Response
    {
        int tick;
        FuncType fun;
    };

public:
    int monitor(int id, const FuncType& fun)
    {
        auto *iceClient = GlobalResources::IceClient::getInstance();
        log2I(iceClient->getCommunicator(), "id=" << id);
        if(_sessions.size() >= RESPONE_TOTAL_NUM)
        {
            log2W(iceClient->getCommunicator(), "exceed max size");
            return 0;
        }
        auto respone = std::make_shared<Response>();
        respone->fun = fun;
        respone->tick = getTicks();
        auto r = _sessions.insert(std::pair<int, std::shared_ptr<Response>>(id, respone));
        if(false == r.second)
        {
            log2E(iceClient->getCommunicator(), "response session insert failed");
            return -1;
        }
        return 0;
    }
    int response(int id, const RespType& resp)
    {
        auto *iceClient = GlobalResources::IceClient::getInstance();
        auto it =_sessions.find(id);
        if(it == _sessions.end())
        {
            log2W(iceClient->getCommunicator(), "response id not found, id=" << id);
            return -1; // not found response id
        }
        it->second->fun(resp);  // response hub dispatch success
        _sessions.erase(id);
        return 0;
    }
    void responseTimeouts(const RespType& resp)
    {
        auto *iceClient = GlobalResources::IceClient::getInstance();
        const time_t ticks = getTicks();
        for(auto it = _sessions.begin(); it != _sessions.end();)
        {
            if((ticks - it->second->tick) > RESPONE_TIMEOUT_TICKS)
            {
                auto *iceClient = GlobalResources::IceClient::getInstance();
                log2D(iceClient->getCommunicator(), it->first << "," << it->second->tick);
                it->second->fun(resp); // response hub dispatch timeout
                it =_sessions.erase(it);
                continue;
            }
            ++it;
        }
    }
    void showRespones(const std::string& where)
    {
//        std::cout << time(NULL) << ">" << where << std::endl;
//        std::cout << time(NULL) << ">" << "--------------------->" << std::endl;
//        for(auto c: _sessions)
//        {
//            std::cout << time(NULL) << ">" << c.first << "," << c.second->tick << std::endl;
//        }
//        std::cout << time(NULL) << ">" << "---------------------<" << std::endl;
    }

private:
    time_t getTicks() // interval seconds
    {
        struct timespec t;
        int ret = clock_gettime(CLOCK_MONOTONIC_RAW, &t);
        if(ret)
        {
            auto *iceClient = GlobalResources::IceClient::getInstance();
            log2E(iceClient->getCommunicator(), "getTicks Error ret=" << ret);
            return -1;
        }
        return  t.tv_sec;
    }

    std::map<int, std::shared_ptr<Response>> _sessions;
};

} // Util



#endif /* RESPONSEMANAGER_HPP_ */
