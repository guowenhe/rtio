/*
 * WebsocketSession.cpp
 *
 *  Created on: Dec 9, 2019
 *      Author: szz
 */

#include "VerijsLog.h"
#include "IceClient.h"
#include "SessionManager.h"
#include "WebsocketSession.h"

#include "MCodec.hpp"
#include "RemoteCode.h"

WebsocketSession::WebsocketSession(tcp::socket &&socket, const std::string &deviceId) :
		_ws(std::move(socket)), _deviceId(deviceId), _timer(_ws.get_executor())

{
	auto *iceClient = GlobalResources::IceClient::getInstance();
	log2D(iceClient->getCommunicator(), "_deviceId=" << _deviceId);
}
WebsocketSession::~WebsocketSession()
{
	auto *iceClient = GlobalResources::IceClient::getInstance();
	log2D(iceClient->getCommunicator(), "_deviceId=" << _deviceId);
	GlobalResources::SessionManager::getInstance()->removeSession(_deviceId);
}

void WebsocketSession::fail(beast::error_code ec, char const *what)
{
	auto *iceClient = GlobalResources::IceClient::getInstance();
	if(_ws.is_open() == false)
	{
		_timer.cancel();
	}

	// setStatus offline
	_targetStatus = DMS::ClientStatus::OFFLINE;
	setStatus();


	// Don't report these
	if(ec == asio::error::operation_aborted || ec == websocket::error::closed)
	{
		log2D(iceClient->getCommunicator(), what << ": " << ec.message());
		return;
	}
	log2E(iceClient->getCommunicator(), what << ": " << ec.message());
}

void WebsocketSession::onAccept(beast::error_code ec)
{
	if(ec)
	{
		return fail(ec, "accept");
	}

	GlobalResources::SessionManager::getInstance()->addSession(_deviceId, this);

	setStatus();
}
void WebsocketSession::setStatus()
{
//    GlobalResources::RetCode ret = GlobalResources::IceClient::getInstance()->setStatus("Device0001",
//            DMS::ClientStatus::ONLINE,
//            beast::bind_front_handler(&WebsocketSession::onSetStatus, shared_from_this()));

    GlobalResources::RetCode ret = GlobalResources::IceClient::getInstance()->setStatus(_deviceId,
            _targetStatus,
             std::bind(&WebsocketSession::onSetStatus, shared_from_this(), std::placeholders::_1));
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

void WebsocketSession::onSetStatus(int code)
{
    auto *iceClient = GlobalResources::IceClient::getInstance();
    if(0 == code)
    {
        log2I(iceClient->getCommunicator(), "succeed, status=" << _targetStatus);
        if(_targetStatus == DMS::ClientStatus::ONLINE)
        {
            asio::post(_ws.get_executor(), beast::bind_front_handler(&WebsocketSession::read, shared_from_this()));
        }
    }
    else
    {
        log2E(iceClient->getCommunicator(), "failed");
    }

}
void WebsocketSession::read()
{
    _ws.async_read(_buffer, beast::bind_front_handler(&WebsocketSession::onRead, shared_from_this()));

    _timer.expires_after(std::chrono::seconds(WEBSOCKETSESSION_TIMER_CYCLE));
    _timer.async_wait(beast::bind_front_handler(&WebsocketSession::onTimer, shared_from_this()) );
}

void WebsocketSession::onRead(beast::error_code ec, std::size_t)
{
	if(ec)
	{
		return fail(ec, "read");
	}
    auto *iceClient = GlobalResources::IceClient::getInstance();

	try
	{
	    std::string binary(beast::buffers_to_string(_buffer.data()));
	    switch(MCodec::headType(binary))
	    {
	    case MCodec::Type::reportReq:
	    {
	        MCodec::Req reportReq;
	        if(MCodec::parse(binary, reportReq))
	        {
	            log2E(iceClient->getCommunicator(), "parse error, binary=" << MCodec::debugShowHex(binary, "reportReq"));
	            throw std::invalid_argument("reportReq parse error");
	        }
	        // todo check message

	        // response resp
	        MCodec::Resp reportResp;
	        reportResp.type = MCodec::Type::reportResp;
	        reportResp.id = reportReq.id;
	        reportResp.code = 0;
	        std::string respBinary;
	        MCodec::serial(reportResp, respBinary);

	        _ws.binary(true);
	        send(std::make_shared<const std::string>(respBinary));

	        GlobalResources::RetCode ret = GlobalResources::IceClient::getInstance()->reportMessage(reportReq.content);
	        if(GlobalResources::RetCode::Success != ret)
	        {
	            log2E(iceClient->getCommunicator(), "reportMessage error, ret=" << ret);
	        }
	        break;
	    }
	    case MCodec::Type::dispatchResp:
	    {
	        MCodec::Resp dispatchResp;
	        if(MCodec::parse(binary, dispatchResp))
	        {
	            log2E(iceClient->getCommunicator(), "parse error, binary=" << MCodec::debugShowHex(binary, "dispatchResp"));
	            throw std::invalid_argument("dispatchResp parse error");
	        }

	        // call response for ice dispatch
	        log2D(iceClient->getCommunicator(), "call response for ice dispatch");

	        if(0 == dispatchResp.code)
	        {
	            _responseSession.response(dispatchResp.id, (int)RC::Code::SUCCEED);
	        }
	        else
	        {
                log2E(iceClient->getCommunicator(), "dispathc to device error, code=" << dispatchResp.code);
	        }
	        break;
	    }

	    default:
	        throw std::invalid_argument("messate type error");
	        break;

	    }
	}
	catch(std::invalid_argument& ex)
	{
	    log2E(iceClient->getCommunicator(), "invalid_argument=" << ex.what());
	}
	catch(std::exception& ex)
	{
        log2E(iceClient->getCommunicator(), "exception=" << ex.what());
	}

	// Clear the buffer
	_buffer.consume(_buffer.size());

	// Read another message
	_ws.async_read(_buffer, beast::bind_front_handler(&WebsocketSession::onRead, shared_from_this()));
}

void WebsocketSession::dispatch(const std::string& message, const ResponseFun& respone)
{
    auto *iceClient = GlobalResources::IceClient::getInstance();
    MCodec::Req req;
    req.id = genMessageId();
    req.type = MCodec::Type::dispatchReq;
    req.content = message; // todo message copy

    auto binary = std::make_shared<std::string>();
    MCodec::serial(req, *binary);

    int ret = _responseSession.monitor(req.id, respone);
    if(ret < 0)
    {
        log2E(iceClient->getCommunicator(), "responseSession.monitor error");
    }
    _ws.binary(true);
    send(binary);
}

void WebsocketSession::send(std::shared_ptr<const std::string> const& message)
{
    // Post our work to the strand, this ensures
    // that the members of `this` will not be
    // accessed concurrently.

    asio::post(_ws.get_executor(), beast::bind_front_handler(&WebsocketSession::onSend, shared_from_this(), message));
}

void WebsocketSession::onSend(std::shared_ptr<std::string const> const& message)
{
	_queue.push_back(message);

	// Are we already writing?
	if(_queue.size() > 1)
		return;

	// We are not currently writing, so send this immediately
	_ws.async_write(asio::buffer(*_queue.front()),
			beast::bind_front_handler(&WebsocketSession::onWrite, shared_from_this()));
}

void WebsocketSession::onWrite(beast::error_code ec, std::size_t)
{
	if(ec)
	{
		return fail(ec, "write");
	}

	_queue.erase(_queue.begin());

	// Send the next message if any
	if(!_queue.empty())
	{
		_ws.async_write(asio::buffer(*_queue.front()),
				beast::bind_front_handler(&WebsocketSession::onWrite, shared_from_this()));
	}
}

void WebsocketSession::onTimer(beast::error_code ec)
{
	if(ec)
	{
		return fail(ec, "onTimer");
	}
	auto *iceClient = GlobalResources::IceClient::getInstance();
	log2D(iceClient->getCommunicator(), "check.");

	_responseSession.responseTimeouts(static_cast<int>(RC::Code::ACCESSSERVER_TIMEOUT));

	_timer.expires_after(std::chrono::seconds(WEBSOCKETSESSION_TIMER_CYCLE));
	_timer.async_wait(beast::bind_front_handler(&WebsocketSession::onTimer, shared_from_this()));

}
time_t WebsocketSession::getClock()
{
	auto *iceClient = GlobalResources::IceClient::getInstance();
	struct timespec t;
	int ret = clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	if(ret)
	{
		log2E(iceClient->getCommunicator(), "clock_gettime error, ret=" << ret << " error=" << errno);
		return -1;
	}
	log2D(iceClient->getCommunicator(), "t.tv_sec=" << t.tv_sec);
	return  t.tv_sec;
}



