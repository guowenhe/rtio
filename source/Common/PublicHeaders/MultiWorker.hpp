/*
 * MultiWorker.hpp
 *
 *  Created on: Jan 4, 2020
 *      Author: wenhe
 */

#ifndef MULTI_WORKER_MULTIWORKER_HPP_
#define MULTI_WORKER_MULTIWORKER_HPP_

#include <memory>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unistd.h>
namespace MultiWorker
{

struct Config   // be shared for every worker
{
};

class Resource  // only for one worker
{
public:
    virtual ~Resource(){}
    virtual void set(std::shared_ptr<Config> config) = 0; // set config para before resource init
    virtual void init() = 0; // call it worker's thread start
    virtual void check(){} // call it for worker's thread loop
};

class Handler: public std::enable_shared_from_this<Handler>
{
public:
    virtual void run(Resource*) = 0;
    virtual ~Handler(){}
};


class Worker
{

public:
    ~Worker()
    {
        stop();
    }
    void working()
    {
        _resource->init();
        std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
        while(_running)
        {
            _resource->check();
            lock.lock();
            if(!_queue.empty())
            {
                auto handler = _queue.front();
                _queue.pop();
                lock.unlock();
                if(handler)
                {
                    handler->run(_resource.get());
                }
            }
            else
            {
                _condition.wait(lock);
                lock.unlock();
            }
        }
        _finished = true;
    }
    void push(std::shared_ptr<Handler> handler)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_queue.empty())
        {
            _condition.notify_one();
        }
        _queue.push(handler);

    }
    void start(std::shared_ptr<Resource> resource)
    {
        _resource = resource;
        _running = true;
        _thread = std::thread(&Worker::working, this);
    }
    void stop()
    {
        // wait for process all request in queue
        std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
        bool isEmpty = false;
        while(_running && !isEmpty)
        {
            lock.lock();
            isEmpty = _queue.empty();
            lock.unlock();
            if(isEmpty)
            {
                break;
            }
            usleep(10000);
        }
        _running = false;

        do
        {
            _condition.notify_one();
            usleep(10000);
        } while(!_finished);

        if(_thread.joinable())
        {
            _thread.join();
        }
    }

private:
    std::thread _thread;
    bool _running = false;
    bool _finished = false;
    std::queue<std::shared_ptr<Handler>> _queue;
    std::mutex _mutex;
    std::shared_ptr<Resource> _resource;
    std::condition_variable _condition;
};

template<typename R, typename C>
class Workers   //auto created Resource for each worker
{
public:
    Workers(int workerNum): _which(0)
    {
        for(int i = 0; i < workerNum; ++i)
        {
            auto worker = std::make_shared<Worker>();
            _workers.push_back(worker);
        }
    }
    ~Workers(){};
    void push(std::shared_ptr<Handler> handler)
    {
        _workers[_which++ % _workers.size()]->push(handler);
    }
    int start(std::shared_ptr<C> config) //auto created Resource for each worker
    {
        if(_workers.size() <= 0)
        {
            return -1;
        }
        for(auto &c: _workers)
        {
            auto resource = std::make_shared<R>();
            resource->set(config);
            c->start(resource);
        }
        return 0;
    }
    void stop()
    {
        for(auto &c: _workers)
        {
            c->stop();
        }
    }

private:
    std::vector<std::shared_ptr<Worker>> _workers;
    std::atomic_ullong _which;

};








} //MultiWorker








#endif /* MULTI_WORKER_MULTIWORKER_HPP_ */
