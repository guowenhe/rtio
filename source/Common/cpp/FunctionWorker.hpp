/*
 * FunctionWorker.hpp
 *
 *  Created on: Oct 21, 2019
 *      Author: szz
 */

#ifndef FUNCTION_WORKER_FUNCTIONWORKER_HPP_
#define FUNCTION_WORKER_FUNCTIONWORKER_HPP_
#include <iostream>
#include <queue>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <unistd.h>
#include <condition_variable>

namespace FunctionWorker
{

template <typename F>
class Worker
{

public:
	~Worker()
	{
		stop();
	}
	void working()
	{
		std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
		while(_run)
		{
			lock.lock();
			if(!_queue.empty())
			{
				auto f = _queue.front();
				_queue.pop();
				lock.unlock();
				f->operator()();
			}
			else
			{
				_condition.wait(lock);
				lock.unlock();
			}
		}
	}
	void push(F f)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if(_queue.empty())
		{
			_condition.notify_one();
		}
		_queue.push(std::make_shared<F>(f));
	}
	void start()
	{
		_run = true;
		_thread = std::thread(&Worker::working, this);
	}
	void stop()
	{
		// wait for process all request in queue
		std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);

		bool isEmpty = false;
		while(_run && !isEmpty)
		{
			lock.lock();
			isEmpty=_queue.empty();
			lock.unlock();
			usleep(10000);
		}
		_run = false;
		_condition.notify_one();

		if(_thread.joinable())
		{
			_thread.join();
		}
	}

private:
	std::thread _thread;
	bool _run = false;
	std::queue<std::shared_ptr<F>> _queue;
	std::mutex _mutex;
	std::condition_variable _condition;
};

} // Util


#endif /* FUNCTION_WORKER_FUNCTIONWORKER_HPP_ */
