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
#include "TCPSession.h"



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

	std::shared_ptr<TCPSession> getSession(const std::string& deviceId);
	int addSession(const std::string& deviceId, TCPSession* session);
	void removeSession(const std::string& deviceId);

private:
	std::mutex _mutex;
	std::unordered_map<std::string, TCPSession*> _sessions;
};

} //GlobalResources


#endif /* SESSIONMANAGER_H_ */
