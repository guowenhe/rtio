/*
 * SessionManager.cpp
 *
 *  Created on: Dec 10, 2019
 *      Author: szz
 */

#include "IceClient.h"
#include "SessionManager.h"
#include "RtioLog.h"

using namespace GlobalResources;

std::shared_ptr<TCPSession> SessionManager::getSession(const std::string& deviceId)
{
	std::lock_guard<std::mutex> lock(_mutex);
	log2I(IceClient::getInstance()->getCommunicator(), "deviceId=" << deviceId);
	auto it = _sessions.find(deviceId);
	if(_sessions.end() == it || nullptr == it->second)
	{
		return nullptr;
	}
	return it->second->shared_from_this(); // get shared_ptr, prevent object release
}
int SessionManager::addSession(const std::string& deviceId, TCPSession* session)
{
	std::lock_guard<std::mutex> lock(_mutex);
	log2I(IceClient::getInstance()->getCommunicator(), "deviceId=" << deviceId);
	auto r = _sessions.insert(std::pair<std::string, TCPSession*>(deviceId, session));
	if(false == r.second)
	{
		return -1;
	}
	return 0;
}
void SessionManager::removeSession(const std::string &deviceId)
{
	std::lock_guard<std::mutex> lock(_mutex);
	log2I(IceClient::getInstance()->getCommunicator(), "deviceId=" << deviceId);
	auto n = _sessions.erase(deviceId);
	if (0 == n)
	{
		log2E(IceClient::getInstance()->getCommunicator(),
				"removeSession a unknown session, deviceId=" << deviceId);
	}
}
