/*
 * IceClient.h
 *
 *  Created on: Dec 7, 2019
 *      Author: szz
 */

#ifndef ICECLIENT_H_
#define ICECLIENT_H_


#include <Ice/Ice.h>
#include "DeviceHub.h"
#include "StatusServer.h"
#include "DeviceManager.h"

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
    AuthFail = -6,
    AuthIdInvalid = -7,
    AuthKeyInvalid = -8,
    AuthPass = 1,

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
    case RetCode::AuthFail:
        out << "AuthFail";
        break;
    case RetCode::AuthPass:
        out << "AuthPass";
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

// deliver message to device
class AccessServerI : public DMS::AccessServer
{
public:
    virtual void dispatchAsync(::std::shared_ptr<DMS::MessageAReq> req,
            ::std::function<void(const ::std::shared_ptr<DMS::MessageAResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override;
};

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
	std::shared_ptr<DMS::DeviceHubAPrx> getMessageHubAPrx()
	{
		return _messageHubAPrx;
	}

    RetCode init(int argc, char* argv[], const std::string& config);
    RetCode reportMessage(const std::string& message, const std::string& deviceId);
//    RetCode reportMessage(std::shared_ptr<std::string> message);
    void authDevice(const std::string& deviceId, const std::string& deviceKey, std::function<void(RetCode)> cb);
    void setStatus(const std::string& deviceId, DMS::ClientStatus status, std::function<void(GlobalResources::RetCode)> cb);

private:
	Ice::CommunicatorHolder _communicatorHolder;
	std::shared_ptr<Ice::Communicator> _communicator;
	std::shared_ptr<DMS::DeviceHubAPrx> _messageHubAPrx;
	std::shared_ptr<DMS::StatusServerPrx> _statusServerPrx;
	std::shared_ptr<DMS::DeviceManagerPrx> _deviceManagerPrx;
	std::string _accessServerPrxStr;
};

} // GlobalResources


#endif /* ICECLIENT_H_ */
