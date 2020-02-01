/*
 * IceClient.h
 *
 *  Created on: Dec 7, 2019
 *      Author: szz
 */

#ifndef ICECLIENT_H_
#define ICECLIENT_H_


#include <Ice/Ice.h>
#include "MessageTrigger.h"
#include "StatusServer.h"

#define AS_STARTUP_CONNECT_HUB_MAX_TIMES 20

namespace GlobalResources
{

enum class RetCode
{
    Success = 0,
    FileExeption = -1,
    CommunicatorError = -2,
    ObjectPrxError = -3,
    AddClientError = -4,
    ConnectHubError = -5,
    UnknowError = -100,
};

static Ice::LoggerOutputBase& operator<<(Ice::LoggerOutputBase& out, const RetCode code) // todo temporary
{
    switch(code)
    {
    case RetCode::Success:
        out << "Success";
        break;
    case RetCode::FileExeption:
        out << "FileExeption";
        break;
    case RetCode::CommunicatorError:
        out << "CommunicatorError";
        break;
    case RetCode::ObjectPrxError:
        out << "ObjectPrxError";
        break;
    case RetCode::AddClientError:
        out << "AddClientError";
        break;
    case RetCode::ConnectHubError:
        out << "ConnectHubError";
        break;
    case RetCode::UnknowError:
        out << "UnknowError";
        break;
    default:
        out << "no-desc(" << static_cast<int>(code) << ")";
    }
    return out;
}

inline Ice::LoggerOutputBase& operator<<(Ice::LoggerOutputBase& out, const DMS::ClientStatus status) // todo temporary
{
    switch(status)
    {
    case DMS::ClientStatus::UNKNOWN:
        out << "UNKNOWN";
        break;
    case DMS::ClientStatus::ONLINE:
        out << "ONLINE";
        break;
    case DMS::ClientStatus::OFFLINE:
        out << "OFFLINE";
        break;
    default:
        out << "no-desc(" << static_cast<int>(status) << ")";
    }

    return out;
}


class IceClient
{
private:
	IceClient(){}

public:
	static IceClient* getInstance()
	{
		static IceClient instance;
		return &instance;
	}
	std::shared_ptr<Ice::Communicator> getCommunicator()
	{
		return _communicator;
	}

    RetCode init(int argc, char* argv[], const std::string& config);
//    RetCode push(Send &&send);


private:
	Ice::CommunicatorHolder _communicatorHolder;
	std::shared_ptr<Ice::Communicator> _communicator;
	std::shared_ptr<DMS::MessageTriggerBPrx> _messageTriggerBPrx;
};

} // GlobalResources


#endif /* ICECLIENT_H_ */
