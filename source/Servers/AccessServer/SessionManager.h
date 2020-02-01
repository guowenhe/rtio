/*
 * SessionManager.h
 *
 *  Created on: Dec 8, 2019
 *      Author: szz
 */

#ifndef SESSIONMANAGER_H_
#define SESSIONMANAGER_H_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "WebsocketSession.h"

namespace GlobalResources
{

class SessionManager
{
private:
	SessionManager() {}

public:
	static SessionManager* getInstance()
	{
		static SessionManager instance;
		return &instance;
	}

	std::shared_ptr<WebsocketSession> getSession(const std::string& deviceId);
	int addSession(const std::string& deviceId, WebsocketSession* session);
	void removeSession(const std::string& deviceId);

private:
	std::mutex _mutex;
	std::unordered_map<std::string, WebsocketSession*> _sessions;
};

} //GlobalResources


//// test session manager
//	{
//	auto* sessionManager = GlobalResources::SessionManager::getInstance();
//
//	WebsocketSession s1, s2, s3;
//	s1.test = "1111";
//	s2.test = "2222";
//	s3.test = "3333";
//
//	std::cout << "addSession" << std::endl;
//	{
//		sessionManager->addSession("d1", &s1);
//		sessionManager->addSession("d2", &s2);
//		sessionManager->addSession("d3", &s3);
//	}
//
//	std::cout << "getSession" << std::endl;
//	{
//		std::string id = "d1";
//		auto* gs = sessionManager->getSession(id);
//		if(gs)
//		{
//			std::cout << id << ":" << gs->test  << std::endl;
//		}
//		else
//		{
//			std::cout << id << ":" << "not exist" << std::endl;
//		}
//	}
//	{
//		std::string id = "d2";
//		auto* gs = sessionManager->getSession(id);
//		if(gs)
//		{
//			std::cout << id << ":" << gs->test  << std::endl;
//		}
//		else
//		{
//			std::cout << id << ":" << "not exist" << std::endl;
//		}
//	}
//	{
//		std::string id = "d3";
//		auto* gs = sessionManager->getSession(id);
//		if(gs)
//		{
//			std::cout << id << ":" << gs->test  << std::endl;
//		}
//		else
//		{
//			std::cout << id << ":" << "not exist" << std::endl;
//		}
//	}
//	{
//		std::string id = "d4";
//		auto* gs = sessionManager->getSession(id);
//		if(gs)
//		{
//			std::cout << id << ":" << gs->test  << std::endl;
//		}
//		else
//		{
//			std::cout << id << ":" << "not exist" << std::endl;
//		}
//	}
//
//	std::cout << "removeSession" << std::endl;
//	{
//		std::string id = "d2";
//		sessionManager->removeSession(id);
//		auto* gs = sessionManager->getSession(id);
//		if(gs)
//		{
//			std::cout << id << ":" << gs->test  << std::endl;
//		}
//		else
//		{
//			std::cout << id << ":" << "not exist" << std::endl;
//		}
//	}
//	{
//		std::string id = "d3";
//		auto* gs = sessionManager->getSession(id);
//		if(gs)
//		{
//			std::cout << id << ":" << gs->test  << std::endl;
//		}
//		else
//		{
//			std::cout << id << ":" << "not exist" << std::endl;
//		}
//	}
//
//
//
//
//	std::cout << "end."  << std::endl;
//	}


#endif /* SESSIONMANAGER_H_ */
